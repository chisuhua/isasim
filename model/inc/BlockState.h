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
  enum SyncStatus {
      SyncInitial = 0,
      SyncArrived = 1,
      SyncWorking = 2,
      SyncUnkolwn = 3,
  };
  static BlockState* GetBlockState(uint32_t bar_count) {
      CUResource* cu_res = CUResource::GetCUResource(0);
      if (cu_res->allocRunningBlock(bar_count)) {
          return new BlockState(cu_res, bar_count);
      }
      assert(false);
      return nullptr;
  }

  BlockState(CUResource *cu_res, uint32_t bar_count);
  virtual ~BlockState() {
      delete[] m_warp_at_bar;
  }
  void init(libcuda::gpgpu_context *ctx, dim3, uint32_t block_warp_count);
  void destroy();

  void addWarp() {};
  void removeWarp(uint32_t warp_id);
  bool arriveBar(uint32_t slot, uint32_t warp_id, uint32_t warp_count = 0);
  void notifyPhaseDone(uint32_t slot) {};
  bool waitBar(uint32_t slot, bool blocking = true);

  void blockWarp() {};
  // void unblockWarp();

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
  void removeWarpAtBar();

  bool* m_warp_at_bar;

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
  std::vector<uint32_t> m_bar_status;
  std::vector<uint32_t> m_bar_expected;
  std::vector<uint32_t> m_bar_counter;
  uint32_t m_bar_count;
  uint32_t m_block_warp_count;
};


