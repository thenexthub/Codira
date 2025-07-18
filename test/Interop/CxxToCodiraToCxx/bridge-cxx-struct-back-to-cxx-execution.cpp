// RUN: %empty-directory(%t)
// RUN: split-file %s %t

// RUN: %target-language-frontend -parse-as-library %platform-module-dir/Codira.codemodule/%module-target-triple.codeinterface -enable-library-evolution -disable-objc-attr-requires-foundation-module -typecheck -module-name Codira -parse-stdlib -enable-experimental-cxx-interop -clang-header-expose-decls=has-expose-attr -emit-clang-header-path %t/Codira.h  -experimental-skip-all-function-bodies -enable-experimental-feature LifetimeDependence

// RUN: %target-language-frontend -typecheck %t/use-cxx-types.code -typecheck -module-name UseCxx -emit-clang-header-path %t/UseCxx.h -I %t -enable-experimental-cxx-interop -clang-header-expose-decls=all-public

// RUN: %target-interop-build-clangxx -std=c++20 -c %t/use-language-cxx-types.cpp -I %t -o %t/language-cxx-execution.o -g
// RUN: %target-interop-build-language %t/use-cxx-types.code -o %t/language-cxx-execution -Xlinker %t/language-cxx-execution.o -module-name UseCxx -Xfrontend -entry-point-function-name -Xfrontend languageMain -I %t -g

// RUN: %target-codesign %t/language-cxx-execution
// RUN: %target-run %t/language-cxx-execution | %FileCheck %s

// REQUIRES: executable_test
// REQUIRES: language_feature_LifetimeDependence

//--- header.h

#include <stdio.h>

struct Trivial {
    int x, y;

    inline Trivial(int x, int y) : x(x), y(y) {}
};

template<class T>
struct NonTrivialTemplate {
    T x;

    inline NonTrivialTemplate(T x) : x(x) {
        puts("create NonTrivialTemplate");
    }
    inline NonTrivialTemplate(const NonTrivialTemplate<T> &other) : x(other.x) {
        puts("copy NonTrivialTemplate");
    }
    inline NonTrivialTemplate(NonTrivialTemplate<T> &&other) : x(static_cast<T &&>(other.x)) {
        puts("move NonTrivialTemplate");
    }
    inline ~NonTrivialTemplate() {
        puts("~NonTrivialTemplate");
    }
};

using NonTrivialTemplateTrivial = NonTrivialTemplate<Trivial>;

//--- module.modulemap
module CxxTest {
    header "header.h"
    requires cplusplus
}

//--- use-cxx-types.code
import CxxTest

public fn retNonTrivial(y: CInt) -> NonTrivialTemplateTrivial {
    return NonTrivialTemplateTrivial(Trivial(42, y))
}

public fn takeNonTrivial(_ x: NonTrivialTemplateTrivial) {
    print(x)
}

public fn passThroughNonTrivial(_ x: NonTrivialTemplateTrivial) -> NonTrivialTemplateTrivial {
    return x
}

public fn inoutNonTrivial(_ x: inout NonTrivialTemplateTrivial) {
    x.x.y *= 2
}

public fn retTrivial(_ x: CInt) -> Trivial {
    return Trivial(x, -x)
}

public fn takeTrivial(_ x: Trivial) {
    print(x)
}

public fn passThroughTrivial(_ x: Trivial) -> Trivial {
    return x
}

public fn inoutTrivial(_ x: inout Trivial) {
    x.x = x.y + x.x - 11
}

public fn takeGeneric<T>(_ x: T) {
    print("GENERIC", x)
}

public fn retPassThroughGeneric<T>(_ x: T) -> T {
    return x
}

public fn retArrayNonTrivial(_ x: CInt) -> [NonTrivialTemplateTrivial] {
    return [NonTrivialTemplateTrivial(Trivial(x, -x))]
}

//--- use-language-cxx-types.cpp

