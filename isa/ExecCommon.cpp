#include "inc/ExecTypes.h"
#include <cassert>

namespace isasim {
reg_t chop(reg_t x, unsigned to_width) {
  switch (to_width) {
    case 8:
      x.mask_and(0, 0xFF);
      break;
    case 16:
      x.mask_and(0, 0xFFFF);
      break;
    case 32:
      x.mask_and(0, 0xFFFFFFFF);
      break;
    case 64:
      break;
    default:
      assert(0);
  }
  return x;
}

reg_t zext(reg_t x, unsigned from_width) {
  return chop(x, from_width);
}

reg_t sext(reg_t x, unsigned from_width) {
  x = chop(x, from_width);
  switch (from_width) {
    case 8:
      if (x.get_bit(7)) x.mask_or(0xFFFFFFFF, 0xFFFFFF00);
      break;
    case 16:
      if (x.get_bit(15)) x.mask_or(0xFFFFFFFF, 0xFFFF0000);
      break;
    case 32:
      if (x.get_bit(31)) x.mask_or(0xFFFFFFFF, 0x00000000);
      break;
    case 64:
      break;
    default:
      assert(0);
  }
  return x;
}

}
