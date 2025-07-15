// RUN: %empty-directory(%t)
// RUN: split-file %s %t

// RUN: %target-language-frontend %t/use-cxx-types.code -module-name UseCxx -typecheck -verify -emit-clang-header-path %t/UseCxx.h -I %t -enable-experimental-cxx-interop -clang-header-expose-decls=all-public -disable-availability-checking

// RUN: %target-interop-build-clangxx -std=c++20 -c %t/use-language-cxx-types.cpp -I %t -o %t/language-cxx-execution.o
// RUN: %target-interop-build-language %t/use-cxx-types.code -o %t/language-cxx-execution -Xlinker %t/language-cxx-execution.o -module-name UseCxx -Xfrontend -entry-point-function-name -Xfrontend languageMain -I %t -O -Xfrontend -disable-availability-checking

// RUN: %target-codesign %t/language-cxx-execution
// RUN: %target-run %t/language-cxx-execution | %FileCheck %s

// REQUIRES: executable_test

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
    inline void testPrint() const {
        puts("testPrint");
    }
};

using NonTrivialTemplateTrivial = NonTrivialTemplate<Trivial>;

class ImmortalFRT {
public:
    int x;
} __attribute__((language_attr("import_reference")))
__attribute__((language_attr("retain:immortal")))
__attribute__((language_attr("release:immortal")));

class SharedFRT {
public:
  SharedFRT() {}
  SharedFRT(int x) : x(x) {}
  int x;
} __attribute__((language_attr("import_reference")))
__attribute__((language_attr("retain:retainShared")))
__attribute__((language_attr("release:releaseShared")));

inline void retainShared(SharedFRT *r) { puts("retainShared"); }
inline void releaseShared(SharedFRT *r) { puts("releaseShared"); }

inline SharedFRT* createSharedFRT() { return new SharedFRT(); }

//--- module.modulemap
module CxxTest {
    header "header.h"
    requires cplusplus
}

//--- use-cxx-types.code
import CxxTest

public fn consumeNonTrivial(_ x: consuming NonTrivialTemplateTrivial) -> CInt {
    print("x and y: \(x.x.x), \(x.x.y)")
    return x.x.x
}

public struct TakesNonTrivial {
    public init(_ x: NonTrivialTemplateTrivial) {
        self.prop = x
    }

    public var prop: NonTrivialTemplateTrivial
}

public fn consumeImmortalFRT(_ x: consuming ImmortalFRT) {
  print("immortal frt x \(x.x)")
}

public fn consumeSharedFRT(_ x : consuming SharedFRT) {
  print("consume shared frt x \(x.x)")
}

public fn takeSharedFRT(_ x : SharedFRT) {
  print("take shared frt x \(x.x)")
}

public fn genericConsumingFunc<T>(_ p: consuming T) {
  print("generic consuming function")
}

public fn returnSharedFRT(_ x : SharedFRT) -> SharedFRT {
  print("return shared frt x \(x.x)")
  return x
}

public fn returnSharedFRT2() -> SharedFRT {
  return createSharedFRT()
}

public struct ValueWrapper {
  let sharedFRT: SharedFRT
  public init(_ x: SharedFRT) {
    self.sharedFRT = x
  }
}

public fn consumeValueWrapper(_ x: consuming  ValueWrapper) {
  print("return shared frt x \(x.sharedFRT.x)")
}

//--- use-language-cxx-types.cpp

#include "header.h"
#include "UseCxx.h"
#include <assert.h>

int main() {
  {
    auto x = NonTrivialTemplate(Trivial(1, 2));
    UseCxx::consumeNonTrivial(x);
    puts("DoneCall");
  }
// CHECK: create NonTrivialTemplate
// CHECK-NEXT: copy NonTrivialTemplate
// CHECK-NEXT: ~NonTrivialTemplate
// CHECK-NEXT: x and y: 1, 2
// CHECK-NEXT: DoneCall
// CHECK-NEXT: ~NonTrivialTemplate
  {
    auto x = NonTrivialTemplate(Trivial(-4, 0));
    puts("call");
    auto languageVal = UseCxx::TakesNonTrivial::init(x);
    puts("DoneCall");
    languageVal.setProp(x);
  }
// CHECK-NEXT: create NonTrivialTemplate
// CHECK-NEXT: call
// CHECK-NEXT: copy NonTrivialTemplate
// CHECK-NEXT: move NonTrivialTemplate
// CHECK-NEXT: ~NonTrivialTemplate
// CHECK-NEXT: DoneCall
// CHECK-NEXT: copy NonTrivialTemplate
// CHECK-NEXT: ~NonTrivialTemplate
// CHECK-NEXT: move NonTrivialTemplate
// CHECK-NEXT: ~NonTrivialTemplate
// CHECK-NEXT: ~NonTrivialTemplate
// CHECK-NEXT: ~NonTrivialTemplate
  {
    ImmortalFRT frt;
    frt.x = 2;
    UseCxx::consumeImmortalFRT(&frt);
  }
  // CHECK-NEXT: immortal frt x 2
  {
    SharedFRT sfrt;
    sfrt.x = 2;
    UseCxx::takeSharedFRT(&sfrt);
    // CHECK-NEXT: retainShared
    // CHECK-NEXT: releaseShared
    // CHECK-NEXT: take shared frt x 2
    UseCxx::consumeSharedFRT(&sfrt);
    // CHECK-NEXT: retainShared
    // CHECK-NEXT: releaseShared
    // CHECK-NEXT: consume shared frt x 2
    SharedFRT *sfrtptr = UseCxx::returnSharedFRT(&sfrt);
    // CHECK-NEXT: retainShared
    // CHECK-NEXT: return shared frt x 2
    SharedFRT *sfrtptr2 = UseCxx::returnSharedFRT2();
    // No retain or release here.
  }
  {
    SharedFRT sfrt;
    sfrt.x = 4;
    auto wrapper = UseCxx::ValueWrapper::init(&sfrt);
    // consumeValueWrapper creates a defensive copy in the thunk.
    UseCxx::consumeValueWrapper(wrapper);
    // CHECK-NEXT: retainShared
    // CHECK-NEXT: retainShared
    // CHECK-NEXT: releaseShared
    // CHECK-NEXT: return shared frt x 4
    // CHECK-NEXT: releaseShared
  }
  {
    SharedFRT sfrt;
    sfrt.x = 4;
    auto wrapper = UseCxx::ValueWrapper::init(&sfrt);
    UseCxx::genericConsumingFunc(wrapper);
    // CHECK-NEXT: retainShared
    // CHECK-NEXT: retainShared
    // CHECK-DAG: releaseShared
    // CHECK-DAG: generic consuming function
    // CHECK-NEXT: releaseShared
  }
  puts("EndOfTest");
// CHECK-NEXT: EndOfTest
  return 0;
}
