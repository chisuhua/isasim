#pragma once

#include "inc/ExecTypes.h"
#include <assert.h>
#include <stdint.h>

template <typename T>
inline uint8_t U8(T v) {
    return *reinterpret_cast<uint8_t*>(&v);
}

template <typename T>
inline uint8_t U8(std::vector<T> &&v) {
    return *reinterpret_cast<uint8_t*>(v.data());
}

template <typename T>
inline int8_t I8(T v) {
    return *reinterpret_cast<int8_t*>(&v);
}
template <typename T>
inline uint16_t U16(T v) {
    return *reinterpret_cast<uint16_t*>(&v);
}
template <typename T>
inline int16_t I16(T v) {
    return *reinterpret_cast<int16_t*>(&v);
}
template <typename T>
inline uint32_t U24(T v) {
    auto tmp = *reinterpret_cast<uint32_t*>(&v);
    return tmp & 0xffffff;
}
template <typename T>
inline int32_t I24(T v) {
    auto tmp = *reinterpret_cast<uint32_t*>(&v);
    tmp = tmp & 0xffffff;
    tmp = (tmp & 0x800000) ? (tmp| 0xff000000) : tmp;
    return *reinterpret_cast<int32_t*>(&tmp);
}
template <typename T>
inline uint32_t U32(T v) {
    return *reinterpret_cast<uint32_t*>(&v);
}

template <typename T>
inline uint32_t U32(std::vector<T> &&v) {
    return *reinterpret_cast<uint32_t*>(&v[0]);
}

// template <>
// inline uint32_t U32<Register>(Register &&v) {
//     return *reinterpret_cast<uint32_t*>(&v.as_uint);
// }

template <typename T>
inline int32_t I32(T v) {
    return *reinterpret_cast<int32_t*>(&v);
}
template <typename T>
inline float F32(T v) {
    return *reinterpret_cast<float*>(&v);
}
template <typename T>
inline uint64_t U64(T v) {
    return *reinterpret_cast<uint64_t*>(&v);
}

template <>
inline uint64_t U64<uint32_t>(uint32_t v) {
    uint64_t d = v;
    return d;
}
template <>
inline uint64_t U64(std::vector<Register> v) {
    assert(v.size() >= 2);
    uint64_t lo = U64(v[0].as_uint);
    uint64_t hi = v[1].as_uint << 32;
    return hi | lo;
}
template <typename T>
inline int64_t I64(T v) {
    return *reinterpret_cast<int64_t*>(&v);
}
template <typename T>
inline double F64(T v) {
    return *reinterpret_cast<double*>(&v);
}

inline int32_t I8_to_I32(uint8_t v) {
    uint32_t tmp = (v & 0x80) ? (0xffffff00 | v) : v;
    return *reinterpret_cast<int32_t*>(&tmp);
}
inline int32_t I16_to_I32(uint16_t v) {
    uint32_t tmp = (v & 0x8000) ? (0xffff0000 | v) : v;
    return *reinterpret_cast<int32_t*>(&tmp);
}
inline int32_t I24_to_I32(uint32_t v) {
    uint32_t tmp = (v & 0x800000) ? (0xff000000 | v) : v & 0x00ffffff;
    return *reinterpret_cast<int32_t*>(&tmp);
}

template <typename T>
inline T Relu(T v);

template <>
inline int32_t Relu<int32_t>(int32_t v) {
    return v < 0 ? 0 : v;
}

template <>
inline int16_t Relu<int16_t>(int16_t v) {
    return v < 0 ? 0 : v;
}

template <>
inline uint16_t Relu<uint16_t>(uint16_t v) {
    return v;
}

template <>
inline int8_t Relu<int8_t>(int8_t v) {
    return v < 0 ? 0 : v;
}

template <>
inline int64_t Relu<int64_t>(int64_t v) {
    return v < 0 ? 0 : v;
}

template <>
inline double Relu<double>(double v) {
    uint32_t tmp = U64(v);
    if (((tmp & 0x7ff0000000000000) == 0x7ff0000000000000) && (tmp & 0x000fffffffffffff)) {
        uint32_t data = 0x7fffffffffffffff;
        return F64(data);
    }
    return (tmp & 0x8000000000000000) >> 63 ? 0 : v;
}

template <typename T>
inline T Saturation(T v) {
    if (v.isinfinity()) {
        return v.sign() ? v.min() : v.max();
    }
    if (v.isnan()) {
        // return positive 0
        return v.zero(0);
    }
    return v;
}

template <>
inline float Saturation<float>(float v) {
    uint32_t d = U32(v);
    // if it is -inf , return min of float
    if ((d & 0xffffffff) == 0xff800000) {
        uint32_t r = 0xff7fffff;
        return F32(r);
    }
    // if it is inf , return max of float
    if ((d & 0xffffffff) == 0x7f800000) {
        uint32_t r = 0x7f7fffff;
        return F32(r);
    }
    // if it is nan , return positive 0
    if ((d & 0xffffffff) > 0x7f800000) {
        return 0;
    }
    return v;
}

template <typename T>
inline T Denorm(T v);

template <>
inline float Denorm<float>(float v) {
    auto d = U32(v);
    // return neg 0
    if ((d & 0xff800000) == 0x80000000) {
        uint32_t r = 0x80000000;
        return F32(r);
    }
    // return pos 0
    if ((d & 0xff800000) == 0) {
        return 0;
    }
    return v;
}
template <>
inline double Denorm<double>(double v) {
    auto d = U64(v);
    // return neg 0
    if ((d & 0xff80000000000000L) == 0x8000000000000000L) {
        uint64_t r = 0x8000000000000000;
        return F64(r);
    }
    // return pos 0
    if ((d & 0xff80000000000000L) == 0) {
        return 0;
    }
    return v;
}

inline uint64_t F64_to_U64(double v) {
}

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
    return I32(temp);
}

inline int32_t SEXT_I32_I16(uint32_t t) {
    uint32_t temp = (t & 0x00008000) ? (0xffff0000 | t) : t & 0x0000ffff;
    return I32(temp);
}

inline int32_t SEXT_I32_I24(uint32_t t) {
    uint32_t temp = (t & 0x00800000) ? (0xff000000 | t) : t & 0x00ffffff;
    return I32(temp);
}

inline int64_t SEXT_I64_I32(uint32_t t) {
    uint32_t temp1 = (t & 0x80000000) ? 0xffffffff : 0x00000000;
    uint32_t temp0 = t;
    Long data(temp0, temp1);
    return data.i64;
}
