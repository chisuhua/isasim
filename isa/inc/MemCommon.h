#pragma once
#include "inc/ExecTypes.h"

namespace isasim {

class mem_space_t {
public:
  enum SpaceType {
    undefined=0,
    // reg,
    local_space,
    shared_space,
    // sstarr,
    // param_unclassified,
    param_kernel_space,  /* global to all threads in a kernel : read-only */
    param_local_space,   /* local to a thread : read-writable */
    const_space,
    tex_space,
    surf_space,
    global_space,
    generic_space
  // instruction
  };
  mem_space_t() {
    type_ = undefined;
    m_bank = 0;
  }
  mem_space_t(const enum SpaceType &from) {
    type_ = from;
    m_bank = 0;
  }
  bool operator==(const mem_space_t &x) const {
    return (m_bank == x.m_bank) && (type_ == x.type_);
  }
  bool operator!=(const mem_space_t &x) const { return !(*this == x); }
  bool operator<(const mem_space_t &x) const {
    if (type_ < x.type_)
      return true;
    else if (type_ > x.type_)
      return false;
    else if (m_bank < x.m_bank)
      return true;
    return false;
  }
  enum SpaceType get_type() const { return type_; }
  void set_type(enum SpaceType t) { type_ = t; }
  unsigned get_bank() const { return m_bank; }
  void set_bank(unsigned b) { m_bank = b; }
  bool is_const() const {
    return (type_ == const_space) || (type_ == param_kernel_space);
  }
  bool is_local() const {
    return (type_ == local_space) || (type_ == param_local_space);
  }
  bool is_global() const { return (type_ == global_space); }

 private:
  enum SpaceType type_;
  unsigned m_bank;  // n in ".const[n]"; note .const == .const[0] (see PTX 2.1
                    // manual, sec. 5.1.3)
};

addr_t shared_to_generic(unsigned smid, addr_t addr) ;
inline addr_t global_to_generic(addr_t addr) { return addr; };
bool isspace_shared(unsigned smid, addr_t addr) ;
bool isspace_global(addr_t addr) ;
mem_space_t::SpaceType whichspace(addr_t addr) ;
addr_t generic_to_shared(unsigned smid, addr_t addr) ;
addr_t local_to_generic(unsigned smid, unsigned hwtid, addr_t addr) ;
bool isspace_local(unsigned smid, unsigned hwtid, addr_t addr) ;
addr_t generic_to_local(unsigned smid, unsigned hwtid, addr_t addr) ;
inline addr_t generic_to_global(addr_t addr) { return addr; };

}
