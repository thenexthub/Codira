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
 * This file implements the sema rules of CODEMP feature.
 */

#include "MPTypeCheckerImpl.h"

#include "../Sema/InheritanceChecker/StructInheritanceChecker.h"
#include "Collector.h"
#include "Diags.h"
#include "TypeCheckUtil.h"
#include "Codira/AST/AttributePack.h"
#include "Codira/AST/Clone.h"
#include "Codira/AST/Node.h"
#include "Codira/AST/Types.h"
#include "Codira/AST/Walker.h"
#include "Codira/Basic/DiagnosticEngine.h"
#include "Codira/Modules/ImportManager.h"
#include "Codira/Option/Option.h"
#include "Codira/Sema/CommonTypeAlias.h"
#include "Codira/Sema/TypeManager.h"
#include "Codira/Utils/CastingTemplate.h"
#include "Codira/Utils/CheckUtils.h"
#include <algorithm>
#include <optional>

using namespace Codira;
using namespace AST;
using namespace TypeCheckUtil;

MPTypeCheckerImpl::MPTypeCheckerImpl(const CompilerInstance& ci)
    : typeManager(*ci.typeManager), diag(ci.diag),
      compileCommon(ci.invocation.globalOptions.outputMode == GlobalOptions::OutputMode::CHIR),
      compilePlatform(ci.invocation.globalOptions.commonPartCodeo != std::nullopt)
{
}

namespace {
std::string GetTypeNameFromTy(
    const Ptr<Ty>& ty, bool forCODEMPMatch, const std::unordered_map<Ptr<Ty>, unsigned>& genericIdx)
{
    CODEC_ASSERT(Ty::IsTyCorrect(ty.get()));
    if (ty->IsPrimitive()) {
        return ty->String();
    } else if (ty->IsGeneric()) {
        if (forCODEMPMatch) {
            auto it = genericIdx.find(ty);
            CODEC_ASSERT(it != genericIdx.end());
            return "T" + std::to_string(it->second);
        } else {
            return ty->name;
        }
    } else {
        std::string str;
        for (auto it = ty->typeArgs.begin(); it != ty->typeArgs.end(); it++) {
            if (it == ty->typeArgs.begin()) {
                str += GetTypeNameFromTy(*it, forCODEMPMatch, genericIdx);
            } else {
                str += "," + GetTypeNameFromTy(*it, forCODEMPMatch, genericIdx);
            }
        }
        if (!ty->typeArgs.empty()) {
            str = "<" + str + ">";
        }
        if (forCODEMPMatch) {
            auto decl = Ty::GetDeclOfTy(ty);
            CODEC_NULLPTR_CHECK(decl);
            return decl->fullPackageName + "." + ty->name + str;
        } else {
            return ty->name + str;
        }
    }
}

std::string CalculatedGenericConstraintsStr(const std::vector<OwnedPtr<GenericConstraint>>& genericConstraints,
    const std::unordered_map<Ptr<Ty>, unsigned>& genericIdx)
{
    std::string ret;
    if (genericConstraints.empty()) {
        return ret;
    }

    std::set<std::string> gcStrs;
    for (auto& genericConstraint : genericConstraints) {
        std::set<std::string> ubStrs;
        for (auto& upperBound : genericConstraint->upperBounds) {
            ubStrs.emplace(GetTypeNameFromTy(upperBound->ty, true, genericIdx));
        }
        CODEC_ASSERT(!ubStrs.empty());

        auto gcStr = "[" + GetTypeNameFromTy(genericConstraint->type->ty, true, genericIdx) + "<:";
        std::for_each(ubStrs.begin(), ubStrs.end(), [&gcStr](const std::string& s) { gcStr += s + ","; });
        gcStr += "]";
        gcStrs.emplace(gcStr);
    }
    CODEC_ASSERT(!gcStrs.empty());

    std::for_each(gcStrs.begin(), gcStrs.end(), [&ret](const std::string& s) { ret += s; });
    return ret;
}

// Diag report
void DiagNotMatchedDecl(DiagnosticEngine &diag, const AST::Decl& decl, const std::string& p0, const std::string& p2)
{
    std::string info;
    if (decl.astKind == ASTKind::FUNC_DECL && decl.TestAttr(Attribute::CONSTRUCTOR)) {
        info = "constructor";
    } else if (decl.TestAttr(Attribute::ENUM_CONSTRUCTOR)) {
        if (decl.outerDecl) {
            info = "enum '" + decl.outerDecl->identifier.GetRawText() + "' constructor" +
                " '" + decl.identifier.GetRawText() + "'";
        }
    } else {
        if (decl.astKind == ASTKind::VAR_WITH_PATTERN_DECL) {
            info = "variable with pattern";
        } else if (decl.astKind == ASTKind::EXTEND_DECL) {
            auto& ed = StaticCast<const ExtendDecl&>(decl);
            info = DeclKindToString(decl) + " '" +
                (Ty::IsTyCorrect(ed.ty.get()) ? GetTypeNameFromTy(ed.extendedType->ty.get(), false, {})
                                              : decl.identifier.GetRawText()) +
                "'";
        } else {
            info = DeclKindToString(decl) + " '" + decl.identifier.GetRawText() + "'";
        }
    }
    diag.DiagnoseRefactor(DiagKindRefactor::sema_not_matched, decl, p0, info, p2);
}

inline void DiagNotMatchedCommonDecl(DiagnosticEngine &diag, const AST::Decl& decl)
{
    DiagNotMatchedDecl(diag, decl, "platform", "common");
}

inline void DiagNotMatchedPlatformDecl(DiagnosticEngine &diag, const AST::Decl& decl)
{
    DiagNotMatchedDecl(diag, decl, "common", "platform");
}

inline void DiagNotMatchedSuperType(DiagnosticEngine &diag, const AST::Decl& decl)
{
    diag.DiagnoseRefactor(DiagKindRefactor::sema_platform_has_different_super_type, decl, DeclKindToString(decl));
}

// Match nominative decl.
bool MatchNominativeDecl(DiagnosticEngine &diag, Decl &commonDecl, Decl& platformDecl)
{
    if (commonDecl.astKind != platformDecl.astKind) {
        diag.DiagnoseRefactor(DiagKindRefactor::platform_has_different_kind,
            platformDecl, DeclKindToString(platformDecl), DeclKindToString(commonDecl));
        return false;
    }

    if (auto commonEnumDecl = DynamicCast<EnumDecl *>(&commonDecl)) {
        auto platformEnumDecl = DynamicCast<EnumDecl *>(&platformDecl);
        CODEC_NULLPTR_CHECK(platformEnumDecl);
        if (commonEnumDecl->hasEllipsis) {
            platformDecl.EnableAttr(Attribute::COMMON_NON_EXHAUSTIVE);
        } else if (platformEnumDecl->hasEllipsis) {
            diag.DiagnoseRefactor(DiagKindRefactor::common_non_exaustive_platfrom_exaustive_mismatch,
                platformDecl, DeclKindToString(commonDecl), DeclKindToString(platformDecl));
        }
    }
    return true;
}

// Update the dependencies: common -> platform one.
void UpdateVarDependencies(const Decl& decl)
{
    for (auto& dep : decl.dependencies) {
        if (dep->platformImplementation) {
            dep = dep->platformImplementation;
        }
    }
}

// Check common instance member without initializer not match with platform one.
void Check4CommonInstanceVar(DiagnosticEngine& diag, const Decl& platformDecl)
{
    auto platformDecls = platformDecl.GetMemberDeclPtrs();
    for (auto decl : platformDecls) {
        if (decl->astKind == ASTKind::VAR_DECL && !decl->IsStaticOrGlobal() && decl->TestAttr(Attribute::COMMON)) {
            if (!decl->TestAttr(Attribute::COMMON_WITH_DEFAULT) && decl->platformImplementation == nullptr) {
                DiagNotMatchedPlatformDecl(diag, *decl);
            }
        }
    }
}

// Merge common nominative decl into platform one, do some match for fields.
void MergeCommonIntoPlatform(DiagnosticEngine& diag, Decl& commonDecl, Decl& platformDecl)
{
    CODEC_ASSERT(commonDecl.TestAttr(AST::Attribute::COMMON));
    CODEC_ASSERT(platformDecl.TestAttr(AST::Attribute::PLATFORM));
    if (!MatchNominativeDecl(diag, commonDecl, platformDecl)) {
        return;
    }
    auto& commonDecls = commonDecl.GetMemberDecls();
    auto& platformDecls = platformDecl.GetMemberDecls();
    std::vector<OwnedPtr<Decl>> mergedDecls;
    mergedDecls.reserve(commonDecls.size() + platformDecls.size());
    // Common instance member vars (including common member params)
    std::unordered_map<std::string, std::size_t> commonVariablesIds;
    // General member instance member vars from member params
    std::unordered_map<std::string, std::size_t> memberParamIds;
    // Collect candidates to be matched in common decl
    for (auto& commonDeclT : commonDecls) {
        auto newDecl = std::move(commonDeclT);
        newDecl->outerDecl = &platformDecl;
        newDecl->doNotExport = false;

        auto id = mergedDecls.size();
        if (newDecl->astKind == ASTKind::VAR_DECL && !newDecl->IsStaticOrGlobal()) {
            auto varDecl = StaticCast<AST::VarDecl>(newDecl.get());
            if (varDecl->TestAttr(Attribute::COMMON)) {
                commonVariablesIds.emplace(varDecl->identifier, id);
            } else if (varDecl->isMemberParam) {
                memberParamIds.emplace(varDecl->identifier, id);
            }
        }
        mergedDecls.emplace_back(std::move(newDecl));
    }
    // Match instance member and merge into platform decl.
    for (auto& platformDeclT : platformDecls) {
        if (platformDeclT->astKind == ASTKind::VAR_DECL && !platformDeclT->IsStaticOrGlobal()) {
            auto varDecl = StaticCast<AST::VarDecl>(platformDeclT.get());
            auto id = varDecl->identifier;
            if (platformDeclT->TestAttr(Attribute::PLATFORM)) {
                auto commonDeclIt = commonVariablesIds.find(id);
                if (commonDeclIt != commonVariablesIds.end()) {
                    // match
                    auto& commonDeclT = mergedDecls[commonDeclIt->second];
                    commonDeclT->platformImplementation = platformDeclT;
                    std::swap(platformDeclT, commonDeclT);
                    continue;
                } else {
                    DiagNotMatchedCommonDecl(diag, *varDecl);
                }
            } else if (varDecl->isMemberParam) {
                if (memberParamIds.find(id) != memberParamIds.end()) {
                    // General platform member params will merge into common if exist.
                    continue;
                }
            }
        }
        // Merge platform members.
        mergedDecls.emplace_back(std::move(platformDeclT));
    }
    std::swap(platformDecls, mergedDecls);
    // all the rest declarations need to be saved, because of, at least initializers of common
    // variables need to be analyzed.
    commonDecls.clear();
    for (auto& decl : mergedDecls) {
        if (decl) {
            commonDecls.emplace_back(std::move(decl));
        }
    }

    for (auto& decl : platformDecls) {
        UpdateVarDependencies(*decl);
    }
    // Check common member without initializer not match with platform one.
    Check4CommonInstanceVar(diag, platformDecl);

    commonDecl.doNotExport = true;
    commonDecl.platformImplementation = &platformDecl;
}
}

