#pragma once

#include <vector>
#include <bitset>
#include "inc/Kernel.h"

#define MAX_CTA_PER_SHADER 32
#define MAX_BARRIERS_PER_CTA 16

typedef uint64_t address_type;

// bounded stack that implements simt reconvergence using pdom mechanism from
// MICRO'07 paper
const uint32_t MAX_WARP_SIZE = 32;
#define MAX_WARP_SIZE_SIMT_STACK MAX_WARP_SIZE
typedef std::vector<address_type> addr_vector_t;

typedef std::bitset<MAX_WARP_SIZE> active_mask_t;
typedef std::bitset<MAX_WARP_SIZE_SIMT_STACK> simt_mask_t;

const unsigned long long GLOBAL_HEAP_START = 0xC0000000;

// Volta max shmem size is 96kB
const uint64_t SHARED_MEM_SIZE_MAX = 96 * (1 << 10);
// Volta max local mem is 16kB
const uint64_t LOCAL_MEM_SIZE_MAX = 1 << 14;
// Volta Titan V has 80 SMs
const uint32_t MAX_STREAMING_MULTIPROCESSORS = 80;
// Max 2048 threads / SM
const uint32_t MAX_THREAD_PER_SM = 1 << 11;
// MAX 64 warps / SM
const unsigned MAX_WARP_PER_SM = 1 << 6;
const uint64_t TOTAL_LOCAL_MEM_PER_SM = MAX_THREAD_PER_SM * LOCAL_MEM_SIZE_MAX;
const uint64_t TOTAL_SHARED_MEM = MAX_STREAMING_MULTIPROCESSORS * SHARED_MEM_SIZE_MAX;
const uint64_t TOTAL_LOCAL_MEM = MAX_STREAMING_MULTIPROCESSORS * MAX_THREAD_PER_SM * LOCAL_MEM_SIZE_MAX;
const uint64_t SHARED_GENERIC_START = GLOBAL_HEAP_START - TOTAL_SHARED_MEM;
const uint64_t LOCAL_GENERIC_START = SHARED_GENERIC_START - TOTAL_LOCAL_MEM;
const uint64_t STATIC_ALLOC_LIMIT = GLOBAL_HEAP_START - (TOTAL_LOCAL_MEM + TOTAL_SHARED_MEM);

enum divergence_support_t { POST_DOMINATOR = 1, NUM_SIMD_MODEL };
const unsigned MAX_ACCESSES_PER_INSN_PER_THREAD = 8;


// the following are operations the timing model can see
#define SPECIALIZED_UNIT_NUM 8
#define SPEC_UNIT_START_ID 100


enum uarch_op_t {
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

enum uarch_bar_t { NOT_BAR = -1, SYNC = 1, ARRIVE, RED };
typedef enum uarch_bar_t barrier_type;

enum uarch_red_t { NOT_RED = -1, POPC_RED = 1, AND_RED, OR_RED };
typedef enum uarch_red_t reduction_type;

enum uarch_operand_type_t { UN_OP = -1, INT_OP, FP_OP };
typedef enum uarch_operand_type_t types_of_operands;

enum special_operations_t {
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

typedef enum special_operations_t special_ops;  // Required to identify for the power model

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

typedef enum uarch_op_t OpType;

#define RECONVERGE_RETURN_PC ((address_type)-2)
#define NO_BRANCH_DIVERGENCE ((address_type)-1)

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



#if 0
class simt_stack {
 public:
  simt_stack(unsigned wid, unsigned warpSize); // , class gpgpu_sim *gpu);

  void reset();
  void launch(address_type start_pc, const simt_mask_t &active_mask);
  void update(simt_mask_t &thread_done, addr_vector_t &next_pc,
              address_type recvg_pc, op_type next_inst_op,
              unsigned next_inst_size, address_type next_inst_pc);

  const simt_mask_t &get_active_mask() const;
  void get_pdom_stack_top_info(unsigned *pc, unsigned *rpc) const;
  unsigned get_rp() const;
  // void print(FILE *fp) const;

 protected:
  unsigned m_warp_id;
  unsigned m_warp_size;

  enum stack_entry_type { STACK_ENTRY_TYPE_NORMAL = 0, STACK_ENTRY_TYPE_CALL };

  struct simt_stack_entry {
    address_type m_pc;
    unsigned int m_calldepth;
    simt_mask_t m_active_mask;
    address_type m_recvg_pc;
    uint64_t m_branch_div_cycle;
    stack_entry_type m_type;
    simt_stack_entry()
        : m_pc(-1),
          m_calldepth(0),
          m_active_mask(),
          m_recvg_pc(-1),
          m_branch_div_cycle(0),
          m_type(STACK_ENTRY_TYPE_NORMAL){};
  };

  std::deque<simt_stack_entry> m_stack;

