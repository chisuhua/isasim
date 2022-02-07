#include "inc/ThreadBlock.h"
#include "inc/ThreadItem.h"
#include "inc/Warp.h"
#include "inc/Instruction.h"
#include <bitset>
//#include "../../libcuda/abstract_hardware_model.h"
#include "../../libcuda/cuda-sim/memory.h"
#include "../../libcuda/gpgpu_context.h"

extern int libcuda::g_debug_execution;

ThreadBlock::ThreadBlock(libcuda::gpgpu_t *gpu, KernelInfo * kernel, libcuda::gpgpu_context *ctx, unsigned warp_size, unsigned threads_per_shader)
{
    m_gpu = gpu;
    m_gpgpu_ctx = ctx;
    m_warp_count = threads_per_shader / m_warp_size;
    // Handle the case where the number of threads is not a
    // multiple of the warp size
    if (threads_per_shader % m_warp_size != 0) {
      m_warp_count += 1;
    }
    assert(m_warp_count * m_warp_size > 0);
    m_thread = (ThreadItem **)calloc(m_warp_count * m_warp_size,
                                          sizeof(ThreadItem *));

    initilizeSIMTStack(m_warp_count, m_warp_size);
    for (unsigned i = 0; i < MAX_CTA_PER_SHADER; i++) {
      for (unsigned j = 0; j < MAX_BARRIERS_PER_CTA; j++) {
        reduction_storage[i][j] = 0;
      }
    }
    m_warpAtBarrier =  new bool [m_warp_count];
    m_liveThreadCount = new unsigned [m_warp_count];
}


void ThreadBlock::executeInstruction(shared_ptr<Instruction> inst, unsigned warpId) {
  active_mask_t active_mask = GetWarp(warpId)->get_warp_active_mask();
  bool leading_thread = true;
  for (unsigned t = 0; t < m_warp_size; t++) {
    if (active_mask.test(t)) {
      // if (warpId == (unsigned(-1))) warpId = inst.warp_id();
      unsigned tid = m_warp_size * warpId + t;
      printf("Info: Warp%d, thread%d, executeInstruction %lx\n", warpId, t, inst->bytes.dword);
      m_thread[tid]->m_leading_thread = leading_thread;
      leading_thread = false;
      m_thread[tid]->Execute(inst);

      // virtual function
      checkExecutionStatusAndUpdate(inst, t, tid);
    }
  }
}

bool ThreadBlock::is_thread_done( unsigned hw_thread_id ) const
{
    return ((m_thread[ hw_thread_id ]==NULL) || m_thread[ hw_thread_id ]->is_done());
}

// FIXME void ThreadBlock::updateSIMTStack(unsigned warpId, warp_inst_t *inst) {
void ThreadBlock::updateSIMTStack(unsigned warpId, std::shared_ptr<Instruction> inst) {
  // Instruction* inst= warpinst->inst;
  simt_mask_t thread_done;
  std::vector<addr_t> next_pc;
  unsigned wtid = warpId * m_warp_size;
  for (unsigned i = 0; i < m_warp_size; i++) {
    if (is_thread_done(wtid + i)) {
      thread_done.set(i);
      next_pc.push_back((address_type)-1);
    } else {
      if (inst->reconvergence_pc == RECONVERGE_RETURN_PC)
        inst->reconvergence_pc = m_thread[wtid + i]->get_return_PC();
      next_pc.push_back(m_thread[wtid + i]->get_pc());
    }
  }
  m_SimtStack[warpId]->update(thread_done, next_pc, inst->reconvergence_pc,
                               inst->GetOpType(), inst->GetSize(), inst->pc);
}

//! Get the warp to be executed using the data taken form the SIMT stack
std::shared_ptr<Instruction> ThreadBlock::getExecuteWarp(unsigned warpId) {
  unsigned pc, rpc;
  m_SimtStack[warpId]->get_pdom_stack_top_info(&pc, &rpc);
  uint64_t opcode;
  m_gpu->get_global_memory()->read(pc, 4, &opcode);
  auto inst = make_instruction(opcode, pc);
  // WarpInst wi(inst);
  // wi.set_active(m_SimtStack[warpId]->get_active_mask());
  // FIXME inst->set_active(m_SimtStack[warpId]->get_active_mask());
  return inst;
}