// PrepareTypeCheck for CODEMP
void MPTypeCheckerImpl::PrepareTypeCheck4CODEMP(Package& pkg)
{
    if (!compilePlatform) {
        return;
    }
    // platform package part
    MergeCODEMPNominalsExceptExtension(pkg);
}

void MPTypeCheckerImpl::MergeCODEMPNominalsExceptExtension(Package& pkg)
{
    std::unordered_map<std::string, Ptr<Decl>> matchedDecls;
    Walker walkerPackage(&pkg, [this, &matchedDecls](const Ptr<Node>& node) -> VisitAction {
        if (!node->IsDecl()) {
            return VisitAction::WALK_CHILDREN;
        }
        auto decl = StaticCast<Decl>(node);
        if (decl->IsNominalDecl() && decl->astKind != ASTKind::EXTEND_DECL) {
            if (auto it = matchedDecls.find(decl->identifier); it != matchedDecls.end()) {
                auto matchedDecl = it->second;
                bool hasGenericMismatch =
                    (decl->generic && !matchedDecl->generic) || (!decl->generic && matchedDecl->generic);
                if (hasGenericMismatch) {
                } else if (decl->TestAttr(Attribute::PLATFORM) && matchedDecl->TestAttr(Attribute::COMMON)) {
                    MergeCommonIntoPlatform(diag, *matchedDecl, *decl);
                } else if (decl->TestAttr(Attribute::COMMON) && matchedDecl->TestAttr(Attribute::PLATFORM)) {
                    MergeCommonIntoPlatform(diag, *decl, *matchedDecl);
                }
            } else if (decl->TestAnyAttr(Attribute::COMMON, Attribute::PLATFORM)) {
                matchedDecls.emplace(decl->identifier, decl);
            }
        }

        return VisitAction::SKIP_CHILDREN;
    });
    walkerPackage.Walk();
}

void MPTypeCheckerImpl::PrepareTypeCheck4CODEMPExtension(CompilerInstance& ci, ScopeManager& scopeManager,
    ASTContext& ctx, const std::unordered_set<Ptr<AST::ExtendDecl>>& extends)
{
    if (!compilePlatform) {
        return;
    }
    MergeCODEMPExtensions(ci, scopeManager, ctx, extends);
}

namespace {
void UpdateDeclMap(DiagnosticEngine& diag, ASTContext& ctx, Ptr<ExtendDecl>& ed)
{
    std::vector<Symbol*> syms;
    std::function<VisitAction(Ptr<Node>)> collector = [&syms](auto node) {
        static std::vector<ASTKind> ignoredKinds = {ASTKind::PRIMARY_CTOR_DECL, ASTKind::VAR_WITH_PATTERN_DECL};
        CODEC_ASSERT(!Utils::In(node->astKind, ignoredKinds));
        if (auto decl = DynamicCast<Decl*>(node);
            decl && !decl->TestAttr(Attribute::IS_BROKEN) && decl->symbol && decl->identifier != WILDCARD_CHAR) {
            syms.emplace_back(decl->symbol);
        }
        return VisitAction::WALK_CHILDREN;
    };
    Walker(ed, collector).Walk();

    for (auto sym : syms) {
        CODEC_NULLPTR_CHECK(sym);
        // macro expanded decls are not added into declMap.
        // macro invoke func can NOT be seen by developer so should not be added.
        if (sym->node->astKind == ASTKind::MACRO_EXPAND_DECL || sym->node->TestAttr(Attribute::MACRO_INVOKE_FUNC)) {
            continue;
        }

        std::string scopeName = ScopeManagerApi::GetScopeNameWithoutTail(sym->scopeName);
        auto names = std::make_pair(sym->name, scopeName);
        if (sym->astKind == ASTKind::PROP_DECL) { // Function redefinition will not be checked in this phase.
            if (auto found = ctx.GetDeclsByName(names); !found.empty()) {
                bool multiPlat =
                    (sym->node->TestAttr(Attribute::COMMON) && found.front()->TestAttr(Attribute::PLATFORM)) ||
                    (sym->node->TestAttr(Attribute::PLATFORM) && found.front()->TestAttr(Attribute::COMMON));
                if (!multiPlat) {
                    Sema::DiagRedefinitionWithFoundNode(diag, StaticCast<Decl>(*sym->node), *found.front());
                }
            }
        }
        ctx.AddDeclName(names, StaticCast<Decl>(*sym->node));
    }
}
} // namespace

