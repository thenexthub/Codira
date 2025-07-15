// RUN: %empty-directory(%t)

// RUN: %target-language-frontend %S/language-class-inheritance-in-cxx.code -module-name Class -clang-header-expose-decls=all-public -typecheck -verify -emit-clang-header-path %t/class.h

// RUN: %target-interop-build-clangxx -c %s -I %t -o %t/language-class-execution.o
// RUN: %target-interop-build-language %S/language-class-inheritance-in-cxx.code -o %t/language-class-execution -Xlinker %t/language-class-execution.o -module-name Class -Xfrontend -entry-point-function-name -Xfrontend languageMain

// RUN: %target-codesign %t/language-class-execution
// RUN: %target-run %t/language-class-execution | %FileCheck %s

// RUN: not %target-interop-build-clangxx -c %s -I %t -o %t/language-class-execution.o -DERROR1
// RUN: not %target-interop-build-clangxx -c %s -I %t -o %t/language-class-execution.o -DERROR2

// RUN: %empty-directory(%t-evo)

// RUN: %target-language-frontend %S/language-class-inheritance-in-cxx.code -module-name Class -enable-library-evolution -clang-header-expose-decls=all-public -typecheck -verify -emit-clang-header-path %t-evo/class.h

// RUN: %target-interop-build-clangxx -c %s -I %t-evo -o %t-evo/language-class-execution.o
// RUN: %target-interop-build-language %S/language-class-inheritance-in-cxx.code -o %t-evo/language-class-execution-evo -Xlinker %t-evo/language-class-execution.o -module-name Class -enable-library-evolution -Xfrontend -entry-point-function-name -Xfrontend languageMain

// RUN: %target-codesign %t-evo/language-class-execution-evo
// RUN: %target-run %t-evo/language-class-execution-evo | %FileCheck %s

// REQUIRES: executable_test

#include <assert.h>
#include "class.h"

using namespace Class;

int passByValue(BaseClass b) { return 0; }
int passByValue(DerivedClass d) { return 1; }

int passByRef(const BaseClass &b) { return 0; }
int passByRef(const DerivedClass &d){ return 1; }

int passByRef2(const BaseClass &b) { return 0; }
int passByRef2(const DerivedClass &d){ return 1; }
int passByRef2(const DerivedDerivedClass &d){ return 2; }

extern "C" size_t language_retainCount(void * _Nonnull obj);

size_t getRetainCount(const BaseClass &languageClass) {
  return language_retainCount(language::_impl::_impl_RefCountedClass::getOpaquePointer(languageClass));
}

#ifdef ERROR1
void checkNotCompile1(BaseClass b) {
  useDerivedClass(b);
}
#endif

#ifdef ERROR2
void checkNotCompile2(SiblingDerivedClass b) {
  DerivedClass c = b;
}
#endif

int main() {
  {
    auto x = returnDerivedClass();
    useBaseClass(x);
    useDerivedClass(x);
    assert(passByValue(x) == 1);
    assert(passByRef(x) == 1);
    {
      assert(getRetainCount(x) == 1);
      BaseClass baseCopy = x;
      useBaseClass(baseCopy);
      assert(getRetainCount(x) == 2);
      baseCopy = x;
      assert(getRetainCount(x) == 2);
    }
  }
// CHECK: init BaseClass
// CHECK-NEXT: init DerivedClass
// CHECK-NEXT: useBaseClass, type=Class.DerivedClass
// CHECK-NEXT: useDerivedClass, type=Class.DerivedClass
// CHECK-NEXT: useBaseClass, type=Class.DerivedClass
// CHECK-NEXT: destroy DerivedClass
// CHECK-NEXT: destroy BaseClass

  {
    auto x = returnDerivedDerivedClass();
    useBaseClass(x);
    useDerivedClass(x);
    assert(passByValue(x) == 1);
    assert(passByRef(x) == 1);
    assert(passByRef2(x) == 2);
  }
// CHECK-NEXT: init BaseClass
// CHECK-NEXT: init DerivedClass
// CHECK-NEXT: init DerivedDerivedClass
// CHECK-NEXT: useBaseClass, type=Class.DerivedDerivedClass
// CHECK-NEXT: useDerivedClass, type=Class.DerivedDerivedClass
// CHECK-NEXT: destroy DerivedDerivedClass
// CHECK-NEXT: destroy DerivedClass
// CHECK-NEXT: destroy BaseClass

  {
    BaseClass x = returnDerivedClass();
    assert(getRetainCount(x) == 1);
    useBaseClass(x);
  }
// CHECK-NEXT: init BaseClass
// CHECK-NEXT: init DerivedClass
// CHECK-NEXT: useBaseClass, type=Class.DerivedClass
// CHECK-NEXT: destroy DerivedClass
// CHECK-NEXT: destroy BaseClass
  return 0;
}
