// RUN: %empty-directory(%t)
// RUN: split-file %s %t

// RUN: %target-language-frontend %t/use-span.code -module-name UseSpan -typecheck -verify -emit-clang-header-path %t/UseSpan.h -I %t -enable-experimental-cxx-interop -Xcc -Xclang -Xcc -fmodule-format=raw -Xcc -std=c++20 -clang-header-expose-decls=all-public

// RUN: %target-interop-build-clangxx -std=c++20 -c %t/use-span.cpp -I %t -o %t/language-cxx-execution.o
// RUN: %target-interop-build-language %t/use-span.code -o %t/language-cxx-execution -Xlinker %t/language-cxx-execution.o -module-name UseSpan -Xfrontend -entry-point-function-name -Xfrontend languageMain -I %t -O -Xcc --std=c++20

// RUN: %target-codesign %t/language-cxx-execution
// RUN: %target-run %t/language-cxx-execution | %FileCheck %s

// FIXME language-ci linux tests do not support std::span
// UNSUPPORTED: OS=linux-gnu

// REQUIRES: executable_test

//--- header.h
#include <string>
#include <span>

using Span = std::span<int>;
using SpanOfString = std::span<std::string>;

namespace ns {
  using SpanOfConstUInt8 = std::span<const uint8_t>;
}

static int staticArr[] = {1, 2, 3};
static Span staticSpan = {staticArr};

//--- module.modulemap
module CxxTest {
  header "header.h"
  requires cplusplus
  export *
}

//--- use-span.code
import CxxTest

public fn createEmptySpan() -> Span {
  return Span()
}

public fn printSpan() {
  print("{\(staticSpan[0]), \(staticSpan[1]), \(staticSpan[2])}")
}

public fn printSpan(_ sp: Span) {
  print("{\(sp[0]), \(sp[1]), \(sp[2])}")
}

public fn printSpanOfString(_ sp: SpanOfString) {
  print("{\(sp[0]), \(sp[1]), \(sp[2])}")
}

public fn passthroughSpan(_ sp: Span) -> Span {
  return sp;
}

public fn changeSpan(_ sp: inout Span) {
  sp[0] = 0;
}

public fn mapSpan(_ sp: Span) {
  let result = sp.map { $0 + 3 }
  print(result)
}

public fn receiveArr(_ arr: inout [Int32]) -> Span {
  arr.withUnsafeMutableBufferPointer { ubpointer in 
    return Span(ubpointer);
  }
}

public typealias SpanConstUInt8 = ns.SpanOfConstUInt8

public fn receiveSpanAlias(_ sp1: SpanConstUInt8, _ sp2: SpanConstUInt8) {
}

//--- use-span.cpp
#include <cassert>
#include "header.h"
#include "UseSpan.h"


int main() {
  using namespace language;  
  {
    Span emptySpan = UseSpan::createEmptySpan();
    assert(emptySpan.empty());
    assert(emptySpan.size() == 0);

    int arr[] = {4, 5, 6};
    Span sp = {arr};
    UseSpan::printSpan();
    UseSpan::printSpan(sp);
    assert(staticSpan[0] == 1);
    assert(sp[0] == 4);
  }
  // CHECK: {1, 2, 3}
  // CHECK-NEXT: {4, 5, 6}
  {
    std::string strArr[] = {"", "a", "abc"};
    SpanOfString strSp = {strArr};
    UseSpan::printSpanOfString(strSp);
  }
  // CHECK-NEXT: {, a, abc}
  { 
    int arr[] = {4, 5, 6};
    Span sp = {arr};
    UseSpan::printSpan(UseSpan::passthroughSpan(sp));
    UseSpan::changeSpan(sp);
    UseSpan::printSpan(sp);
  }
  // CHECK-NEXT: {4, 5, 6}
  // CHECK-NEXT: {0, 5, 6}
  {
    int arr[] = {4, 5, 6};
    Span sp = {arr};
    UseSpan::mapSpan(sp);
  }
  // CHECK-NEXT: [7, 8, 9] 
  {
    Array<int> arr = Array<int>::init();
    arr.append(2);
    arr.append(4);
    arr.append(6);
    Span sp = UseSpan::receiveArr(arr);
    UseSpan::printSpan(sp);
  }
  // CHECK-NEXT: {2, 4, 6}
  return 0;
}