void MPTypeCheckerImpl::MergeCODEMPExtensions(CompilerInstance& ci, ScopeManager& scopeManager, ASTContext& ctx,
    const std::unordered_set<Ptr<ExtendDecl>>& extends)
{
    std::unordered_map<std::string, Ptr<ExtendDecl>> platformExtendDecls;
    std::unordered_map<std::string, std::set<Ptr<ExtendDecl>>> commonExtendDecls;
    for (auto ed : extends) {
        if (!Ty::IsTyCorrect(ed->ty.get()) || !ed->TestAnyAttr(Attribute::COMMON, Attribute::PLATFORM)) {
            continue;
        }

        std::unordered_map<Ptr<Ty>, unsigned> genericIdx;
        if (ed->generic) {
            for (unsigned idx = 0; idx < ed->generic->typeParameters.size(); ++idx) {
                genericIdx.emplace(ed->generic->typeParameters[idx]->ty, idx);
            }
        }

        std::string key;
        // serialize the extended type.
        key += GetTypeNameFromTy(ed->extendedType->ty, true, genericIdx);
        // serialize the inherited types.
        std::set<std::string> inheritedTysStr;
        for (auto& inheritedType : ed->inheritedTypes) {
            inheritedTysStr.emplace(GetTypeNameFromTy(inheritedType->ty, true, genericIdx));
        }
        if (!inheritedTysStr.empty()) {
            key += "<:";
        }
        std::for_each(inheritedTysStr.begin(), inheritedTysStr.end(), [&key](const std::string& str) { key += str; });
        // serialize the generic constraints.
        if (ed->generic) {
            key += CalculatedGenericConstraintsStr(ed->generic->genericConstraints, genericIdx);
        }

        // After merging `common extend` into `platform extend`, the symbol table (scopeName) needs to be updated,
        // otherwise redefined functions cannot be detected.
        // After updating the symbol table, the `declMap` must also be updated,
        // otherwise symbols within function bodies won't find their definitions.
        if (ed->TestAttr(Attribute::COMMON)) {
            if (auto it = platformExtendDecls.find(key); it != platformExtendDecls.end()) {
                auto platformExtendDecl = it->second;
                MergeCommonIntoPlatform(diag, *ed, *platformExtendDecl);
                Collector collector(scopeManager, ci.invocation.globalOptions.enableMacroInLSP);
                collector.BuildSymbolTable(ctx, platformExtendDecl, ci.IsBuildTrie());
                UpdateDeclMap(diag, ctx, platformExtendDecl);
            }
            commonExtendDecls[key].emplace(ed);
        } else {
            if (platformExtendDecls.find(key) != platformExtendDecls.end()) {
                // A common declaration may have one or more matching specific declarations
                // in descending source sets (at most one specific per source set)
                diag.DiagnoseRefactor(
                    DiagKindRefactor::sema_platform_has_duplicate_extensions, *ed, ed->extendedType->ToString());
                continue;
            } else if (auto it = commonExtendDecls.find(key); it != commonExtendDecls.end()) {
                for (auto ced : it->second) {
                    MergeCommonIntoPlatform(diag, *ced, *ed);
                }
                Collector collector(scopeManager, ci.invocation.globalOptions.enableMacroInLSP);
                collector.BuildSymbolTable(ctx, ed, ci.IsBuildTrie());
                UpdateDeclMap(diag, ctx, ed);
            }
            platformExtendDecls.emplace(key, ed);
        }
    }
}

namespace {
// Check whether cls has general sub class.
bool HasGeneralSubClass(const AST::ClassDecl& cls)
{
    const AST::ClassDecl* cur = &cls;
    while (!cur->subDecls.empty()) {
        cur = StaticCast<AST::ClassDecl>(*(cur->subDecls.begin()));
        if (!cur->TestAttr(Attribute::COMMON)) {
            return true;
        }
    }
    return false;
}
}

// Precheck for CODEMP
void MPTypeCheckerImpl::PreCheck4CODEMP(const Package& pkg)
{
    if (!compileCommon) {
        return;
    }
    // common package part
    IterateToplevelDecls(pkg, [this](auto& decl) {
        if (decl->astKind == ASTKind::CLASS_DECL) {
            // Precheck for class
            PreCheckCODEMPClass(*StaticCast<ClassDecl>(decl.get()));
        }
    });
}

// Precheck for class
void MPTypeCheckerImpl::PreCheckCODEMPClass(const ClassDecl& cls)
{
    // Report error for common open | abstract class without init inherited by general class in common part.
    if (cls.TestAttr(Attribute::COMMON) && cls.TestAnyAttr(Attribute::OPEN, Attribute::ABSTRACT)) {
        const auto& decls = cls.GetMemberDeclPtrs();
        bool hasInit = std::any_of(decls.cbegin(), decls.cend(), [](const Ptr<Decl>& decl) {
            return decl->TestAttr(Attribute::CONSTRUCTOR);
        });
        if (!hasInit && HasGeneralSubClass(cls)) {
            // report error: please implement the constructor explicitly for common open class 'xxx'
            diag.DiagnoseRefactor(DiagKindRefactor::sema_common_open_class_no_init, cls, cls.identifier.Val());
        }
    }
}

void MPTypeCheckerImpl::FilterOutCommonCandidatesIfPlatformExist(
    std::map<Names, std::vector<Ptr<FuncDecl>>>& candidates)
{
    for (auto& [names, funcs] : candidates) {
        bool hasPlatformCandidates = false;

        for (auto& func : funcs) {
            if (func->TestAttr(Attribute::PLATFORM)) {
                hasPlatformCandidates = true;
                break;
            }
        }

        if (hasPlatformCandidates) {
            funcs.erase(
                std::remove_if(funcs.begin(), funcs.end(),
                    [](const Ptr<FuncDecl> decl) { return decl->TestAttr(Attribute::COMMON); }),
                funcs.end());
        }
    }
}

// TypeCheck for CODEMP
void MPTypeCheckerImpl::RemoveCommonCandidatesIfHasPlatform(std::vector<Ptr<FuncDecl>>& candidates)
{
    Utils::EraseIf(candidates, [candidates](const Ptr<FuncDecl> candidate) {
        if (candidate->TestAttr(Attribute::COMMON) && candidate->platformImplementation) {
            // erase if the platform decl also contains in the candidates vector
            return std::find(candidates.begin(), candidates.end(), 
                candidate->platformImplementation) != candidates.end();
        }
        return false;
    });
}

static Ptr<Ty> CheckFuncReturnType(Ptr<Ty> ty)
{
    CODEC_ASSERT_WITH_MSG(!!ty, "Function should be already resolved");
    CODEC_ASSERT_WITH_MSG(ty->IsFunc(), "Function's type should be a FuncTy");
    auto retTy = StaticCast<FuncTy>(ty)->retTy;

    CODEC_ASSERT_WITH_MSG(retTy && !Ty::IsInitialTy(retTy), "Function's return type should be already resolved");
    return retTy;
}

