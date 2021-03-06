#pragma once
#include "inc/ExecContext.h"
#include "inc/Kernel.h"

namespace libcuda {
class gpgpu_context;
class gpgpu_t;
}

class Kernel;

/*!
 * This class functionally executes a kernel. It uses the basic data structures and procedures in core_t
 */
class IsaSim {
public:
  IsaSim(libcuda::gpgpu_t* gpu, libcuda::gpgpu_context *ctx) {
    m_gpu = gpu;
    m_ctx = ctx;
  }
  void launch(DispatchInfo *disp_info, unsigned kernel_uid, bool openCL = false);

  libcuda::gpgpu_t *m_gpu;
  libcuda::gpgpu_context *m_ctx;
  int cp_count;
  int cp_cta_resume;
};

