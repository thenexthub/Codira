// RUN: %empty-directory(%t)
// RUN: split-file %s %t

// RUN: %target-language-frontend -parse-as-library %platform-module-dir/Codira.codemodule/%module-target-triple.codeinterface -enable-library-evolution -disable-objc-attr-requires-foundation-module -typecheck -module-name Codira -parse-stdlib -enable-experimental-cxx-interop -clang-header-expose-decls=has-expose-attr -emit-clang-header-path %t/Codira.h  -experimental-skip-all-function-bodies -enable-experimental-feature LifetimeDependence
// RUN: %target-language-frontend -typecheck %t/use-cxx-types.code -typecheck -module-name UseCxx -emit-clang-header-path %t/UseCxx.h -I %t -enable-experimental-cxx-interop -clang-header-expose-decls=all-public

// RUN: %target-interop-build-clangxx -std=c++20 -c %t/use-language-cxx-types.cpp -I %t -o %t/language-cxx-execution.o -g
// RUN: %target-interop-build-language %t/use-cxx-types.code -o %t/language-cxx-execution -Xlinker %t/language-cxx-execution.o -module-name UseCxx -Xfrontend -entry-point-function-name -Xfrontend languageMain -I %t -g

// RUN: %target-codesign %t/language-cxx-execution
// RUN: %target-run %t/language-cxx-execution

// REQUIRES: executable_test
// REQUIRES: language_feature_LifetimeDependence

//--- header.h
enum class SomeEnum {
  first,
  second
};

//--- module.modulemap
module CxxTest {
    header "header.h"
    requires cplusplus
}

//--- use-cxx-types.code
import CxxTest

public class SomethingCodira {
  public var someEnum: SomeEnum { get { return .first } }
  public init() {}
}

//--- use-language-cxx-types.cpp

#include "header.h"
#include "Codira.h"
#include "UseCxx.h"
#include <assert.h>

class SomethingCxx {
public:
  SomethingCxx(UseCxx::SomethingCodira languagePart): _languagePart(languagePart) { }

  SomeEnum getSomeEnum() { return _languagePart.getSomeEnum(); }

private:
  UseCxx::SomethingCodira _languagePart;
};

int main() {
  auto sp = UseCxx::SomethingCodira::init();
  auto sc = SomethingCxx(sp);
  auto e = sc.getSomeEnum();
  assert(e == SomeEnum::first);
  return 0;
}
