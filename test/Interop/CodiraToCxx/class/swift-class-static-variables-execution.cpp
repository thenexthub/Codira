// RUN: %empty-directory(%t)

// RUN: %target-language-frontend %S/language-class-static-variables.code -module-name Class -clang-header-expose-decls=all-public -typecheck -verify -emit-clang-header-path %t/class.h

// RUN: %target-interop-build-clangxx -c %s -I %t -o %t/language-class-execution.o
// RUN: %target-interop-build-language %S/language-class-static-variables.code -o %t/language-class-execution -Xlinker %t/language-class-execution.o -module-name Class -Xfrontend -entry-point-function-name -Xfrontend languageMain

// RUN: %target-codesign %t/language-class-execution
// RUN: %target-run %t/language-class-execution

// REQUIRES: executable_test


#include "class.h"
#include <assert.h>
#include <cstdio>

using namespace Class;

int main() {
    auto x = FileUtilities::getShared();
    assert(x.getField() == 42);
}