void ThreadBlock::deleteSIMTStack() {
  if (m_SimtStack) {
    for (unsigned i = 0; i < m_warp_count; ++i) delete m_SimtStack[i];
    delete[] m_SimtStack;
    m_SimtStack = NULL;
  }
}

void ThreadBlock::initilizeSIMTStack(unsigned warp_count, unsigned warp_size) {
  m_SimtStack = new SimtStack *[warp_count];
  for (unsigned i = 0; i < warp_count; ++i)
    m_SimtStack[i] = new SimtStack(i, warp_size, m_gpgpu_ctx);
  m_warp_size = warp_size;
  m_warp_count = warp_count;
}

void ThreadBlock::get_pdom_stack_top_info(unsigned warpId, unsigned *pc,
                                     unsigned *rpc) const {
  m_SimtStack[warpId]->get_pdom_stack_top_info(pc, rpc);
}

void ThreadBlock::initializeCTA(unsigned ctaid_cp) {
  int ctaLiveThreads = 0;

  for (int i = 0; i < m_warp_count; i++) {
    m_warpAtBarrier[i] = false;
    m_liveThreadCount[i] = 0;
  }
  for (int i = 0; i < m_warp_count * m_warp_size; i++) m_thread[i] = nullptr;

  // get threads for a cta
  for (unsigned i = 0; i < m_kernel->threads_per_cta(); i++) {
    createThread(*m_kernel, &m_thread[i], 0, i,
                        m_kernel->threads_per_cta() - i,
                        m_kernel->threads_per_cta(), this, 0, i / m_warp_size,
                        (libcuda::gpgpu_t *)m_gpu, true);
    assert(m_thread[i] != NULL && !m_thread[i]->is_done());
    char fname[2048];
    snprintf(fname, 2048, "checkpoint_files/thread_%d_0_reg.txt", i);
    // if (m_gpu->gpgpu_ctx->func_sim->cp_cta_resume == 1)
    //  m_thread[i]->resume_reg_thread(fname, symtab);
    ctaLiveThreads++;
  }

  for (int k = 0; k < m_warp_count; k++) createWarp(k);
}

