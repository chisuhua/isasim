#include "inc/MemCommon.h"
#include "coasm_define.h"
#include <cassert>

namespace isasim {

addr_t shared_to_generic(unsigned smid, addr_t addr) {
  assert(addr < _SHARED_MEM_SIZE_MAX);
  return _SHARED_GENERIC_START + smid * _SHARED_MEM_SIZE_MAX + addr;
}

bool isspace_shared(unsigned smid, addr_t addr) {
  addr_t start = _SHARED_GENERIC_START + smid * _SHARED_MEM_SIZE_MAX;
  addr_t end = _SHARED_GENERIC_START + (smid + 1) * _SHARED_MEM_SIZE_MAX;
  if ((addr >= end) || (addr < start)) return false;
  return true;
}

bool isspace_global(addr_t addr) {
  return (addr >= _GLOBAL_HEAP_START) || (addr < _STATIC_ALLOC_LIMIT);
}

mem_space_t::SpaceType whichspace(addr_t addr) {
  if ((addr >= _GLOBAL_HEAP_START) || (addr < _STATIC_ALLOC_LIMIT)) {
    return mem_space_t::global_space;
  } else if (addr >= _SHARED_GENERIC_START) {
    return mem_space_t::shared_space;
  } else {
    return mem_space_t::local_space;
  }
}

addr_t generic_to_shared(unsigned smid, addr_t addr) {
  assert(isspace_shared(smid, addr));
  return addr - (_SHARED_GENERIC_START + smid * _SHARED_MEM_SIZE_MAX);
}

addr_t local_to_generic(unsigned smid, unsigned hwtid, addr_t addr) {
  assert(addr < _LOCAL_MEM_SIZE_MAX);
  return _LOCAL_GENERIC_START + (_TOTAL_LOCAL_MEM_PER_SM * smid) +
         (_LOCAL_MEM_SIZE_MAX * hwtid) + addr;
}

bool isspace_local(unsigned smid, unsigned hwtid, addr_t addr) {
  addr_t start = _LOCAL_GENERIC_START + (_TOTAL_LOCAL_MEM_PER_SM * smid) +
                 (_LOCAL_MEM_SIZE_MAX * hwtid);
  addr_t end = _LOCAL_GENERIC_START + (_TOTAL_LOCAL_MEM_PER_SM * smid) +
               (_LOCAL_MEM_SIZE_MAX * (hwtid + 1));
  if ((addr >= end) || (addr < start)) return false;
  return true;
}

addr_t generic_to_local(unsigned smid, unsigned hwtid, addr_t addr) {
  assert(isspace_local(smid, hwtid, addr));
  return addr - (_LOCAL_GENERIC_START + (_TOTAL_LOCAL_MEM_PER_SM * smid) +
                 (_LOCAL_MEM_SIZE_MAX * hwtid));
}


}
