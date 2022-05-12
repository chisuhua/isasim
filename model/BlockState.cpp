#include "inc/BlockState.h"
#include "inc/ThreadItem.h"
#include "inc/Warp.h"
#include "inc/WarpState.h"
#include "inc/CUResource.h"
#include "inc/Instruction.h"
#include <bitset>
//#include "../../libcuda/abstract_hardware_model.h"
#include "../../libcuda/cuda-sim/memory.h"
#include "../../libcuda/gpgpu_context.h"
#include "inc/HwOp.h"

BlockState::BlockState(CUResource *cu_res, uint32_t bar_count)
    : m_resource(cu_res)
    , m_bar_count(bar_count)
{
    m_bar_slot.resize(bar_count);
    initBar(bar_count);
}

void BlockState::destroy() {
    m_resource->freeBarSlot(m_bar_count);
    m_resource->freeRunningBlock();
}

void BlockState::init(gpgpu_context *ctx, dim3 cta_id) {
  m_uid = (ctx->g_ptx_cta_info_uid)++;
  // FIXME, use real kernel bar requirment num
  m_bar_threads = 0;
  m_cta_id = cta_id;
  gpgpu_ctx = ctx;
}

void BlockState::add_thread( ThreadItem *thd )
{
   m_threads_in_cta.insert(thd);
}

unsigned BlockState::num_threads() const
{
   return m_threads_in_cta.size();
}

void BlockState::check_cta_thread_status_and_reset() {
  bool fail = false;
  if (m_threads_that_have_exited.size() != m_threads_in_cta.size()) {
    printf("\n\n");
    printf(
        "Execution error: Some threads still running in CTA during CTA "
        "reallocation! (1)\n");
    printf("   CTA uid = %Lu : %lu running out of %zu total\n",
           m_uid,
           (m_threads_in_cta.size() - m_threads_that_have_exited.size()),
           m_threads_in_cta.size());
    printf("   These are the threads that are still running:\n");
    std::set<ThreadItem *>::iterator t_iter;
    for (t_iter = m_threads_in_cta.begin(); t_iter != m_threads_in_cta.end();
         ++t_iter) {
      ThreadItem *t = *t_iter;
      if (m_threads_that_have_exited.find(t) ==
          m_threads_that_have_exited.end()) {
        if (m_dangling_pointers.find(t) != m_dangling_pointers.end()) {
          printf("       <thread deleted>\n");
        } else {
          printf("       [done=%c] : ", (t->is_done() ? 'Y' : 'N'));
          t->print_insn(t->get_pc(), stdout);
          printf("\n");
        }
      }
    }
    printf("\n\n");
    fail = true;
  }
  if (fail) {
    abort();
  }

  bool fail2 = false;
  std::set<ThreadItem *>::iterator t_iter;
  for (t_iter = m_threads_in_cta.begin(); t_iter != m_threads_in_cta.end();
       ++t_iter) {
    ThreadItem *t = *t_iter;
    if (m_dangling_pointers.find(t) == m_dangling_pointers.end()) {
      if (!t->is_done()) {
        if (!fail2) {
          printf(
              "Execution error: Some threads still running in CTA during CTA "
              "reallocation! (2)\n");
          printf("   CTA uid = %Lu :\n", m_uid );
          fail2 = true;
        }
        printf("       ");
        t->print_insn(t->get_pc(), stdout);
        printf("\n");
      }
    }
  }
  if (fail2) {
    abort();
  }
  m_threads_in_cta.clear();
  m_threads_that_have_exited.clear();
  m_dangling_pointers.clear();
}

void BlockState::register_thread_exit(ThreadItem *thd) {
  assert(m_threads_that_have_exited.find(thd) ==
         m_threads_that_have_exited.end());
  m_threads_that_have_exited.insert(thd);
}

void BlockState::register_deleted_thread(ThreadItem *thd) {
  m_dangling_pointers.insert(thd);
}


unsigned BlockState::get_bar_threads() const { return m_bar_threads; }

void BlockState::inc_bar_threads() { m_bar_threads++; }

void BlockState::reset_bar_threads() { m_bar_threads = 0; }

unsigned BlockState::get_reduction_value(unsigned barid) {
    return m_bar_slot[barid];
}

void BlockState::and_reduction(unsigned barid, bool value) {
    m_bar_slot[barid] &= value;
}

void BlockState::or_reduction(unsigned barid, bool value) {
    m_bar_slot[barid] |= value;
}

void BlockState::popc_reduction(unsigned barid, bool value) {
    m_bar_slot[barid] += value;
}

void BlockState::initBar(uint32_t bar_slot_count) {
    for (int i = 0; i < bar_slot_count; i++) {
        m_bar_slot[i] = 0;
    }
}


