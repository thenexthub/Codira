// RUN: %empty-directory(%t)

// RUN: %target-language-frontend %S/consuming-parameter-in-cxx.code -module-name Init -clang-header-expose-decls=all-public -typecheck -verify -emit-clang-header-path %t/consuming.h -enable-library-evolution

// RUN: %target-interop-build-clangxx -c %S/consuming-parameter-in-cxx-execution.cpp -I %t -o %t/language-consume-execution.o
// RUN: %target-interop-build-language %S/consuming-parameter-in-cxx.code -o %t/language-consume-execution-evo -Xlinker %t/language-consume-execution.o -module-name Init -Xfrontend -entry-point-function-name -Xfrontend languageMain -enable-library-evolution

// RUN: %target-codesign %t/language-consume-execution-evo
// RUN: %target-run %t/language-consume-execution-evo | %FileCheck %S/consuming-parameter-in-cxx-execution.cpp

// REQUIRES: executable_test