unsigned ThreadBlock::createThread(KernelInfo &kernel,
                             ThreadItem** thread_info, int sid,
                             unsigned tid, unsigned threads_left,
                             unsigned num_threads, ThreadBlock *tb,
                             unsigned hw_cta_id, unsigned hw_warp_id,
                             libcuda::gpgpu_t *gpu, bool isInFunctionalSimulationMode) {
  std::list<ThreadItem*> &active_threads = kernel.m_active_threads;

  static std::map<unsigned, libcuda::memory_space *> shared_memory_lookup;
  static std::map<unsigned, libcuda::memory_space *> sstarr_memory_lookup;
  static std::map<unsigned, cta_info_t *> ptx_cta_lookup;
  static std::map<unsigned, Warp *> ptx_warp_lookup;
  static std::map<unsigned, std::map<unsigned, libcuda::memory_space *> >
      local_memory_lookup;

  if (*thread_info != NULL) {
    ThreadItem *thd = *thread_info;
    assert(thd->is_done());
    if (libcuda::g_debug_execution == -1) {
      dim3 ctaid = thd->get_ctaid();
      dim3 t = thd->get_tid();
      printf(
          "GPGPU-Sim PTX simulator:  thread exiting ctaid=(%u,%u,%u) "
          "tid=(%u,%u,%u) uid=%u\n",
          ctaid.x, ctaid.y, ctaid.z, t.x, t.y, t.z, thd->get_uid());
      fflush(stdout);
    }
    thd->m_cta_info->register_deleted_thread(thd);
    thd = nullptr;
    *thread_info = nullptr;
  }

  if (!active_threads.empty()) {
    assert(active_threads.size() <= threads_left);
    ThreadItem* thd = active_threads.front();
    active_threads.pop_front();
    *thread_info = thd;
    thd->init(gpu, tb, sid, hw_cta_id, hw_warp_id, tid);
    return 1;
  }

  if (kernel.no_more_ctas_to_run()) {
    return 0;  // finished!
  }

  if (threads_left < kernel.threads_per_cta()) {
    return 0;
  }

  if (libcuda::g_debug_execution == -1) {
    printf("GPGPU-Sim PTX simulator:  STARTING THREAD ALLOCATION --> \n");
    fflush(stdout);
  }

  // initializing new CTA
  cta_info_t *cta_info = NULL;
  libcuda::memory_space *shared_mem = NULL;
  libcuda::memory_space *sstarr_mem = NULL;

  unsigned cta_size = kernel.threads_per_cta();
  unsigned max_cta_per_sm = num_threads / cta_size;  // e.g., 256 / 48 = 5
  assert(max_cta_per_sm > 0);

  unsigned sm_idx = (tid/cta_size)*m_gpgpu_param_num_shaders + sid;
  // unsigned sm_idx = hw_cta_id * m_gpgpu_ctx->func_sim->gpgpu_param_num_shaders + sid;

  if (shared_memory_lookup.find(sm_idx) == shared_memory_lookup.end()) {
    if (libcuda::g_debug_execution >= 1) {
      printf("  <CTA alloc> : sm_idx=%u sid=%u max_cta_per_sm=%u\n", sm_idx,
             sid, max_cta_per_sm);
    }
    char buf[512];
    snprintf(buf, 512, "shared_%u", sid);
    shared_mem = new libcuda::memory_space_impl<16 * 1024>(buf, 4);
    shared_memory_lookup[sm_idx] = shared_mem;
    snprintf(buf, 512, "sstarr_%u", sid);
    sstarr_mem = new libcuda::memory_space_impl<16 * 1024>(buf, 4);
    sstarr_memory_lookup[sm_idx] = sstarr_mem;
    cta_info_t *cta_info = new cta_info_t(sm_idx, gpu->gpgpu_ctx);
    ptx_cta_lookup[sm_idx] = cta_info;
  } else {
    if (libcuda::g_debug_execution >= 1) {
      printf("  <CTA realloc> : sm_idx=%u sid=%u max_cta_per_sm=%u\n", sm_idx,
             sid, max_cta_per_sm);
    }
    shared_mem = shared_memory_lookup[sm_idx];
    sstarr_mem = sstarr_memory_lookup[sm_idx];
    cta_info = ptx_cta_lookup[sm_idx];
    cta_info->check_cta_thread_status_and_reset();
  }

  std::map<unsigned, libcuda::memory_space *> &local_mem_lookup =
      local_memory_lookup[sid];
  while (kernel.more_threads_in_cta()) {
    dim3 ctaid3d = kernel.get_next_cta_id();
    unsigned new_tid = kernel.get_next_thread_id();
    dim3 tid3d = kernel.get_next_thread_id_3d();
    kernel.increment_thread_id();
    new_tid += tid;
    ThreadItem *thd = new ThreadItem();
    Warp *warp_info = NULL;
    if (ptx_warp_lookup.find(hw_warp_id) == ptx_warp_lookup.end()) {
      warp_info = new Warp(hw_warp_id, m_warp_size, this);
      ptx_warp_lookup[hw_warp_id] = warp_info;
    } else {
      warp_info = ptx_warp_lookup[hw_warp_id];
    }
    thd->m_warp = warp_info;

    libcuda::memory_space *local_mem = NULL;
    std::map<unsigned, libcuda::memory_space *>::iterator l =
        local_mem_lookup.find(new_tid);
    if (l != local_mem_lookup.end()) {
      local_mem = l->second;
    } else {
      char buf[512];
      snprintf(buf, 512, "local_%u_%u", sid, new_tid);
      local_mem = new libcuda::memory_space_impl<32>(buf, 32);
      local_mem_lookup[new_tid] = local_mem;
    }
    // thd->set_info(kernel.entry());
    thd->set_nctaid(kernel.get_grid_dim());
    thd->set_ntid(kernel.get_cta_dim());
    thd->set_ctaid(ctaid3d);
    thd->set_tid(tid3d);
    //if (kernel.entry()->get_ptx_version().extensions())
    //  thd->cpy_tid_to_reg(tid3d);
    thd->set_valid();
    thd->m_shared_mem = shared_mem;
    thd->m_sstarr_mem = sstarr_mem;
    /* FIXME
    function_info *finfo = thd->func_info();
    symbol_table *st = finfo->get_symtab();
    thd->func_info()->param_to_shared(thd->m_shared_mem, st);
    thd->func_info()->param_to_shared(thd->m_sstarr_mem, st);
    */
    thd->m_cta_info = cta_info;
    cta_info->add_thread(thd);
    thd->m_local_mem = local_mem;
    if (libcuda::g_debug_execution == -1) {
      printf(
          "GPGPU-Sim PTX simulator:  allocating thread ctaid=(%u,%u,%u) "
          "tid=(%u,%u,%u) @ 0x%Lx\n",
          ctaid3d.x, ctaid3d.y, ctaid3d.z, tid3d.x, tid3d.y, tid3d.z,
          (unsigned long long)thd);
      fflush(stdout);
    }
    active_threads.push_back(thd);
  }
  if (libcuda::g_debug_execution == -1) {
    printf("GPGPU-Sim PTX simulator:  <-- FINISHING THREAD ALLOCATION\n");
    fflush(stdout);
  }

  kernel.increment_cta_id();

  assert(active_threads.size() <= threads_left);
  *thread_info = active_threads.front();
  (*thread_info)->init(gpu, tb, sid, hw_cta_id, hw_warp_id, tid);
  active_threads.pop_front();
  return 1;
}