void MPTypeCheckerImpl::CheckMatchedFunctionReturnTypes(FuncDecl& platformFunc, FuncDecl& commonFunc)
{
    auto commonType = commonFunc.funcBody->ty;
    auto platformType = platformFunc.funcBody->ty;

    auto commonRetTy = CheckFuncReturnType(commonType);
    auto platformRetTy = CheckFuncReturnType(platformType);

    if (commonRetTy == platformRetTy) return;
    if (typeManager.IsSubtype(platformRetTy, commonRetTy)) return;

    // the subtype check will handle most of the cases, however in the case of generics, types must 
    // be substituted properly before checking for subtype that is properly done in MatchCODEMPFunction 
    // that we use as a fallback

    // we need to erase the pointer temporarily to avoid the multiple implementations error
    commonFunc.platformImplementation = nullptr;
    if (!MatchCODEMPFunction(platformFunc, commonFunc)) {
        diag.DiagnoseRefactor(
            DiagKindRefactor::sema_return_type_incompatible,
            platformFunc,
            platformFunc.identifier);
    }
    commonFunc.platformImplementation = &platformFunc;
}

void MPTypeCheckerImpl::CheckMatchedVariableTypes(AST::VarDecl& platformVar, AST::VarDecl& commonVar)
{
    CODEC_ASSERT_WITH_MSG(commonVar.ty && !Ty::IsInitialTy(commonVar.ty),
        "Common variable type must be already resolved");

    if (!platformVar.ty || Ty::IsInitialTy(platformVar.ty)) {
        // this should already be reported as parse_expected_one_of_type_or_initializer
        // here we skip it
        return;
    }

    if (commonVar.ty == platformVar.ty) return;
    if (typeManager.IsTyEqual(platformVar.ty, commonVar.ty)) return;

    // the type equality check will handle most of the cases except for the generics case when types 
    // need to be substituted in order to compare them that is handled in MatchCODEMPVar
    //  that is fallback here

    // member variables are paired during the merge, global and statics during the Match function
    // unlike for functions, we should NOT erase the platform pointer
    MatchCODEMPVar(platformVar, commonVar); // the diagnostic is reported by the function
}

static bool IsCommon(const AST::Node& decl)
{
    return decl.TestAttr(Attribute::COMMON);
}

static bool IsEnumConstructor(const AST::Node& decl)
{
    return decl.TestAttr(Attribute::ENUM_CONSTRUCTOR);
}

void MPTypeCheckerImpl::CheckReturnAndVariableTypes(AST::Package& pkg)
{
    std::function<VisitAction(Ptr<Node>)> visitor = [this](const Ptr<Node> &node) {
        if (node->astKind == ASTKind::FUNC_DECL) {
            if (!IsCommon(*node) || IsEnumConstructor(*node)) {
                return VisitAction::SKIP_CHILDREN;
            }
            auto common = StaticAs<ASTKind::FUNC_DECL>(node);
            auto platform = common->platformImplementation;
            if (platform) {
                CODEC_ASSERT_WITH_MSG(platform->astKind == AST::ASTKind::FUNC_DECL, 
                    "Common function can be only matched to a function but got another kind");
                auto& commonFunc = *common;
                auto& platformFunc = *StaticCast<FuncDecl>(platform);

                CheckMatchedFunctionReturnTypes(platformFunc, commonFunc);
            }
            return VisitAction::SKIP_CHILDREN;
        } else if (node->astKind == ASTKind::VAR_DECL) {
            if (!IsCommon(*node) || IsEnumConstructor(*node)) {
                return VisitAction::SKIP_CHILDREN;
            }
            auto common = StaticAs<ASTKind::VAR_DECL>(node);
            auto platform = common->platformImplementation;
            if (platform) {
                CODEC_ASSERT_WITH_MSG(platform->astKind == AST::ASTKind::VAR_DECL, 
                    "Common variable can be only matched to a variable but got another kind");
                auto& commonVar = *common;
                auto& platformVar = *StaticCast<VarDecl>(platform);

                CheckMatchedVariableTypes(platformVar, commonVar);
            } else if (!common->TestAttr(Attribute::INITIALIZED) && common->TestAttr(Attribute::FROM_COMMON_PART)) {
                // this has been postponed, we need this to handle the case when a common var has no matched specified
                // but it does't fail because there is an initializer for it
                DiagNotMatchedPlatformDecl(diag, *common);
            }
            return VisitAction::SKIP_CHILDREN;
        }
        return VisitAction::WALK_CHILDREN;
    };
    Walker walker(&pkg, visitor);
    walker.Walk();
}

void MPTypeCheckerImpl::ValidateMatchedAnnotationsAndModifiers(AST::Package& pkg)
{
    std::function<VisitAction(Ptr<Node>)> visitor = [this](const Ptr<Node> &node) {
        if (node->IsDecl()) {
            if (!IsCommon(*node)) { return VisitAction::WALK_CHILDREN; }
            auto commonDecl = StaticCast<Decl>(node);
            if (!commonDecl->platformImplementation) {
                return VisitAction::WALK_CHILDREN;
            }

            MatchCODEMPDeclAnnotations(*commonDecl, *commonDecl->platformImplementation);
        }
        return VisitAction::WALK_CHILDREN;
    };
    Walker walker(&pkg, visitor);
    walker.Walk();
}

namespace {
// Collect common or platform decl.
void CollectDecl(
    Ptr<Decl> decl, std::vector<Ptr<Decl>>& commonDecls, std::vector<Ptr<Decl>>& platformDecls)
{
    if (decl->TestAttr(Attribute::COMMON)) {
        commonDecls.emplace_back(decl);
    } else if (decl->TestAttr(Attribute::PLATFORM)) {
        platformDecls.emplace_back(decl);
    }
}

// Collect common and platform decls.
void CollectCODEMPDecls(Package& pkg, std::vector<Ptr<Decl>>& commonDecls, std::vector<Ptr<Decl>>& platformDecls)
{
    std::function<VisitAction(Ptr<Node>)> visitor = [&commonDecls, &platformDecls](const Ptr<Node> &node) {
        if (node->IsDecl() && node->astKind != ASTKind::PRIMARY_CTOR_DECL) {
            CollectDecl(StaticAs<ASTKind::DECL>(node), commonDecls, platformDecls);
        }
        if (node->astKind == ASTKind::PACKAGE || node->astKind == ASTKind::FILE ||
            node->IsNominalDecl() || node->IsNominalDeclBody()) {
            return VisitAction::WALK_CHILDREN;
        }
        return VisitAction::SKIP_CHILDREN;
    };
    Walker walker(&pkg, visitor);
    walker.Walk();
}

// Check whether the common decl must be matched with paltform decl.
bool MustMatchWithPlatform(const Decl& decl)
{
    CODEC_ASSERT(decl.TestAttr(Attribute::COMMON));
    if (decl.platformImplementation) {
        return false;
    }
    // var/func with default implementation
    if (decl.TestAttr(Attribute::COMMON_WITH_DEFAULT)) {
        return false;
    }
    // No match is required for enum constructors when the outer enum is marked as
    // COMMON_WITH_DEFAULT and has no platform implementation
    if (decl.TestAttr(Attribute::ENUM_CONSTRUCTOR) && decl.outerDecl &&
        decl.outerDecl->TestAttr(AST::Attribute::COMMON_WITH_DEFAULT) && !decl.outerDecl->platformImplementation) {
        return false;
    }

    // common member in interface allow no platform member, maybe use abstract attr.
    if (decl.outerDecl && decl.outerDecl->astKind == ASTKind::INTERFACE_DECL) {
        return false;
    }
    // var already initialized
    if (decl.astKind == ASTKind::VAR_DECL && decl.TestAttr(Attribute::INITIALIZED)) {
        return false;
    }
    // local var member
    if (decl.astKind == ASTKind::VAR_DECL && !decl.IsStaticOrGlobal() &&
        decl.outerDecl->astKind != ASTKind::ENUM_DECL) {
        return false;
    }
    return true;
}
} // namespace

