#pragma once

#include "../libcuda/abstract_hardware_model.h"
#include "inc/ExecContext.h"
#include <inttypes.h>

class ThreadItem;
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

// we expect block block level in compute unit
// for grid level, it have gridStartX/Y/Z is used by CP
#define SHADER_ABI_USER_DATA_REGISTER_NUM_MAX 32

union SHADER_ABI_KERNEL_CONTROL {
    struct {
        uint16_t    grid_dim_x_en       : 1;
        uint16_t    grid_dim_y_en       : 1;
        uint16_t    grid_dim_z_en       : 1;
        uint16_t    block_dim_x_en        : 1;
        uint16_t    block_dim_y_en        : 1;
        uint16_t    block_dim_z_en        : 1;
        uint16_t    block_idx_x_en      : 1;
        uint16_t    block_idx_y_en      : 1;
        uint16_t    block_idx_z_en      : 1;
        uint16_t    start_thread_idx_en   : 1;
        uint16_t    user_sreg_num       : 6;
    } bits;
    uint16_t    val;
};

union SHADER_ABI_KERNEL_MODE {
    struct {
        uint32_t    fp_rndmode          : 3;
        uint32_t    i_rndmode           : 2;
        uint32_t    fp_denorm_flush     : 2;
        uint32_t    saturation          : 3;
        uint32_t    exception_en        : 8;
        uint32_t    relu                : 1;
        uint32_t    nan                 : 1;
        uint32_t    vmem_ooo            : 1;
        uint32_t    saturation_fp64     : 1;
        uint32_t    rsvd_23_22          : 1;
        uint32_t    trap_exception      : 1;
        uint32_t    debug_en            : 1;
        uint32_t    trap_en             : 1;
        uint32_t    rsvd_32_27          : 5;
    } bits;
    uint32_t    val;
};

union SHADER_ABI_KERNEL_RESOURCE {
    struct {
        uint32_t    vreg_number         : 9;
        uint32_t    sreg_number         : 9;
        uint32_t    shared_memory_size  : 12;
        uint32_t    treg_en             : 1;
        uint32_t    rsvd_31             : 1;
    } bits;
    uint32_t        val;
};

union SHADER_ABI_THREADBLOCK_DIM {
    struct {
        uint32_t    x   : 12;
        uint32_t    y   : 12;
        uint32_t    z   : 8;
    } bits;
    uint32_t    val;
};

struct DispatchInfo {
    uint64_t    kernel_prog_addr;
    uint64_t    kernel_param_addr;
    uint64_t    kernel_name_addr;
    uint64_t    start_pC;
    uint32_t    grid_dim_x;
    uint32_t    grid_dim_y;
    uint32_t    grid_dim_z;
    uint32_t    block_idx;
    uint32_t    block_idy;
    uint32_t    block_idz;
    uint16_t    block_dim_x;
    uint16_t    block_dim_y;
    uint16_t    block_dim_z;
    SHADER_ABI_KERNEL_CONTROL kernel_ctrl;
    SHADER_ABI_KERNEL_MODE    kernel_mode;
    SHADER_ABI_KERNEL_RESOURCE  kernel_resource;
    // SHADER_ABI_THREADBLOCK_DIM block_dim;
    uint32_t    userSreg[SHADER_ABI_USER_DATA_REGISTER_NUM_MAX];
    int lmem;
    int smem;
    int cmem;
    int gmem;
    int regs;
};

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
  KernelInfo(DispatchInfo &disp_info);
  ~KernelInfo();
  bool m_valid;

  void inc_running() { m_num_cores_running++; }
  void dec_running() {
    assert(m_num_cores_running > 0);
    m_num_cores_running--;
  }
  bool running() const { return m_num_cores_running > 0; }
  bool done() const { return no_more_ctas_to_run() && !running(); }

  addr_t kernel_addr() {
    return m_prog_addr;
  }

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

  std::list<ThreadItem *> &active_threads() {
    return m_active_threads;
  }
  std::list<ThreadItem  *> m_active_threads;

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

  uint64_t m_prog_addr;
  uint64_t m_param_addr;

public:
  unsigned get_args_aligned_size();
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
};

