// RUN: %empty-directory(%t)

// RUN: %target-language-frontend %S/consuming-parameter-in-cxx.code -module-name Init -clang-header-expose-decls=all-public -typecheck -verify -emit-clang-header-path %t/consuming.h

// RUN: %target-interop-build-clangxx -c %s -I %t -o %t/language-consume-execution.o
// RUN: %target-interop-build-language %S/consuming-parameter-in-cxx.code -o %t/language-consume-execution -Xlinker %t/language-consume-execution.o -module-name Init -Xfrontend -entry-point-function-name -Xfrontend languageMain

// RUN: %target-codesign %t/language-consume-execution
// RUN: %target-run %t/language-consume-execution | %FileCheck %s

// REQUIRES: executable_test

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

size_t allocCount = 0;
size_t totalAllocs = 0;

void * _Nonnull trackedAlloc(size_t size, size_t align) {
    ++allocCount;
    ++totalAllocs;
    return malloc(size);
}
void trackedFree(void *_Nonnull p) {
    --allocCount;
    free(p);
}

#define LANGUAGE_CXX_INTEROPERABILITY_OVERRIDE_OPAQUE_STORAGE_alloc trackedAlloc
#define LANGUAGE_CXX_INTEROPERABILITY_OVERRIDE_OPAQUE_STORAGE_free  trackedFree

#include "consuming.h"

extern "C" size_t language_retainCount(void * _Nonnull obj);

size_t getRetainCount(const Init::AKlass & languageClass) {
  void *p = language::_impl::_impl_RefCountedClass::getOpaquePointer(languageClass);
  return language_retainCount(p);
}

int main() {
  using namespace Init;

  {
    auto k = AKlass::init();
    k.takeKlass();
    assert(getRetainCount(k) == 1);
  }
// CHECK: destroy AKlass
  {
    auto k = AKlass::init();
    auto x = createSmallStructNonTrivial(k);
    auto x2 = InitFromSmall::init(x);
    assert(getRetainCount(k) == 2);
  }
// CHECK-NEXT: destroy AKlass
  {
    auto k = AKlass::init();
    auto x = createSmallStructNonTrivial(k);
    auto c = TheGenericContainer<SmallStructNonTrivial>::init(x);
    assert(getRetainCount(k) == 3);
    c.takeGenericContainer();
    assert(getRetainCount(k) == 3);
  }
// CHECK-NEXT: destroy AKlass
  // verify that all of the opaque buffers are freed.
  assert(allocCount == 0);
  assert(totalAllocs != 0);
  return 0;
}
