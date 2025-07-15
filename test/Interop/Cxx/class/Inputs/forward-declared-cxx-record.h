#ifndef TEST_INTEROP_CXX_CLASS_INPUTS_FORWARD_DECLARED_CXX_RECORD_H
#define TEST_INTEROP_CXX_CLASS_INPUTS_FORWARD_DECLARED_CXX_RECORD_H

#include "forward-declared-cxx-record-used.h"

struct __attribute__((language_attr("import_reference")))
ForwardDeclaredInOtherHeader {
  ~ForwardDeclaredInOtherHeader() = delete;
};

#endif // TEST_INTEROP_CXX_CLASS_INPUTS_FORWARD_DECLARED_CXX_RECORD_H
