//===- GtestMatchers.h - AST Matchers for GTest -----------------*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//
//
//  This file implements matchers specific to structures in the Googletest
//  (gtest) framework.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_ASTMATCHERS_GTESTMATCHERS_H
#define LANGUAGE_CORE_ASTMATCHERS_GTESTMATCHERS_H

#include "language/Core/AST/Stmt.h"
#include "language/Core/ASTMatchers/ASTMatchers.h"
#include "toolchain/ADT/StringRef.h"

namespace language::Core {
namespace ast_matchers {

/// Gtest's comparison operations.
enum class GtestCmp {
  Eq,
  Ne,
  Ge,
  Gt,
  Le,
  Lt,
};

/// This enum indicates whether the mock method in the matched ON_CALL or
/// EXPECT_CALL macro has arguments. For example, `None` can be used to match
/// `ON_CALL(mock, TwoParamMethod)` whereas `Some` can be used to match
/// `ON_CALL(mock, TwoParamMethod(m1, m2))`.
enum class MockArgs {
  None,
  Some,
};

/// Matcher for gtest's ASSERT comparison macros including ASSERT_EQ, ASSERT_NE,
/// ASSERT_GE, ASSERT_GT, ASSERT_LE and ASSERT_LT.
internal::BindableMatcher<Stmt> gtestAssert(GtestCmp Cmp, StatementMatcher Left,
                                            StatementMatcher Right);

/// Matcher for gtest's ASSERT_THAT macro.
internal::BindableMatcher<Stmt> gtestAssertThat(StatementMatcher Actual,
                                                StatementMatcher Matcher);

/// Matcher for gtest's EXPECT comparison macros including EXPECT_EQ, EXPECT_NE,
/// EXPECT_GE, EXPECT_GT, EXPECT_LE and EXPECT_LT.
internal::BindableMatcher<Stmt> gtestExpect(GtestCmp Cmp, StatementMatcher Left,
                                            StatementMatcher Right);

/// Matcher for gtest's EXPECT_THAT macro.
internal::BindableMatcher<Stmt> gtestExpectThat(StatementMatcher Actual,
                                                StatementMatcher Matcher);

/// Matcher for gtest's EXPECT_CALL macro. `MockObject` matches the mock
/// object and `MockMethodName` is the name of the method invoked on the mock
/// object.
internal::BindableMatcher<Stmt> gtestExpectCall(StatementMatcher MockObject,
                                                toolchain::StringRef MockMethodName,
                                                MockArgs Args);

/// Matcher for gtest's EXPECT_CALL macro. `MockCall` matches the whole mock
/// member method call. This API is more flexible but requires more knowledge of
/// the AST structure of EXPECT_CALL macros.
internal::BindableMatcher<Stmt> gtestExpectCall(StatementMatcher MockCall,
                                                MockArgs Args);

/// Like the first `gtestExpectCall` overload but for `ON_CALL`.
internal::BindableMatcher<Stmt> gtestOnCall(StatementMatcher MockObject,
                                            toolchain::StringRef MockMethodName,
                                            MockArgs Args);

/// Like the second `gtestExpectCall` overload but for `ON_CALL`.
internal::BindableMatcher<Stmt> gtestOnCall(StatementMatcher MockCall,
                                            MockArgs Args);

} // namespace ast_matchers
} // namespace language::Core

#endif // LANGUAGE_CORE_ASTMATCHERS_GTESTMATCHERS_H

