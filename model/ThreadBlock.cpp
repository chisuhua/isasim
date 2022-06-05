#include "inc/ThreadBlock.h"
#include "inc/BlockState.h"
#include "inc/ThreadItem.h"
#include "inc/Warp.h"
#include "inc/WarpState.h"
// #include "inc/CUResource.h"
#include "inc/Instruction.h"
#include <bitset>
//#include "../../libcuda/abstract_hardware_model.h"
#include "../../libcuda/cuda-sim/memory.h"
#include "../../libcuda/gpgpu_context.h"
#include "inc/FunUnit.h"

extern int g_debug_exec;

ThreadBlock::ThreadBlock(libcuda::gpgpu_t *gpu, KernelInfo* kernel, libcuda::gpgpu_context *ctx, BlockState *block_state, unsigned warp_size, unsigned threads_per_shader, dim3 cta_id, uint32_t kernel_const_num)
    : m_block_state(block_state)
    , m_warp_size(warp_size)
    , m_kernel(kernel)
{
    m_kernel_addr = kernel->state()->getProgAddr();
    m_kernel_args = kernel->state()->getParamAddr();
    m_gpu = gpu;
    m_global_mem = gpu->get_global_memory();
    m_gpgpu_ctx = ctx;
    m_kernel_const_num = kernel_const_num;
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
    m_warp_live_thread = new unsigned [m_warp_count];
    m_funit = new FunUnit();
}

ThreadBlock::~ThreadBlock(){
    m_funit->dumpVCD(m_dump_tb_name);
    // warp_exit(0);
    delete[] m_warp_live_thread;
    // delete[] m_warp_at_bar;
    delete m_block_state;
    delete m_funit;
    // delete m_kernel;
    // free(m_thread);
}


