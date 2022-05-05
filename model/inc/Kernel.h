#pragma once

#include "../libcuda/abstract_hardware_model.h"
#include "inc/ExecContext.h"
#include "inc/KernelState.h"
#include <inttypes.h>

class ThreadItem;
class DispatchInfo;
struct KernelParam {
  const void *pdata;
  int type;
  size_t size;
  size_t offset;
};

namespace libcuda {
class memory_space_t;
class gpgpu_context;
class CUstream_st;
}



class ParamInfo {
 public:
  ParamInfo() {
    m_valid = false;
    m_value_set = false;
    m_size = 0;
    m_is_ptr = false;
  }
  /*
  ParamInfo(std::string name, int type, size_t size, bool is_ptr,
             memory_space_t ptr_space) {
    m_valid = true;
    m_value_set = false;
    m_name = name;
    m_type = type;
    m_size = size;
    m_is_ptr = is_ptr;
    m_ptr_space = ptr_space;
  }
  */
  void add_data(KernelParam v) {
    assert((!m_value_set) ||
           (m_value.size == v.size));  // if this fails concurrent kernel
                                       // launches might execute incorrectly
    m_value_set = true;
    m_value = v;
  }
  void add_offset(unsigned offset) { m_offset = offset; }
  unsigned get_offset() {
    assert(m_valid);
    return m_offset;
  }
  std::string get_name() const {
    assert(m_valid);
    return m_name;
  }
  int get_type() const {
    assert(m_valid);
    return m_type;
  }
  KernelParam get_value() const {
    assert(m_value_set);
    return m_value;
  }
  size_t get_size() const {
    assert(m_valid);
    return m_size;
  }
  bool is_ptr_shared() const ;

 private:
  bool m_valid;
  std::string m_name;
  int m_type;
  size_t m_size;
  bool m_value_set;
  KernelParam m_value;
  unsigned m_offset;
  bool m_is_ptr;
  libcuda::memory_space_t m_ptr_space;
};

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

  void increment_cta_id() {
    libcuda::increment_x_then_y_then_z(m_next_cta, m_grid_dim);
    m_next_tid.x = 0;
    m_next_tid.y = 0;
    m_next_tid.z = 0;
  }
  dim3 get_next_cta_id() const { return m_next_cta; }
  unsigned get_next_cta_id_single() const {
    return m_next_cta.x + m_grid_dim.x * m_next_cta.y +
           m_grid_dim.x * m_grid_dim.y * m_next_cta.z;
  }
  bool no_more_ctas_to_run() const {
    return (m_next_cta.x >= m_grid_dim.x || m_next_cta.y >= m_grid_dim.y ||
            m_next_cta.z >= m_grid_dim.z);
  }

  void increment_thread_id() {
    libcuda::increment_x_then_y_then_z(m_next_tid, m_block_dim);
  }
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
  std::string name() const;

  std::list<ThreadItem *> &thread_pool() {
    return m_thread_pool;
  }
  std::list<ThreadItem  *> m_thread_pool;

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

  //Jin: parent and child kernel management for CDP
  void set_parent(KernelInfo * parent, dim3 parent_ctaid, dim3 parent_tid);
  void set_child(KernelInfo * child);
  void remove_child(KernelInfo * child);
  bool is_finished();
  bool children_all_finished();
  void notify_parent_finished();
  libcuda::CUstream_st *create_stream_cta(dim3 ctaid);
  libcuda::CUstream_st *get_default_stream_cta(dim3 ctaid);
  bool cta_has_stream(dim3 ctaid, libcuda::CUstream_st* stream);
  void destroy_cta_streams();
  void print_parent_info();
  KernelInfo *get_parent() { return m_parent_kernel; }

  uint32_t kernel_ctrl() { return m_kernel_ctrl; }

  std::map<unsigned, ParamInfo> m_kernel_param_info;
  int m_args_aligned_size;

  libcuda::gpgpu_context *m_gpgpu_ctx;

private:
  KernelInfo * m_parent_kernel;
  dim3 m_parent_ctaid;
  dim3 m_parent_tid;
  std::list<KernelInfo *> m_child_kernels; //child kernel launched
  std::map< dim3, std::list<libcuda::CUstream_st *>, dim3comp >
      m_cta_streams; //streams created in each CTA

//Jin: kernel timing
public:
  unsigned long long launch_cycle;
  unsigned long long start_cycle;
  unsigned long long end_cycle;
  KernelState*  m_state;
  uint32_t m_kernel_ctrl;
};

