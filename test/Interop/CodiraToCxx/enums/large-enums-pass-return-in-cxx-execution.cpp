// RUN: %empty-directory(%t)

// RUN: %target-language-frontend %S/large-enums-pass-return-in-cxx.code -module-name Enums -clang-header-expose-decls=all-public -typecheck -verify -emit-clang-header-path %t/enums.h

// RUN: %target-interop-build-clangxx -c %s -I %t -o %t/language-enums-execution.o
// RUN: %target-interop-build-language %S/large-enums-pass-return-in-cxx.code -o %t/language-enums-execution -Xlinker %t/language-enums-execution.o -module-name Enums -Xfrontend -entry-point-function-name -Xfrontend languageMain

// RUN: %target-codesign %t/language-enums-execution
// RUN: %target-run %t/language-enums-execution | %FileCheck %s

// REQUIRES: executable_test

#include <cassert>
#include <cstdint>
#include "enums.h"

int main() {
    using namespace Enums;

    // sizeof(generated cxx class) = 1 + max(sizeof(case) for all cases) + padding
    static_assert(sizeof(Large) == 56, "MemoryLayout<Large>.stride == 56");

    auto large = makeLarge(-1);
    printLarge(large);
    // CHECK: Large.second
    inoutLarge(large, 10);
    printLarge(large);
    // CHECK: Large.first(-1, -2, -3, -4, -5, -6)
    printLarge(passThroughLarge(large));
    // CHECK: Large.first(-1, -2, -3, -4, -5, -6)

    return 0;
}
