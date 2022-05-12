#include "debug.h"
#include <cstdlib>

class debug::Impl {
  int dbg_level_;
  int dbg_node_;
  int cflags_;

  Impl()
    : dbg_level_(0)
    , dbg_node_(0)
    , cflags_(0) {

    auto dbg_level = std::getenv("ISASIM_DEBUG_LEVEL");
    if (dbg_level) {
      dbg_level_ = atoi(dbg_level);
    }

    auto dbg_node = std::getenv("ISASIM_DEBUG_NODE");
    if (dbg_node) {
      dbg_node_ = atoi(dbg_node);
    }

    auto cflags = std::getenv("ISASIM_CFLAGS");
    if (cflags) {
      cflags_ = atoi(cflags);
    }
  }

  friend class debug;
};

debug::debug() {
  impl_ = new Impl();
}

debug::~debug() {
  delete impl_;
}

int debug::dbg_level() const {
  return impl_->dbg_level_;
}

int debug::dbg_node() const {
  return impl_->dbg_node_;
}

cflags debug::get_cflags() const {
  return cflags(impl_->cflags_);
}

void debug::set_cflags(cflags value) {
  impl_->cflags_ = static_cast<int>(value);
}

debug& debug::self() {
  static debug s_instance;
  return s_instance;
}

///////////////////////////////////////////////////////////////////////////////

void set_cflags(cflags flags) {
  return debug::self().set_cflags(flags);
}

cflags get_cflags() {
  return debug::self().get_cflags();
}