void ThreadBlock::createWarp(unsigned warpId) {
  simt_mask_t initialMask;
  unsigned liveThreadsCount = 0;
  initialMask.set();
  for (int i = warpId * m_warp_size; i < warpId * m_warp_size + m_warp_size;
       i++) {
    if (m_thread[i] == NULL)
      initialMask.reset(i - warpId * m_warp_size);
    else
      liveThreadsCount++;
  }

  assert(m_thread[warpId * m_warp_size] != NULL);
  m_SimtStack[warpId]->launch(m_thread[warpId * m_warp_size]->get_pc(),
                               initialMask);
  char fname[2048];
  snprintf(fname, 2048, "checkpoint_files/warp_%d_0_simt.txt", warpId);

  if (m_gpgpu_ctx->func_sim->cp_cta_resume == 1) {
    unsigned pc, rpc;
    m_SimtStack[warpId]->resume(fname);
    m_SimtStack[warpId]->get_pdom_stack_top_info(&pc, &rpc);
    for (int i = warpId * m_warp_size; i < warpId * m_warp_size + m_warp_size;
         i++) {
      m_thread[i]->set_npc(pc);
      m_thread[i]->update_pc();
    }
  }
  m_liveThreadCount[warpId] = liveThreadsCount;
}

void ThreadBlock::execute(int inst_count, unsigned ctaid_cp) {
  m_gpgpu_ctx->func_sim->cp_count = m_gpu->checkpoint_insn_Y;
  m_gpgpu_ctx->func_sim->cp_cta_resume = m_gpu->checkpoint_CTA_t;
  initializeCTA(ctaid_cp);

  int count = 0;
  while (true) {
    bool someOneLive = false;
    bool allAtBarrier = true;
    for (unsigned i = 0; i < m_warp_count; i++) {
      executeWarp(i, allAtBarrier, someOneLive);
      count++;
    }

    if (inst_count > 0 && count > inst_count &&
        (m_kernel->get_uid() == m_gpu->checkpoint_kernel) &&
        (ctaid_cp >= m_gpu->checkpoint_CTA) &&
        (ctaid_cp < m_gpu->checkpoint_CTA_t) && m_gpu->checkpoint_option == 1) {
      someOneLive = false;
      break;
    }
    if (!someOneLive) break;
    if (allAtBarrier) {
      for (unsigned i = 0; i < m_warp_count; i++) m_warpAtBarrier[i] = false;
    }
  }

  checkpoint *g_checkpoint;
  g_checkpoint = new checkpoint();

  ptx_reg_t regval;
  regval.u64 = 123;

  unsigned ctaid = m_kernel->get_next_cta_id_single();
  if (m_gpu->checkpoint_option == 1 &&
      (m_kernel->get_uid() == m_gpu->checkpoint_kernel) &&
      (ctaid_cp >= m_gpu->checkpoint_CTA) &&
      (ctaid_cp < m_gpu->checkpoint_CTA_t)) {
    char fname[2048];
    snprintf(fname, 2048, "checkpoint_files/shared_mem_%d.txt", ctaid - 1);
    g_checkpoint->store_global_mem(m_thread[0]->m_shared_mem, fname,
                                   (char *)"%08x");
    for (int i = 0; i < 32 * m_warp_count; i++) {
      char fname[2048];
      snprintf(fname, 2048, "checkpoint_files/thread_%d_%d_reg.txt", i,
               ctaid - 1);
      // FIXME m_thread[i]->print_reg_thread(fname);
      char f1name[2048];
      snprintf(f1name, 2048, "checkpoint_files/local_mem_thread_%d_%d_reg.txt",
               i, ctaid - 1);
      g_checkpoint->store_global_mem(m_thread[i]->m_local_mem, f1name,
                                     (char *)"%08x");
      m_thread[i]->set_done();
      m_thread[i]->exitCore();
      m_thread[i]->registerExit();
    }

    for (int i = 0; i < m_warp_count; i++) {
      char fname[2048];
      snprintf(fname, 2048, "checkpoint_files/warp_%d_%d_simt.txt", i,
               ctaid - 1);
      FILE *fp = fopen(fname, "w");
      assert(fp != NULL);
      m_SimtStack[i]->print_checkpoint(fp);
      fclose(fp);
    }
  }
}

