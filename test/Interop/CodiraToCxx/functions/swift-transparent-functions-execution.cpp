// RUN: %empty-directory(%t)

// RUN: %target-language-frontend %S/language-transparent-functions-cxx-bridging.code -module-name Functions -clang-header-expose-decls=all-public -typecheck -verify -emit-clang-header-path %t/functions.h

// RUN: %target-interop-build-clangxx -c %s -I %t -o %t/language-functions-execution.o
// RUN: %target-interop-build-language %S/language-transparent-functions-cxx-bridging.code -o %t/language-functions-execution -Xlinker %t/language-functions-execution.o -module-name Functions -Xfrontend -entry-point-function-name -Xfrontend languageMain

// RUN: %target-codesign %t/language-functions-execution
// RUN: %target-run %t/language-functions-execution

// REQUIRES: executable_test

#include <cassert>
#include "functions.h"

int main() {
    using namespace Functions;
    
    assert(transparentPrimitiveFunc(12) == 144);

    return 0;
}
