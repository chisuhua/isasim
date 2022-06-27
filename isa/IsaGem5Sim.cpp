#include "inc/IsaGem5Sim.h"
#include "inc/ThreadBlock.h"
#include "KernelDispInfo.h"
#include <algorithm>
#include "../../libcuda/gpgpu_context.h"

extern "C" IsaGem5Sim* make_isagem5sim(gpgpu_t* gpu, gpgpu_context *ctx) {
  IsaGem5Sim* sim = new IsaGem5Sim(gpu, ctx);
  return sim;
}

#if 0
unsigned gpgpu_context::translate_pc_to_ptxlineno(unsigned pc) {
  // this function assumes that the kernel fits inside a single PTX file
  // function_info *pFunc = g_func_info; // assume that the current kernel is
  // the one in query
  const ptx_instruction *pInsn = pc_to_instruction(pc);
  unsigned ptx_line_number = pInsn->source_line();

  return ptx_line_number;
}
#endif

unsigned max_cta(const struct DispatchInfo *disp_info,
                 unsigned threads_per_cta, unsigned int warp_size,
                 unsigned int n_thread_per_shader,
                 unsigned int gpgpu_shmem_size,
                 unsigned int gpgpu_shader_registers,
                 unsigned int max_cta_per_core) {
  unsigned int padded_cta_size = threads_per_cta;
  if (padded_cta_size % warp_size)
    padded_cta_size = ((padded_cta_size / warp_size) + 1) * (warp_size);
  unsigned int result_thread = n_thread_per_shader / padded_cta_size;

  unsigned int result_shmem = (unsigned)-1;
  if (disp_info->smem > 0)
    result_shmem = gpgpu_shmem_size / disp_info->smem;
  unsigned int result_regs = (unsigned)-1;
  if (disp_info->regs > 0)
    result_regs = gpgpu_shader_registers /
                  (padded_cta_size * ((disp_info->regs + 3) & ~3));
  printf("padded cta size is %d and %d and %d", padded_cta_size,
         disp_info->regs, ((disp_info->regs + 3) & ~3));
  // Limit by CTA
  unsigned int result_cta = max_cta_per_core;

  unsigned result = result_thread;
  result = std::min(result, result_shmem);
  result = std::min(result, result_regs);
  result = std::min(result, result_cta);

  printf("GPGPU-Sim uArch: CTA/core = %u, limited by:", result);
  if (result == result_thread) printf(" threads");
  if (result == result_shmem) printf(" shmem");
  if (result == result_regs) printf(" regs");
  if (result == result_cta) printf(" cta_limit");
  printf("\n");

  return result;
}

void IsaGem5Sim::make_threaditem(dim3 nctaid, dim3 

/*!
This function simulates the CUDA code functionally, it takes a disp_info_t
parameter which holds the data for the CUDA kernel to be executed
!*/
void IsaGem5Sim::launch(DispatchInfo &disp_info, unsigned kernel_uid, bool openCL) {
  Kernel *kernel = new Kernel(disp_info);
  checkpoint *g_checkpoint;
  g_checkpoint = new checkpoint();
/*
  printf(
      "GPGPU-Sim: Performing Functional Simulation, executing kernel %s...\n",
      kernel.name().c_str());
*/
  unsigned max_cta_tot = max_cta(
      &disp_info, kernel->threads_per_cta(),
      m_ctx->the_gpgpusim->g_the_gpu->getShaderCoreConfig()->warp_size,
      m_ctx->the_gpgpusim->g_the_gpu->getShaderCoreConfig()->n_thread_per_shader,
      m_ctx->the_gpgpusim->g_the_gpu->getShaderCoreConfig()
          ->gpgpu_shmem_size,
      m_ctx->the_gpgpusim->g_the_gpu->getShaderCoreConfig()
          ->gpgpu_shader_registers,
      m_ctx->the_gpgpusim->g_the_gpu->getShaderCoreConfig()
          ->max_cta_per_core);
  printf("Max CTA : %d\n", max_cta_tot);

  int cp_op = m_ctx->the_gpgpusim->g_the_gpu->checkpoint_option;
  int cp_kernel = m_ctx->the_gpgpusim->g_the_gpu->checkpoint_kernel;
  cp_count = m_ctx->the_gpgpusim->g_the_gpu->checkpoint_insn_Y;
  cp_cta_resume = m_ctx->the_gpgpusim->g_the_gpu->checkpoint_CTA_t;
  int cta_launched = 0;

  // we excute the kernel one CTA (Block) at the time, as synchronization
  // functions work block wise
  while (!kernel->no_more_ctas_to_run()) {
    unsigned temp = kernel->get_next_cta_id_single();

    if (cp_op == 0 ||
        (cp_op == 1 && cta_launched < cp_cta_resume &&
         kernel->get_uid() == cp_kernel) ||
        kernel->get_uid() < cp_kernel)  // just fro testing
    {
      ThreadBlock cta(
        m_gpu, kernel, m_ctx,
        m_ctx->the_gpgpusim->g_the_gpu->getShaderCoreConfig()->warp_size,
        kernel->threads_per_cta());

      cta.execute(cp_count, temp);

// #if (CUDART_VERSION >= 5000)
//      m_ctx->device_runtime->launch_all_device_kernels();
// #endif
    } else {
      kernel->increment_cta_id();
    }
    cta_launched++;
  }

  if (cp_op == 1) {
    char f1name[2048];
    snprintf(f1name, 2048, "checkpoint_files/global_mem_%d.txt",
             kernel->get_uid());
    g_checkpoint->store_global_mem(
        m_ctx->the_gpgpusim->g_the_gpu->get_global_memory(), f1name,
        (char *)"%08x");
  }

  // registering this kernel as done

  // openCL kernel simulation calls don't register the kernel so we don't
  // register its exit
  if (!openCL) {
    // extern stream_manager *g_stream_manager;
    m_ctx->the_gpgpusim->g_stream_manager->register_finished_kernel(
        kernel_uid, false /* no check no_more_ctas_to_run at libcuda, since we don't pass kernel_info_t to isasim*/);
  }

}

