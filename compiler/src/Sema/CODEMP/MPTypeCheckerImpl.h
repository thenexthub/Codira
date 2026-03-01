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
 * This file declares the MPTypeCheckerImpl related classes, which provides typecheck capabilities for CODEMP.
 */
#ifndef CODIRA_SEMA_MPTYPECHECKER_IMPL_H
#define CODIRA_SEMA_MPTYPECHECKER_IMPL_H

#include "ScopeManager.h"
#include "Codira/Basic/DiagnosticEngine.h"
#include "Codira/Frontend/CompilerInstance.h"
#include "Codira/Sema/CommonTypeAlias.h"
#include "Codira/Sema/TypeManager.h"

namespace Codira {
class MPTypeCheckerImpl {
public:
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    explicit MPTypeCheckerImpl(const CompilerInstance& ci);
    // PrepareTypeCheck for CODEMP
    void PrepareTypeCheck4CODEMP(AST::Package& pkg);
    void PrepareTypeCheck4CODEMPExtension(CompilerInstance& ci, ScopeManager& scopeManager, ASTContext& ctx,
        const std::unordered_set<Ptr<AST::ExtendDecl>>& extends);
    // Precheck for CODEMP
    void PreCheck4CODEMP(const AST::Package& pkg);
    // TypeCheck for CODEMP
    void RemoveCommonCandidatesIfHasPlatform(std::vector<Ptr<AST::FuncDecl>>& candidates);
    void CheckReturnAndVariableTypes(AST::Package& pkg);
    void ValidateMatchedAnnotationsAndModifiers(AST::Package& pkg);
    void CheckMatchedFunctionReturnTypes(AST::FuncDecl& platformFunc, AST::FuncDecl& commonFunc);
    void CheckMatchedVariableTypes(AST::VarDecl& platformVar, AST::VarDecl& commonVar);
    void MatchPlatformWithCommon(AST::Package& pkg);
    void CheckNotAllowedAnnotations(AST::Package& pkg);

    static void FilterOutCommonCandidatesIfPlatformExist(std::map<Names, std::vector<Ptr<AST::FuncDecl>>>& candidates);
    void MapCODEMPGenericTypeArgs(TypeSubst& genericTyMap, const AST::Decl& commonDecl, const AST::Decl& platformDecl);
    void UpdateGenericTyInMemberFromCommon(TypeSubst& genericTyMap, Ptr<AST::Decl>& member);
    void UpdatePlatformMemberGenericTy(
        ASTContext& ctx, const std::function<std::vector<AST::Symbol*>(ASTContext&, AST::ASTKind)>& getSymsFunc);
#endif
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
private:
    // PrepareTypeCheck for CODEMP
    void MergeCODEMPNominalsExceptExtension(AST::Package& pkg);
    void MergeCODEMPExtensions(CompilerInstance& ci, ScopeManager& scopeManager, ASTContext& ctx,
        const std::unordered_set<Ptr<AST::ExtendDecl>>& extends);
    // Precheck for CODEMP
    void PreCheckCODEMPClass(const AST::ClassDecl& cls);
    // PostTypeCheck for CODEMP
    void CheckCommonExtensions(std::vector<Ptr<AST::Decl>>& commonDecls);
    void MatchCODEMPDecls(std::vector<Ptr<AST::Decl>>& commonDecls, std::vector<Ptr<AST::Decl>>& platformDecls);
    bool MatchPlatformDeclWithCommonDecls(AST::Decl& platformDecl, const std::vector<Ptr<AST::Decl>>& commonDecls);
    void CheckAbstractClassMembers(const AST::InheritableDecl& platformDecl);

    bool MatchEnumFuncTypes(const AST::FuncDecl& platform, const AST::FuncDecl& common);
    bool MatchCODEMPEnumConstructor(AST::Decl& platformDecl, AST::Decl& commonDecl);
    bool MatchCODEMPFunction(AST::FuncDecl& platformFunc, AST::FuncDecl& commonFunc);
    bool MatchCODEMPProp(AST::PropDecl& platformProp, AST::PropDecl& commonProp);
    bool MatchCODEMPVar(AST::VarDecl& platformVar, AST::VarDecl& commonVar);
    bool TryMatchVarWithPatternWithVarDecls(
        AST::VarWithPatternDecl& platformDecl, const std::vector<Ptr<AST::Decl>>& commonDecls);

    bool IsCODEMPDeclMatchable(AST::Decl& lhsDecl, AST::Decl& rhsDecl) const;
    bool MatchCODEMPDeclAttrs(
        const std::vector<AST::Attribute>& attrs, const AST::Decl& common, const AST::Decl& platform) const;
    bool MatchCODEMPDeclAnnotations(const AST::Decl& common, AST::Decl& platform) const;
    void PropagateCODEMPDeclAnnotations(const AST::Decl& common, AST::Decl& platform) const;

    bool TrySetPlatformImpl(AST::Decl& platformDecl, AST::Decl& commonDecl, const std::string& kind);
    bool MatchCommonNominalDeclWithPlatform(const AST::InheritableDecl& commonDecl);
    void CheckCommonSpecificGenericMatch(const AST::Decl& platformDecl, const AST::Decl& commonDecl);

public:
    /**
     * @brief Get inherited types, replacing common types with platform implementations when compiling platform code
     *
     * This function processes the inherited types list and replaces any common types
     * with their corresponding platform implementations when compiling platform code.
     * The replacement only occurs if the current declaration has a platform implementation
     * available.
     *
     * @param inheritedTypes The list of inherited types to process
     * @param hasPlatformImpl Whether the current declaration has a platform implementation
     * @param compilePlatform Whether we are currently compiling platform code
     */
    static void GetInheritedTypesWithPlatformImpl(
        std::vector<OwnedPtr<AST::Type>>& inheritedTypes, bool hasPlatformImpl, bool compilePlatform);

private:
    TypeManager& typeManager;
    DiagnosticEngine& diag;
    bool compileCommon{false};   // true if compiling common part
    bool compilePlatform{false}; // true if compiling platform part
#endif
};
} // namespace Codira
#endif // CODIRA_SEMA_MPTYPECHECKER_IMPL_H
