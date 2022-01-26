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


#define STRING2(x) #x
#define STRING(x) STRING2(x)

#define PASTE2(x, y) x##y
#define PASTE(x, y) PASTE2(x, y)

#ifdef NDEBUG
#define debug_warning(exp)                                                                         \
  do {                                                                                             \
  } while (false)
#else
#define debug_warning(exp)                                                                         \
  do {                                                                                             \
    if (!(exp))                                                                                    \
      fprintf(stderr, "Warning: " STRING(exp) " in %s, " __FILE__ ":" STRING(__LINE__) "\n",       \
              __PRETTY_FUNCTION__);                                                                \
  } while (false)
#endif

#ifdef NDEBUG
#define debug_print(fmt, ...)                                                                      \
  do {                                                                                             \
  } while (false)
#else
#define debug_print(fmt, ...)                                                                      \
  do {                                                                                             \
    fprintf(stderr, fmt, ##__VA_ARGS__);                                                           \
  } while (false)
#endif
#ifdef NDEBUG
#define ifdebug if (false)
#else
#define ifdebug if (true)
#endif


#define DTRACE(x) (false)
#define DDUMP(x, data, count) do {} while (0)
#define DPRINTF(x, ...) do {} while (0)
#define DPRINTFS(x, ...) do {} while (0)
#define DPRINTFR(...) do {} while (0)
#define DDUMPN(data, count) do {} while (0)
#define DPRINTFN(...) do {} while (0)
#define DPRINTFNR(...) do {} while (0)
#define DPRINTF_UNCONDITIONAL(x, ...) do {} while (0)



// A macro to disallow the copy and move constructor and operator= functions
#define DISALLOW_COPY_AND_ASSIGN(TypeName)                                                         \
  TypeName(const TypeName&) = delete;                                                              \
  TypeName(TypeName&&) = delete;                                                                   \
  void operator=(const TypeName&) = delete;                                                        \
  void operator=(TypeName&&) = delete;

#if 0
template <typename lambda>
class ScopeGuard {
 public:
  explicit inline ScopeGuard(const lambda& release)
      : release_(release), dismiss_(false) {}

  ScopeGuard(ScopeGuard& rhs) { *this = rhs; }

  inline ~ScopeGuard() {
    if (!dismiss_) release_();
  }
  inline ScopeGuard& operator=(ScopeGuard& rhs) {
    dismiss_ = rhs.dismiss_;
    release_ = rhs.release_;
    rhs.dismiss_ = true;
  }
  inline void Dismiss() { dismiss_ = true; }

 private:
  lambda release_;
  bool dismiss_;
};

template <typename lambda>
static inline ScopeGuard<lambda> MakeScopeGuard(lambda rel) {
  return ScopeGuard<lambda>(rel);
}

#define MAKE_SCOPE_GUARD_HELPER(lname, sname, ...) \
  auto lname = __VA_ARGS__;                        \
  ScopeGuard<decltype(lname)> sname(lname);
#define MAKE_SCOPE_GUARD(...)                                   \
  MAKE_SCOPE_GUARD_HELPER(PASTE(scopeGuardLambda, __COUNTER__), \
                          PASTE(scopeGuard, __COUNTER__), __VA_ARGS__)
#define MAKE_NAMED_SCOPE_GUARD(name, ...)                             \
  MAKE_SCOPE_GUARD_HELPER(PASTE(scopeGuardLambda, __COUNTER__), name, \
                          __VA_ARGS__)

#endif