void ThreadBlock::executeWarp(unsigned warpId, bool &allAtBarrier,
                                    bool &someOneLive) {
  if (!m_warpAtBarrier[warpId] && m_liveThreadCount[warpId] != 0) {
    std::shared_ptr<Instruction> inst = getExecuteWarp(warpId);
    executeInstruction(inst, warpId);
    if (inst->isatomic()) inst->do_atomic(true);
    if (inst->GetOpType() == opu_op_t::BARRIER_OP || inst->GetOpType() == opu_op_t::MEMORY_BARRIER_OP)
      m_warpAtBarrier[warpId] = true;
    updateSIMTStack(warpId, inst);
  }
  if (m_liveThreadCount[warpId] > 0) someOneLive = true;
  if (!m_warpAtBarrier[warpId] && m_liveThreadCount[warpId] > 0) allAtBarrier = false;
}

void ThreadBlock::warp_exit(unsigned warp_id) {
  for (int i = 0; i < m_warp_count * m_warp_size; i++) {
    if (m_thread[i] != NULL) {
      m_thread[i]->m_cta_info->register_deleted_thread(m_thread[i]);
      delete m_thread[i];
    }
  }
}
bool ThreadBlock::warp_waiting_at_barrier( unsigned warp_id ) const
{
  return (m_warpAtBarrier[warp_id] || !(m_liveThreadCount[warp_id]>0));
}

void ThreadBlock::checkExecutionStatusAndUpdate(shared_ptr<Instruction> &inst, unsigned t, unsigned tid)
{
  if(m_thread[tid]==NULL || m_thread[tid]->is_done()){
        m_liveThreadCount[tid/m_warp_size]--;
  }
}

