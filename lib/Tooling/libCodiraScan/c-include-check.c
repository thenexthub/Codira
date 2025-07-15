// This file serves as a soundness check to ensure that the header is parseable
// from C and that no C++ code has sneaked in.

#include "language-c/DependencyScan/DependencyScan.h"
typedef languagescan_module_details_t _check_module_details_exists;
