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
 * This file implements parse and check CODEMP Decl.
 */

#include "ParserImpl.h"
#include "Codira/AST/Node.h"
#include "Codira/AST/Utils.h"
#include "Codira/Basic/DiagnosticEngine.h"
#include <functional>

using namespace Codira;
using namespace Utils;
using namespace AST;

namespace {
// Check whether the decl has body or initializer.
bool HasDefault(const AST::Decl& decl)
{
    // Check on match CODEMP decl for platform member.
    if (decl.TestAttr(Attribute::PLATFORM)) {
        return true;
    }
    switch (decl.astKind) {
        case ASTKind::FUNC_DECL: {
            auto& funcDecl = StaticCast<AST::FuncDecl&>(decl);
            if (funcDecl.funcBody->body) {
                return true;
            }
            break;
        }
        case ASTKind::PROP_DECL: {
            auto& propDecl = StaticCast<AST::PropDecl&>(decl);
            if (!propDecl.getters.empty() || !propDecl.setters.empty()) {
                return true;
            }
            break;
        }
        case ASTKind::VAR_DECL: {
            auto& varDecl = StaticCast<AST::VarDecl&>(decl);
            if (varDecl.initializer) {
                return true;
            }
            break;
        }
        default: ;
    }
    return false;
}

void SetCODEMPAttrs(AST::Decl& decl)
{
    if (!decl.TestAttr(Attribute::COMMON)) {
        return;
    }
    if (HasDefault(decl)) {
        decl.EnableAttr(Attribute::COMMON_WITH_DEFAULT);
    }
}

// For codemp diag error info.
const std::unordered_map<ASTKind, std::string> KIND_TO_STR = {
    {ASTKind::VAR_DECL, "variable"},
    {ASTKind::VAR_WITH_PATTERN_DECL, "variable"},
    {ASTKind::PRIMARY_CTOR_DECL, "primary constructor"},
    {ASTKind::FUNC_DECL, "function"},
    {ASTKind::CLASS_DECL, "class"},
    {ASTKind::INTERFACE_DECL, "interface"},
    {ASTKind::STRUCT_DECL, "struct"},
    {ASTKind::ENUM_DECL, "enum"},
    {ASTKind::PROP_DECL, "property"},
    {ASTKind::EXTEND_DECL, "extend"},
    {ASTKind::ENUM_PATTERN, "enum"},
    {ASTKind::TUPLE_PATTERN, "tuple"},
    {ASTKind::WILDCARD_PATTERN, "wildcard"},
    {ASTKind::FUNC_PARAM, "parameter"},
    {ASTKind::TYPE_ALIAS_DECL, "type"},
    {ASTKind::MACRO_EXPAND_DECL,"macro_expand_decl"}
};

std::string GetDiagKind(const AST::Node& node)
{
    // init constructor
    if (node.astKind == ASTKind::FUNC_DECL && node.TestAttr(Attribute::CONSTRUCTOR)) {
        return "constructor";
    }
    auto it = KIND_TO_STR.find(node.astKind);
    if (it != KIND_TO_STR.end()) {
        return it->second;
    }
    CODEC_ASSERT(false);
    return "unknown decl";
}
}  // namespace

void MPParserImpl::SetCompileOptions(const GlobalOptions& opts)
{
    this->compileCommon = (opts.outputMode == GlobalOptions::OutputMode::CHIR);
    this->compilePlatform = (opts.commonPartCodeo != std::nullopt);
}

bool MPParserImpl::CheckCODEMPModifiers(const std::set<AST::Modifier>& modifiers) const
{
    auto currentFile = ref->currentFile;
    if (ref->HasModifier(modifiers, TokenKind::PLATFORM)) {
        if (!compilePlatform) {
            ref->diag.DiagnoseRefactor(DiagKindRefactor::parse_platform_in_non_platform_file, *currentFile);
            return false;
        }
        if (currentFile->isCommon) {
            ref->diag.DiagnoseRefactor(DiagKindRefactor::parse_common_and_platform_in_the_same_file, *currentFile);
            return false;
        }
        if (currentFile->package != nullptr) {
            currentFile->package->hasPlatform = true;
        }
        currentFile->isPlatform = true;
    }
    if (ref->HasModifier(modifiers, TokenKind::COMMON)) {
        if (!compileCommon) {
            ref->diag.DiagnoseRefactor(DiagKindRefactor::parse_common_in_non_common_file, *currentFile);
            return false;
        }
        if (currentFile->isPlatform) {
            ref->diag.DiagnoseRefactor(DiagKindRefactor::parse_common_and_platform_in_the_same_file, *currentFile);
            return false;
        }
        if (currentFile->package != nullptr) {
            currentFile->package->hasCommon = true;
        }
        currentFile->isCommon = true;
    }
    return true;
}