bool NeedToReportMissingBody(const Decl& common, const Decl& platform)
{
    if (common.outerDecl && common.TestAttr(Attribute::COMMON_WITH_DEFAULT) && !common.TestAttr(Attribute::ABSTRACT) &&
        platform.TestAttr(Attribute::ABSTRACT)) {
        return true;
    }

    return false;
}

// PostTypeCheck for CODEMP
bool MPTypeCheckerImpl::MatchCODEMPDeclAttrs(
    const std::vector<Attribute>& attrs, const Decl& common, const Decl& platform) const
{
    for (auto attr : attrs) {
        if (common.TestAttr(attr) != platform.TestAttr(attr)) {
            if ((attr == Attribute::ABSTRACT || attr == Attribute::OPEN)) {
                // Error `sema_platform_member_must_have_implementation` should be reported
                // If common have body, but platform not. Diagnostic about wrong modifiers is confusing.
                if (NeedToReportMissingBody(common, platform)) {
                    continue;
                }
                if (platform.TestAttr(Attribute::ABSTRACT) && common.TestAttr(Attribute::OPEN) &&
                    common.IsFuncOrProp()) {
                    auto kindStr = common.astKind == ASTKind::FUNC_DECL ? "function" : "property";
                    diag.DiagnoseRefactor(DiagKindRefactor::sema_open_abstract_platform_can_not_replace_open_common,
                        platform, kindStr, kindStr);
                }
                // ABSTRACT member can be replaced with OPEN
                bool funcOrProp = common.astKind == ASTKind::FUNC_DECL || common.astKind == ASTKind::PROP_DECL;
                if (funcOrProp && common.TestAttr(Attribute::ABSTRACT) && platform.TestAttr(Attribute::OPEN)) {
                    continue;
                }
                // Same as previous check, however static functions has no OPEN modifier
                if (common.TestAttr(Attribute::ABSTRACT) &&
                    common.TestAttr(Attribute::STATIC) && platform.TestAttr(Attribute::STATIC)) {
                    continue;
                }
            }
            if (common.astKind == ASTKind::PROP_DECL && attr == Attribute::MUT) {
                if (common.TestAttr(attr)) {
                    diag.DiagnoseRefactor(DiagKindRefactor::sema_property_have_same_declaration_in_inherit_mut,
                        platform, platform.identifier.Val());
                } else {
                    diag.DiagnoseRefactor(DiagKindRefactor::sema_property_have_same_declaration_in_inherit_immut,
                        common, common.identifier.Val());
                }
            } else if (common.astKind == ASTKind::CLASS_DECL && (attr == Attribute::SEALED || attr == Attribute::OPEN) &&
                // Keep silent due to overloaded common funcs.
                // Allow platform sealed abstract when common is abstract
                platform.TestAttr(Attribute::SEALED) && common.TestAttr(Attribute::ABSTRACT)) {
                continue;
            } else {
                diag.DiagnoseRefactor(
                    DiagKindRefactor::sema_platform_has_different_modifier, platform, DeclKindToString(platform));
            }
            return false;
        }
    }
    return true;
}

void MPTypeCheckerImpl::CheckCommonSpecificGenericMatch(const AST::Decl& platformDecl, const AST::Decl& commonDecl)
{
    // check generic constraints
    auto parentBounds = GetAllGenericUpperBounds(typeManager, commonDecl);
    auto childBounds = GetAllGenericUpperBounds(typeManager, platformDecl);

    CheckGenericTypeBoundsMapped(commonDecl, platformDecl, parentBounds, childBounds, diag, typeManager);
}

// Match common nominal decl with platform for details.
bool MPTypeCheckerImpl::MatchCommonNominalDeclWithPlatform(const InheritableDecl& commonDecl)
{
    auto platformDecl = commonDecl.platformImplementation;
    if (platformDecl == nullptr) {
        if (commonDecl.TestAttr(Attribute::COMMON_WITH_DEFAULT)) {
            return false;
        }
        DiagNotMatchedPlatformDecl(diag, commonDecl);
        return false;
    }
    // Match attributes (modifiers).
    std::vector<Attribute> matchedAttr = {
        Attribute::ABSTRACT, Attribute::PUBLIC, Attribute::OPEN, Attribute::PROTECTED, Attribute::C, Attribute::SEALED};
    if (!MatchCODEMPDeclAttrs(matchedAttr, commonDecl, *platformDecl)) {
        return false;
    }
    // Match super types.
    std::set<Ptr<InterfaceTy>> comSupInters;
    auto platSupInters = StaticCast<InheritableDecl>(platformDecl)->GetSuperInterfaceTys();
    TypeSubst genericTyMap;
    MapCODEMPGenericTypeArgs(genericTyMap, commonDecl, *platformDecl);
    if (!genericTyMap.empty()) {
        for (auto superInterface : commonDecl.GetSuperInterfaceTys()) {
            auto updatedInterfaceType = typeManager.GetInstantiatedTy(superInterface, genericTyMap);
            comSupInters.emplace(StaticCast<InterfaceTy*>(updatedInterfaceType));
        }
    } else {
        comSupInters = commonDecl.GetSuperInterfaceTys();
    }
    if (comSupInters.size() != platSupInters.size()) {
        DiagNotMatchedSuperType(diag, *platformDecl);
        return false;
    }
    bool match = false;
    for (auto& comSupInter : comSupInters) {
        for (auto& platSupInter : platSupInters) {
            if (typeManager.IsTyEqual(comSupInter, platSupInter)) {
                match = true;
                break;
            }
        }
        if (!match) {
            DiagNotMatchedSuperType(diag, *platformDecl);
            return false;
        }
    }
    // Match super class if need.
    if (commonDecl.astKind == ASTKind::CLASS_DECL) {
        auto comSupClass = StaticCast<ClassDecl>(&commonDecl)->GetSuperClassDecl();
        auto platSupIClass = StaticCast<ClassDecl>(platformDecl)->GetSuperClassDecl();
        if (!typeManager.IsTyEqual(comSupClass->ty, platSupIClass->ty)) {
            DiagNotMatchedSuperType(diag, *platformDecl);
            return false;
        }
    }

    CheckCommonSpecificGenericMatch(*platformDecl, commonDecl);

    return true;
}

static size_t GenericsCount(const Decl& decl)
{
    auto generic = decl.GetGeneric();
    if (!generic) {
        return 0;
    }
    return generic->typeParameters.size();
}

bool MPTypeCheckerImpl::IsCODEMPDeclMatchable(Decl& lhsDecl, Decl& rhsDecl) const
{
    bool isLeftCommon = lhsDecl.TestAttr(Attribute::COMMON);
    Decl& commonDecl = isLeftCommon ? lhsDecl : rhsDecl;
    Decl& platformDecl = isLeftCommon ? rhsDecl : lhsDecl;
    if (commonDecl.identifier.GetRawText() != platformDecl.identifier.GetRawText()) {
        return false;
    }
    if (platformDecl.IsMemberDecl() != commonDecl.IsMemberDecl()) {
        return false;
    }
    if (platformDecl.IsMemberDecl()) {
        CODEC_NULLPTR_CHECK(platformDecl.outerDecl);
        CODEC_NULLPTR_CHECK(commonDecl.outerDecl);
        if (platformDecl.outerDecl->rawMangleName != commonDecl.outerDecl->rawMangleName) {
            return false;
        }
    }

    if (GenericsCount(lhsDecl) != GenericsCount(rhsDecl)) {
        return false;
    }

    return true;
}

