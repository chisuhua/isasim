#pragma once
#include "inc/IsaSim.h"
#include "inc/Instruction.h"
#include <set>
#include <list>
#include <memory>
#include "inc/CUResource.h"
// #include "../../libcuda/abstract_hardware_model.h"

class ThreadItem;

namespace libcuda {
class gpgpu_context;
}


class BlockState {
public:
  static BlockState* GetBlockState(uint32_t bar_count) {
      CUResource* cu_res = CUResource::GetCUResource(0);
      if (cu_res->allocRunningBlock(bar_count)) {
          return new BlockState(cu_res, bar_count);
      }
      assert(false);
      return nullptr;
  }

  BlockState(CUResource *cu_res, uint32_t bar_count);
  void init(libcuda::gpgpu_context *ctx, dim3);
  void destroy();
  void add_thread(ThreadItem *thd);
  unsigned num_threads() const;
  void check_cta_thread_status_and_reset();
  void register_thread_exit(ThreadItem *thd);
  void register_deleted_thread(ThreadItem *thd);
  unsigned get_bar_threads() const;
  void inc_bar_threads();
  void reset_bar_threads();
  dim3 get_cta_id() {return m_cta_id;}

  unsigned get_reduction_value(unsigned barid);
  void and_reduction(unsigned barid, bool value);
  void or_reduction(unsigned barid, bool value);
  void popc_reduction(unsigned barid, bool value);

  void initBar(uint32_t bar_count);

 private:
  // backward pointer
  libcuda::gpgpu_context *gpgpu_ctx;
  unsigned m_bar_threads;
  unsigned long long m_uid;
  dim3     m_cta_id;
  std::set<ThreadItem *> m_threads_in_cta;
  std::set<ThreadItem *> m_threads_that_have_exited;
  std::set<ThreadItem *> m_dangling_pointers;
  CUResource *m_resource;
  std::vector<uint32_t> m_bar_slot;
  uint32_t m_bar_count;
};


