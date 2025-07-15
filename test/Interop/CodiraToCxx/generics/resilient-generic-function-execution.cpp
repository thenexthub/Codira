// RUN: %empty-directory(%t)

// RUN: %target-language-frontend %S/generic-function-in-cxx.code -module-name Functions -enable-library-evolution -clang-header-expose-decls=all-public -typecheck -verify -emit-clang-header-path %t/functions.h

// RUN: %target-interop-build-clangxx -std=gnu++20 -c %s -I %t -o %t/language-functions-execution.o
// RUN: %target-interop-build-language %S/generic-function-in-cxx.code -o %t/language-functions-execution -Xlinker %t/language-functions-execution.o -enable-library-evolution -module-name Functions -Xfrontend -entry-point-function-name -Xfrontend languageMain

// RUN: %target-codesign %t/language-functions-execution
// RUN: %target-run %t/language-functions-execution | %FileCheck %S/generic-function-execution.cpp

// REQUIRES: executable_test

#include "generic-function-execution.cpp"
