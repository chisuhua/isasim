#pragma once

#include "../libcuda/abstract_hardware_model.h"
#include "inc/ExecContext.h"
#include "inc/KernelState.h"
#include "inc/KernelInfo.h"
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

class Kernel : public KernelInfo {
public:
  Kernel(DispatchInfo *disp_info) : KernelInfo(disp_info) {};
  ~Kernel();


  std::string name() const;

  std::list<ThreadItem *> &thread_pool() {
    return m_thread_pool;
  }
  std::list<ThreadItem  *> m_thread_pool;

private:
  libcuda::CUstream_st *create_stream_cta(dim3 ctaid);
  libcuda::CUstream_st *get_default_stream_cta(dim3 ctaid);
  bool cta_has_stream(dim3 ctaid, libcuda::CUstream_st* stream);
  void destroy_cta_streams();

  std::map<unsigned, ParamInfo> m_kernel_param_info;
  int m_args_aligned_size;

  libcuda::gpgpu_context *m_gpgpu_ctx;

private:
  std::map< dim3, std::list<libcuda::CUstream_st *>, dim3comp >
      m_cta_streams; //streams created in each CTA

//Jin: kernel timing
public:
  unsigned long long launch_cycle;
  unsigned long long start_cycle;
  unsigned long long end_cycle;
  KernelState*  m_state;
  uint32_t m_kernel_ctrl;
  uint32_t m_bar_used;
};