bool MPTypeCheckerImpl::TrySetPlatformImpl(Decl& platformDecl, Decl& commonDecl, const std::string& kind)
{
    if (commonDecl.platformImplementation) {
        diag.DiagnoseRefactor(DiagKindRefactor::sema_multiple_common_implementations, commonDecl, kind);
        return false;
    }
    // common with default but platform without default
    if (NeedToReportMissingBody(commonDecl, platformDecl)) {
        diag.DiagnoseRefactor(DiagKindRefactor::sema_platform_member_must_have_implementation,
            platformDecl, platformDecl.identifier.Val(), commonDecl.outerDecl->identifier.Val());
        return false;
    }
    commonDecl.platformImplementation = &platformDecl;
    commonDecl.doNotExport = true;
    return true;
}

bool MPTypeCheckerImpl::MatchCODEMPFunction(FuncDecl& platformFunc, FuncDecl& commonFunc)
{
    if (!IsCODEMPDeclMatchable(platformFunc, commonFunc)) {
        return false;
    }

    bool isGenericFuncMatch = false;
    TypeSubst genericTyMap;
    MapCODEMPGenericTypeArgs(genericTyMap, commonFunc, platformFunc);
    if (!genericTyMap.empty()) {
        auto newCommonFuncTy = StaticCast<FuncTy*>(typeManager.GetInstantiatedTy(commonFunc.ty, genericTyMap));
        auto platformFuncTy = StaticCast<FuncTy*>(platformFunc.ty);
        if (typeManager.IsFuncTySubType(*platformFuncTy, *newCommonFuncTy)) {
            isGenericFuncMatch = true;
        }
    }
    if (!isGenericFuncMatch && !typeManager.IsFuncDeclSubType(platformFunc, commonFunc)) {
        return false;
    }
    auto& commonParams = commonFunc.funcBody->paramLists[0]->params;
    auto& platformParams = platformFunc.funcBody->paramLists[0]->params;
    for (size_t i = 0; i < commonFunc.funcBody->paramLists[0]->params.size(); i++) {
        if (commonParams[i]->isNamedParam != platformParams[i]->isNamedParam) {
            diag.DiagnoseRefactor(DiagKindRefactor::sema_platform_has_different_parameter, *platformParams[i]);
            return false;
        }
        if (commonParams[i]->isNamedParam && platformParams[i]->isNamedParam) {
            if (commonParams[i]->identifier.GetRawText() != platformParams[i]->identifier.GetRawText()) {
                diag.DiagnoseRefactor(DiagKindRefactor::sema_platform_has_different_parameter, *platformParams[i]);
                return false;
            }
        }

        // Check default value consistency: default values should be on either common or platform side, not both
        bool commonHasDefault = commonParams[i]->assignment != nullptr;
        bool platformHasDefault = platformParams[i]->assignment != nullptr;

        if (commonHasDefault && platformHasDefault) {
            diag.DiagnoseRefactor(DiagKindRefactor::sema_codemp_parameter_default_value_both_sides,
                *platformParams[i]);
            return false;
        }

        // desugar platform default value, desugarDecl export all the time, assignment only export const value
        if (commonParams[i]->desugarDecl && !platformParams[i]->desugarDecl) {
            platformParams[i]->assignment = ASTCloner::Clone(commonParams[i]->assignment.get());
            platformParams[i]->desugarDecl = ASTCloner::Clone(commonParams[i]->desugarDecl.get());
            platformParams[i]->desugarDecl->outerDecl = platformFunc.outerDecl;
            platformParams[i]->EnableAttr(Attribute::HAS_INITIAL);
        }
    }

    // For init or primary constructor
    if (platformFunc.TestAttr(AST::Attribute::CONSTRUCTOR) || commonFunc.TestAttr(AST::Attribute::CONSTRUCTOR)) {
        if (!platformFunc.TestAttr(AST::Attribute::PRIMARY_CONSTRUCTOR) &&
            commonFunc.TestAttr(AST::Attribute::PRIMARY_CONSTRUCTOR)) {
            diag.DiagnoseRefactor(DiagKindRefactor::sema_platform_init_common_primary_constructor, commonFunc);
            return false;
        }
        for (size_t i = 0; i < platformParams.size(); ++i) {
            if (commonParams[i]->isMemberParam && !platformParams[i]->isMemberParam) {
                diag.DiagnoseRefactor(DiagKindRefactor::sema_platform_primary_unmatched_var_decl, *platformParams[i]);
                return false;
            }
        }
    }

    if (GenericsCount(commonFunc) != GenericsCount(platformFunc)) {
        diag.Diagnose(
            platformFunc, DiagKind::sema_generic_member_type_argument_different, platformFunc.identifier.Val());
        return false;
    }

    CheckCommonSpecificGenericMatch(platformFunc, commonFunc);

    return TrySetPlatformImpl(platformFunc, commonFunc, "function");
}

bool MPTypeCheckerImpl::MatchCODEMPProp(PropDecl& platformProp, PropDecl& commonProp)
{
    if (!IsCODEMPDeclMatchable(platformProp, commonProp)) {
        return false;
    }
    if (!typeManager.IsTyEqual(platformProp.ty, commonProp.ty)) {
        diag.DiagnoseRefactor(DiagKindRefactor::sema_platform_has_different_type, platformProp, "property");
    }
    bool ret = TrySetPlatformImpl(platformProp, commonProp, "property " + platformProp.identifier);
    if (ret && !platformProp.getters.empty() && !commonProp.getters.empty()) {
        ret &= TrySetPlatformImpl(*platformProp.getters[0], *commonProp.getters[0],
            "property getter " + platformProp.identifier);
    }
    if (ret && !platformProp.setters.empty() && !commonProp.setters.empty()) {
        ret &= TrySetPlatformImpl(*platformProp.setters[0], *commonProp.setters[0],
            "property setter " + platformProp.identifier);
    }

    return ret;
}

bool MPTypeCheckerImpl::MatchEnumFuncTypes(const FuncDecl& platform, const FuncDecl& common)
{
    if (typeManager.IsFuncDeclEqualType(platform, common)) {
        // if types are equal
        return true;
    }

    TypeSubst genericTyMap;
    MapCODEMPGenericTypeArgs(genericTyMap, *common.outerDecl, *platform.outerDecl);
    if (genericTyMap.empty()) {
        return false;
    }

    auto mappedCommonType = typeManager.GetInstantiatedTy(common.ty, genericTyMap);
    return typeManager.IsTyEqual(mappedCommonType, platform.ty);
}

bool MPTypeCheckerImpl::MatchCODEMPEnumConstructor(Decl& platformDecl, Decl& commonDecl)
{
    if (!IsCODEMPDeclMatchable(platformDecl, commonDecl)) {
        return false;
    }

    // identifiers are already checked
    if (platformDecl.astKind == ASTKind::FUNC_DECL) { // enum constructrs with values
        auto& platformFunc = StaticCast<FuncDecl>(platformDecl);
        auto& commonFunc = StaticCast<FuncDecl>(commonDecl);
        if (!MatchEnumFuncTypes(platformFunc, commonFunc)) {
            return false;
        }
    }

    auto enumName = platformDecl.outerDecl->identifier.GetRawText();
    return TrySetPlatformImpl(platformDecl, commonDecl, "enum '" + enumName + "' constructor");
}

