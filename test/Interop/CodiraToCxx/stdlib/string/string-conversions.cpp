// RUN: %empty-directory(%t)
// RUN: split-file %s %t

// RUN: %target-language-frontend %t/print-string.code -module-name Stringer -enable-experimental-cxx-interop -typecheck -verify -emit-clang-header-path %t/Stringer.h

// RUN: %target-interop-build-clangxx -std=gnu++20 -c %t/string-conversions.cpp -I %t -o %t/language-stdlib-execution.o
// RUN: %target-build-language %t/print-string.code -o %t/language-stdlib-execution -Xlinker %t/language-stdlib-execution.o -module-name Stringer -Xfrontend -entry-point-function-name -Xfrontend languageMain %target-cxx-lib
// RUN: %target-codesign %t/language-stdlib-execution
// RUN: %target-run %t/language-stdlib-execution | %FileCheck %s

// REQUIRES: executable_test

//--- print-string.code

@_expose(Cxx)
public fn printString(_ s: String) {
    print("'''\(s)'''")
}

@_expose(Cxx)
public fn makeString(_ s: String, _ y: String) -> String {
    return "\(s)++\(y)"
}

//--- string-conversions.cpp

#include <cassert>
#include "Stringer.h"

int main() {
  using namespace language;
  using namespace Stringer;

  {
    auto s = String("hello world");
    printString(s);
    language::String s2 = "Hello literal";
    printString(s2);
    const char *literal = "Test literal via ptr";
    printString(literal);
    language::String s3 = nullptr;
    printString(s3);
  }
// CHECK: '''hello world'''
// CHECK-NEXT: '''Hello literal'''
// CHECK-NEXT: '''Test literal via ptr'''
// CHECK-NEXT: ''''''

  {
    std::string str = "test std::string";
    printString(str);
  }
// CHECK-NEXT: '''test std::string'''
  {
    auto s = makeString(String("start"), String("end"));
    std::string str = s;
    assert(str == "start++end");
    str += "++cxx";
    printString(String(str));
  }
// CHECK-NEXT: '''start++end++cxx'''
  return 0;
}
