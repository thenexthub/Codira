// RUN: %empty-directory(%t)

// RUN: %target-language-frontend %S/generic-struct-in-cxx.code -D KNOWN_LAYOUT -enable-library-evolution -module-name Generics -clang-header-expose-decls=all-public -typecheck -verify -emit-clang-header-path %t/generics.h

// RUN: %target-interop-build-clangxx -std=gnu++20 -c %s -I %t -o %t/language-generics-execution.o
// RUN: %target-interop-build-language %S/generic-struct-in-cxx.code -D KNOWN_LAYOUT -o %t/language-generics-execution -Xlinker %t/language-generics-execution.o -enable-library-evolution -module-name Generics -Xfrontend -entry-point-function-name -Xfrontend languageMain

// RUN: %target-codesign %t/language-generics-execution
// RUN: %target-run %t/language-generics-execution | %FileCheck %S/generic-struct-execution-known-layout-direct.cpp

// REQUIRES: executable_test

#include "generic-struct-execution-known-layout-direct.cpp"
