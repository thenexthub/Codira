// RUN: %empty-directory(%t)

// RUN: %target-language-frontend %S/language-functions-errors.code -module-name Functions -Xcc -fno-exceptions -enable-experimental-cxx-interop -clang-header-expose-decls=has-expose-attr-or-stdlib -enable-experimental-feature GenerateBindingsForThrowingFunctionsInCXX -typecheck -verify -emit-clang-header-path %t/functions.h

// RUN: %target-interop-build-clangxx -c %s -I %t -fno-exceptions -o %t/language-expected-execution.o -DLANGUAGE_CXX_INTEROP_EXPERIMENTAL_LANGUAGE_ERROR
// RUN: %target-interop-build-language %S/language-functions-errors.code -o %t/language-expected-execution -Xlinker %t/language-expected-execution.o -module-name Functions -Xfrontend -entry-point-function-name -Xfrontend languageMain -enable-experimental-feature GenerateBindingsForThrowingFunctionsInCXX

// RUN: %target-codesign %t/language-expected-execution
// RUN: %target-run %t/language-expected-execution | %FileCheck %s

// REQUIRES: executable_test
// REQUIRES: language_feature_GenerateBindingsForThrowingFunctionsInCXX
// UNSUPPORTED: OS=windows-msvc
// UNSUPPORTED: CPU=arm64e

#include <cassert>
#include <cstdio>
#include "functions.h"

int main() {

  // Test Empty Constructor
  auto testIntEmpty = language::Expected<int>();

  // Test Error Constructor
  language::Error e;
  auto testIntError = language::Expected<int>(e);

  // Test Value Constructor
  auto testIntValue = language::Expected<int>(42);

  // Test Copy Constructor
  auto testCopy = testIntEmpty;

  // TODO: Test Move Constructor

  // Test Destructor
  auto testDestEmpty = language::Expected<int>();
  auto testDestInt = language::Expected<int>(42);
  auto testDestError = language::Expected<int>(e);
  testDestEmpty.~Expected();
  testDestInt.~Expected();
  testDestError.~Expected();

  // TODO: Test Assignment (Move)

  // Test Access to T's members (const)
  const auto exp = testIntValue;
  if (*exp.operator->() == 42)
    printf("Test Access to T's members (const)\n");

  // Test Access to T's members
  if (*testIntValue.operator->() == 42)
    printf("Test Access to T's members\n");

  // Test Reference to T's members (const)
  const auto refExp = testIntValue;
  if (*refExp == 42)
    printf("Test Reference to T's members (const)\n");

  // Test Reference to T's members
  if (*testIntValue == 42)
    printf("Test Reference to T's members\n");

  // Test bool operator
  if (testIntValue) {
    printf("Test operator bool\n");
  }

  const auto constExpectedResult = Functions::throwFunctionWithPossibleReturn(0);
  if (!constExpectedResult.has_value()) {
    auto constError = constExpectedResult.error();
    auto optionalError = constError.as<Functions::NaiveErrors>();
    assert(optionalError.isSome());
    auto valueError = optionalError.get();
    assert(valueError == Functions::NaiveErrors::returnError);
    valueError.getMessage();
  }

  auto expectedResult = Functions::throwFunctionWithPossibleReturn(0);
  if (!expectedResult.has_value()) {
    auto error = expectedResult.error();
    auto optionalError = error.as<Functions::NaiveErrors>();
    assert(optionalError.isSome());
    auto valueError = optionalError.get();
    assert(valueError == Functions::NaiveErrors::returnError);
    valueError.getMessage();
  }

  auto expectedResult2 = Functions::throwFunctionWithNeverReturn();
  if (!expectedResult2.has_value()) {
    auto error = expectedResult2.error();
    auto optionalError = error.as<Functions::NaiveErrors>();
    assert(optionalError.isSome());
    auto valueError = optionalError.get();
    assert(valueError == Functions::NaiveErrors::returnError);
    valueError.getMessage();
  }

  // Test get T's Value (const)
  const auto valueExp = testIntValue;
  if (valueExp.value() == 42)
    printf("Test get T's Value (const)\n");

  // Test get T's Value
  if (testIntValue.value() == 42)
    printf("Test get T's Value\n");

  // Test has Value
  if (testIntValue.has_value())
    printf("testIntValue has a value\n");
  if (!testIntError.has_value())
    printf("testIntError doesn't have a value\n");

  return 0;
}

// CHECK: Test Access to T's members (const)
// CHECK-NEXT: Test Access to T's members
// CHECK-NEXT: Test Reference to T's members (const)
// CHECK-NEXT: Test Reference to T's members
// CHECK-NEXT: Test operator bool
// CHECK-NEXT: passThrowFunctionWithPossibleReturn
// CHECK-NEXT: returnError
// CHECK-NEXT: passThrowFunctionWithPossibleReturn
// CHECK-NEXT: returnError
// CHECK-NEXT: passThrowFunctionWithNeverReturn
// CHECK-NEXT: returnError
// CHECK-NEXT: Test get T's Value (const)
// CHECK-NEXT: Test get T's Value
// CHECK-NEXT: testIntValue has a value
// CHECK-NEXT: testIntError doesn't have a value
