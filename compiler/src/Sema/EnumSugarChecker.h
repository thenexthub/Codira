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
 * This file declares the EnumSugarChecker class.
 */

#ifndef CODIRA_SEMA_ENUMSUGARCHECKER_H
#define CODIRA_SEMA_ENUMSUGARCHECKER_H

#include <vector>

#include "EnumSugarTargetsFinder.h"

#include "TypeCheckerImpl.h"
#include "Codira/AST/ASTContext.h"
#include "Codira/AST/Node.h"

namespace Codira {
class TypeChecker::EnumSugarChecker {
public:
    EnumSugarChecker(TypeCheckerImpl& typeChecker, ASTContext& ctx, AST::RefExpr& re)
        : typeChecker(typeChecker),
          ctx(ctx),
          refExpr(re),
          enumSugarTargetsFinder(typeChecker.typeManager, ctx, re)
    {
    }
    /**
     * According to found targets, try to resolve enum sugar related targets.
     * @return the first of pair is true when error detected, and the second of pair is resolved targets.
     */
    std::pair<bool, std::vector<Ptr<AST::Decl>>> Resolve();

private:
    void CheckGenericEnumSugarWithTypeArgs(Ptr<AST::EnumDecl> ed);
    void CheckGenericEnumSugarWithoutTypeArgs(Ptr<const AST::EnumDecl> ed);
    Ptr<AST::Decl> CheckEnumSugarTargets();
    bool CheckVarDeclTargets();
    std::vector<Ptr<AST::Decl>> enumSugarTargets;
    TypeCheckerImpl& typeChecker;
    ASTContext& ctx;
    AST::RefExpr& refExpr;
    EnumSugarTargetsFinder enumSugarTargetsFinder;
};
} // namespace Codira

#endif
