// RUN: %empty-directory(%t)
// RUN: split-file %s %t
// RUN: %target-languagexx-frontend -emit-module %t/Inputs/test.code -module-name TestA -I %t/Inputs -o %t/test-part.codemodule
// RUN: %target-languagexx-frontend -merge-modules -emit-module %t/test-part.codemodule -module-name TestA -I %t/Inputs -o %t/TestA.codemodule -sil-verify-none
// RUN: %target-language-ide-test -print-module -module-to-print=TestA -I %t/ -source-filename=test -enable-experimental-cxx-interop | %FileCheck %s

//--- Inputs/module.modulemap
module CxxHeader {
    header "header.h"
    requires cplusplus
}

//--- Inputs/header.h

#include <stddef.h>

namespace std2 {

template<class T>
class vec {
public:
  using Element = T;
  using RawIterator = const T  * _Nonnull;
  vec() {}
  vec(const vec<T> &other) : items(other.items) { }
  ~vec() {}

  T * _Nonnull begin() {
    return items;
  }
  T * _Nonnull end() {
    return items + 10;
  }
  RawIterator begin() const {
    return items;
  }
  RawIterator end() const {
    return items + 10;
  }
  size_t size() const {
    return 10;
  }

private:
  T items[10];
};

} // namespace std2

namespace ns2 {

class App {
public:

  inline std2::vec<App> getApps() const {
    return {};
  }
  int x  = 0;
};

} // namespace ns2

using vec2Apps = std2::vec<ns2::App>;

//--- Inputs/test.code

import Cxx
import CxxHeader

extension vec2Apps : CxxSequence {
}

public fn testFunction() -> [Int] {
  let applications = ns2.App().getApps()
  return applications.map { Int($0.x) }
}

// CHECK: fn testFunction() -> [Int]
