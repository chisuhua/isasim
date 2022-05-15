#pragma once
#include "inc/IsaSim.h"
#include "inc/Instruction.h"
#include <set>
#include <list>
#include <memory>
// #include "../../libcuda/abstract_hardware_model.h"

class ThreadItem;
class Warp;
class BlockState;

namespace libcuda {
class gpgpu_t;
class gpgpu_context;
}


/*!
 * This class functionally executes a kernel. It uses the basic data structures and procedures in core_t
 */
class ThreadBlock {
public:
  ThreadBlock(libcuda::gpgpu_t *gpu, KernelInfo *kernel, libcuda::gpgpu_context *ctx,
          BlockState *block_state,
          unsigned warp_size, unsigned threads_per_shader, dim3 cta_id, uint32_t kernel_const_reg_num);

  virtual ~ThreadBlock(){
    // warp_exit(0);
    delete[] m_warp_live_thread;
    // delete[] m_warp_at_bar;
    delete m_block_state;
    delete m_hwop;
    // delete m_kernel;
    // free(m_thread);
  }

  //! executes all warps till completion
  bool execute(uint32_t ctaid_cp);
  void warp_exit( unsigned warp_id );
  bool warp_is_blocking( unsigned warp_id ) const;
  bool warp_waiting_at_barrier( unsigned warp_id ) const;
  void executeInstruction(std::shared_ptr<Instruction> inst, unsigned warpId);
  void checkExecutionStatusAndUpdate(std::shared_ptr<Instruction> &inst, unsigned t, unsigned tid);

public:
  void executeWarp(unsigned);
  //initializes threads in the CTA block which we are executing
  void initializeCTA(unsigned ctaid_cp);

  unsigned createThread(KernelInfo &kernel,
                             ThreadItem** thread_item, int sid,
                             unsigned tid, unsigned threads_left,
                             unsigned num_threads, ThreadBlock *tb,
                             unsigned hw_cta_id, unsigned hw_warp_id,
                             libcuda::gpgpu_t *gpu, bool isInFunctionalSimulationMode);

  // lunches the stack and set the threads count
  void  createWarp(unsigned warpId, bool dump_enable);

  //each warp live thread count and barrier indicator
  unsigned * m_warp_live_thread;

  ThreadItem** m_thread;

  bool is_thread_done(unsigned hw_thread_id) const;
  virtual void updateSIMTStack(unsigned warpId, std::shared_ptr<Instruction> inst);
  void initilizeSIMTStack(unsigned warp_count, unsigned warps_size);
  void deleteSIMTStack();
  std::shared_ptr<Instruction> getExecuteWarp(unsigned warpId);
  void get_pdom_stack_top_info(unsigned warpId, unsigned *pc,
                               unsigned *rpc) const;

  unsigned get_warp_size() const { return m_warp_size; }
#if 0
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
#endif
  std::shared_ptr<Instruction> getInstruction(address_type pc);
  std::map<address_type, std::shared_ptr<Instruction>> m_insts;

  libcuda::gpgpu_t *m_gpu;
  KernelInfo *m_kernel;
  Warp **m_Warp;  // pdom based reconvergence context for each warp
  HwOp *m_hwop;
  unsigned m_warp_size;
  unsigned m_warp_count;
  // unsigned reduction_storage[MAX_CTA_PER_SHADER][MAX_BARRIERS_PER_CTA];
  int m_gpgpu_param_num_shaders = 1000; // FIXME

  BlockState *m_block_state;
  libcuda::gpgpu_context *m_gpgpu_ctx;
  uint32_t m_kernel_const_num;

  address_type m_kernel_addr;
  address_type m_kernel_args;

  libcuda::memory_space *m_shared_mem;
  libcuda::memory_space *m_global_mem;
};

