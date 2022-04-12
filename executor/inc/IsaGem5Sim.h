#pragma once
// #include "inc/ExecContext.h"
//#include "inc/Kernel.h"

class gpgpu_context;
class gpgpu_t;
class warp_inst_t;

/*!
 * This class functionally executes a kernel. It uses the basic data structures and procedures in core_t
 */
class IsaGem5Sim {
public:
  IsaGem5Sim(gpgpu_t* gpu, gpgpu_context *ctx) {
    m_gpu = gpu;
    m_ctx = ctx;
  }

  void exec_inst(warp_inst_t *inst);
  ThreadItem *make_threaditem();


  gpgpu_t *m_gpu;
  gpgpu_context *m_ctx;
};


