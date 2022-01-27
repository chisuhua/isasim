#include "inc/Compute.h"
#include "inc/ThreadItem.h"
#include "inc/Warp.h"


unsigned gpgpu_context::translate_pc_to_ptxlineno(unsigned pc) {
  // this function assumes that the kernel fits inside a single PTX file
  // function_info *pFunc = g_func_info; // assume that the current kernel is
  // the one in query
  const ptx_instruction *pInsn = pc_to_instruction(pc);
  unsigned ptx_line_number = pInsn->source_line();

  return ptx_line_number;
}

/*!
This function simulates the CUDA code functionally, it takes a kernel_info_t
parameter which holds the data for the CUDA kernel to be executed
!*/
void IsaSim::main_func(kernel_info_t &kernel,
                                            bool openCL) {
  printf(
      "GPGPU-Sim: Performing Functional Simulation, executing kernel %s...\n",
      kernel.name().c_str());

  // using a shader core object for book keeping, it is not needed but as most
  // function built for performance simulation need it we use it here
  // extern gpgpu_sim *g_the_gpu;
  // before we execute, we should do PDOM analysis for functional simulation
  // scenario.
  function_info *kernel_func_info = kernel.entry();
  const struct gpgpu_ptx_sim_info *kernel_info =
      ptx_sim_kernel_info(kernel_func_info);
  checkpoint *g_checkpoint;
  g_checkpoint = new checkpoint();

  if (kernel_func_info->is_pdom_set()) {
    printf("GPGPU-Sim PTX: PDOM analysis already done for %s \n",
           kernel.name().c_str());
  } else {
    printf("GPGPU-Sim PTX: finding reconvergence points for \'%s\'...\n",
           kernel.name().c_str());
    kernel_func_info->do_pdom();
    kernel_func_info->set_pdom();
  }

  unsigned max_cta_tot = max_cta(
      kernel_info, kernel.threads_per_cta(),
      gpgpu_ctx->the_gpgpusim->g_the_gpu->getShaderCoreConfig()->warp_size,
      gpgpu_ctx->the_gpgpusim->g_the_gpu->getShaderCoreConfig()
          ->n_thread_per_shader,
      gpgpu_ctx->the_gpgpusim->g_the_gpu->getShaderCoreConfig()
          ->gpgpu_shmem_size,
      gpgpu_ctx->the_gpgpusim->g_the_gpu->getShaderCoreConfig()
          ->gpgpu_shader_registers,
      gpgpu_ctx->the_gpgpusim->g_the_gpu->getShaderCoreConfig()
          ->max_cta_per_core);
  printf("Max CTA : %d\n", max_cta_tot);

  int cp_op = gpgpu_ctx->the_gpgpusim->g_the_gpu->checkpoint_option;
  int cp_kernel = gpgpu_ctx->the_gpgpusim->g_the_gpu->checkpoint_kernel;
  cp_count = gpgpu_ctx->the_gpgpusim->g_the_gpu->checkpoint_insn_Y;
  cp_cta_resume = gpgpu_ctx->the_gpgpusim->g_the_gpu->checkpoint_CTA_t;
  int cta_launched = 0;

  // we excute the kernel one CTA (Block) at the time, as synchronization
  // functions work block wise
  while (!kernel.no_more_ctas_to_run()) {
    unsigned temp = kernel.get_next_cta_id_single();

    if (cp_op == 0 ||
        (cp_op == 1 && cta_launched < cp_cta_resume &&
         kernel.get_uid() == cp_kernel) ||
        kernel.get_uid() < cp_kernel)  // just fro testing
    {
      functionalCoreSim cta(
          &kernel, gpgpu_ctx->the_gpgpusim->g_the_gpu,
          gpgpu_ctx->the_gpgpusim->g_the_gpu->getShaderCoreConfig()->warp_size);
      cta.execute(cp_count, temp);

#if (CUDART_VERSION >= 5000)
      gpgpu_ctx->device_runtime->launch_all_device_kernels();
#endif
    } else {
      kernel.increment_cta_id();
    }
    cta_launched++;
  }

  if (cp_op == 1) {
    char f1name[2048];
    snprintf(f1name, 2048, "checkpoint_files/global_mem_%d.txt",
             kernel.get_uid());
    g_checkpoint->store_global_mem(
        gpgpu_ctx->the_gpgpusim->g_the_gpu->get_global_memory(), f1name,
        (char *)"%08x");
  }

  // registering this kernel as done

  // openCL kernel simulation calls don't register the kernel so we don't
  // register its exit
  if (!openCL) {
    // extern stream_manager *g_stream_manager;
    gpgpu_ctx->the_gpgpusim->g_stream_manager->register_finished_kernel(
        kernel.get_uid());
  }

  //******PRINTING*******
  printf("GPGPU-Sim: Done functional simulation (%u instructions simulated).\n",
         g_ptx_sim_num_insn);
#if 0
  if (gpgpu_ptx_instruction_classification) {
    StatDisp(g_inst_classification_stat[g_ptx_kernel_count]);
    StatDisp(g_inst_op_classification_stat[g_ptx_kernel_count]);
  }
  // time_t variables used to calculate the total simulation time
  // the start time of simulation is hold by the global variable
  // g_simulation_starttime g_simulation_starttime is initilized by
  // gpgpu_ptx_sim_init_perf() in gpgpusim_entrypoint.cc upon starting gpgpu-sim
  time_t end_time, elapsed_time, days, hrs, minutes, sec;
  end_time = time((time_t *)NULL);
  elapsed_time =
      MAX(end_time - gpgpu_ctx->the_gpgpusim->g_simulation_starttime, 1);

  // calculating and printing simulation time in terms of days, hours, minutes
  // and seconds
  days = elapsed_time / (3600 * 24);
  hrs = elapsed_time / 3600 - 24 * days;
  minutes = elapsed_time / 60 - 60 * (hrs + 24 * days);
  sec = elapsed_time - 60 * (minutes + 60 * (hrs + 24 * days));

  fflush(stderr);
  printf(
      "\n\ngpgpu_simulation_time = %u days, %u hrs, %u min, %u sec (%u sec)\n",
      (unsigned)days, (unsigned)hrs, (unsigned)minutes, (unsigned)sec,
      (unsigned)elapsed_time);
  printf("gpgpu_simulation_rate = %u (inst/sec)\n",
         (unsigned)(g_ptx_sim_num_insn / elapsed_time));
  fflush(stdout);
#endif
}

