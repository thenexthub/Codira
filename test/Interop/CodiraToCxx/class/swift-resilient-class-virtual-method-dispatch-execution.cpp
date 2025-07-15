// RUN: %empty-directory(%t)

// RUN: %target-language-frontend %S/language-class-virtual-method-dispatch.code -module-name Class -clang-header-expose-decls=all-public -typecheck -verify -emit-clang-header-path %t/class.h -enable-library-evolution

// RUN: %target-interop-build-clangxx -c %s -I %t -o %t/language-class-execution.o
// RUN: %target-interop-build-language %S/language-class-virtual-method-dispatch.code -o %t/language-class-execution -Xlinker %t/language-class-execution.o -module-name Class -Xfrontend -entry-point-function-name -Xfrontend languageMain -enable-library-evolution

// RUN: %target-codesign %t/language-class-execution
// RUN: %target-run %t/language-class-execution | %FileCheck %S/language-class-virtual-method-dispatch-execution.cpp

// REQUIRES: executable_test

#include "language-class-virtual-method-dispatch-execution.cpp"
