// RUN: rm -rf %t
// RUN: split-file %s %t
// RUN: %target-languagexx-frontend -emit-ir -I %t/Inputs -validate-tbd-against-ir=none %t/test.code | %FileCheck %s

//--- Inputs/module.modulemap
module BaseConstructor {
  header "test.h"
  requires cplusplus
}
//--- Inputs/test.h

extern void referencedSymbol();
inline void emittedIntoCodiraObject() { referencedSymbol(); }

class BaseClass {
public:
    inline BaseClass() : x(0) {}
    inline BaseClass(bool v, const BaseClass &) {
        if (v)
          emittedIntoCodiraObject();
    }

    int x;
};

class DerivedClass: public BaseClass {
    int y;
public:
    using BaseClass::BaseClass;

    inline DerivedClass(int y) : y(y) {}

    inline int test() const {
        DerivedClass m(true, *this);
        return m.x;
    }
};

//--- test.code

import BaseConstructor

public fn test() {
  let i = DerivedClass(0)
  let v = i.test()
}

// Make sure we reach clang declarations accessible from base constructors:

// CHECK: define linkonce_odr{{( dso_local)?}} void @{{_Z22emittedIntoCodiraObjectv|"\?emittedIntoCodiraObject@@YAXXZ"}}
