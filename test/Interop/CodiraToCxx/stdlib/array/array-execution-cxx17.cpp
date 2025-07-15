// RUN: %empty-directory(%t)
// RUN: split-file %S/array-execution.cpp %t

// RUN: %target-language-frontend %t/use-array.code -module-name UseArray -enable-experimental-cxx-interop -typecheck -verify -emit-clang-header-path %t/UseArray.h

// RUN: %target-interop-build-clangxx -fno-exceptions -std=gnu++17 -c %t/array-execution.cpp -I %t -o %t/language-stdlib-execution.o
// RUN: %target-build-language %t/use-array.code -o %t/language-stdlib-execution -Xlinker %t/language-stdlib-execution.o -module-name UseArray -Xfrontend -entry-point-function-name -Xfrontend languageMain
// RUN: %target-codesign %t/language-stdlib-execution
// RUN: %target-run %t/language-stdlib-execution | %FileCheck %S/array-execution.cpp

// REQUIRES: executable_test
