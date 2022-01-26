#pragma once

union Int {
    uint8_t     u8[4];
    int8_t      i8[4];
    uint16_t    u16[2];
    int16_t     i16[2];
    uint32_t    u32;
    int32_t     i32;
};

union Long {
    Int       int32[2];
    uint64_t    u64;
    int64_t     i64;
    double      f64;
    void       *ptr;
    struct {
        uint32_t low;
        uint32_t high;
    };

    Long() {}
    Long(void* ptr) : ptr(ptr) {}
    Long(uint64_t  val) : u64(val) {}
    Long(uint32_t  low, uint32_t high) : low(low), high(high) {}
};

inline int32_t SEXT_I32_I8(uint32_t t) {
    uint32_t temp = (t & 0x00000080) ? (0xffffff00 | t) : t & 0x000000ff;
    return *reinterpret_cast<int32_t*>(&temp);
}

inline int32_t SEXT_I32_I16(uint32_t t) {
    uint32_t temp = (t & 0x00008000) ? (0xffff0000 | t) : t & 0x0000ffff;
    return *reinterpret_cast<int32_t*>(&temp);
}

inline int32_t SEXT_I32_I24(uint32_t t) {
    uint32_t temp = (t & 0x00800000) ? (0xff000000 | t) : t & 0x00ffffff;
    return *reinterpret_cast<int32_t*>(&temp);
}

inline int64_t SEXT_I64_I32(uint32_t t) {
    uint32_t temp0 = (t & 0x80000000) ? 0xffffffff : 0x00000000;
    uint32_t temp1 = t;
    Long data(temp0, temp1);
    return *reinterpret_cast<int64_t*>(&data.i64);
}