bool MPTypeCheckerImpl::MatchCODEMPVar(VarDecl& platformVar, VarDecl& commonVar)
{
    if (!IsCODEMPDeclMatchable(platformVar, commonVar)) {
        return false;
    }
    auto cType = commonVar.ty;
    if (platformVar.IsMemberDecl()) {
        TypeSubst genericTyMapForNominals;
        MapCODEMPGenericTypeArgs(genericTyMapForNominals, *commonVar.outerDecl, *platformVar.outerDecl);
        if (!genericTyMapForNominals.empty()) {
            cType = typeManager.GetInstantiatedTy(cType, genericTyMapForNominals);
        }
    }
    auto pType = platformVar.ty;
    if (!typeManager.IsTyEqual(cType, pType) && !Ty::IsInitialTy(pType)) {
        // if the platform type is initial then the check will be restarted after the type get resolved
        // so we can suppress the check for now
        auto platformKind = platformVar.isVar ? "var" : "let";
        diag.DiagnoseRefactor(DiagKindRefactor::sema_platform_has_different_type, platformVar, platformKind);
    }
    if (platformVar.isVar != commonVar.isVar) {
        auto platformKind = platformVar.isVar ? "var" : "let";
        auto commonKind = commonVar.isVar ? "var" : "let";
        diag.DiagnoseRefactor(DiagKindRefactor::sema_platform_var_not_match_let, platformVar, platformKind, commonKind);
    }
    if (platformVar.IsStaticOrGlobal()) {
        commonVar.platformImplementation = &platformVar;
        commonVar.doNotExport = true;
    } else if (commonVar.TestAttr(Attribute::STATIC) == platformVar.TestAttr(Attribute::STATIC)) {
        // Instance variables must already be matched unless we have modifiers mismatch
        // that will be reported later
        CODEC_ASSERT(commonVar.platformImplementation == &platformVar);
    } else {
        // we assign it even despite the mismatch
        // and let the generic validation report it later
        commonVar.platformImplementation = &platformVar;
    }
    return true;
}

bool MPTypeCheckerImpl::TryMatchVarWithPatternWithVarDecls(
    AST::VarWithPatternDecl& platformDecl, const std::vector<Ptr<AST::Decl>>& commonDecls)
{
    if (platformDecl.irrefutablePattern->astKind != ASTKind::TUPLE_PATTERN) {
        return false;
    }
    auto platformTuplePattern = StaticCast<TuplePattern>(platformDecl.irrefutablePattern.get());

    bool matchedAll = true;
    for (auto& pattern : platformTuplePattern->patterns) {
        if (pattern->astKind != ASTKind::VAR_PATTERN) {
            matchedAll = false;
            break;
        }

        auto patternDecl = StaticCast<VarPattern>(pattern.get());
        if (!MatchPlatformDeclWithCommonDecls(*patternDecl->varDecl, commonDecls)) {
            matchedAll = false;
        }
    }

    return matchedAll;
}

// this is never invoked for nominal decls
bool MPTypeCheckerImpl::MatchPlatformDeclWithCommonDecls(
    AST::Decl& platformDecl, const std::vector<Ptr<AST::Decl>>& commonDecls)
{
    bool matched = false;
    bool isEnumConstructor = platformDecl.TestAttr(Attribute::ENUM_CONSTRUCTOR);
    auto kind = platformDecl.astKind;
    for (auto& commonDecl : commonDecls) {
        if (matched) {
            break;
        }
        if (commonDecl->astKind != kind) {
            continue;
        }
        if (isEnumConstructor && commonDecl->TestAttr(Attribute::ENUM_CONSTRUCTOR)) {
            matched = MatchCODEMPEnumConstructor(platformDecl, *commonDecl) || matched;
        } else if (kind == ASTKind::FUNC_DECL) {
            matched = MatchCODEMPFunction(*StaticCast<FuncDecl>(&platformDecl), *StaticCast<FuncDecl>(commonDecl)) ||
                matched;
        } else if (kind == ASTKind::PROP_DECL) {
            matched = MatchCODEMPProp(*StaticCast<PropDecl>(&platformDecl), *StaticCast<PropDecl>(commonDecl)) ||
                matched;
        } else if (kind == ASTKind::VAR_DECL) {
            matched = MatchCODEMPVar(*StaticCast<VarDecl>(&platformDecl), *StaticCast<VarDecl>(commonDecl)) ||
                matched;
        }
    }

    // VarWithPattern can match several decls from common part
    if (kind == ASTKind::VAR_WITH_PATTERN_DECL) {
        matched = TryMatchVarWithPatternWithVarDecls(*StaticCast<VarWithPatternDecl>(&platformDecl), commonDecls);
    }

    // For enum constructor
    if (!matched) {
        if (platformDecl.outerDecl && platformDecl.outerDecl->TestAttr(Attribute::COMMON_NON_EXHAUSTIVE)) {
            return false;
        }
        DiagNotMatchedCommonDecl(diag, platformDecl);
    }

    return matched;
}

// When there are several common extend without interfaces declared,
// if there are same-named private functions declared, a clash is reported
void MPTypeCheckerImpl::CheckCommonExtensions(std::vector<Ptr<Decl>>& commonDecls)
{
    std::map<Ptr<Ty>, std::set<std::string>> privateFunctionsOfExtensions;
    for (auto decl : commonDecls) {
        if (decl->astKind != ASTKind::EXTEND_DECL) {
            continue;
        }

        auto extendDecl = StaticCast<ExtendDecl>(decl);
        if (!extendDecl->GetSuperInterfaceTys().empty()) {
            continue;
        }

        auto& privateFunctions = privateFunctionsOfExtensions[extendDecl->extendedType->ty];
        for (auto& memberDecl : extendDecl->GetMemberDecls()) {
            if (!memberDecl->IsFuncOrProp() || !memberDecl->TestAttr(Attribute::PRIVATE)) {
                continue;
            }
            if (privateFunctions.find(memberDecl->rawMangleName) != privateFunctions.end()) {
                diag.DiagnoseRefactor(DiagKindRefactor::sema_common_direct_extension_has_duplicate_private_members,
                    *memberDecl, extendDecl->extendedType->ToString(), memberDecl->IsFunc() ? "function" : "property",
                    memberDecl->identifier.GetRawText());
            } else if (memberDecl->TestAttr(Attribute::COMMON)) {
                diag.DiagnoseRefactor(DiagKindRefactor::sema_common_direct_extension_has_common_private_members,
                    *memberDecl, memberDecl->IsFunc() ? "function" : "property",
                    memberDecl->identifier.GetRawText());
            } else {
                privateFunctions.emplace(memberDecl->rawMangleName);
            }
        }
    }
}

void MPTypeCheckerImpl::MatchCODEMPDecls(std::vector<Ptr<Decl>>& commonDecls, std::vector<Ptr<Decl>>& platformDecls)
{
    for (auto& platformDecl : platformDecls) {
        CODEC_ASSERT(platformDecl->TestAttr(Attribute::PLATFORM) && !platformDecl->TestAttr(Attribute::COMMON));
        if (platformDecl->TestAttr(Attribute::IS_BROKEN) || platformDecl->IsNominalDecl()) {
            continue;
        }
        MatchPlatformDeclWithCommonDecls(*platformDecl, commonDecls);
    }
    std::unordered_set<Decl*> matchedIds;
    // Report error for common decl having no matched platform decl.
    for (auto& decl : commonDecls) {
        if (decl->IsNominalDecl() && MatchCommonNominalDeclWithPlatform(*StaticCast<InheritableDecl>(decl))) {
            matchedIds.insert(decl->platformImplementation.get());
        }
        if (!MustMatchWithPlatform(*decl)) {
            continue;
        }
        if (decl->astKind == ASTKind::VAR_DECL && !decl->TestAttr(Attribute::ENUM_CONSTRUCTOR)) {
            continue; // postpone the check
        }
        DiagNotMatchedPlatformDecl(diag, *decl);
    }
    // Report error for platform nominal decl having no matched common decl.
    for (auto& decl : platformDecls) {
        if (decl->IsNominalDecl() && matchedIds.find(decl.get()) == matchedIds.end()) {
            DiagNotMatchedCommonDecl(diag, *decl);
        }
        if (decl->IsNominalDecl()) {
            CheckAbstractClassMembers(*StaticCast<InheritableDecl>(decl));
        }
    }
    for (auto& commonDecl : commonDecls) {
        if (commonDecl->IsNominalDecl()) {
            continue; // this is already handled in the loop above
        }
        auto platformDecl = commonDecl->platformImplementation;
        if (platformDecl == nullptr) {
            continue;
        }
            // need check Attribute::ABSTRACT for abstract class?
        std::vector<Attribute> matchedAttrs = { Attribute::STATIC, Attribute::MUT, Attribute::PRIVATE, Attribute::PUBLIC,
            Attribute::PROTECTED, Attribute::FOREIGN, Attribute::UNSAFE, Attribute::OPEN,
            Attribute::ABSTRACT};
        MatchCODEMPDeclAttrs(matchedAttrs, *commonDecl, *platformDecl);
    }
}

