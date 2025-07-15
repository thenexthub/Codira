// RUN: %empty-directory(%t)

// RUN: %target-language-frontend %S/language-impl-defs-in-cxx.code -module-name Core -clang-header-expose-decls=all-public -typecheck -verify -emit-clang-header-path %t/core.h

// RUN: %target-interop-build-clangxx -std=c++17 -c %s -I %t -o %t/language-core-validation.o
// RUN: %target-interop-build-clangxx -std=c++20 -c %s -I %t -o %t/language-core-validation.o
// RUN: %target-interop-build-clangxx -std=c++14 -c %s -I %t -o %t/language-core-validation.o -D SHOULD_FAIL

#include <assert.h>
#include "core.h"

#define CHECK(x) (x)

#ifdef SHOULD_FAIL
#  undef CHECK
#  define CHECK(x) !(x)
#endif

int main() {
  language::_impl::ValueWitnessDestroyTy destroyFn;
  static_assert(CHECK(noexcept(destroyFn(nullptr, nullptr))), "value witness table fns are noexcept");
  return 0;
}
