#include "inc/Kernel.h"
#include "KernelDispInfo.h"
#include "../libcuda/abstract_hardware_model.h"
#include "../libcuda/gpgpu_context.h"
#include "../libcuda/stream_manager.h"

bool ParamInfo::is_ptr_shared() const {
    assert(m_valid);
    return (m_is_ptr and m_ptr_space == libcuda::shared_space);
}

#if 0
struct dim3comp {
  bool operator()(const dim3 &a, const dim3 &b) const {
    if (a.z < b.z)
      return true;
    else if (a.y < b.y)
      return true;
    else if (a.x < b.x)
      return true;
    else
      return false;
  }
};

void increment_x_then_y_then_z( dim3 &i, const dim3 &bound)
{
   i.x++;
   if ( i.x >= bound.x ) {
      i.x = 0;
      i.y++;
      if ( i.y >= bound.y ) {
         i.y = 0;
         if( i.z < bound.z )
            i.z++;
      }
   }
}
#endif

// Kernel is used in control firmware, KernelState is accessed by kernel itself.


Kernel::~Kernel() {
  // assert(m_thread_pool.empty());
  for (auto i : m_thread_pool) {
      delete i;
  }
  destroy_cta_streams();
  // delete m_param_mem;
}

/*
std::string Kernel::name() const {
    return m_kernel_entry->get_name();
}
*/

/*
unsigned Kernel::get_args_aligned_size() {
  if (m_args_aligned_size >= 0) return m_args_aligned_size;

  unsigned param_address = 0;
  unsigned int total_size = 0;
  for (std::map<unsigned, ParamInfo>::iterator i =
           m_kernel_param_info.begin();
       i != m_kernel_param_info.end(); i++) {
    ParamInfo &p = i->second;
    // std::string name = p.get_name();
    // symbol *param = m_symtab->lookup(name.c_str());

    size_t arg_size = p.get_size() / 8;  // size of param in bytes
    total_size = (total_size + arg_size - 1) / arg_size * arg_size;  // aligned
    p.add_offset(total_size);
    // param->set_address(param_address + total_size);
    total_size += arg_size;
  }

  m_args_aligned_size = (total_size + 3) / 4 * 4;  // final size aligned to word

  return m_args_aligned_size;
}
*/


libcuda::CUstream_st *Kernel::create_stream_cta(dim3 ctaid) {
  assert(get_default_stream_cta(ctaid));
  libcuda::CUstream_st *stream = new libcuda::CUstream_st();
  m_gpgpu_ctx->the_gpgpusim->g_stream_manager->add_stream(stream);
  assert(m_cta_streams.find(ctaid) != m_cta_streams.end());
  assert(m_cta_streams[ctaid].size() >= 1);  // must have default stream
  m_cta_streams[ctaid].push_back(stream);

  return stream;
}

libcuda::CUstream_st *Kernel::get_default_stream_cta(dim3 ctaid) {
  if (m_cta_streams.find(ctaid) != m_cta_streams.end()) {
    assert(m_cta_streams[ctaid].size() >=
           1);  // already created, must have default stream
    return *(m_cta_streams[ctaid].begin());
  } else {
    m_cta_streams[ctaid] = std::list<libcuda::CUstream_st *>();
    libcuda::CUstream_st *stream = new libcuda::CUstream_st();
    m_gpgpu_ctx->the_gpgpusim->g_stream_manager->add_stream(
        stream);
    m_cta_streams[ctaid].push_back(stream);
    return stream;
  }
}

bool Kernel::cta_has_stream(dim3 ctaid, libcuda::CUstream_st *stream) {
  if (m_cta_streams.find(ctaid) == m_cta_streams.end()) return false;

  std::list<libcuda::CUstream_st *> &stream_list = m_cta_streams[ctaid];
  if (std::find(stream_list.begin(), stream_list.end(), stream) ==
      stream_list.end())
    return false;
  else
    return true;
}

void Kernel::destroy_cta_streams() {
  printf("Destroy streams for kernel %d: ", get_uid());
  size_t stream_size = 0;
  for (auto s = m_cta_streams.begin(); s != m_cta_streams.end(); s++) {
    stream_size += s->second.size();
    for (auto ss = s->second.begin(); ss != s->second.end(); ss++)
      m_gpgpu_ctx->the_gpgpusim->g_stream_manager->destroy_stream(
          *ss);
    s->second.clear();
  }
  printf("size %lu\n", stream_size);
  m_cta_streams.clear();
}

