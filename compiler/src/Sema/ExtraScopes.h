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
 * This file manages type check information that should be controlled by scope.
 */

#ifndef CODIRA_SEMA_EXTRASCOPES_H
#define CODIRA_SEMA_EXTRASCOPES_H

#include "Codira/Sema/TypeManager.h"
#include "Codira/AST/ASTContext.h"
#include "TypeCheckerImpl.h"
#include "Diags.h"

namespace Codira {
// scope of introducing placeholder type var
class TyVarScope {
public:
    explicit TyVarScope(TypeManager& tyMgr);
    ~TyVarScope();

    TyVarScope(const TyVarScope& other) = delete;
    TyVarScope& operator =(const TyVarScope& other) = delete;

private:
    friend class TypeManager;
    void AddTyVar(Ptr<AST::GenericsTy> tyVar);
    std::vector<Ptr<AST::GenericsTy>> tyVars;
    TypeManager& tyMgr;
    std::string scope;
};

// scope of type instantiation context
class InstCtxScope {
public:
    explicit InstCtxScope(TypeChecker::TypeCheckerImpl& typeChecker);
    ~InstCtxScope();

    /**
     * If we are using FuncA within FuncB, then current decl is FuncB,
     * referenced decl is FuncA.
     */
    // generate mapping between decl and its instantiated ty
    void SetRefDecl(const AST::Decl& decl, Ptr<AST::Ty> instTy);
    // generate all needed mappings with all available info for a CallExpr,
    // ty vars remaining to be solved are not mapped in inst map
    bool SetRefDecl(ASTContext& ctx, AST::FuncDecl& fd, AST::CallExpr& ce);
    // simply generate u2i mapping for all universal ty vars used for a CallExpr
    void SetRefDeclSimple(const AST::FuncDecl& fd, const AST::CallExpr& ce);

    InstCtxScope(const InstCtxScope& other) = delete;
    InstCtxScope& operator =(const InstCtxScope& other) = delete;

private:
    friend class TypeManager;
    SubstPack curMaps; // mapping only from current decl
    SubstPack refMaps; // mapping only from referenced decl
    SubstPack maps; // merged mapping, users should use this

    TypeManager& tyMgr;
    DiagnosticEngine& diag;
    TypeChecker::TypeCheckerImpl& typeChecker;

    bool GenerateExtendGenericTypeMapping(
        const ASTContext& ctx, const AST::FuncDecl& fd, const AST::CallExpr& ce, SubstPack& typeMapping);
    bool GenerateTypeMappingByCallContext(
        const ASTContext& ctx, const AST::FuncDecl& fd, const AST::CallExpr& ce, SubstPack& typeMapping);
    void GenerateSubstPackByTyArgs(
        SubstPack& tmaps, const std::vector<Ptr<AST::Type>>& typeArgs, const AST::Generic& generic) const;
};
}

#endif
