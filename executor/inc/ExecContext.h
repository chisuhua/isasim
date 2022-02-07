#pragma once

#include <vector>
#include <bitset>
#include <deque>
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
const uint32_t MAX_WARPSIZE = 32;
#define MAX_WARP_SIZE_SimtStack MAX_WARPSIZE
// typedef std::vector<addr_t> addr_vector_t;

typedef std::bitset<MAX_WARPSIZE> active_mask_t;
typedef std::bitset<MAX_WARP_SIZE_SimtStack> simt_mask_t;

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

/// 4-byte register
union Register
{
	int as_int;
	unsigned int as_uint;

	short int as_short[2];
	unsigned short int as_ushort[2];

	char as_byte[4];
	unsigned char as_ubyte[4];

	float as_float;
};

/// Constants for special registers
static const unsigned RegisterM0 = 124;
static const unsigned RegisterVcc = 106;
static const unsigned RegisterVccz = 251;
static const unsigned RegisterExec = 126;
static const unsigned RegisterExecz = 252;
static const unsigned RegisterScc = 253;

	/// Memory accesses types
	enum MemoryAccessType
	{
		MemoryAccessInvalid = 0,
		MemoryAccessRead,
		MemoryAccessWrite
	};

	/// Used for local memory accesses
	struct MemoryAccess
	{
		MemoryAccessType type;
		unsigned addr;
		unsigned size;
	};

	/// Image descriptor, as specified in Table 8.11 in SI documentation
	struct ImageDescriptor
	{
		unsigned long long base_addr : 40;   //    [39:0]
		unsigned int mid_lod         : 12;   //   [51:40]
		unsigned int data_fmt        : 6;    //   [57:52]
		unsigned int num_fmt         : 4;    //   [61:58]
		unsigned int                 : 2;    //   [63:62]
		unsigned int width           : 14;   //   [77:64]
		unsigned int height          : 14;   //   [91:78]
		unsigned int perf_mod        : 3;    //   [94:92]
		unsigned int interlaced      : 1;    //       95
		unsigned int dst_sel_x       : 3;    //   [98:96]
		unsigned int dst_sel_y       : 3;    //  [101:99]
		unsigned int dst_sel_z       : 3;    // [104:102]
		unsigned int dst_sel_w       : 3;    // [107:105]
		unsigned int base_level      : 4;    // [111:108]
		unsigned int last_level      : 4;    // [115:112]
		unsigned int tiling_idx      : 5;    // [120:116]
		unsigned int pow2pad         : 1;    //      121
		unsigned int                 : 2;    // [123:122]
		unsigned int type            : 4;    // [127:124]
		unsigned int depth           : 13;   // [140:128]
		unsigned int pitch           : 14;   // [154:141]
		unsigned int                 : 5;    // [159:155]
		unsigned int base_array      : 13;   // [172:160]
		unsigned int last_array      : 13;   // [185:173]
		unsigned int                 : 6;    // [191:186]
		unsigned int min_lod_warn    : 12;   // [203:192]
		unsigned long long           : 52;   // [255:204]
	}__attribute__((packed));


	/// Buffer descriptor, as specified in Table 8.5 in SI documentation
	struct BufferDescriptor
	{
		unsigned long long base_addr : 48;   //    [47:0]
		unsigned int stride          : 14;   //   [61:48]
		unsigned int cache_swizzle   : 1;    //       62
		unsigned int swizzle_enable  : 1;    //       63
		unsigned int num_records     : 32;   //   [95:64]
		unsigned int dst_sel_x       : 3;    //   [98:96]
		unsigned int dst_sel_y       : 3;    //  [101:99]
		unsigned int dst_sel_z       : 3;    // [104:102]
		unsigned int dst_sel_w       : 3;    // [107:105]
		unsigned int num_format      : 3;    // [110:108]
		unsigned int data_format     : 4;    // [114:111]
		unsigned int elem_size       : 2;    // [116:115]
		unsigned int index_stride    : 2;    // [118:117]
		unsigned int add_tid_enable  : 1;    //      119
		unsigned int reserved        : 1;    //      120
		unsigned int hash_enable     : 1;    //      121
		unsigned int heap            : 1;    //      122
		unsigned int unused          : 3;    // [125:123]
		unsigned int type            : 2;    // [127:126]
	}__attribute__((packed));

	// Sampler descriptor, as specified in Table 8.12 in SI documentation
	struct SamplerDescriptor
	{
		unsigned int clamp_x            : 3;    //     [2:0]
		unsigned int clamp_y            : 3;    //     [5:3]
		unsigned int clamp_z            : 3;    //     [8:6]
		unsigned int max_aniso_ratio    : 3;    //    [11:9]
		unsigned int depth_cmp_func     : 3;    //   [14:12]
		unsigned int force_unnorm       : 1;    //       15
		unsigned int aniso_thresh       : 3;    //   [18:16]
		unsigned int mc_coord_trunc     : 1;    //       19
		unsigned int force_degamma      : 1;    //       20
		unsigned int aniso_bias         : 6;    //   [26:21]
		unsigned int trunc_coord        : 1;    //       27
		unsigned int disable_cube_wrap  : 1;    //       28
		unsigned int filter_mode        : 2;    //   [30:29]
		unsigned int                    : 1;    //       31
		unsigned int min_lod            : 12;   //   [43:32]
		unsigned int max_lod            : 12;   //   [55:44]
		unsigned int perf_mip           : 4;    //   [59:56]
		unsigned int perf_z             : 4;    //   [63:60]
		unsigned int lod_bias           : 14;   //   [77:64]
		unsigned int lod_bias_sec       : 6;    //   [83:78]
		unsigned int xy_mag_filter      : 2;    //   [85:84]
		unsigned int xy_min_filter      : 2;    //   [87:86]
		unsigned int z_filter           : 2;    //   [89:88]
		unsigned int mip_filter         : 2;    //   [91:90]
		unsigned int mip_point_preclamp : 1;    //       92
		unsigned int disable_lsb_cell   : 1;    //       93
		unsigned int                    : 2;    //   [95:94]
		unsigned int border_color_ptr   : 12;   //  [107:96]
		unsigned int                    : 18;   // [125:108]
		unsigned int border_color_type  : 2;    // [127:126]
	}__attribute__((packed));

	// Memory pointer object, stored in 2 consecutive 32-bit SI registers
	struct MemoryPointer
	{
		unsigned long long addr : 48;
		unsigned int unused     : 16;
	}__attribute__((packed));