#include "header.h"
#include "Codira.h"
#include "UseCxx.h"
#include <assert.h>

int main() {
  {
    auto x = UseCxx::retTrivial(423421);
    assert(x.x == 423421);
    assert(x.y == -423421);
    UseCxx::takeTrivial(UseCxx::passThroughTrivial(x));
    assert(x.x == 423421);
    assert(x.y == -423421);
    UseCxx::inoutTrivial(x);
    assert(x.x == -11);
    assert(x.y == -423421);
    UseCxx::takeTrivial(x);
    UseCxx::takeGeneric(x);
    auto xPrime = UseCxx::retPassThroughGeneric(x);
    assert(xPrime.x == -11);
    assert(xPrime.y == -423421);
    UseCxx::takeTrivial(xPrime);
  }
// CHECK: Trivial(x: 423421, y: -423421)
// CHECK-NEXT: Trivial(x: -11, y: -423421)
// CHECK-NEXT: GENERIC Trivial(x: -11, y: -423421)
// CHECK-NEXT: Trivial(x: -11, y: -423421)
  {
    auto x = UseCxx::retNonTrivial(-942);
    assert(x.x.y == -942);
    assert(x.x.x == 42);
    UseCxx::takeNonTrivial(UseCxx::passThroughNonTrivial(x));
    puts("done non trivial");
    UseCxx::inoutNonTrivial(x);
    assert(x.x.y == -1884);
    assert(x.x.x == 42);
    UseCxx::takeGeneric(x);
    {
      auto xPrime = UseCxx::retPassThroughGeneric(x);
      assert(xPrime.x.y == -1884);
      assert(xPrime.x.x == 42);
      UseCxx::takeNonTrivial(xPrime);
    }
    puts("secondon non trivial");
  }
// CHECK-NEXT: create NonTrivialTemplate
// CHECK-NEXT: move NonTrivialTemplate
// CHECK-NEXT: ~NonTrivialTemplate
// CHECK-NEXT: copy NonTrivialTemplate
// CHECK-NEXT: move NonTrivialTemplate
// CHECK-NEXT: ~NonTrivialTemplate
// CHECK-NEXT: copy NonTrivialTemplate
// CHECK-NEXT: NonTrivialTemplate<Trivial>(x: __C.Trivial(x: 42, y: -942))
// CHECK-NEXT: ~NonTrivialTemplate
// CHECK-NEXT: ~NonTrivialTemplate
// CHECK-NEXT: done non trivial
// CHECK-NEXT: copy NonTrivialTemplate
// CHECK-NEXT: GENERIC NonTrivialTemplate<Trivial>(x: __C.Trivial(x: 42, y: -1884))
// CHECK-NEXT: ~NonTrivialTemplate
// CHECK-NEXT: copy NonTrivialTemplate
// CHECK-NEXT: move NonTrivialTemplate
// CHECK-NEXT: ~NonTrivialTemplate
// CHECK-NEXT: copy NonTrivialTemplate
// CHECK-NEXT: NonTrivialTemplate<Trivial>(x: __C.Trivial(x: 42, y: -1884))
// CHECK-NEXT: ~NonTrivialTemplate
// CHECK-NEXT: ~NonTrivialTemplate
// CHECK-NEXT: secondon non trivial
// CHECK-NEXT: ~NonTrivialTemplate
  {
    auto arr = UseCxx::retArrayNonTrivial(1234);
    auto val = arr[0];
    assert(val.x.x == 1234);
    assert(val.x.y == -1234);
  }
// CHECK-NEXT: create NonTrivialTemplate
// CHECK-NEXT: copy NonTrivialTemplate
// CHECK-NEXT: move NonTrivialTemplate
// CHECK-NEXT: ~NonTrivialTemplate
// CHECK-NEXT: ~NonTrivialTemplate
// CHECK-NEXT: ~NonTrivialTemplate
  puts("EndOfTest");
// CHECK-NEXT: EndOfTest
  return 0;
}