void ThreadBlock::executeInstruction(shared_ptr<Instruction> inst, unsigned warpId) {
  active_mask_t active_mask = m_Warp[warpId]->get_simt_active_mask();
  bool leading_thread = true;
  if (g_debug_exec > 1) {
    inst->Issue(m_Warp[warpId]->m_warp_state);
    inst->OperandCollect(m_Warp[warpId]->m_warp_state);
  }
  m_Warp[warpId]->m_warp_state->setActiveMask(active_mask);
  for (unsigned t = 0; t < m_warp_size; t++) {
    if (active_mask.test(t)) {
      // if (warpId == (unsigned(-1))) warpId = inst.warp_id();
      unsigned tid = m_warp_size * warpId + t;
      // printf("Info: Warp%d, thread%d, executeInstruction %lx\n", warpId, t, inst->bytes.dword);
      m_thread[tid]->m_leading_thread = leading_thread;
      leading_thread = false;
      m_thread[tid]->Execute(inst, m_Warp[warpId]->m_warp_state);
      if (m_Warp[warpId]->m_warp_state->isLaneExit(t)) {
        m_thread[tid]->set_done();
        // m_kernel->m_thread_pool.push_back(m_thread[tid]);
      }

      // virtual function
      checkExecutionStatusAndUpdate(inst, t, tid);
    }
  }
  if (g_debug_exec > 1) {
    inst->WriteBack(m_Warp[warpId]->m_warp_state);
    m_Warp[warpId]->m_warp_state->flush();
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
  m_Warp[warpId]->update(thread_done, next_pc, inst->reconvergence_pc,
                               inst->GetOpType(), inst->GetSize(), inst->pc);
}

shared_ptr<Instruction> ThreadBlock::getInstruction(address_type pc) {
    if (m_insts.find(pc) == m_insts.end()) {
        uint64_t pc_address = m_kernel_addr + pc;
        // FIXME
        // uint64_t opcode = *(uint64_t*)(pc_address);
        uint64_t opcode;
        m_gpu->get_global_memory()->read(pc_address, 8, &opcode);
        printf("IFETCH PC=%4ld(%lx): opcode %lx\n", pc, pc_address, opcode);
        // printf("make instrction PC=%lx for opcode %lx\n", pc, opcode);
        m_insts[pc] = make_instruction(opcode, m_funit);
        m_insts[pc]->pc = pc;
        m_insts[pc]->Decode(opcode);
        m_insts[pc]->print();
        return m_insts[pc];
    }
    // FIXME modify m_insts to instrubuffer
    return m_insts[pc];
}

//! Get the warp to be executed using the data taken form the SIMT stack
std::shared_ptr<Instruction> ThreadBlock::getExecuteWarp(unsigned warpId) {
    unsigned pc, rpc;
    m_Warp[warpId]->get_pdom_stack_top_info(&pc, &rpc);
    auto inst = getInstruction(pc);
    // WarpInst wi(inst);
    // wi.set_active(m_Warp[warpId]->get_active_mask());
    // FIXME inst->set_active(m_Warp[warpId]->get_active_mask());
    m_Warp[warpId]->update_simt_active_mask(inst->isatomic());
    return inst;
}

void ThreadBlock::deleteSIMTStack() {
    if (m_Warp) {
        for (unsigned i = 0; i < m_warp_count; ++i) delete m_Warp[i];
        delete[] m_Warp;
        m_Warp = NULL;
    }
}

void ThreadBlock::initilizeSIMTStack(unsigned warp_count, unsigned warp_size) {
  m_Warp = new Warp *[warp_count];
  for (unsigned i = 0; i < warp_count; ++i) {
    m_Warp[i] = new Warp(i, warp_size, m_gpgpu_ctx);
  }
  m_warp_size = warp_size;
  m_warp_count = warp_count;
}

void ThreadBlock::get_pdom_stack_top_info(unsigned warpId, unsigned *pc,
                                     unsigned *rpc) const {
  m_Warp[warpId]->get_pdom_stack_top_info(pc, rpc);
}

void ThreadBlock::initializeCTA(uint32_t ctaid_cp) {
  int ctaLiveThreads = 0;

  for (int i = 0; i < m_warp_count; i++) {
    m_warp_live_thread[i] = 0;
  }
  for (int i = 0; i < m_warp_count * m_warp_size; i++) m_thread[i] = nullptr;

  // get threads for a cta
  for (unsigned i = 0; i < m_kernel->threads_per_cta(); i++) {
    createThread(*m_kernel, &m_thread[i], 0, i,
                        m_kernel->threads_per_cta() - i,
                        m_kernel->threads_per_cta(), this, 0, i / m_warp_size,
                        (libcuda::gpgpu_t *)m_gpu, true);
    assert(m_thread[i] != NULL);
    // assert(m_thread[i]->is_done());
    char fname[2048];
    snprintf(fname, 2048, "checkpoint_files/thread_%d_0_reg.txt", i);
    // if (m_gpu->gpgpu_ctx->func_sim->cp_cta_resume == 1)
    //  m_thread[i]->resume_reg_thread(fname, symtab);
    ctaLiveThreads++;
  }


  uint32_t sid = 0;
  char buf[512];
  snprintf(buf, 512, "shared_%u", sid);
  m_shared_mem = new libcuda::memory_space_impl<16 * 1024>(buf, 4);

  int32_t block_idx_x_vreg_num = -1;
  int32_t block_idx_y_vreg_num = -1;
  int32_t block_idx_z_vreg_num = -1;

  if ((m_kernel->kernel_ctrl() >> KERNEL_CTRL_BIT_BLOCK_IDX_X) & 0x1) {
    block_idx_x_vreg_num = 0;
  }
  if ((m_kernel->kernel_ctrl() >> KERNEL_CTRL_BIT_BLOCK_IDX_Y) & 0x1) {
    block_idx_y_vreg_num = block_idx_x_vreg_num + 1;
  }
  if ((m_kernel->kernel_ctrl() >> KERNEL_CTRL_BIT_BLOCK_IDX_Y) & 0x1) {
    block_idx_z_vreg_num = block_idx_y_vreg_num + 1;
  }

  uint32_t x = m_block_state->get_cta_id().x;
  uint32_t y = m_block_state->get_cta_id().y;
  uint32_t z = m_block_state->get_cta_id().z;
  if (block_idx_x_vreg_num >= 0)
      m_shared_mem->write(block_idx_x_vreg_num, 4, &x, nullptr, nullptr);
  if (block_idx_y_vreg_num >= 0)
      m_shared_mem->write(block_idx_x_vreg_num, 4, &y, nullptr, nullptr);
  if (block_idx_z_vreg_num >= 0)
      m_shared_mem->write(block_idx_x_vreg_num, 4, &z, nullptr, nullptr);

  const char *tbid = getenv("ISASIM_DUMP");
  int32_t x_dump, y_dump, z_dump, w_dump;
  if (tbid) {
      sscanf(tbid, "%d:%d:%d:%d", &x_dump, &y_dump, &z_dump, &w_dump);
      if (((x_dump == -1) || (x_dump == x)) && ((y_dump == - 1) || (y_dump == y)) &&
          ((z_dump == -1) || (z_dump == z))) m_dump_enable = true;

      m_dump_tb_name = "tb_" + std::to_string(m_block_state->get_cta_id().x) +
          "_" + std::to_string(m_block_state->get_cta_id().y) +
          "_" + std::to_string(m_block_state->get_cta_id().z);
  }
  for (int k = 0; k < m_warp_count; k++) {
      createWarp(k, m_dump_enable && ((w_dump == -1) || (w_dump == k)));
  }
}

unsigned ThreadBlock::createThread(KernelInfo &kernel,
                             ThreadItem** thread_info, int sid,
                             unsigned tid, unsigned threads_left,
                             unsigned num_threads, ThreadBlock *tb,
                             unsigned hw_cta_id, unsigned hw_warp_id,
                             libcuda::gpgpu_t *gpu, bool isInFunctionalSimulationMode) {
#if 0
  std::list<ThreadItem*> &thread_pool = kernel.m_thread_pool;
  if (*thread_info != NULL) {
    ThreadItem *thd = *thread_info;
    assert(thd->is_done());
    if (g_debug_exec == -1) {
      dim3 ctaid = thd->get_ctaid();
      dim3 t = thd->get_tid();
      printf(
          "GPGPU-Sim PTX simulator:  thread exiting ctaid=(%u,%u,%u) "
          "tid=(%u,%u,%u) uid=%u\n",
          ctaid.x, ctaid.y, ctaid.z, t.x, t.y, t.z, thd->get_uid());
      fflush(stdout);
    }
    thd->m_block_state->register_deleted_thread(thd);
    thd = nullptr;
    *thread_info = nullptr;
  }
#endif
  ThreadItem* thd;
  /*
  if (!thread_pool.empty()) {
    assert(thread_pool.size() <= threads_left);
    thd = thread_pool.front();
    thread_pool.pop_front();
    *thread_info = thd;
  } else {
  */
    thd = new ThreadItem();
    *thread_info = thd;
  //}

  thd->init(gpu, tb, sid, hw_cta_id, hw_warp_id, tid);
  if (g_debug_exec > 1) {
    printf("GPGPU-Sim PTX simulator:  STARTING THREAD ALLOCATION --> \n");
    fflush(stdout);
  }

  unsigned new_tid = kernel.get_next_thread_id();
  dim3 ctaid3d = kernel.get_next_cta_id();
  dim3 tid3d = kernel.get_next_thread_id_3d();
  kernel.increment_thread_id();
  new_tid += tid;

  thd->set_nctaid(kernel.get_grid_dim());
  thd->set_ntid(kernel.get_cta_dim());
  thd->set_ctaid(ctaid3d);
  thd->set_tid(tid3d);
  thd->set_valid();
  thd->set_valid();
  thd->set_pc(0);
  if (g_debug_exec == -1) {
    printf(
          "GPGPU-Sim PTX simulator:  allocating thread ctaid=(%u,%u,%u) "
          "tid=(%u,%u,%u) @ 0x%Lx\n",
          ctaid3d.x, ctaid3d.y, ctaid3d.z, tid3d.x, tid3d.y, tid3d.z,
          (unsigned long long)thd);
      fflush(stdout);
  }
  // thread_pool.push_back(thd);
  if (g_debug_exec == -1) {
    printf("GPGPU-Sim PTX simulator:  <-- FINISHING THREAD ALLOCATION\n");
    fflush(stdout);
  }
}

void ThreadBlock::createWarp(unsigned warpId, bool dump_enable) {
  std::function<dsm_access_ftype> dsm_read_func =
      [mem = this->m_shared_mem](uint64_t addr, size_t length, void* data) {
      mem->read(addr, length, data);
  };
  std::function<dsm_access_ftype> dsm_write_func =
      [mem = this->m_shared_mem](uint64_t addr, size_t length, void* data) {
      mem->write(addr, length, data, nullptr, nullptr);
  };

  std::function<mem_access_ftype> mem_read_func =
      [mem_global = this->m_global_mem,
       mem_shared = this->m_shared_mem
      ](uint64_t addr, size_t length, void* data, isasim::mem_space_t::SpaceType mem_space) {
      switch (mem_space) {
        case isasim::mem_space_t::global_space:
        case isasim::mem_space_t::const_space:
        case isasim::mem_space_t::param_local_space:
        case isasim::mem_space_t::param_kernel_space:
        case isasim::mem_space_t::local_space:
            mem_global->read(addr, length, data);
            break;
        case isasim::mem_space_t::shared_space:
            mem_shared->read(addr, length, data);
            break;
        case isasim::mem_space_t::generic_space:
            printf("Warnning, we need to check share space\n");
            mem_global->read(addr, length, data);
            break;
        default:
            mem_shared->read(addr, length, data);
            // assert(false);
      }
  };

  std::function<mem_access_ftype> mem_write_func =
      [mem_global = this->m_global_mem,
       mem_shared = this->m_shared_mem
      ](uint64_t addr, size_t length, void* data, isasim::mem_space_t::SpaceType mem_space) {
      switch (mem_space) {
        case isasim::mem_space_t::global_space:
        case isasim::mem_space_t::const_space:
        case isasim::mem_space_t::param_local_space:
        case isasim::mem_space_t::param_kernel_space:
        case isasim::mem_space_t::local_space:
            mem_global->write(addr, length, data, nullptr, nullptr);
            break;
        case isasim::mem_space_t::shared_space:
            mem_shared->write(addr, length, data, nullptr, nullptr);
            break;
        case isasim::mem_space_t::generic_space:
            printf("Warnning, we need to check share space\n");
            mem_global->write(addr, length, data, nullptr, nullptr);
            break;
        default:
            mem_shared->write(addr, length, data, nullptr, nullptr);
            // assert(false);
      }
  };

  m_Warp[warpId]->m_warp_state = new WarpState(256, 265, m_warp_size,
          m_kernel_const_num,
          dsm_read_func, dsm_write_func,
          mem_read_func, mem_write_func);

  WarpState *warp_state = m_Warp[warpId]->m_warp_state;
  warp_state->init(warpId, m_block_state, m_thread);

  uint64_t warp_stack_pointer =
        m_kernel->state()->getLocalAddr() + m_kernel->state()->getLocalSize() * warpId;
  warp_state->setStackPointer(warp_stack_pointer);

  // const buffer is used param_kernel_space
  // uint64_t const_buffer = m_kernel->state()->getParamAddr();
  uint32_t *const_buffer = m_kernel->state()->getConstBuffer();
  warp_state->setConstBuffer(const_buffer);
#if 0
  // FIXME move block const to share memory
  uint32_t block_const_reg_num = BLOCK_CONST_REG_BASE; // stackpointer have use sreg[0:1]
  // below behavior is same as coasm, which kernel access reg in same way
  // FIXME we can move block const reg in shared memory
  if ((m_kernel->kernel_ctrl() >> KERNEL_CTRL_BIT_BLOCK_IDX_X) & 0x1) {
      warp_state->setSreg(block_const_reg_num, m_block_state->get_cta_id().x);
      block_const_reg_num++;
  }
  if ((m_kernel->kernel_ctrl() >> KERNEL_CTRL_BIT_BLOCK_IDX_Y) & 0x1) {
      warp_state->setSreg(block_const_reg_num, m_block_state->get_cta_id().y);
      block_const_reg_num++;
  }
  if ((m_kernel->kernel_ctrl() >> KERNEL_CTRL_BIT_BLOCK_IDX_Z) & 0x1) {
      warp_state->setSreg(block_const_reg_num, m_block_state->get_cta_id().z);
      block_const_reg_num++;
  }
#endif
  simt_mask_t initialMask;
  unsigned liveThreadsCount = 0;
  initialMask.set();

  int32_t thread_idx_x_vreg_num = -1;
  int32_t thread_idx_y_vreg_num = -1;
  int32_t thread_idx_z_vreg_num = -1;

  if ((m_kernel->kernel_ctrl() >> KERNEL_CTRL_BIT_THREAD_IDX_X) & 0x1) {
    thread_idx_x_vreg_num = 0;
  }
  if ((m_kernel->kernel_ctrl() >> KERNEL_CTRL_BIT_THREAD_IDX_Y) & 0x1) {
    thread_idx_y_vreg_num = thread_idx_x_vreg_num + 1;
  }
  if ((m_kernel->kernel_ctrl() >> KERNEL_CTRL_BIT_THREAD_IDX_Y) & 0x1) {
    thread_idx_z_vreg_num = thread_idx_y_vreg_num + 1;
  }

  for (int i = warpId * m_warp_size; i < warpId * m_warp_size + m_warp_size;
       i++) {
    int lane_id = i -  warpId * m_warp_size;
    if (m_thread[i] == NULL)
      initialMask.reset(lane_id);
    else {
      m_thread[i]->set_laneid(lane_id);
      liveThreadsCount++;
      if (thread_idx_x_vreg_num >= 0)
          warp_state->setVreg(thread_idx_x_vreg_num + THREAD_REG_BASE,
                  m_thread[i]->get_tid().x, lane_id);
      if (thread_idx_y_vreg_num >= 0)
          warp_state->setVreg(thread_idx_y_vreg_num + THREAD_REG_BASE,
                  m_thread[i]->get_tid().y, lane_id);
      if (thread_idx_z_vreg_num >= 0)
          warp_state->setVreg(thread_idx_z_vreg_num + THREAD_REG_BASE,
                  m_thread[i]->get_tid().z, lane_id);
    }
  }

  if (dump_enable) {
    std::string dump_file = "instdump_";
    m_Warp[warpId]->m_warp_state->initDump(
          dump_file + m_dump_tb_name + "_warp_" + std::to_string(warpId) + ".log");
  }

  m_Warp[warpId]->launch(m_thread[warpId * m_warp_size]->get_pc(), initialMask);

  char fname[2048];
  snprintf(fname, 2048, "checkpoint_files/warp_%d_0_simt.txt", warpId);

  if (m_gpgpu_ctx->func_sim->cp_cta_resume == 1) {
    unsigned pc, rpc;
    m_Warp[warpId]->resume(fname);
    m_Warp[warpId]->get_pdom_stack_top_info(&pc, &rpc);
    for (int i = warpId * m_warp_size; i < warpId * m_warp_size + m_warp_size;
         i++) {
      m_thread[i]->set_npc(pc);
      m_thread[i]->update_pc();
    }
  }
  m_warp_live_thread[warpId] = liveThreadsCount;
}

static std::vector<uint32_t> GetWarpExecuteOrder(uint32_t warp_count) {
    std::vector<uint32_t> warp_order(warp_count);
    for (uint32_t i = 0; i < warp_count; i++) {
        warp_order[i] = i;
    }
#if WARP_EXECUTE_RANDOM_ORDER
    auto rng = std::default_random_engine {};
    std::shuffle(std::begin(warp_order), std::end(warp_order), rng);
#endif
    return warp_order;
}

bool ThreadBlock::execute(uint32_t ctaid_cp) {
    m_gpgpu_ctx->func_sim->cp_count = m_gpu->checkpoint_insn_Y;
    m_gpgpu_ctx->func_sim->cp_cta_resume = m_gpu->checkpoint_CTA_t;
    initializeCTA(ctaid_cp);

    while (true) {
        bool all_done = true;
        bool allwarp_at_barrier = true;
        for (unsigned warp_id : GetWarpExecuteOrder(m_warp_count)) {
            if (m_warp_live_thread[warp_id]>0) { all_done = false;}
            if (warp_is_blocking(warp_id) || warp_waiting_at_barrier(warp_id)) {
                continue;
            }
            allwarp_at_barrier = false;
            executeWarp(warp_id);
            if (m_warp_live_thread[warp_id] == 0) {
                m_block_state->removeWarp(warp_id);
            }
        }

        if ((m_kernel->get_uid() == m_gpu->checkpoint_kernel) &&
            (ctaid_cp >= m_gpu->checkpoint_CTA) &&
            (ctaid_cp < m_gpu->checkpoint_CTA_t) && m_gpu->checkpoint_option == 1) {
            all_done = true;
            break;
        }
        if (all_done) break;
        /*
        if (allwarp_at_barrier) {
            for (unsigned i = 0; i < m_warp_count; i++) m_warp_at_bar[i] = false;
        }
        */
    }

    checkpoint *g_checkpoint;
    g_checkpoint = new checkpoint();

    unsigned ctaid = m_kernel->get_next_cta_id_single();
    if (m_gpu->checkpoint_option == 1 &&
      (m_kernel->get_uid() == m_gpu->checkpoint_kernel) &&
      (ctaid_cp >= m_gpu->checkpoint_CTA) &&
      (ctaid_cp < m_gpu->checkpoint_CTA_t)) {
#if 0
    char fname[2048];
    snprintf(fname, 2048, "checkpoint_files/shared_mem_%d.txt", ctaid - 1);
    g_checkpoint->store_global_mem(m_thread[0]->m_shared_mem, fname,
                                   (char *)"%08x");
#endif
        for (int i = 0; i < 32 * m_warp_count; i++) {
            char fname[2048];
            snprintf(fname, 2048, "checkpoint_files/thread_%d_%d_reg.txt", i,
               ctaid - 1);
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
            m_Warp[i]->print_checkpoint(fp);
            fclose(fp);
        }
    }
    return true;
}

void ThreadBlock::executeWarp(unsigned warp_id) {
    std::shared_ptr<Instruction> inst = getExecuteWarp(warp_id);
    executeInstruction(inst, warp_id);
    // FIXME if (inst->isatomic()) inst->do_atomic(true);
    /*
    if (inst->GetOpType() == opu_op_t::BARRIER_OP || inst->GetOpType() == opu_op_t::MEMORY_BARRIER_OP)
    {
      m_warp_at_bar[warpId] = true;
    }
    */
    updateSIMTStack(warp_id, inst);
}

void ThreadBlock::warp_exit(unsigned warp_id) {
  for (int i = 0; i < m_warp_count * m_warp_size; i++) {
    // FIXME 
    if (m_thread[i] != NULL) {
      // m_thread[i]->m_block_state->register_deleted_thread(m_thread[i]);
      // delete m_thread[i];
    }
  }
}
bool ThreadBlock::warp_is_blocking( unsigned warp_id ) const {
  bool is_blocking = m_Warp[warp_id]->m_warp_state->isBlocking();
  return is_blocking;
}

bool ThreadBlock::warp_waiting_at_barrier( unsigned warp_id ) const
{
  return (m_block_state->m_warp_at_bar[warp_id] || !(m_warp_live_thread[warp_id]>0));
}

void ThreadBlock::checkExecutionStatusAndUpdate(shared_ptr<Instruction> &inst, unsigned t, unsigned tid)
{
  if(m_thread[tid]==NULL || m_thread[tid]->is_done()){
    m_warp_live_thread[tid/m_warp_size]--;
  }
}

