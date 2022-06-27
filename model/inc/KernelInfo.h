#pragma once
#include "inc/ExecTypes.h"
#include <list>
#include <cassert>

class KernelState;
class DispatchInfo;

#ifdef TMPHACK
struct int3 {
	int x, y, z;
};

struct uint3 {
	unsigned int x, y, z;
};

struct int4 {
	int x, y, z, w;
};

struct uint4 {
	unsigned int x, y, z, w;
};

struct dim3
{
    unsigned int x, y, z;
    constexpr dim3(unsigned int vx = 1, unsigned int vy = 1, unsigned int vz = 1) : x(vx), y(vy), z(vz) {}
    constexpr dim3(uint3 v) : x(v.x), y(v.y), z(v.z) {}
    constexpr operator uint3(void) const { return uint3{x, y, z}; }
};
#endif


class KernelInfo {
public:
  KernelInfo(DispatchInfo *disp_info);
  ~KernelInfo();
  bool m_valid;

  void inc_running() { m_num_cores_running++; }
  void dec_running() {
    assert(m_num_cores_running > 0);
    m_num_cores_running--;
  }
  bool running() const { return m_num_cores_running > 0; }
  bool done() const { return no_more_ctas_to_run() && !running(); }

  // addr_t kernel_addr() {
  //  return m_prog_addr;
  //}

  size_t num_blocks() const {
    return m_grid_dim.x * m_grid_dim.y * m_grid_dim.z;
  }

  size_t threads_per_cta() const {
    return m_block_dim.x * m_block_dim.y * m_block_dim.z;
  }

  dim3 get_grid_dim() const { return m_grid_dim; }
  dim3 get_cta_dim() const { return m_block_dim; }

  void increment_cta_id();

  dim3 get_next_cta_id() const { return m_next_cta; }
  unsigned get_next_cta_id_single() const {
    return m_next_cta.x + m_grid_dim.x * m_next_cta.y +
           m_grid_dim.x * m_grid_dim.y * m_next_cta.z;
  }
  bool no_more_ctas_to_run() const {
    return (m_next_cta.x >= m_grid_dim.x || m_next_cta.y >= m_grid_dim.y ||
            m_next_cta.z >= m_grid_dim.z);
  }

  void increment_thread_id() ;

  dim3 get_next_thread_id_3d() const { return m_next_tid; }
  unsigned get_next_thread_id() const {
    return m_next_tid.x + m_block_dim.x * m_next_tid.y +
           m_block_dim.x * m_block_dim.y * m_next_tid.z;
  }
  bool more_threads_in_cta() const {
    return m_next_tid.z < m_block_dim.z && m_next_tid.y < m_block_dim.y &&
           m_next_tid.x < m_block_dim.x;
  }
  unsigned get_uid() const { return m_uid; }
private:
  KernelInfo(const KernelInfo & ); // disable copy constructor
  void operator=(const KernelInfo & ); // disable copy operator

  unsigned m_uid;
  static unsigned m_next_uid;

  dim3 m_grid_dim;
  dim3 m_block_dim;
  dim3 m_next_cta;
  dim3 m_next_tid;

  unsigned m_num_cores_running;

public:
  KernelState* state() {return m_state; };

  // unsigned get_args_aligned_size();
  // TODO schi
  addr_t m_inst_text_base_vaddr;
  addr_t get_inst_base_vaddr() { return m_inst_text_base_vaddr; };
  void set_inst_base_vaddr(addr_t addr) { m_inst_text_base_vaddr = addr; };

  //Jin: parent and child KernelInfo management for CDP
  void set_parent(KernelInfo * parent, dim3 parent_ctaid, dim3 parent_tid);
  void set_child(KernelInfo * child);
  void remove_child(KernelInfo * child);
  bool is_finished();
  bool children_all_finished();
  void notify_parent_finished();

  void print_parent_info();
  KernelInfo *get_parent() { return m_parent_kernel; }

  uint32_t kernel_ctrl() { return m_kernel_ctrl; }
  uint32_t bar_used() { return m_bar_used; }

  std::string name() {};

private:
  KernelInfo * m_parent_kernel;
  dim3 m_parent_ctaid;
  dim3 m_parent_tid;
  std::list<KernelInfo *> m_child_kernels; //child KernelInfo launched

//Jin: kernel timing
public:
  unsigned long long launch_cycle;
  unsigned long long start_cycle;
  unsigned long long end_cycle;
  KernelState*  m_state;
  uint32_t m_kernel_ctrl;
  uint32_t m_bar_used;
  uint32_t m_kernel_TB_latency;
};