void MPParserImpl::CheckCODEMPDecl(AST::Decl& decl) const
{
    if (!compileCommon && !compilePlatform) {
        return;
    }
    if (!CheckCODEMPModifiersOf(decl)) {
        return;
    }
    // Enable COMMON_WITH_DEFAULT attr for func/constructor/var
    SetCODEMPAttrs(decl);

    // Check if all members have COMMON_WITH_DEFAULT for common side class, interface, struct, enum, extend
    if (decl.TestAttr(Attribute::COMMON) &&
        (decl.astKind == ASTKind::CLASS_DECL || decl.astKind == ASTKind::INTERFACE_DECL ||
            decl.astKind == ASTKind::STRUCT_DECL || decl.astKind == ASTKind::ENUM_DECL ||
            decl.astKind == ASTKind::EXTEND_DECL)) {

        bool allMembersHaveDefault = true;
        for (auto& member : decl.GetMemberDeclPtrs()) {
            if (member->TestAttr(Attribute::COMMON) && !member->TestAttr(Attribute::COMMON_WITH_DEFAULT) &&
                !member->TestAttr(Attribute::ENUM_CONSTRUCTOR)) {
                allMembersHaveDefault = false;
                break;
            }
        }

        // Set COMMON_WITH_DEFAULT on the parent declaration if all members have it
        if (allMembersHaveDefault) {
            decl.EnableAttr(Attribute::COMMON_WITH_DEFAULT);
        }
    }

    // Check sema rules
    if (decl.astKind == ASTKind::INTERFACE_DECL) {
        // Check that the member of platform interface must have the body
        CheckPlatformInterface(StaticCast<AST::InterfaceDecl&>(decl));
    } else if (decl.astKind == ASTKind::PRIMARY_CTOR_DECL) {
        auto& fn = StaticCast<AST::PrimaryCtorDecl&>(decl);
        CheckCODEMPFuncParams(fn, fn.funcBody.get());
    } else if (decl.astKind == ASTKind::FUNC_DECL) {
        auto& fn = StaticCast<AST::FuncDecl&>(decl);
        CheckCODEMPFuncParams(fn, fn.funcBody.get());
    } else if ((decl.astKind == ASTKind::CLASS_DECL || decl.astKind == ASTKind::STRUCT_DECL) && 
        decl.TestAttr(Attribute::COMMON)) {
        CheckCODEMPCtorPresence(decl);
    }
}

bool MPParserImpl::HasCODEMPModifiers(const AST::Modifier& modifier) const
{
    if (!compileCommon && !compilePlatform) {
        return false;
    }
    return (modifier.modifier == TokenKind::COMMON || modifier.modifier == TokenKind::PLATFORM);
}

static bool CheckGenericDeclFrozen(const AST::Decl& decl, DiagnosticEngine& diag)
{
    if (decl.HasAnno(AnnotationKind::FROZEN)) {
        const AST::Node* reportAt = &decl;

        auto found = std::find_if(decl.annotations.begin(), decl.annotations.end(),
            [](auto& item) { return item->kind == AnnotationKind::FROZEN; });
        if (found != decl.annotations.end()) {
            reportAt = &**found; // unwrap interator, then unwrap OwnedPtr
        }
        diag.DiagnoseRefactor(
            DiagKindRefactor::sema_common_generic_frozen_not_supported, *reportAt, decl.identifier.Val());
        return false;
    }

    return true;
}

