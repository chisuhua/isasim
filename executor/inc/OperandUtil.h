#pragma once

#include <cassert>
#include <iostream>
#include <memory>
#include <vector>



//
// Bitwise Functions
//
template <typename T >
inline T getBits(T x, int high, int low)
{
    T one = 1;
	return (x >> low) & ((one << (high - low + 1)) - 1);
}

template <typename T>
inline bool getBit(T x, int bit)
{
    T one = 1;
	return x & (one << bit);
}

template<typename T>
inline T clearBits(T x, int high, int low)
{
	return x & (((1ull << low) - 1) | ~((1ull << (high + 1)) - 1));
}

template<typename T>
inline T truncBits(T x, int num)
{
	return x & ((1ull << num) - 1);
}

template<typename T>
inline T setBits(T x, int high, int low, T value)
{
	return clearBits(x, high, low) | (truncBits(value, high - low + 1) << low);
}

template<typename T>
inline T setBit(T x, int bit)
{
    T one = 1;
	return x | (one << bit);
}

template<typename T>
inline T clearBit(T x, int bit)
{
    T one = 1;
	return x & ~(one << bit);
}

template<typename T>
inline T setBit(T x, int bit, bool value)
{
	return value ? setBit(x, bit) : clearBit(x, bit);
}


template<typename T>
inline T SignExtend(T x, uint32_t b)
{
    T one = 1;
	return x & (one << (b - 1)) ? x | ~((one << b) - 1) :
			x & ((one << b) - 1);
}


//
// Miscellaneous
//


inline bool inRange(int value, int min, int max)
{
	return value >= min && value <= max;
}

template<typename T>
inline T AlignUp(T n, uint32_t align)
{
	assert(!(align & (align - 1)));
	return (n + (align - 1)) & ~(align - 1);
}


template<typename T>
inline uint32_t AlignDown(T n, uint32_t align)
{
	assert(!(align & (align - 1)));
	return n & ~(align - 1);
}

union hfpack
{
	uint32_t as_uint32;
	struct
	{
		uint16_t s1f;
		uint16_t s0f;
	} as_f16f16;
};

inline uint16_t Float32to16(float value)
{
	union Bits
	{
		float f;
		int si;
		uint32_t ui;
	};

	const uint32_t F_shift= 13;
	const uint32_t F_shiftSign = 16;

	const int F_infN = 0x7F800000; // flt32 infinity
	const int F_maxN = 0x477FE000; // max flt16 normal as a flt32
	const int F_minN = 0x38800000; // min flt16 normal as a flt32
	const int F_signN = 0x80000000; // flt32 sign bit

	const int F_infC = F_infN >> F_shift;
	const int F_nanN = (F_infC + 1) << F_shift; // minimum flt16 nan as a flt32
	const int F_maxC = F_maxN >> F_shift;
	const int F_minC = F_minN >> F_shift;
	const int F_mulN = 0x52000000; // (1 << 23) / F_minN
	const int F_subC = 0x003FF; // max flt32 subnormal down shifted
	const int F_maxD = F_infC - F_maxC - 1;
	const int F_minD = F_minC - F_subC - 1;

	union Bits v, s;
	v.f = value;
	uint32_t sign = v.si & F_signN;
	v.si ^= sign;
	sign >>= F_shiftSign; // logical F_shift
	s.si = F_mulN;
	s.si = s.f * v.f; // correct subnormals
	v.si ^= (s.si ^ v.si) & -(F_minN > v.si);
	v.si ^= (F_infN ^ v.si) & -((F_infN > v.si) & (v.si > F_maxN));
	v.si ^= (F_nanN ^ v.si) & -((F_nanN > v.si) & (v.si > F_infN));
	v.ui >>= F_shift; // logical F_shift
	v.si ^= ((v.si - F_maxD) ^ v.si) & -(v.si > F_maxC);
	v.si ^= ((v.si - F_minD) ^ v.si) & -(v.si > F_subC);
	return v.ui | sign;
}

inline float Float16to32(uint16_t value)
{
	union Bits
	{
		float f;
		int si;
		uint32_t ui;
	};

	const uint32_t F_shift = 13;
	const uint32_t F_shiftSign = 16;

	const int F_infN = 0x7F800000; // flt32 infinity
	const int F_maxN = 0x477FE000; // max flt16 normal as a flt32
	const int F_minN = 0x38800000; // min flt16 normal as a flt32
	const int F_signN = 0x80000000; // flt32 sign bit

	const int F_infC = F_infN >> F_shift;
	const int F_maxC = F_maxN >> F_shift;
	const int F_minC = F_minN >> F_shift;
	const int F_signC = F_signN >> F_shiftSign; // flt16 sign bit

	const int F_mulC =  0x33800000; // F_minN / (1 << (23 - F_shift))

	const int F_subC = 0x003FF; // max flt32 subnormal down shifted
	const int F_norC = 0x00400; // min flt32 normal down shifted

	const int F_maxD = F_infC - F_maxC - 1;
	const int F_minD = F_minC - F_subC - 1;

	union Bits v;
	v.ui = value;
	int sign = v.si & F_signC;
	v.si ^= sign;
	sign <<= F_shiftSign;
	v.si ^= ((v.si + F_minD) ^ v.si) & -(v.si > F_subC);
	v.si ^= ((v.si + F_maxD) ^ v.si) & -(v.si > F_maxC);
	union Bits s;
	s.si = F_mulC;
	s.f *= v.si;
	int mask = -(F_norC > v.si);
	v.si <<= F_shift;
	v.si ^= (s.si ^ v.si) & mask;
	v.si |= sign;
	return v.f;
}



