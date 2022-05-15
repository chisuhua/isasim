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

// KernelInfo is used in control firmware, KernelState is accessed by kernel itself.

KernelInfo::KernelInfo(DispatchInfo *disp_info) {
  m_grid_dim = {disp_info->grid_dim_x, disp_info->grid_dim_y, disp_info->grid_dim_z};
  m_block_dim = {disp_info->block_dim_x, disp_info->block_dim_y, disp_info->block_dim_z};
  m_next_cta.x = 0;
  m_next_cta.y = 0;
  m_next_cta.z = 0;
  m_next_tid = m_next_cta;
  m_num_cores_running = 0;
  m_state = new KernelState(1024);  // const num is access throught icache
  m_state->setParamAddr(disp_info->kernel_param_addr);
  m_state->setParamSize(disp_info->kernel_param_size);
  m_state->setProgAddr(disp_info->kernel_prog_addr);
  m_state->setLocalAddr(disp_info->private_mem_addr);
  m_state->setLocalSize(disp_info->private_memsize);
  m_kernel_ctrl = disp_info->kernel_ctrl;
  m_bar_used = disp_info->bar_used;
  // m_local_mem_size = disp_info.local_mem_size;
  // m_uid = (entry->gpgpu_ctx->kernel_info_m_next_uid)++;

  // Jin: parent and child kernel management for CDP
  m_parent_kernel = NULL;
}

KernelInfo::~KernelInfo() {
  // assert(m_thread_pool.empty());
  for (auto i : m_thread_pool) {
      delete i;
  }
  destroy_cta_streams();
  delete m_state;
  // delete m_param_mem;
}

/*
std::string KernelInfo::name() const {
    return m_kernel_entry->get_name();
}
*/

//Jin: parent and child kernel management for CDP
void KernelInfo::set_parent(KernelInfo *parent, dim3 parent_ctaid,
                               dim3 parent_tid) {
  m_parent_kernel = parent;
  m_parent_ctaid = parent_ctaid;
  m_parent_tid = parent_tid;
  parent->set_child(this);
}

void KernelInfo::set_child(KernelInfo * child) {
  m_child_kernels.push_back(child);
}

void KernelInfo::remove_child(KernelInfo * child) {
  assert(std::find(m_child_kernels.begin(), m_child_kernels.end(), child) !=
         m_child_kernels.end());
  m_child_kernels.remove(child);
}

bool KernelInfo::is_finished() {
  if(done() && children_all_finished())
     return true;
  else
     return false;
}

bool KernelInfo::children_all_finished() {
  if (!m_child_kernels.empty()) return false;

  return true;
}

/*
unsigned KernelInfo::get_args_aligned_size() {
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

void KernelInfo::notify_parent_finished() {
  if (m_parent_kernel) {
    m_gpgpu_ctx->device_runtime->g_total_param_size -=
        ((m_state->getParamSize() + 255) / 256 * 256);
    m_parent_kernel->remove_child(this);
    /* FIXME
    m_gpgpu_ctx->the_gpgpusim->g_stream_manager
        ->register_finished_kernel(m_parent_kernel->get_uid());
        */
  }
}

libcuda::CUstream_st *KernelInfo::create_stream_cta(dim3 ctaid) {
  assert(get_default_stream_cta(ctaid));
  libcuda::CUstream_st *stream = new libcuda::CUstream_st();
  m_gpgpu_ctx->the_gpgpusim->g_stream_manager->add_stream(stream);
  assert(m_cta_streams.find(ctaid) != m_cta_streams.end());
  assert(m_cta_streams[ctaid].size() >= 1);  // must have default stream
  m_cta_streams[ctaid].push_back(stream);

  return stream;
}

libcuda::CUstream_st *KernelInfo::get_default_stream_cta(dim3 ctaid) {
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

bool KernelInfo::cta_has_stream(dim3 ctaid, libcuda::CUstream_st *stream) {
  if (m_cta_streams.find(ctaid) == m_cta_streams.end()) return false;

  std::list<libcuda::CUstream_st *> &stream_list = m_cta_streams[ctaid];
  if (std::find(stream_list.begin(), stream_list.end(), stream) ==
      stream_list.end())
    return false;
  else
    return true;
}

void KernelInfo::print_parent_info() {
    /* FIXME for missing name function
  if (m_parent_kernel) {
    printf("Parent %d: \'%s\', Block (%d, %d, %d), Thread (%d, %d, %d)\n",
           m_parent_kernel->get_uid(), m_parent_kernel->name().c_str(),
           m_parent_ctaid.x, m_parent_ctaid.y, m_parent_ctaid.z, m_parent_tid.x,
           m_parent_tid.y, m_parent_tid.z);
  }*/
}

void KernelInfo::destroy_cta_streams() {
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