void MPTypeCheckerImpl::CheckAbstractClassMembers(const InheritableDecl& platformDecl)
{
    if (!platformDecl.TestAttr(Attribute::PLATFORM) || !platformDecl.TestAttr(Attribute::ABSTRACT)) {
        return;
    }

    if (platformDecl.astKind != ASTKind::CLASS_DECL) {
        return;
    }

    const auto& classDecl = StaticCast<const ClassDecl&>(platformDecl);

    for (const auto& memberDecl : classDecl.GetMemberDeclPtrs()) {
        // Check if member is a function or property with abstract modifier
        // and is NOT a platform-specific member or from common part
        if (memberDecl->TestAttr(Attribute::ABSTRACT) && !memberDecl->TestAttr(Attribute::PLATFORM) &&
            !memberDecl->TestAttr(Attribute::FROM_COMMON_PART) &&
            (memberDecl->astKind == ASTKind::FUNC_DECL || memberDecl->astKind == ASTKind::PROP_DECL)) {

            // Report error: cannot add abstract members to platform abstract class
            auto memberKind = memberDecl->astKind == ASTKind::FUNC_DECL ? "function" : "property";
            diag.DiagnoseRefactor(DiagKindRefactor::sema_codemp_non_platform_abstract_member_in_platform_class,
                *memberDecl, platformDecl.identifier.Val(), memberKind);
        }
    }
}

void MPTypeCheckerImpl::MatchPlatformWithCommon(Package& pkg)
{
    std::vector<Ptr<Decl>> commonDecls;
    std::vector<Ptr<Decl>> platformDecls;
    CollectCODEMPDecls(pkg, commonDecls, platformDecls);
    if (compileCommon) { // check common extensions
        CheckCommonExtensions(commonDecls);
    } else if (compilePlatform) { // match common decls and platform decls
        MatchCODEMPDecls(commonDecls, platformDecls);
    }

    for (auto common : commonDecls) {
        auto platform = common->platformImplementation;
        if (platform) {
            PropagateCODEMPDeclAnnotations(*common, *platform);
        }
    }
}

// Maps type parameters between common and platform declarations.
void MPTypeCheckerImpl::MapCODEMPGenericTypeArgs(
    TypeSubst& genericTyMap, const AST::Decl& commonDecl, const AST::Decl& platformDecl)
{
    if (!(commonDecl.TestAttr(Attribute::GENERIC) && platformDecl.TestAttr(Attribute::GENERIC))) {
        return;
    }
    // 1. Handle nominalDecl type parameters
    if (commonDecl.IsNominalDecl() && commonDecl.ty && !commonDecl.ty->typeArgs.empty()) {
        for (size_t i = 0; i < commonDecl.ty->typeArgs.size(); i++) {
            auto commonType = commonDecl.ty->typeArgs[i];

            if (commonType->IsGeneric()) {
                Ptr<TyVar> commonGeneric = RawStaticCast<TyVar*>(commonType);
                genericTyMap[commonGeneric] = platformDecl.ty->typeArgs[i];
            }
        }
    }

    CheckCommonSpecificGenericMatch(platformDecl, commonDecl);
    if (GenericsCount(commonDecl) != GenericsCount(platformDecl)) {
        return;
    }
    auto mapping = typeManager.GenerateGenericMappingFromGeneric(commonDecl, platformDecl);
    genericTyMap.insert(mapping.begin(), mapping.end());
}

void MPTypeCheckerImpl::UpdateGenericTyInMemberFromCommon(TypeSubst& genericTyMap, Ptr<AST::Decl>& member)
{
    Walker walker(member, [this, &genericTyMap](Ptr<Node> node) -> VisitAction {
        if (node->ty) {
            node->ty = typeManager.GetInstantiatedTy(node->ty, genericTyMap);
        }
        if (auto ref = DynamicCast<NameReferenceExpr*>(node); ref) {
            for (auto& instTy : ref->instTys) {
                instTy = typeManager.GetInstantiatedTy(instTy, genericTyMap);
            }
        }
        return VisitAction::WALK_CHILDREN;
    });
    walker.Walk();
}

void MPTypeCheckerImpl::UpdatePlatformMemberGenericTy(
    ASTContext& ctx, const std::function<std::vector<AST::Symbol*>(ASTContext&, ASTKind)>& getSymsFunc)
{
    // Collect all relevant symbol types
    auto syms = getSymsFunc(ctx, ASTKind::CLASS_DECL);
    auto interfaceSyms = getSymsFunc(ctx, ASTKind::INTERFACE_DECL);
    syms.insert(syms.end(), interfaceSyms.begin(), interfaceSyms.end());
    auto structSyms = getSymsFunc(ctx, ASTKind::STRUCT_DECL);
    syms.insert(syms.end(), structSyms.begin(), structSyms.end());
    auto enumSyms = getSymsFunc(ctx, ASTKind::ENUM_DECL);
    syms.insert(syms.end(), enumSyms.begin(), enumSyms.end());
    auto extendSyms = getSymsFunc(ctx, ASTKind::EXTEND_DECL);
    syms.insert(syms.end(), extendSyms.begin(), extendSyms.end());

    // Process common declarations and update generic types in platform members
    for (auto sym : syms) {
        CODEC_ASSERT(sym && sym->node);
        auto decl = StaticCast<Decl*>(sym->node);
        if (decl->TestAttr(Attribute::COMMON) && decl->TestAttr(Attribute::GENERIC)) {
            auto platformDecl = decl->platformImplementation;
            if (platformDecl) {
                TypeSubst genericTyMap;
                MapCODEMPGenericTypeArgs(genericTyMap, *decl, *platformDecl);
                for (auto& member : platformDecl->GetMemberDecls()) {
                    if (member->TestAttr(Attribute::FROM_COMMON_PART)) {
                        auto ptr = member.get();
                        UpdateGenericTyInMemberFromCommon(genericTyMap, ptr);
                    }
                }
            }
        }
    }
}


void MPTypeCheckerImpl::GetInheritedTypesWithPlatformImpl(
    std::vector<OwnedPtr<AST::Type>>& inheritedTypes, bool hasPlatformImpl, bool compilePlatform)
{
    if (!compilePlatform || hasPlatformImpl) {
        return;
    }

    for (auto& inhType : inheritedTypes) {
        auto decl = Ty::GetDeclOfTy(inhType->ty);
        if (decl && decl->platformImplementation) {
            inhType->ty = decl->platformImplementation->ty;
        }
    }
}
