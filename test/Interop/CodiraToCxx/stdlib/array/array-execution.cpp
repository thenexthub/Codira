// RUN: %empty-directory(%t)
// RUN: split-file %s %t

// RUN: %target-language-frontend %t/use-array.code -module-name UseArray -enable-experimental-cxx-interop -typecheck -verify -emit-clang-header-path %t/UseArray.h

// RUN: %target-interop-build-clangxx -fno-exceptions -std=gnu++20 -c %t/array-execution.cpp -I %t -o %t/language-stdlib-execution.o
// RUN: %target-build-language %t/use-array.code -o %t/language-stdlib-execution -Xlinker %t/language-stdlib-execution.o -module-name UseArray -Xfrontend -entry-point-function-name -Xfrontend languageMain
// RUN: %target-codesign %t/language-stdlib-execution
// RUN: %target-run %t/language-stdlib-execution | %FileCheck %s

// REQUIRES: executable_test

//--- use-array.code
@_expose(Cxx)
public fn createArray(_ val: CInt) -> [CInt] {
    return [val, val]
}

@_expose(Cxx)
public fn passthroughArray(_ val: [CInt]) -> Array<CInt> {
    return val
}

@_expose(Cxx)
public fn printArray(_ val: Array<CInt>) {
    print(val)
}

public fn printStrings(_ strings: [String]) {
  for s in strings {
    print("GOT STRING '\(s)'")
  }
  print("DONE PRINTING.")
}

//--- array-execution.cpp

#include <cassert>
#include "UseArray.h"

int main() {
  using namespace language;
  {
    Array<int> val = UseArray::createArray(2);
    UseArray::printArray(UseArray::passthroughArray(val));
    int count = 2;
    for (int x: val) {
      assert(x == 2);
      --count;
    }
    assert(count == 0);
  }
// CHECK: [2, 2]
  {
    auto val = Array<int>::init();
    val.append(-11);
    UseArray::printArray(val);
    assert(val.getCount() == 1);
    assert(val.getCapacity() >= 1);
    auto zeroInt = val[0];
    assert(zeroInt == -11);
    auto firstInt = val.removeAt(0);
    assert(firstInt == -11);
    assert(val.getCount() == 0);
    UseArray::printArray(val);
  }
// CHECK-NEXT: [-11]
// CHECK-NEXT: []
  {
    auto array = language::Array<language::String>::init();
    array.append("123456789ABCDEFG");
    UseArray::printStrings(array);
  }
// CHECK-NEXT: GOT STRING '123456789ABCDEFG'
// CHECK-NEXT: DONE PRINTING
  return 0;
}
