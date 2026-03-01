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
 * This file declares the TypeChecker related classes, which provides typecheck capabilities.
 */

#ifndef CODIRA_SEMA_TYPECHECKER_H
#define CODIRA_SEMA_TYPECHECKER_H

#include "Codira/AST/ASTContext.h"
#include "Codira/AST/Node.h"
#include "Codira/Frontend/CompilerInstance.h"

namespace Codira {
class InstCtxScope;
class TypeChecker {
public:
    explicit TypeChecker(CompilerInstance* ci);
    ~TypeChecker();

    /**
     * Using control statement "for" to finish packages' typecheck. It invokes two functions as followed.
     * @see PrepareTypeCheck
     * @see TypeCheck
     */
    void TypeCheckForPackages(const std::vector<Ptr<AST::Package>>& pkgs) const;
    void SetOverflowStrategy(const std::vector<Ptr<AST::Package>>& pkgs) const;
    /**
     * Perform autobox and recursive type resolving of enum.
     */
    void PerformDesugarAfterInstantiation(ASTContext& ctx, AST::Package& pkg) const;

    // Desugar after sema.
    void PerformDesugarAfterSema(const std::vector<Ptr<AST::Package>>& pkgs) const;

    /**
     * Synthesize the given @p expr in given @p scopeName and return the found candidate decls or types.
     * If the @p hasLocal is true, the target will be found from local scope firstly.
     * @param ctx cached sema context.
     * @param scopeName the scopeName of current position.
     * @param expr the expression waiting to found candidate decls or types.
     * @param hasLocalDecl whether the given expression is existed in the given scope.
     * @return found candidate decls or types.
     */
    Candidate SynReferenceSeparately(
        ASTContext& ctx, const std::string& scopeName, AST::Expr& expr, bool hasLocalDecl) const;
    /**
     * Remove members from @p targets that do not satisfy extensions of generic instantiated types.
     * @param baseTy specialized extended type.
     * @param targets all candidate members.
     */
    void RemoveTargetNotMeetExtendConstraint(const Ptr<AST::Ty> baseTy, std::vector<Ptr<AST::Decl>>& targets);

private:
    friend class InstCtxScope;
    /**
     * The class used to synthesize fuzzy results for LSP usage.
     */
    class Synthesizer;
    /** The class for checking enum sugar. */
    class EnumSugarChecker;
    class TypeCheckerImpl;
    std::unique_ptr<TypeCheckerImpl> impl;
}; // class TypeChecker
} // namespace Codira
#endif
