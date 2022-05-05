#pragma once
#include "coasm_define.h"
#include <bitset>

const uint32_t MAX_WARPSIZE = 32;
#define MAX_WARP_SIZE_SimtStack MAX_WARPSIZE
// typedef std::vector<addr_t> addr_vector_t;

typedef std::bitset<MAX_WARPSIZE> active_mask_t;
typedef std::bitset<MAX_WARP_SIZE_SimtStack> simt_mask_t;
typedef unsigned long long addr_t;

union reg_t {
  reg_t() {
    bits.ms = 0;
    bits.ls = 0;
    u128.low = 0;
    u128.lowest = 0;
    u128.highest = 0;
    u128.high = 0;
    i8 = 0;
    i16 = 0;
    i32 = 0;
    i64 = 0;
    u8 = 0;
    u16 = 0;
    u64 = 0;
    // f16 = 0;
    f32 = 0;
    f64 = 0;
    pred = 0;
  }
  reg_t(uint32_t x) {
    bits.ms = 0;
    bits.ls = 0;
    u128.low = 0;
    u128.lowest = 0;
    u128.highest = 0;
    u128.high = 0;
    i8 = 0;
    i16 = 0;
    i32 = 0;
    i64 = 0;
    u8 = 0;
    u16 = 0;
    u64 = 0;
    // f16 = 0;
    f32 = 0;
    f64 = 0;
    pred = 0;
    u32 = x;
  }
  operator uint32_t() { return u32; }
  operator uint16_t() { return u16; }
  operator uint8_t() { return u8; }
  operator uint64_t() { return u64; }
  /*
  operator unsigned int() { return u32; }
  operator unsigned short() { return u16; }
  operator unsigned char() { return u8; }
  operator unsigned long long() { return u64; }
  */

  void mask_and(uint32_t ms, uint32_t ls) {
    bits.ms &= ms;
    bits.ls &= ls;
  }

  void mask_or(uint32_t ms, uint32_t ls) {
    bits.ms |= ms;
    bits.ls |= ls;
  }
  int get_bit(uint32_t bit) {
    if (bit < 32)
      return (bits.ls >> bit) & 1;
    else
      return (bits.ms >> (bit - 32)) & 1;
  }

  int8_t i8;
  int16_t i16;
  int32_t i32;
  int64_t i64;
  uint8_t u8;
  uint16_t u16;
  uint32_t u32;
  uint64_t u64;
  // half f16;
  float f32;
  double f64;
  struct {
    uint32_t ls;
    uint32_t ms;
  } bits;
  struct {
    uint32_t lowest;
    uint32_t low;
    uint32_t high;
    uint32_t highest;
  } u128;
  uint32_t reg[4];
  uint32_t pred : 4;
};
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

// Memory pointer object, stored in 2 consecutive 32-bit SI registers
struct MemoryPointer
{
	unsigned long long addr : 48;
	unsigned int unused     : 16;
}__attribute__((packed));

union RegisterX2 {
    uint64_t as_long;
    Register as_reg[2];
    MemoryPointer as_ptr;
};

/// Constants for special registers
static const unsigned RegisterM0 = SREG_M0;
static const unsigned RegisterTcc = SREG_TCC;
static const unsigned RegisterTccz = SREG_TCCZ;
static const unsigned RegisterVcc = SREG_VCC;
static const unsigned RegisterVccz = SREG_VCCZ;
static const unsigned RegisterExec = SREG_EXEC;
static const unsigned RegisterExecz = SREG_EXECZ;
static const unsigned RegisterScc = SREG_SCC;

enum class reg_space_t {
  undefined=0,
  SCALAR,
  VECTOR,
  DATA
};

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






