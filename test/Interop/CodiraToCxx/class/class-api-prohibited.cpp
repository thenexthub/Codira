// RUN: %empty-directory(%t)

// RUN: %target-language-frontend %S/language-class-in-cxx.code -module-name Class -clang-header-expose-decls=all-public -typecheck -verify -emit-clang-header-path %t/class.h

// RUN: not %target-interop-build-clangxx -c %s -I %t -o %t/language-class-execution.o

#include "class.h"

void test(void * _Nonnull p) {
  // Prohibited to construct class reference directly from opaque pointer.
  Class::ClassWithIntField x(p);
}
