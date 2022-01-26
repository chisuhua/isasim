#pragma once

#include "stdint.h"
#include "stddef.h"
#include "stdlib.h"
#include <assert.h>
#include <iostream>
#include <string>
#include <algorithm>

typedef unsigned int uint;
typedef uint64_t uint64;

#define INVALID_ADDR 0xFFFFFFFFFFFFFFFFULL

/// @brief: Finds out the min one of two inputs, input must support ">"
/// operator.
/// @param: a(Input), a reference to type T.
/// @param: b(Input), a reference to type T.
/// @return: T.
template <class T>
static inline T Min(const T& a, const T& b) {
  return (a > b) ? b : a;
}

template <class T, class... Arg>
static inline T Min(const T& a, const T& b, Arg... args) {
  return Min(a, Min(b, args...));
}

/// @brief: Find out the max one of two inputs, input must support ">" operator.
/// @param: a(Input), a reference to type T.
/// @param: b(Input), a reference to type T.
/// @return: T.
template <class T>
static inline T Max(const T& a, const T& b) {
  return (b > a) ? b : a;
}

template <class T, class... Arg>
static inline T Max(const T& a, const T& b, Arg... args) {
  return Max(a, Max(b, args...));
}


/// @brief: Checks if a value is power of two, if it is, return true. Be careful
/// when passing 0.
/// @param: val(Input), the data to be checked.
/// @return: bool.
template <typename T>
static inline bool IsPowerOfTwo(const T& n) {
  // return (val & (val - 1)) == 0;
  // If n is non-zero, and subtracting one borrows all the way to the MSB
  // and flips all bits, then this is a power of 2.
  return n && !(n & (n - 1));
}

/// @brief: Calculates the floor value aligned based on parameter of alignment.
/// If value is at the boundary of alignment, it is unchanged.
/// @param: value(Input), value to be calculated.
/// @param: alignment(Input), alignment value.
/// @return: T.
template <typename T>
static inline T AlignDown(T value, size_t alignment) {
  assert(IsPowerOfTwo(alignment));
  return (T)(value & ~(alignment - 1));
}

/// @brief: Same as previous one, but first parameter becomes pointer, for more
/// info, see the previous desciption.
/// @param: value(Input), pointer to type T.
/// @param: alignment(Input), alignment value.
/// @return: T*, pointer to type T.
template <typename T>
static inline T* AlignDown(T* value, size_t alignment) {
  return (T*)AlignDown((intptr_t)value, alignment);
}

/// @brief: Calculates the ceiling value aligned based on parameter of
/// alignment.
/// If value is at the boundary of alignment, it is unchanged.
/// @param: value(Input), value to be calculated.
/// @param: alignment(Input), alignment value.
/// @param: T.
template <typename T>
static inline T AlignUp(T value, size_t alignment) {
  return AlignDown((T)(value + alignment - 1), alignment);
}

/// @brief: Same as previous one, but first parameter becomes pointer, for more
/// info, see the previous desciption.
/// @param: value(Input), pointer to type T.
/// @param: alignment(Input), alignment value.
/// @return: T*, pointer to type T.
template <typename T>
static inline T* AlignUp(T* value, size_t alignment) {
  return (T*)AlignDown((intptr_t)((uint8_t*)value + alignment - 1), alignment);
}

/// @brief: Checks if the input value is at the boundary of alignment, if it is,
/// @return true.
/// @param: value(Input), value to be checked.
/// @param: alignment(Input), alignment value.
/// @return: bool.
template <typename T>
static inline bool IsMultipleOf(T value, size_t alignment) {
  return (AlignUp(value, alignment) == value);
}

/// @brief: Same as previous one, but first parameter becomes pointer, for more
/// info, see the previous desciption.
/// @param: value(Input), pointer to type T.
/// @param: alignment(Input), alignment value.
/// @return: bool.
template <typename T>
static inline bool IsMultipleOf(T* value, size_t alignment) {
  return (AlignUp(value, alignment) == value);
}

static inline uint32_t NextPow2(uint32_t value) {
  if (value == 0) return 1;
  uint32_t v = value - 1;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  return v + 1;
}

static inline uint64_t NextPow2(uint64_t value) {
  if (value == 0) return 1;
  uint64_t v = value - 1;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v |= v >> 32;
  return v + 1;
}
static inline bool strIsEmpty(const char* str) noexcept { return str[0] == '\0'; }

static inline std::string& ltrim(std::string& s) {
  auto it = std::find_if(s.begin(), s.end(),
                         [](char c) { return !std::isspace<char>(c, std::locale::classic()); });
  s.erase(s.begin(), it);
  return s;
}

static inline std::string& rtrim(std::string& s) {
  auto it = std::find_if(s.rbegin(), s.rend(),
                         [](char c) { return !std::isspace<char>(c, std::locale::classic()); });
  s.erase(it.base(), s.end());
  return s;
}

static inline std::string& trim(std::string& s) { return ltrim(rtrim(s)); }


template <uint32_t lowBit, uint32_t highBit, typename T>
static inline uint32_t BitSelect(T p) {
  static_assert(sizeof(T) <= sizeof(uintptr_t), "Type out of range.");
  static_assert(highBit < sizeof(uintptr_t) * 8, "Bit index out of range.");

  uintptr_t ptr = p;
  if (highBit != (sizeof(uintptr_t) * 8 - 1))
    return (uint32_t)((ptr & ((1ull << (highBit + 1)) - 1)) >> lowBit);
  else
    return (uint32_t)(ptr >> lowBit);
}

inline uint32_t PtrLow16Shift8(const void* p) {
  uintptr_t ptr = reinterpret_cast<uintptr_t>(p);
  return (uint32_t)((ptr & 0xFFFFULL) >> 8);
}

inline uint32_t PtrHigh64Shift16(const void* p) {
  uintptr_t ptr = reinterpret_cast<uintptr_t>(p);
  return (uint32_t)((ptr & 0xFFFFFFFFFFFF0000ULL) >> 16);
}

inline uint32_t PtrLow40Shift8(const void* p) {
  uintptr_t ptr = reinterpret_cast<uintptr_t>(p);
  return (uint32_t)((ptr & 0xFFFFFFFFFFULL) >> 8);
}

inline uint32_t PtrHigh64Shift40(const void* p) {
  uintptr_t ptr = reinterpret_cast<uintptr_t>(p);
  return (uint32_t)((ptr & 0xFFFFFF0000000000ULL) >> 40);
}

inline uint32_t PtrLow32(const void* p) {
  return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(p));
}

inline uint32_t PtrHigh32(const void* p) {
  uint32_t ptr = 0;
#ifdef HSA_LARGE_MODEL
  ptr = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(p) >> 32);
#endif
  return ptr;
}

inline uint64_t PackHighLow(uint32_t hi, uint32_t lo) {
  uint64_t ret = hi;
  ret = (ret << 32 ) | lo;
  return ret;
}

