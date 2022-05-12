#pragma once

#include "cflags.h"

class debug {
public:

  debug();

  ~debug();

  static debug& self();

  int dbg_level() const;

  int dbg_node() const;

  cflags get_cflags() const;

  void set_cflags(cflags value);

protected:
  class Impl;
  Impl* impl_;
};
