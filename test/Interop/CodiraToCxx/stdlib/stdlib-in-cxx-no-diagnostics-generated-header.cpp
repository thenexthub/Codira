// RUN: %empty-directory(%t)
// RUN: split-file %s %t
// RUN: %target-language-frontend %t/print-string.code -module-name Stringer -enable-experimental-cxx-interop -typecheck -verify -emit-clang-header-path %t/Stringer.h

// Ensure: we don't hit any spurious warnings instantiating
// C++ standard library templates because of the generated header.

// RUN: %target-interop-build-clangxx -std=gnu++20 -fsyntax-only -c %t/test-stdlib.cpp -I %t -Wall -Werror -Werror=ignored-attributes -Wno-error=unused-command-line-argument

//--- print-string.code

public fn printString(_ s: String) {
}

//--- test-stdlib.cpp

#include "Stringer.h"
#include <iostream>

int main() {
  // Ensure that template instantiations inside C++
  // standard library in this call don't cause any new diagnostics
  // because of the generated header.
  std::cout << "test invoke std::cout\n" << std::endl;

  auto _ = language::String::init();
  return 0;
}
