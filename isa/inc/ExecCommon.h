#include "inc/ExecTypes.h"

namespace isasim {
reg_t chop(reg_t x, unsigned to_width);
reg_t zext(reg_t x, unsigned from_width);
reg_t sext(reg_t x, unsigned from_width);
}
