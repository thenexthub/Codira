/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

/**
 * @file
 *
 * AST nodes mainly includes eXtra AST nodes. These nodes are not available to Parser and libast, but visible to
 * semantic checkings. In other words, these nodes are pure semantic nodes.
 */

#ifndef CODIRA_AST_NODEX_H
#define CODIRA_AST_NODEX_H

#include "Codira/AST/Node.h"

namespace Codira::AST {
/// @IfAvailable(name: arg, lambda1, lambda2) after macro expansion (before it is a MacroExpandExpr).
struct IfAvailableExpr : public Expr {
    IfAvailableExpr(OwnedPtr<FuncArg> namedArg, OwnedPtr<LambdaExpr> lambdaArg1, OwnedPtr<LambdaExpr> lambdaArg2)
        : Expr{ASTKind::IF_AVAILABLE_EXPR}, arg{std::move(namedArg)}, lambda1{std::move(lambdaArg1)},
        lambda2{std::move(lambdaArg2)}
    {
    }

    Ptr<FuncArg> GetArg() const
    {
        return arg.get();
    }
    Ptr<LambdaExpr> GetLambda1() const
    {
        return lambda1.get();
    }
    Ptr<LambdaExpr> GetLambda2() const
    {
        return lambda2.get();
    }

private:
    OwnedPtr<FuncArg> arg;
    OwnedPtr<LambdaExpr> lambda1;
    OwnedPtr<LambdaExpr> lambda2;
};

} // namespace Codira::AST
#endif
