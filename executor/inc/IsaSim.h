#include "../../libcuda/abstract_hardware_model.h"

/*!
 * This class functionally executes a kernel. It uses the basic data structures and procedures in core_t
 */
class IsaSim
{
public:
    void launch(libcuda::kernel_info_t &kernel);
};

