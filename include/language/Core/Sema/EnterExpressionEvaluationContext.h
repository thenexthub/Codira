//===--- EnterExpressionEvaluationContext.h ---------------------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_SEMA_ENTEREXPRESSIONEVALUATIONCONTEXT_H
#define LANGUAGE_CORE_SEMA_ENTEREXPRESSIONEVALUATIONCONTEXT_H

#include "language/Core/Sema/Sema.h"

namespace language::Core {

class Decl;

/// RAII object that enters a new expression evaluation context.
class EnterExpressionEvaluationContext {
  Sema &Actions;
  bool Entered = true;

public:
  EnterExpressionEvaluationContext(
      Sema &Actions, Sema::ExpressionEvaluationContext NewContext,
      Decl *LambdaContextDecl = nullptr,
      Sema::ExpressionEvaluationContextRecord::ExpressionKind ExprContext =
          Sema::ExpressionEvaluationContextRecord::EK_Other,
      bool ShouldEnter = true)
      : Actions(Actions), Entered(ShouldEnter) {
    if (Entered)
      Actions.PushExpressionEvaluationContext(NewContext, LambdaContextDecl,
                                              ExprContext);
  }
  EnterExpressionEvaluationContext(
      Sema &Actions, Sema::ExpressionEvaluationContext NewContext,
      Sema::ReuseLambdaContextDecl_t,
      Sema::ExpressionEvaluationContextRecord::ExpressionKind ExprContext =
          Sema::ExpressionEvaluationContextRecord::EK_Other)
      : Actions(Actions) {
    Actions.PushExpressionEvaluationContext(
        NewContext, Sema::ReuseLambdaContextDecl, ExprContext);
  }

  enum InitListTag { InitList };
  EnterExpressionEvaluationContext(Sema &Actions, InitListTag,
                                   bool ShouldEnter = true)
      : Actions(Actions), Entered(false) {
    // In C++11 onwards, narrowing checks are performed on the contents of
    // braced-init-lists, even when they occur within unevaluated operands.
    // Therefore we still need to instantiate constexpr functions used in such
    // a context.
    if (ShouldEnter && Actions.isUnevaluatedContext() &&
        Actions.getLangOpts().CPlusPlus11) {
      Actions.PushExpressionEvaluationContext(
          Sema::ExpressionEvaluationContext::UnevaluatedList);
      Entered = true;
    }
  }

  ~EnterExpressionEvaluationContext() {
    if (Entered)
      Actions.PopExpressionEvaluationContext();
  }
};

/// RAII object that enters a new function expression evaluation context.
class EnterExpressionEvaluationContextForFunction {
  Sema &Actions;

public:
  EnterExpressionEvaluationContextForFunction(
      Sema &Actions, Sema::ExpressionEvaluationContext NewContext,
      FunctionDecl *FD = nullptr)
      : Actions(Actions) {
    Actions.PushExpressionEvaluationContextForFunction(NewContext, FD);
  }
  ~EnterExpressionEvaluationContextForFunction() {
    Actions.PopExpressionEvaluationContext();
  }
};

} // namespace language::Core

#endif
