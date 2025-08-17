//===- GtestMatchers.cpp - AST Matchers for Gtest ---------------*- C++ -*-===//
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
// This file implements several matchers for popular gtest macros. In general,
// AST matchers cannot match calls to macros. However, we can simulate such
// matches if the macro definition has identifiable elements that themselves can
// be matched. In that case, we can match on those elements and then check that
// the match occurs within an expansion of the desired macro. The more uncommon
// the identified elements, the more efficient this process will be.
//
//===----------------------------------------------------------------------===//

#include "language/Core/ASTMatchers/GtestMatchers.h"
#include "toolchain/ADT/StringRef.h"

namespace language::Core {
namespace ast_matchers {
namespace {

enum class MacroType {
  Expect,
  Assert,
  On,
};

} // namespace

static DeclarationMatcher getComparisonDecl(GtestCmp Cmp) {
  switch (Cmp) {
  case GtestCmp::Eq:
    return cxxMethodDecl(hasName("Compare"),
                         ofClass(cxxRecordDecl(isSameOrDerivedFrom(
                             hasName("::testing::internal::EqHelper")))));
  case GtestCmp::Ne:
    return functionDecl(hasName("::testing::internal::CmpHelperNE"));
  case GtestCmp::Ge:
    return functionDecl(hasName("::testing::internal::CmpHelperGE"));
  case GtestCmp::Gt:
    return functionDecl(hasName("::testing::internal::CmpHelperGT"));
  case GtestCmp::Le:
    return functionDecl(hasName("::testing::internal::CmpHelperLE"));
  case GtestCmp::Lt:
    return functionDecl(hasName("::testing::internal::CmpHelperLT"));
  }
  toolchain_unreachable("Unhandled GtestCmp enum");
}

static toolchain::StringRef getMacroTypeName(MacroType Macro) {
  switch (Macro) {
  case MacroType::Expect:
    return "EXPECT";
  case MacroType::Assert:
    return "ASSERT";
  case MacroType::On:
    return "ON";
  }
  toolchain_unreachable("Unhandled MacroType enum");
}

static toolchain::StringRef getComparisonTypeName(GtestCmp Cmp) {
  switch (Cmp) {
  case GtestCmp::Eq:
    return "EQ";
  case GtestCmp::Ne:
    return "NE";
  case GtestCmp::Ge:
    return "GE";
  case GtestCmp::Gt:
    return "GT";
  case GtestCmp::Le:
    return "LE";
  case GtestCmp::Lt:
    return "LT";
  }
  toolchain_unreachable("Unhandled GtestCmp enum");
}

static std::string getMacroName(MacroType Macro, GtestCmp Cmp) {
  return (getMacroTypeName(Macro) + "_" + getComparisonTypeName(Cmp)).str();
}

static std::string getMacroName(MacroType Macro, toolchain::StringRef Operation) {
  return (getMacroTypeName(Macro) + "_" + Operation).str();
}

// Under the hood, ON_CALL is expanded to a call to `InternalDefaultActionSetAt`
// to set a default action spec to the underlying function mocker, while
// EXPECT_CALL is expanded to a call to `InternalExpectedAt` to set a new
// expectation spec.
static toolchain::StringRef getSpecSetterName(MacroType Macro) {
  switch (Macro) {
  case MacroType::On:
    return "InternalDefaultActionSetAt";
  case MacroType::Expect:
    return "InternalExpectedAt";
  default:
    toolchain_unreachable("Unhandled MacroType enum");
  }
  toolchain_unreachable("Unhandled MacroType enum");
}

// In general, AST matchers cannot match calls to macros. However, we can
// simulate such matches if the macro definition has identifiable elements that
// themselves can be matched. In that case, we can match on those elements and
// then check that the match occurs within an expansion of the desired
// macro. The more uncommon the identified elements, the more efficient this
// process will be.
//
// We use this approach to implement the derived matchers gtestAssert and
// gtestExpect.
static internal::BindableMatcher<Stmt>
gtestComparisonInternal(MacroType Macro, GtestCmp Cmp, StatementMatcher Left,
                        StatementMatcher Right) {
  return callExpr(isExpandedFromMacro(getMacroName(Macro, Cmp)),
                  callee(getComparisonDecl(Cmp)), hasArgument(2, Left),
                  hasArgument(3, Right));
}

static internal::BindableMatcher<Stmt>
gtestThatInternal(MacroType Macro, StatementMatcher Actual,
                  StatementMatcher Matcher) {
  return cxxOperatorCallExpr(
      isExpandedFromMacro(getMacroName(Macro, "THAT")),
      hasOverloadedOperatorName("()"), hasArgument(2, Actual),
      hasArgument(
          0, expr(hasType(classTemplateSpecializationDecl(hasName(
                      "::testing::internal::PredicateFormatterFromMatcher"))),
                  ignoringImplicit(
                      callExpr(callee(functionDecl(hasName(
                                   "::testing::internal::"
                                   "MakePredicateFormatterFromMatcher"))),
                               hasArgument(0, ignoringImplicit(Matcher)))))));
}

static internal::BindableMatcher<Stmt>
gtestCallInternal(MacroType Macro, StatementMatcher MockCall, MockArgs Args) {
  // A ON_CALL or EXPECT_CALL macro expands to different AST structures
  // depending on whether the mock method has arguments or not.
  switch (Args) {
  // For example,
  // `ON_CALL(mock, TwoParamMethod)` is expanded to
  // `mock.gmock_TwoArgsMethod(WithoutMatchers(),
  // nullptr).InternalDefaultActionSetAt(...)`.
  // EXPECT_CALL is the same except
  // that it calls `InternalExpectedAt` instead of `InternalDefaultActionSetAt`
  // in the end.
  case MockArgs::None:
    return cxxMemberCallExpr(
        isExpandedFromMacro(getMacroName(Macro, "CALL")),
        callee(functionDecl(hasName(getSpecSetterName(Macro)))),
        onImplicitObjectArgument(ignoringImplicit(MockCall)));
  // For example,
  // `ON_CALL(mock, TwoParamMethod(m1, m2))` is expanded to
  // `mock.gmock_TwoParamMethod(m1,m2)(WithoutMatchers(),
  // nullptr).InternalDefaultActionSetAt(...)`.
  // EXPECT_CALL is the same except that it calls `InternalExpectedAt` instead
  // of `InternalDefaultActionSetAt` in the end.
  case MockArgs::Some:
    return cxxMemberCallExpr(
        isExpandedFromMacro(getMacroName(Macro, "CALL")),
        callee(functionDecl(hasName(getSpecSetterName(Macro)))),
        onImplicitObjectArgument(ignoringImplicit(cxxOperatorCallExpr(
            hasOverloadedOperatorName("()"), argumentCountIs(3),
            hasArgument(0, ignoringImplicit(MockCall))))));
  }
  toolchain_unreachable("Unhandled MockArgs enum");
}

static internal::BindableMatcher<Stmt>
gtestCallInternal(MacroType Macro, StatementMatcher MockObject,
                  toolchain::StringRef MockMethodName, MockArgs Args) {
  return gtestCallInternal(
      Macro,
      cxxMemberCallExpr(
          onImplicitObjectArgument(MockObject),
          callee(functionDecl(hasName(("gmock_" + MockMethodName).str())))),
      Args);
}

internal::BindableMatcher<Stmt> gtestAssert(GtestCmp Cmp, StatementMatcher Left,
                                            StatementMatcher Right) {
  return gtestComparisonInternal(MacroType::Assert, Cmp, Left, Right);
}

internal::BindableMatcher<Stmt> gtestExpect(GtestCmp Cmp, StatementMatcher Left,
                                            StatementMatcher Right) {
  return gtestComparisonInternal(MacroType::Expect, Cmp, Left, Right);
}

internal::BindableMatcher<Stmt> gtestAssertThat(StatementMatcher Actual,
                                                StatementMatcher Matcher) {
  return gtestThatInternal(MacroType::Assert, Actual, Matcher);
}

internal::BindableMatcher<Stmt> gtestExpectThat(StatementMatcher Actual,
                                                StatementMatcher Matcher) {
  return gtestThatInternal(MacroType::Expect, Actual, Matcher);
}

internal::BindableMatcher<Stmt> gtestOnCall(StatementMatcher MockObject,
                                            toolchain::StringRef MockMethodName,
                                            MockArgs Args) {
  return gtestCallInternal(MacroType::On, MockObject, MockMethodName, Args);
}

internal::BindableMatcher<Stmt> gtestOnCall(StatementMatcher MockCall,
                                            MockArgs Args) {
  return gtestCallInternal(MacroType::On, MockCall, Args);
}

internal::BindableMatcher<Stmt> gtestExpectCall(StatementMatcher MockObject,
                                                toolchain::StringRef MockMethodName,
                                                MockArgs Args) {
  return gtestCallInternal(MacroType::Expect, MockObject, MockMethodName, Args);
}

internal::BindableMatcher<Stmt> gtestExpectCall(StatementMatcher MockCall,
                                                MockArgs Args) {
  return gtestCallInternal(MacroType::Expect, MockCall, Args);
}

} // end namespace ast_matchers
} // end namespace language::Core