  class gpgpu_sim *m_gpu;
};


enum _memory_space_t {
  undefined_space = 0,
  reg_space,
  local_space,
  shared_space,
  sstarr_space,
  param_space_unclassified,
  param_space_kernel, /* global to all threads in a kernel : read-only */
  param_space_local,  /* local to a thread : read-writable */
  const_space,
  tex_space,
  surf_space,
  global_space,
  generic_space,
  instruction_space
};

typedef uint64_t new_addr_type;
typedef unsigned address_type;
typedef unsigned addr_t;

enum uarch_op_t {
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


typedef enum uarch_op_t op_type;

enum uarch_bar_t { NOT_BAR = -1, SYNC = 1, ARRIVE, RED };
typedef enum uarch_bar_t barrier_type;

enum uarch_red_t { NOT_RED = -1, POPC_RED = 1, AND_RED, OR_RED };
typedef enum uarch_red_t reduction_type;

enum uarch_operand_type_t { UN_OP = -1, INT_OP, FP_OP };
typedef enum uarch_operand_type_t types_of_operands;

enum special_operations_t {
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

typedef enum special_operations_t special_ops;  // Required to identify for the power model

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

typedef enum operation_pipeline_t operation_pipeline;
enum mem_operation_t { NOT_TEX, TEX };
typedef enum mem_operation_t mem_operation;

enum cache_operator_type {
  CACHE_UNDEFINED,

  // loads
  CACHE_ALL,       // .ca
  CACHE_LAST_USE,  // .lu
  CACHE_VOLATILE,  // .cv
  CACHE_L1,        // .nc

  // loads and stores
  CACHE_STREAMING,  // .cs
  CACHE_GLOBAL,     // .cg

  // stores
  CACHE_WRITE_BACK,    // .wb
  CACHE_WRITE_THROUGH  // .wt
};

// the maximum number of destination, source, or address uarch operands in a
// instruction
#define MAX_REG_OPERANDS 32


/*
 * This abstract class used as a base for functional and performance and
 * simulation, it has basic functional simulation data structures and
 * procedures.
 */
class core_t {
 public:
  core_t(shared_ptr<DispatchInfo> disp_info, unsigned warp_size,
         unsigned blk_threads)
      : m_disp_info(kernel),
        m_simt_stack(NULL),
        m_thread(NULL),
        m_warp_size(warp_size),
        m_blk_threads(blk_threads) {
    m_warp_count = blk_threads / m_warp_size;
    // Handle the case where the number of threads is not a
    // multiple of the warp size
    if (blk_threads % m_warp_size != 0) {
      m_warp_count += 1;
    }
    assert(m_warp_count * m_warp_size > 0);
    m_thread = (thread_info **)calloc(m_warp_count * m_warp_size,
                                          sizeof(thread_info *));
    initilizeSIMTStack(m_warp_count, m_warp_size);

    for (unsigned i = 0; i < MAX_CTA_PER_SHADER; i++) {
      for (unsigned j = 0; j < MAX_BARRIERS_PER_CTA; j++) {
        reduction_storage[i][j] = 0;
      }
    }
  }
  virtual ~core_t() { free(m_thread); }
  virtual void warp_exit(unsigned warp_id) = 0;
  virtual bool warp_waiting_at_barrier(unsigned warp_id) const = 0;
  virtual void checkExecutionStatusAndUpdate(warp_inst_t &inst, unsigned t,
                                             unsigned tid) = 0;
  void execute_warp_inst_t(warp_inst_t &inst, unsigned warpId = (unsigned)-1);
  bool ptx_thread_done(unsigned hw_thread_id) const;
  virtual void updateSIMTStack(unsigned warpId, warp_inst_t *inst);
  void initilizeSIMTStack(unsigned warp_count, unsigned warps_size);
  void deleteSIMTStack();
  warp_inst_t getExecuteWarp(unsigned warpId);
  void get_pdom_stack_top_info(unsigned warpId, unsigned *pc,
                               unsigned *rpc) const;
  // kernel_info_t *get_kernel_info() { return m_kernel; }
  shared_ptr<DispatchInfo> get_disp_info() { return m_disp_info; }
  class thread_info **get_thread_info() {
    return m_thread;
  }
  unsigned get_warp_size() const { return m_warp_size; }
  void and_reduction(unsigned ctaid, unsigned barid, bool value) {
    reduction_storage[ctaid][barid] &= value;
  }
  void or_reduction(unsigned ctaid, unsigned barid, bool value) {
    reduction_storage[ctaid][barid] |= value;
  }
  void popc_reduction(unsigned ctaid, unsigned barid, bool value) {
    reduction_storage[ctaid][barid] += value;
  }
  unsigned get_reduction_value(unsigned ctaid, unsigned barid) {
    return reduction_storage[ctaid][barid];
  }

