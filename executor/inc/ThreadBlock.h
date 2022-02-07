#pragma once
#include "inc/IsaSim.h"
#include "inc/Instruction.h"
#include <set>
#include <list>
#include <memory>
// #include "../../libcuda/abstract_hardware_model.h"

class ThreadItem;
class WarpInst;

namespace libcuda {
class kernel_info_t;
class gpgpu_t;
class gpgpu_context;
}


class cta_info_t {
public:
  cta_info_t(unsigned sm_idx, libcuda::gpgpu_context *ctx);
  void add_thread(ThreadItem *thd);
  unsigned num_threads() const;
  void check_cta_thread_status_and_reset();
  void register_thread_exit(ThreadItem *thd);
  void register_deleted_thread(ThreadItem *thd);
  unsigned get_sm_idx() const;
  unsigned get_bar_threads() const;
  void inc_bar_threads();
  void reset_bar_threads();

 private:
  // backward pointer
  libcuda::gpgpu_context *gpgpu_ctx;
  unsigned m_bar_threads;
  unsigned long long m_uid;
  unsigned m_sm_idx;
  std::set<ThreadItem *> m_threads_in_cta;
  std::set<ThreadItem *> m_threads_that_have_exited;
  std::set<ThreadItem *> m_dangling_pointers;
};

/*!
 * This class functionally executes a kernel. It uses the basic data structures and procedures in core_t
 */
class ThreadBlock {
public:
  ThreadBlock(libcuda::gpgpu_t *gpu, KernelInfo *kernel, libcuda::gpgpu_context *ctx, unsigned warp_size, unsigned threads_per_shader);

  virtual ~ThreadBlock(){
    warp_exit(0);
    delete[] m_liveThreadCount;
    delete[] m_warpAtBarrier;
    free(m_thread);
  }

  //! executes all warps till completion
  void execute(int inst_count, unsigned ctaid_cp);
  void warp_exit( unsigned warp_id );
  bool warp_waiting_at_barrier( unsigned warp_id ) const;
  void checkExecutionStatusAndUpdate(shared_ptr<Instruction> &inst, unsigned t, unsigned tid);

  void executeInstruction(shared_ptr<Instruction> inst, unsigned warpId);

public:
  void executeWarp(unsigned, bool &, bool &);
  //initializes threads in the CTA block which we are executing
  void initializeCTA(unsigned ctaid_cp);

  unsigned createThread(KernelInfo &kernel,
                             ThreadItem** thread_item, int sid,
                             unsigned tid, unsigned threads_left,
                             unsigned num_threads, ThreadBlock *tb,
                             unsigned hw_cta_id, unsigned hw_warp_id,
                             libcuda::gpgpu_t *gpu, bool isInFunctionalSimulationMode);

  // lunches the stack and set the threads count
  void  createWarp(unsigned warpId);

  //each warp live thread count and barrier indicator
  unsigned * m_liveThreadCount;
  bool* m_warpAtBarrier;

  ThreadItem** m_thread;

  bool is_thread_done(unsigned hw_thread_id) const;
  virtual void updateSIMTStack(unsigned warpId, std::shared_ptr<Instruction> inst);
  void initilizeSIMTStack(unsigned warp_count, unsigned warps_size);
  void deleteSIMTStack();
  std::shared_ptr<Instruction> getExecuteWarp(unsigned warpId);
  void get_pdom_stack_top_info(unsigned warpId, unsigned *pc,
                               unsigned *rpc) const;

  unsigned get_warp_size() const { return m_warp_size; }
  void and_reduction(unsigned ctaid, unsigned barid, bool value) {
    reduction_storage[ctaid][barid] &= value;
  }
  void or_reduction(unsigned ctaid, unsigned barid, bool value) {
    reduction_storage[ctaid][barid] |= value;
  }
  void popc_reduction(unsigned ctaid, unsigned barid, bool value) {
    reduction_storage[ctaid][barid] += value;
  }
  unsigned get_reduction_value(unsigned ctaid, unsigned barid) {
    return reduction_storage[ctaid][barid];
  }

  libcuda::gpgpu_t *m_gpu;
  KernelInfo *m_kernel;
  SimtStack **m_SimtStack;  // pdom based reconvergence context for each warp
  unsigned m_warp_size;
  unsigned m_warp_count;
  unsigned reduction_storage[MAX_CTA_PER_SHADER][MAX_BARRIERS_PER_CTA];
  int m_gpgpu_param_num_shaders = 1000; // FIXME

  libcuda::gpgpu_context *m_gpgpu_ctx;
};