class SimtStack {
 public:
  SimtStack(unsigned wid, unsigned warpSize, libcuda::gpgpu_context *ctx);

  void reset();
  void launch(addr_t start_pc, const simt_mask_t &active_mask);
  void update(simt_mask_t &thread_done, std::vector<addr_t> &next_pc,
              addr_t recvg_pc, op_type_t next_inst_op,
              unsigned next_inst_size, addr_t next_inst_pc);

  const simt_mask_t &get_active_mask() const;
  void get_pdom_stack_top_info(unsigned *pc, unsigned *rpc) const;
  unsigned get_rp() const;
  void print(FILE *fp) const;
  void resume(char * fname) ;
  void print_checkpoint (FILE *fout) const;

 protected:
  unsigned m_warp_id;
  unsigned m_warp_size;

  enum stack_entry_type { STACK_ENTRY_TYPE_NORMAL = 0, STACK_ENTRY_TYPE_CALL };

  struct SimtStack_entry {
    addr_t m_pc;
    unsigned int m_calldepth;
    simt_mask_t m_active_mask;
    addr_t m_recvg_pc;
    uint64_t m_branch_div_cycle;
    stack_entry_type m_type;
    SimtStack_entry()
        : m_pc(-1),
          m_calldepth(0),
          m_active_mask(),
          m_recvg_pc(-1),
          m_branch_div_cycle(0),
          m_type(STACK_ENTRY_TYPE_NORMAL){};
  };

  std::deque<SimtStack_entry> m_stack;

  libcuda::gpgpu_context *m_gpgpu_ctx;
};



