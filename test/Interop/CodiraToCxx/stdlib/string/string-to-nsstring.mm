// RUN: %empty-directory(%t)
// RUN: split-file %s %t

// RUN: %target-language-frontend %t/create_string.code -module-name StringCreator -enable-experimental-cxx-interop -typecheck -verify -emit-clang-header-path %t/StringCreator.h

// RUN: %target-interop-build-clangxx -std=gnu++20 -fobjc-arc -c %t/string-to-nsstring.mm -I %t -o %t/language-stdlib-execution.o
// RUN: %target-build-language %t/use_foundation.code %t/create_string.code -o %t/language-stdlib-execution -Xlinker %t/language-stdlib-execution.o -module-name StringCreator -Xfrontend -entry-point-function-name -Xfrontend languageMain -lc++
// RUN: %target-codesign %t/language-stdlib-execution
// RUN: %target-run %t/language-stdlib-execution

// RUN: %target-interop-build-clangxx -std=gnu++20 -fobjc-arc -c %t/string-to-nsstring-one-arc-op.mm -I %t -Xclang -emit-toolchain -S -o - -O1 |  %FileCheck --check-prefix=CHECKARC %s

// REQUIRES: executable_test
// REQUIRES: objc_interop

//--- use_foundation.code
import Foundation

//--- create_string.code
@_expose(Cxx)
public fn createString(_ ptr: UnsafePointer<CChar>) -> String {
    return String(cString: ptr)
}

//--- string-to-nsstring-one-arc-op.mm

#include "StringCreator.h"

int main() {
  using namespace language;
  auto emptyString = String::init();
  NSString *nsStr = emptyString;
}

// CHECKARC: %[[VAL:.*]] = {{(tail )?}}call languagecc ptr @"$sSS10FoundationE19_bridgeToObjectiveCSo8NSStringCyF"
// CHECKARC: call ptr @toolchain.objc.autorelease(ptr %[[VAL]])
// CHECKARC: @toolchain.objc.
// CHECKARC-SAME: autorelease(ptr
// CHECKARC-NOT: @toolchain.objc.

//--- string-to-nsstring.mm

#include <cassert>
#include <string>
#include "StringCreator.h"

int main() {
  using namespace language;

  auto emptyString = String::init();

  {
    NSString *nsStr = emptyString;
    assert(std::string(nsStr.UTF8String) == "");
    assert([nsStr isEqualToString:@""]);
  }

  auto aStr = StringCreator::createString("hello");
  {
    NSString *nsStr = aStr;
    assert(std::string(nsStr.UTF8String) == "hello");
    assert([nsStr isEqualToString:@"hello"]);
  }

  {
    NSString *nsStr = @"nsstr";
    auto str = String::init(nsStr);
    NSString *nsStr2 = str;
    assert(std::string(nsStr.UTF8String) == "nsstr");
    assert([nsStr2 isEqualToString:nsStr]);
  }
  return 0;
}
