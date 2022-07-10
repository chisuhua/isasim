#include <stdint.h>
#include <cstddef>
#include <algorithm>
#include "KernelDispInfo.h"

#define TMPHACK
#include "inc/KernelInfo.h"
#include "inc/KernelState.h"

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


KernelInfo::KernelInfo(DispatchInfo *disp_info) :
    KernelInfoBase()
{
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
  m_state->setSharedSize(disp_info->shared_memsize);
  m_state->setVRegUsed(disp_info->vregs);
  m_state->setSRegUsed(disp_info->sregs);
  m_kernel_ctrl = disp_info->kernel_ctrl;
  m_bar_used = disp_info->bar_used;
  set_inst_base_vaddr(disp_info->kernel_prog_addr);
  // m_local_mem_size = disp_info.local_mem_size;
  // m_uid = (entry->gpgpu_ctx->kernel_info_m_next_uid)++;

  // Jin: parent and child kernel management for CDP
  m_parent_kernel = NULL;
  m_kernel_TB_latency = 10;
}

KernelInfo::~KernelInfo() {
  delete m_state;
}

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

void KernelInfo::notify_parent_finished() {
  if (m_parent_kernel) {
    /* FIXME
    m_gpgpu_ctx->device_runtime->g_total_param_size -=
        ((m_state->getParamSize() + 255) / 256 * 256);
    m_parent_kernel->remove_child(this);
    m_gpgpu_ctx->the_gpgpusim->g_stream_manager
        ->register_finished_kernel(m_parent_kernel->get_uid());
        */
  }
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

void KernelInfo::increment_cta_id() {
    increment_x_then_y_then_z(m_next_cta, m_grid_dim);
    m_next_tid.x = 0;
    m_next_tid.y = 0;
    m_next_tid.z = 0;
}

void KernelInfo::increment_thread_id() {
    increment_x_then_y_then_z(m_next_tid, m_block_dim);
}

uint32_t KernelInfo::get_shared_memsize() const { return m_state->getSharedSize(); }
uint32_t KernelInfo::get_prog_addr() { return m_state->getProgAddr(); }
uint32_t KernelInfo::get_vreg_used() const { return m_state->getVRegUsed(); }
uint32_t KernelInfo::get_sreg_used() { return m_state->getSRegUsed(); }