void MPParserImpl::CheckCODEMPCtorPresence(const AST::Decl& decl) const
{      
    bool hasCtor{false};
    if (decl.astKind == ASTKind::CLASS_DECL) {
        auto& cd = StaticCast<AST::ClassDecl&>(decl);
        hasCtor = Utils::In(cd.body->decls, [&](const auto& decl) 
            { return (decl->TestAttr(Attribute::CONSTRUCTOR)) || decl->TestAttr(Attribute::PRIMARY_CONSTRUCTOR); } 
        );
    } else if  (decl.astKind == ASTKind::STRUCT_DECL) {
        auto& sd = StaticCast<AST::StructDecl&>(decl);
        hasCtor = Utils::In(sd.body->decls, [&](const auto& decl) 
            { return (decl->TestAttr(Attribute::CONSTRUCTOR)) || decl->TestAttr(Attribute::PRIMARY_CONSTRUCTOR); } 
        );
    } else  {
        return ;
    }
    if (!hasCtor) {
        const Identifier& ident = decl.identifier;
        std::string declType{(decl.astKind == ASTKind::CLASS_DECL) ? "class" : "struct"};
        ref->diag.DiagnoseRefactor(DiagKindRefactor::parse_codemp_in_common_ctor_required,
            MakeRange(ident), declType, ident.Val());
    }
}

bool MPParserImpl::CheckCODEMPModifiersOf(const AST::Decl& decl) const
{
    if (decl.IsCommonOrPlatform()) {
        auto kind = decl.TestAttr(Attribute::COMMON) ? "common" : "platform";
        // generic decl
        if (decl.TestAttr(Attribute::GENERIC)) {
            if (!CheckGenericDeclFrozen(decl, ref->diag)) {
                return false;
            }
        }
        // tuple, enum, _ pattern
        if (decl.astKind == ASTKind::VAR_WITH_PATTERN_DECL && decl.TestAttr(Attribute::COMMON)) {
            auto& varDecl = StaticCast<AST::VarWithPatternDecl&>(decl);
            auto& pattern = *(varDecl.irrefutablePattern);
            ref->diag.DiagnoseRefactor(DiagKindRefactor::parse_codemp_pattern_decl, pattern, GetDiagKind(pattern), kind);
            return false;
        }
        // static init
        if (IsStaticInitializer(decl)) {
            ref->diag.DiagnoseRefactor(DiagKindRefactor::parse_codemp_static_init, decl, kind);
            return false;
        }
    }
    bool ret = true;
    // Check whether modifiers are same between members and outer decl.
    for (auto& member : decl.GetMemberDeclPtrs()) {
        ret = CheckCODEMPModifiersBetween(*member, decl) && ret;
    }
    return ret;
}

/**
 * Checks whether the codemp modifiers of two declarations are same.
 *
 * @param inner The inner declaration, member or member param decl.
 * @param outer The outer declaration, nominal decl or primary constructor.
 * @return True if the modifiers are same.
 *
 */
bool MPParserImpl::CheckCODEMPModifiersBetween(const AST::Decl& inner, const AST::Decl& outer) const
{
    auto p0 = GetDiagKind(inner) + " " + inner.identifier.Val();
    if (inner.TestAttr(Attribute::COMMON) && !outer.TestAttr(Attribute::COMMON)) {
        DiagOuterDeclMissMatch(inner, p0, "common", GetDiagKind(outer), "common");
        return false;
    }
    if (inner.TestAttr(Attribute::PLATFORM) && !outer.TestAttr(Attribute::PLATFORM)) {
        DiagOuterDeclMissMatch(inner, p0, "platform", GetDiagKind(outer), "platform");
        return false;
    }
    return true;
}

void MPParserImpl::CheckCODEMPFuncParams(AST::Decl& decl, const Ptr<AST::FuncBody> funcBody) const
{
    if (!funcBody || funcBody->paramLists.size() != 1) {
        return;
    }
    auto& params = funcBody->paramLists[0]->params;
    for (size_t index = 0; index < params.size(); index++) {
        CheckCODEMPModifiersBetween(*params[index], decl);
    }
}

void MPParserImpl::CheckPlatformInterface(const AST::InterfaceDecl& decl) const
{
    if (!decl.TestAttr(Attribute::PLATFORM)) {
        return;
    }
    // Check that the general member of platform interface must have the body
    for (auto& member : decl.GetMemberDeclPtrs()) {
        if (!HasDefault(*member)) {
            ref->diag.DiagnoseRefactor(DiagKindRefactor::parse_platform_member_must_have_implementation,
                *member, member->identifier.Val(), decl.identifier.Val());
        }
    }
}

void MPParserImpl::DiagOuterDeclMissMatch(const AST::Node& node,
    const std::string& p0, const std::string& p1, const std::string& p2, const std::string& p3) const
{
    ref->diag.DiagnoseRefactor(DiagKindRefactor::parse_codemp_outdecl_miss_match, node, p0, p1, p2, p3);
}