 protected:
  shared_ptr<DispatchInfo> m_disp_info;
  simt_stack **m_simt_stack;  // pdom based reconvergence context for each warp
  class thread_info **m_thread;
  unsigned m_blk_threads;
  unsigned m_warp_size;
  unsigned m_warp_count;
  unsigned reduction_storage[MAX_CTA_PER_SHADER][MAX_BARRIERS_PER_CTA];
};

// register that can hold multiple instructions.
class register_set {
 public:
  register_set(unsigned num, const char *name) {
    for (unsigned i = 0; i < num; i++) {
      regs.push_back(new warp_inst_t());
    }
    m_name = name;
  }
  bool has_free() {
    for (unsigned i = 0; i < regs.size(); i++) {
      if (regs[i]->empty()) {
        return true;
      }
    }
    return false;
  }
  bool has_free(bool sub_core_model, unsigned reg_id) {
    // in subcore model, each sched has a one specific reg to use (based on
    // sched id)
    if (!sub_core_model) return has_free();

    assert(reg_id < regs.size());
    return regs[reg_id]->empty();
  }
  bool has_ready() {
    for (unsigned i = 0; i < regs.size(); i++) {
      if (not regs[i]->empty()) {
        return true;
      }
    }
    return false;
  }

  void move_in(warp_inst_t *&src) {
    warp_inst_t **free = get_free();
    move_warp(*free, src);
  }
  // void copy_in( warp_inst_t* src ){
  //   src->copy_contents_to(*get_free());
  //}
  void move_out_to(warp_inst_t *&dest) {
    warp_inst_t **ready = get_ready();
    move_warp(dest, *ready);
  }

  warp_inst_t **get_ready() {
    warp_inst_t **ready;
    ready = NULL;
    for (unsigned i = 0; i < regs.size(); i++) {
      if (not regs[i]->empty()) {
        if (ready and (*ready)->get_uid() < regs[i]->get_uid()) {
          // ready is oldest
        } else {
          ready = &regs[i];
        }
      }
    }
    return ready;
  }

  void print(FILE *fp) const {
    fprintf(fp, "%s : @%p\n", m_name, this);
    for (unsigned i = 0; i < regs.size(); i++) {
      fprintf(fp, "     ");
      regs[i]->print(fp);
      fprintf(fp, "\n");
    }
  }

  warp_inst_t **get_free() {
    for (unsigned i = 0; i < regs.size(); i++) {
      if (regs[i]->empty()) {
        return &regs[i];
      }
    }
    assert(0 && "No free registers found");
    return NULL;
  }

  warp_inst_t **get_free(bool sub_core_model, unsigned reg_id) {
    // in subcore model, each sched has a one specific reg to use (based on
    // sched id)
    if (!sub_core_model) return get_free();

    assert(reg_id < regs.size());
    if (regs[reg_id]->empty()) {
      return &regs[reg_id];
    }
    assert(0 && "No free register found");
    return NULL;
  }

  unsigned get_size() { return regs.size(); }

 private:
  std::vector<warp_inst_t *> regs;
  const char *m_name;
};

struct param_t {
  const void *pdata;
  int type;
  size_t size;
  size_t offset;
};

union ptx_reg_t {
  ptx_reg_t() {
    bits.ms = 0;
    bits.ls = 0;
    u128.low = 0;
    u128.lowest = 0;
    u128.highest = 0;
    u128.high = 0;
    s8 = 0;
    s16 = 0;
    s32 = 0;
    s64 = 0;
    u8 = 0;
    u16 = 0;
    u64 = 0;
    f16 = 0;
    f32 = 0;
    f64 = 0;
    pred = 0;
  }
  ptx_reg_t(unsigned x) {
    bits.ms = 0;
    bits.ls = 0;
    u128.low = 0;
    u128.lowest = 0;
    u128.highest = 0;
    u128.high = 0;
    s8 = 0;
    s16 = 0;
    s32 = 0;
    s64 = 0;
    u8 = 0;
    u16 = 0;
    u64 = 0;
    f16 = 0;
    f32 = 0;
    f64 = 0;
    pred = 0;
    u32 = x;
  }
  operator unsigned int() { return u32; }
  operator unsigned short() { return u16; }
  operator unsigned char() { return u8; }
  operator uint64_t() { return u64; }

  void mask_and(unsigned ms, unsigned ls) {
    bits.ms &= ms;
    bits.ls &= ls;
  }

  void mask_or(unsigned ms, unsigned ls) {
    bits.ms |= ms;
    bits.ls |= ls;
  }
  int get_bit(unsigned bit) {
    if (bit < 32)
      return (bits.ls >> bit) & 1;
    else
      return (bits.ms >> (bit - 32)) & 1;
  }

  signed char s8;
  signed short s16;
  signed int s32;
  signed long long s64;
  unsigned char u8;
  unsigned short u16;
  unsigned int u32;
  uint64_t u64;
  half f16;
  float f32;
  double f64;
  struct {
    unsigned ls;
    unsigned ms;
  } bits;
  struct {
    unsigned int lowest;
    unsigned int low;
    unsigned int high;
    unsigned int highest;
  } u128;
  unsigned pred : 4;
};

#endif

