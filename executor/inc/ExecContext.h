#pragma once

#include <vector>
#include <deque>
// #include "inc/ExecTypes.h"
// #include "inc/Kernel.h"
// #include "../../libcuda/abstract_hardware_model.h"

namespace libcuda {
class gpgpu_context;
}

typedef unsigned long long addr_t;

#if 0
struct int3 {
	int x, y, z;
};

struct uint3 {
	unsigned int x, y, z;
};

struct int4 {
	int x, y, z, w;
};

struct uint4 {
	unsigned int x, y, z, w;
};

struct dim3
{
    unsigned int x, y, z;
#if defined(__cplusplus)
#if __cplusplus >= 201103L
    constexpr dim3(unsigned int vx = 1, unsigned int vy = 1, unsigned int vz = 1) : x(vx), y(vy), z(vz) {}
    constexpr dim3(uint3 v) : x(v.x), y(v.y), z(v.z) {}
    constexpr operator uint3(void) const { return uint3{x, y, z}; }
#else
    dim3(unsigned int vx = 1, unsigned int vy = 1, unsigned int vz = 1) : x(vx), y(vy), z(vz) {}
    dim3(uint3 v) : x(v.x), y(v.y), z(v.z) {}
    operator uint3(void) const { uint3 t; t.x = x; t.y = y; t.z = z; return t; }
#endif
#endif /* __cplusplus */
};
#endif

#define MAX_CTA_PER_SHADER 32
#define MAX_BARRIERS_PER_CTA 16

// typedef uint64_t addr_t;

// bounded stack that implements simt reconvergence using pdom mechanism from
// MICRO'07 paper
// const unsigned long long GLOBAL_HEAP_START = 0xC0000000;

// Volta max shmem size is 96kB
// const uint64_t SHARED_MEM_SIZE_MAX = 96 * (1 << 10);
// Volta max local mem is 16kB
// const uint64_t LOCAL_MEM_SIZE_MAX = 1 << 14;
// Volta Titan V has 80 SMs
// const uint32_t MAX_STREAMING_MULTIPROCESSORS = 80;
// Max 2048 threads / SM
// const uint32_t MAX_THREAD_PER_SM = 1 << 11;
// MAX 64 warps / SM
// const unsigned MAX_WARP_PER_SM = 1 << 6;
// const uint64_t TOTAL_LOCAL_MEM_PER_SM = MAX_THREAD_PER_SM * LOCAL_MEM_SIZE_MAX;
// const uint64_t TOTAL_SHARED_MEM = MAX_STREAMING_MULTIPROCESSORS * SHARED_MEM_SIZE_MAX;
// const uint64_t TOTAL_LOCAL_MEM = MAX_STREAMING_MULTIPROCESSORS * MAX_THREAD_PER_SM * LOCAL_MEM_SIZE_MAX;
// const uint64_t SHARED_GENERIC_START = GLOBAL_HEAP_START - TOTAL_SHARED_MEM;
// const uint64_t LOCAL_GENERIC_START = SHARED_GENERIC_START - TOTAL_LOCAL_MEM;
// const uint64_t STATIC_ALLOC_LIMIT = GLOBAL_HEAP_START - (TOTAL_LOCAL_MEM + TOTAL_SHARED_MEM);

enum divergence_support_t { POST_DOMINATOR = 1, NUM_SIMD_MODEL };
// const unsigned MAX_ACCESSES_PER_INSN_PER_THREAD = 8;


// the following are operations the timing model can see
#define SPECIALIZED_UNIT_NUM 8
#define SPEC_UNIT_START_ID 100


enum class opu_op_t {
  NO_OP = -1,
  ALU_OP = 1,
  SFU_OP,
  TENSOR_CORE_OP,
  DP_OP,
  SP_OP,
  INTP_OP,
  ALU_SFU_OP,
  LOAD_OP,
  TENSOR_CORE_LOAD_OP,
  TENSOR_CORE_STORE_OP,
  STORE_OP,
  BRANCH_OP,
  BARRIER_OP,
  MEMORY_BARRIER_OP,
  WAITCNT_OP,
  CALL_OPS,
  RET_OPS,
  EXIT_OPS,
  SPECIALIZED_UNIT_1_OP = SPEC_UNIT_START_ID,
  SPECIALIZED_UNIT_2_OP,
  SPECIALIZED_UNIT_3_OP,
  SPECIALIZED_UNIT_4_OP,
  SPECIALIZED_UNIT_5_OP,
  SPECIALIZED_UNIT_6_OP,
  SPECIALIZED_UNIT_7_OP,
  SPECIALIZED_UNIT_8_OP
};

enum opu_bar_t { NOT_BAR = -1, SYNC = 1, ARRIVE, RED };
typedef enum opu_bar_t barrier_type_t;

enum opu_red_t { NOT_RED = -1, POPC_RED = 1, AND_RED, OR_RED };
typedef enum opu_red_t reduction_type_t;

enum opu_operand_type_t { UN_OP = -1, INT_OP, FP_OP };
typedef enum opu_operand_type_t types_of_operands_t;

enum opu_special_operations_t {
  OTHER_OP,
  INT__OP,
  INT_MUL24_OP,
  INT_MUL32_OP,
  INT_MUL_OP,
  INT_DIV_OP,
  FP_MUL_OP,
  FP_DIV_OP,
  FP__OP,
  FP_SQRT_OP,
  FP_LG_OP,
  FP_SIN_OP,
  FP_EXP_OP
};

typedef enum opu_special_operations_t special_ops_t;  // Required to identify for the power model

enum operation_pipeline_t {
  UNKOWN_OP,
  SP__OP,
  DP__OP,
  INTP__OP,
  SFU__OP,
  TENSOR_CORE__OP,
  MEM__OP,
  SPECIALIZED__OP,
};

typedef enum opu_op_t op_type_t;

#define RECONVERGE_RETURN_PC ((addr_t)-2)
#define NO_BRANCH_DIVERGENCE ((addr_t)-1)

