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
 * This file provides the function of checking APILevel customized macros.
 */

#include "PluginCustomAnnoChecker.h"

#include <functional>
#include <iterator>
#include <iostream>
#include <stack>
#include <unordered_map>

#include "ParseJson.h"
#include "Codira/AST/Create.h"
#include "Codira/AST/Match.h"
#include "Codira/AST/Walker.h"
#include "Codira/Basic/StringConvertor.h"
#include "Codira/Utils/CastingTemplate.h"
#include "Codira/Utils/SafePointer.h"
#include "Codira/Utils/StdUtils.h"

using namespace Codira;
using namespace AST;
using namespace PluginCheck;

namespace {
constexpr std::string_view PKG_NAME_OHOS_LABELS = "ohos.labels";
constexpr std::string_view APILEVEL_ANNO_NAME = "APILevel";
constexpr std::string_view SINCE_IDENTIFIER = "since";
constexpr std::string_view LEVEL_IDENTIFIER = "level";
constexpr std::string_view SYSCAP_IDENTIFIER = "syscap";
constexpr std::string_view CFG_PARAM_LEVEL_NAME = "APILevel_level";
constexpr std::string_view CFG_PARAM_SYSCAP_NAME = "APILevel_syscap";
// For level check:
const LevelType IFAVAILABLE_LOWER_LIMITLEVEL = 19;

// For Annotation Hide:
constexpr std::string_view HIDE_ANNO_NAME = "Hide";
constexpr std::string_view HIDE_ARG_NAME = "isChecked";

LevelType Str2LevelType(std::string s)
{
    return static_cast<LevelType>(Stoull(s).value_or(0));
}

void ParseLevel(const Expr& e, PluginCustomAnnoInfo& apilevel, DiagnosticEngine& diag)
{
    Ptr<const LitConstExpr> lce = nullptr;
    if (e.astKind == ASTKind::BINARY_EXPR) {
        auto be = StaticCast<BinaryExpr>(&e);
        CODEC_NULLPTR_CHECK(be->rightExpr);
        lce = DynamicCast<LitConstExpr>(be->rightExpr.get());
    } else if (e.astKind == ASTKind::LIT_CONST_EXPR) {
        lce = StaticCast<LitConstExpr>(&e);
    }
    if (!lce || lce->kind != LitConstKind::INTEGER) {
        diag.DiagnoseRefactor(DiagKindRefactor::sema_only_literal_support, e, "integer");
        return;
    }
    auto newLevel = Str2LevelType(lce->stringValue);
    apilevel.since = apilevel.since == 0 ? newLevel : std::min(newLevel, apilevel.since);
}

void ParseSince(const Expr& e, PluginCustomAnnoInfo& apilevel, DiagnosticEngine& diag)
{
    Ptr<const LitConstExpr> lce = nullptr;
    if (e.astKind == ASTKind::BINARY_EXPR) {
        auto be = StaticCast<BinaryExpr>(&e);
        CODEC_NULLPTR_CHECK(be->rightExpr);
        lce = DynamicCast<LitConstExpr>(be->rightExpr.get());
    } else if (e.astKind == ASTKind::LIT_CONST_EXPR) {
        lce = StaticCast<LitConstExpr>(&e);
    }
    if (!lce || lce->kind != LitConstKind::STRING) {
        diag.DiagnoseRefactor(DiagKindRefactor::sema_only_literal_support, e, "string");
        return;
    }
    auto newLevel = Str2LevelType(lce->stringValue);
    apilevel.since = apilevel.since == 0 ? newLevel : std::min(newLevel, apilevel.since);
}

void ParseSysCap(const Expr& e, PluginCustomAnnoInfo& apilevel, DiagnosticEngine& diag)
{
    Ptr<const LitConstExpr> lce = nullptr;
    if (e.astKind == ASTKind::CALL_EXPR) {
        auto ce = StaticCast<CallExpr>(&e);
        CODEC_ASSERT(ce->args.size() == 1 && ce->args[0]->expr);
        lce = DynamicCast<LitConstExpr>(ce->args[0]->expr.get());
    } else if (e.astKind == ASTKind::LIT_CONST_EXPR) {
        lce = StaticCast<LitConstExpr>(&e);
    }
    if (!lce || lce->kind != LitConstKind::STRING) {
        diag.DiagnoseRefactor(DiagKindRefactor::sema_only_literal_support, e, "string");
        return;
    }
    apilevel.syscap = lce->stringValue;
}

void ParseCheckingHide(const Expr& e, PluginCustomAnnoInfo& apilevel, DiagnosticEngine& diag)
{
    Ptr<const LitConstExpr> lce = nullptr;
    if (e.astKind == ASTKind::LIT_CONST_EXPR) {
        lce = StaticCast<LitConstExpr>(&e);
    }
    if (!lce || lce->kind != LitConstKind::BOOL) {
        diag.DiagnoseRefactor(DiagKindRefactor::sema_only_literal_support, e, "Bool");
        return;
    }
    apilevel.hasHideAnno =
        (apilevel.hasHideAnno.has_value() && apilevel.hasHideAnno.value()) || lce->constNumValue.asBoolean;
}

using ParseNameParamFunc = std::function<void(const Expr&, PluginCustomAnnoInfo&, DiagnosticEngine&)>;
std::unordered_map<std::string_view, ParseNameParamFunc> parseNameParam = {
    {SINCE_IDENTIFIER, ParseSince},
    {LEVEL_IDENTIFIER, ParseLevel},
    {SYSCAP_IDENTIFIER, ParseSysCap},
    {HIDE_ARG_NAME, ParseCheckingHide},
};

void ClearAnnoInfoOfDepPkg(ImportManager& importManager)
{
    auto clearAnno = [](Ptr<Node> node) {
        auto decl = DynamicCast<Decl>(node);
        if (!decl) {
            return VisitAction::WALK_CHILDREN;
        }
        auto isCustomAnno = [](auto& a) { return a->kind == AnnotationKind::CUSTOM; };
        decl->annotations.erase(
            std::remove_if(decl->annotations.begin(), decl->annotations.end(), isCustomAnno), decl->annotations.end());
        return VisitAction::WALK_CHILDREN;
    };
    for (auto& [fullPackageName, _] : importManager.GetDepPkgCodeoPaths()) {
        auto depPkg = importManager.GetPackage(fullPackageName);
        if (!depPkg) {
            continue;
        }
        Walker(depPkg, clearAnno).Walk();
    }
}

void MarkTargetAsExternalWeak(Ptr<Node> node)
{
    if (!node) {
        return;
    }
    Ptr<Decl> target = nullptr;
    if (node->GetTarget()) {
        target = node->GetTarget();
    } else if (auto ce = DynamicCast<CallExpr>(node); ce && ce->resolvedFunction) {
        target = ce->resolvedFunction;
    }
    if (!target) {
        return;
    }
    target->linkage = Linkage::EXTERNAL_WEAK;
    if (auto fd = DynamicCast<FuncDecl>(target)) {
        for (auto& param : fd->funcBody->paramLists[0]->params) {
            if (param->desugarDecl) {
                param->desugarDecl->linkage = Linkage::EXTERNAL_WEAK;
            }
        }
        if (fd->propDecl) {
            fd->propDecl->linkage = Linkage::EXTERNAL_WEAK;
        }
    } else if (auto md = DynamicCast<MacroDecl>(target)) {
        if (md->desugarDecl) {
            md->desugarDecl->linkage = Linkage::EXTERNAL_WEAK;
        }
    } else if (auto pd = DynamicCast<PropDecl>(target)) {
        for (auto& getter : pd->getters) {
            if (!getter) {
                continue;
            }
            getter->linkage = Linkage::EXTERNAL_WEAK;
        }
        for (auto& setter : pd->setters) {
            if (!setter) {
                continue;
            }
            setter->linkage = Linkage::EXTERNAL_WEAK;
        }
    }
    if (target->outerDecl && target->outerDecl->IsNominalDecl()) {
        target->outerDecl->linkage = Linkage::EXTERNAL_WEAK;
        MarkTargetAsExternalWeak(target->outerDecl);
    }
}

inline std::string GetModuleName(const std::string& fullPackageName)
{
    return fullPackageName.substr(0, fullPackageName.find('.'));
}

std::string FormatSyscapsString(const std::string& scopeSyscap, const SysCapSet& syscapSet)
{
    std::stringstream scopeSyscapsStr;
    // 3 is maximum number of syscap limit.
    size_t limit = 3;
    size_t count = 0;
    if (!scopeSyscap.empty() && syscapSet.find(scopeSyscap) == syscapSet.end()) {
        scopeSyscapsStr << scopeSyscap;
        ++count;
    }
    for (const auto& syscap : syscapSet) {
        if (count >= limit) {
            scopeSyscapsStr << "...";
            break;
        }
        if (count > 0) {
            scopeSyscapsStr << ", ";
        }
        scopeSyscapsStr << syscap;
        ++count;
    }
    return scopeSyscapsStr.str();
}
} // namespace

bool PluginCustomAnnoChecker::ParseJsonFile(const std::vector<uint8_t>& in) noexcept
{
    size_t startPos = static_cast<size_t>(std::find(in.begin(), in.end(), '{') - in.begin());
    auto root = ParseJsonObject(startPos, in);
    auto deviceSysCapObj = GetJsonObject(root, "deviceSysCap", 0);
    if (!deviceSysCapObj) {
        return false;
    }
    std::map<std::string, SysCapSet> dev2SyscapsMap;
    for (auto& subObj : deviceSysCapObj->pairs) {
        SysCapSet syscapsOneDev;
        for (auto path : subObj->valueStr) {
            std::vector<uint8_t> buffer;
            std::string failedReason;
            FileUtil::ReadBinaryFileToBuffer(path, buffer, failedReason);
            if (!failedReason.empty()) {
                diag.DiagnoseRefactor(
                    DiagKindRefactor::module_read_file_to_buffer_failed, DEFAULT_POSITION, path, failedReason);
                return false;
            }
            startPos = static_cast<size_t>(std::find(buffer.begin(), buffer.end(), '{') - buffer.begin());
            auto rootOneDevice = ParseJsonObject(startPos, buffer);
            auto curSyscaps = GetJsonString(rootOneDevice, "SysCaps");
            for (auto& syscap : curSyscaps) {
                syscapsOneDev.insert(syscap);
            }
        }
        dev2SyscapsMap.emplace(subObj->key, syscapsOneDev);
    }
    std::optional<SysCapSet> lastSyscap = std::nullopt;
    for (auto& dev2Syscaps : dev2SyscapsMap) {
        SysCapSet& curSyscaps = dev2Syscaps.second;
        SysCapSet intersection;
        if (lastSyscap.has_value()) {
            std::set_intersection(lastSyscap.value().begin(), lastSyscap.value().end(), curSyscaps.begin(),
                curSyscaps.end(), std::inserter(intersection, intersection.end()));
        } else {
            intersection = curSyscaps;
        }
        lastSyscap = intersection;
        for (auto& syscap : curSyscaps) {
            unionSet.insert(syscap);
        }
    }
    if (lastSyscap) {
        intersectionSet = std::move(*lastSyscap);
    }
    return true;
}

void PluginCustomAnnoChecker::ParseOption() noexcept
{
    auto& option = ci.invocation.globalOptions;
    auto found = option.passedWhenKeyValue.find(std::string(CFG_PARAM_LEVEL_NAME));
    if (found != option.passedWhenKeyValue.end()) {
        globalLevel = Str2LevelType(found->second);
        optionWithLevel = true;
    }
    found = option.passedWhenKeyValue.find(std::string(CFG_PARAM_SYSCAP_NAME));
    if (found != option.passedWhenKeyValue.end()) {
        auto syscapsCfgPath = found->second;
        std::vector<uint8_t> jsonContent;
        std::string failedReason;
        FileUtil::ReadBinaryFileToBuffer(syscapsCfgPath, jsonContent, failedReason);
        if (!failedReason.empty()) {
            diag.DiagnoseRefactor(
                DiagKindRefactor::module_read_file_to_buffer_failed, DEFAULT_POSITION, syscapsCfgPath, failedReason);
            return;
        }
        optionWithSyscap = ParseJsonFile(jsonContent);
    }
}

bool PluginCustomAnnoChecker::IsAnnoAPILevel(Ptr<Annotation> anno, [[maybe_unused]] const Decl& decl)
{
    if (ctx && ctx->curPackage && ctx->curPackage->fullPackageName == PKG_NAME_OHOS_LABELS) {
        return anno->identifier == APILEVEL_ANNO_NAME;
    }
    if (!anno) {
        return false;
    }
    auto target = anno->baseExpr ? anno->baseExpr->GetTarget() : nullptr;
    if (target) {
        // With semantic info, check by target and its package name.
        return target->GetFullPackageName() == PKG_NAME_OHOS_LABELS && target->outerDecl &&
            target->outerDecl->identifier == APILEVEL_ANNO_NAME;
    }
    // Without semantic info, check by annotation name only.
    return anno->identifier == APILEVEL_ANNO_NAME;
}

bool PluginCustomAnnoChecker::IsAnnoHide(Ptr<Annotation> anno)
{
    if (ctx && ctx->curPackage && ctx->curPackage->fullPackageName == PKG_NAME_OHOS_LABELS) {
        return anno->identifier == HIDE_ANNO_NAME;
    }
    if (!anno) {
        return false;
    }
    auto target = anno->baseExpr ? anno->baseExpr->GetTarget() : nullptr;
    if (target) {
        // With semantic info, check by target and its package name.
        return target->GetFullPackageName() == PKG_NAME_OHOS_LABELS && target->outerDecl &&
            target->outerDecl->identifier == HIDE_ANNO_NAME;
    }
    // Without semantic info, check by annotation name only.
    return anno->identifier == HIDE_ANNO_NAME;
}

void PluginCustomAnnoChecker::ParseHideArg(const Annotation& anno, PluginCustomAnnoInfo& annoInfo)
{
    if (anno.args.empty() || !anno.args[0] || !anno.args[0]->expr) {
        annoInfo.hasHideAnno = false;
        return;
    }
    std::string argName = anno.args[0]->name.Val();
    if (argName != HIDE_ARG_NAME) {
        // Should diagnostic before here.
        return;
    }
    parseNameParam[argName](*anno.args[0]->expr.get(), annoInfo, diag);
}

void PluginCustomAnnoChecker::ParseAPILevelArgs(
    const Decl& decl, const Annotation& anno, PluginCustomAnnoInfo& annoInfo)
{
    for (size_t i = 0; i < anno.args.size(); ++i) {
        // To support old APILevel definition that constructor parameter list is 'level: Int8, ...'.
        std::string_view argName = anno.args[i]->name.Val();
        if (argName.empty()) {
            argName = LEVEL_IDENTIFIER;
        }
        if (parseNameParam.count(argName) <= 0) {
            continue;
        }
        std::string preSyscap = annoInfo.syscap;
        parseNameParam[argName](*anno.args[i]->expr.get(), annoInfo, diag);
        if (!preSyscap.empty() && preSyscap != annoInfo.syscap) {
            diag.DiagnoseRefactor(DiagKindRefactor::sema_apilevel_multi_diff_syscap, decl);
        }
    }
    // In the APILevel definition, only "since" does not provide a default value. Here, the warning that
    // there is an issue with the APILevel annotation, which may originnate from the code.d file.
    if (annoInfo.since == 0) {
        diag.DiagnoseRefactor(DiagKindRefactor::sema_apilevel_missing_arg, anno.begin, "since!: String");
    }
}

void PluginCustomAnnoChecker::CheckHideOfExtendDecl(const Decl& decl, const PluginCustomAnnoInfo& annoInfo)
{
    if (decl.astKind != ASTKind::EXTEND_DECL) {
        return;
    }
    auto extendedDecl = Ty::GetDeclPtrOfTy(decl.ty);
    if (!extendedDecl) {
        return;
    }
    PluginCustomAnnoInfo extendedAnnoInfo;
    Parse(*extendedDecl, extendedAnnoInfo);

    if (extendedAnnoInfo.hasHideAnno.has_value() && !annoInfo.hasHideAnno.has_value()) {
        // @!Hide class A{}; extend A {} -> error
        // class A{}; @!Hide extend A {} -> ok
        auto builder = diag.DiagnoseRefactor(DiagKindRefactor::sema_hide_missing_hide, decl);
        builder.AddNote(*extendedDecl, "the extended declaration is marked with '@!Hide'");
    } else if (extendedAnnoInfo.hasHideAnno.has_value() && annoInfo.hasHideAnno.has_value() &&
        extendedAnnoInfo.hasHideAnno.value() && !annoInfo.hasHideAnno.value()) {
        // @!Hide[isChecked: true] class A {}; @!Hide[isChecked: false] extend A {} -> error
        // @!Hide[isChecked: false] class A {}; @!Hide[isChecked: true] extend A {} -> ok
        // @!Hide[isChecked: false] class A {}; @!Hide[isChecked: false] extend A {} -> ok
        // @!Hide[isChecked: true] class A {}; @!Hide[isChecked: true] extend A {} -> ok
        auto builder = diag.DiagnoseRefactor(
            DiagKindRefactor::sema_hide_diff_param, decl, annoInfo.hasHideAnno.value() ? "true" : "false");
        builder.AddNote(*extendedDecl, "should be same with");
    }

    if (!annoInfo.hasHideAnno.has_value()) {
        return;
    }
    for (auto member : decl.GetMemberDeclPtrs()) {
        PluginCustomAnnoInfo memberAnnoInfo;
        Parse(*member, memberAnnoInfo);
        if (memberAnnoInfo.hasHideAnno.has_value() &&
            memberAnnoInfo.hasHideAnno.value() != annoInfo.hasHideAnno.value()) {
            auto builder = diag.DiagnoseRefactor(
                DiagKindRefactor::sema_hide_diff_param, *member, memberAnnoInfo.hasHideAnno.value() ? "true" : "false");
            builder.AddNote(decl, "should be same with");
        }
    }
}

void PluginCustomAnnoChecker::CheckHideOfOverrideFunction(const Decl& decl, const PluginCustomAnnoInfo& annoInfo)
{
    if (decl.astKind != ASTKind::FUNC_DECL || !decl.outerDecl) {
        return;
    }
    auto fd = StaticCast<FuncDecl>(&decl);
    CODEC_NULLPTR_CHECK(ci.typeManager);
    std::optional<bool> functionWithHide = annoInfo.hasHideAnno;
    if (!functionWithHide.has_value()) {
        PluginCustomAnnoInfo outerAnnoInfo;
        Parse(*decl.outerDecl, outerAnnoInfo);
        if (outerAnnoInfo.hasHideAnno.has_value()) {
            functionWithHide = outerAnnoInfo.hasHideAnno.value();
        }
    }

    auto overriddenFd = ci.typeManager->GetTopOverriddenFuncDecl(fd);
    if (!overriddenFd) {
        return;
    }
    PluginCustomAnnoInfo overriddenAnnoInfo;
    Parse(*overriddenFd, overriddenAnnoInfo);
    std::optional<bool> overriddenFdWithHide = overriddenAnnoInfo.hasHideAnno;
    if (!overriddenFdWithHide.has_value()) {
        CODEC_NULLPTR_CHECK(overriddenFd->outerDecl);
        PluginCustomAnnoInfo outerAnnoInfo;
        Parse(*overriddenFd->outerDecl, outerAnnoInfo);
        if (outerAnnoInfo.hasHideAnno.has_value()) {
            overriddenFdWithHide = outerAnnoInfo.hasHideAnno.value();
        }
    }

    if (functionWithHide.has_value() && !overriddenFdWithHide.has_value()) {
        // func f(): Unit {}; @!Hide override func f(): Unit {} -> error
        auto builder = diag.DiagnoseRefactor(DiagKindRefactor::sema_hide_missing_hide, *overriddenFd);
        builder.AddNote(*fd, "the override function is marked with '@!Hide'");
    } else if (!functionWithHide.has_value() && overriddenFdWithHide.has_value()) {
        // @!Hide func f(): Unit {}; override func f(): Unit {} -> ok for now
    } else if (functionWithHide.has_value() && functionWithHide.value() != overriddenFdWithHide.value()) {
        // @!Hide[isChecked: true] func f(): Unit {}; @!Hide[isChecked: false] override func f(): Unit {} -> error
        // @!Hide[isChecked: false] func f(): Unit {}; @!Hide[isChecked: true] override func f(): Unit {} -> error
        // @!Hide[isChecked: true] func f(): Unit {}; @!Hide[isChecked: true] override func f(): Unit {} -> ok
        // @!Hide[isChecked: false] func f(): Unit {}; @!Hide[isChecked: false] override func f(): Unit {} -> ok
        auto builder = diag.DiagnoseRefactor(
            DiagKindRefactor::sema_hide_diff_param, decl, functionWithHide.value() ? "true" : "false");
        builder.AddNote(*overriddenFd, "should be same with");
    }
}

void PluginCustomAnnoChecker::Parse(const Decl& decl, PluginCustomAnnoInfo& annoInfo)
{
    if (auto found = levelCache.find(&decl); found != levelCache.end()) {
        annoInfo.since = annoInfo.since == 0 ? found->second.since : std::min(found->second.since, annoInfo.since);
        annoInfo.syscap = found->second.syscap;
        if (found->second.hasHideAnno.has_value()) {
            annoInfo.hasHideAnno = found->second.hasHideAnno;
        } else if (annoInfo.hasHideAnno.has_value()) {
            // keep the existing value.
        } else {
            annoInfo.hasHideAnno = std::nullopt;
        }
        return;
    }
    bool hideExist = false;
    for (auto& anno : decl.annotations) {
        if (!anno) {
            continue;
        }
        if (IsAnnoHide(anno)) {
            if (hideExist) {
                diag.DiagnoseRefactor(DiagKindRefactor::sema_hide_multi_annotation, decl);
                continue;
            }
            hideExist = true;
            if (auto param = DynamicCast<FuncParam>(&decl); param && !param->isMemberParam) {
                diag.DiagnoseRefactor(DiagKindRefactor::sema_hide_at_func_param, decl);
                continue;
            }
            if (!anno->isCompileTimeVisible) {
                diag.DiagnoseRefactor(DiagKindRefactor::sema_hide_compile_time_invisible, *anno);
            }
            ParseHideArg(*anno, annoInfo);
        } else if (IsAnnoAPILevel(anno.get(), decl)) {
            ParseAPILevelArgs(decl, *anno, annoInfo);
        }
    }
    levelCache[&decl] = annoInfo;
    CheckHideOfExtendDecl(decl, annoInfo);
    CheckHideOfOverrideFunction(decl, annoInfo);
}

bool PluginCustomAnnoChecker::CheckLevel(
    const Decl& target, const PluginCustomAnnoInfo& scopeAnnoInfo, DiagConfig diagCfg)
{
    if (!optionWithLevel) {
        return true;
    }
    LevelType scopeLevel = scopeAnnoInfo.since != 0 ? scopeAnnoInfo.since : globalLevel;
    PluginCustomAnnoInfo targetAPILevel;
    Parse(target, targetAPILevel);
    if (targetAPILevel.since > scopeLevel && !diagCfg.node->begin.IsZero()) {
        if (diagCfg.reportDiag && !diagCfg.message.empty()) {
            diag.DiagnoseRefactor(DiagKindRefactor::sema_apilevel_ref_higher, *diagCfg.node, diagCfg.message[0],
                std::to_string(targetAPILevel.since), std::to_string(scopeLevel));
        }
        return false;
    }
    return true;
}

bool PluginCustomAnnoChecker::CheckSyscap(
    const Decl& target, const PluginCustomAnnoInfo& scopeAnnoInfo, DiagConfig diagCfg)
{
    if (!optionWithSyscap) {
        return true;
    }
    PluginCustomAnnoInfo targetAPILevel;
    Parse(target, targetAPILevel);
    std::string targetLevel = targetAPILevel.syscap;
    if (targetLevel.empty()) {
        return true;
    }
    // Create a lambda for diagnostic purposes that only creates a temporary collection when needed.
    auto diagForSyscap = [this, &diagCfg, &targetLevel](
                             const std::string& scopeSyscap, const SysCapSet& syscapSet, DiagKindRefactor kind) {
        auto builder = diag.DiagnoseRefactor(kind, *diagCfg.node, targetLevel);
        std::string formattedSyscaps = FormatSyscapsString(scopeSyscap, syscapSet);
        builder.AddNote("the following syscaps are supported: " + formattedSyscaps);
    };

    // Check unionSet (all possible syscaps)
    // If scopeAnnoInfo.syscap is not empty and equals targetLevel, it is considered a match.
    bool inUnionSet = (unionSet.find(targetLevel) != unionSet.end()) ||
        (!scopeAnnoInfo.syscap.empty() && scopeAnnoInfo.syscap == targetLevel);
    if (!inUnionSet && !diagCfg.node->begin.IsZero()) {
        if (diagCfg.reportDiag) {
            diagForSyscap(scopeAnnoInfo.syscap, unionSet, DiagKindRefactor::sema_apilevel_syscap_error);
        }
        return false;
    }

    // Check intersectionSet (all syscaps supported by all devices)
    // If scopeAnnoInfo.syscap is not empty and equals targetLevel, it is considered a match.
    bool inIntersectionSet = (intersectionSet.find(targetLevel) != intersectionSet.end()) ||
        (!scopeAnnoInfo.syscap.empty() && scopeAnnoInfo.syscap == targetLevel);
    if (!inIntersectionSet && !diagCfg.node->begin.IsZero()) {
        if (diagCfg.reportDiag) {
            diagForSyscap(scopeAnnoInfo.syscap, intersectionSet, DiagKindRefactor::sema_apilevel_syscap_warning);
        }
        return false;
    }
    return true;
}

bool PluginCustomAnnoChecker::CheckCheckingHide(const Decl& target, DiagConfig diagCfg)
{
    PluginCustomAnnoInfo targetPluginAnnoInfo;
    Parse(target, targetPluginAnnoInfo);
    if (targetPluginAnnoInfo.hasHideAnno.has_value() && targetPluginAnnoInfo.hasHideAnno.value() &&
        curModuleName != GetModuleName(target.GetFullPackageName())) {
        if (diagCfg.reportDiag && !diagCfg.message.empty()) {
            diag.DiagnoseRefactor(DiagKindRefactor::sema_undeclared_identifier, *diagCfg.node, diagCfg.message[0]);
        }
        return false;
    }
    return true;
}

bool PluginCustomAnnoChecker::CheckNode(Ptr<Node> node, const PluginCustomAnnoInfo& scopeAnnoInfo, bool reportDiag)
{
    if (!node) {
        return true;
    }
    auto target = node->GetTarget();
    if (auto ce = DynamicCast<CallExpr>(node); ce && ce->resolvedFunction) {
        if (ce->callKind == CallKind::CALL_SUPER_FUNCTION) {
            // The check has been completed in the parent type checker.
            return false;
        }
        target = ce->resolvedFunction;
    }
    if (!target) {
        return true;
    }
    bool ret = true;
    if (target->outerDecl) {
        auto identifier = target->outerDecl->identifier.Val();
        if (identifier.empty()) {
            identifier = target->identifier.Val();
        }
        ret = ret && CheckCheckingHide(*target->outerDecl, {reportDiag, node, {identifier}});
        ret = ret && CheckLevel(*target->outerDecl, scopeAnnoInfo, {reportDiag, node, {identifier}});
        ret = ret && CheckSyscap(*target->outerDecl, scopeAnnoInfo, {reportDiag, node, {}});
        if (!ret) {
            return false;
        }
    }
    // The priority of the Hide check is higher than that of the APILevel.
    ret = ret && CheckCheckingHide(*target, {reportDiag, node, {target->identifier.Val()}});
    ret = ret && CheckLevel(*target, scopeAnnoInfo, {reportDiag, node, {target->identifier.Val()}});
    ret = ret && CheckSyscap(*target, scopeAnnoInfo, {reportDiag, node, {target->identifier.Val()}});
    return ret;
}

void PluginCustomAnnoChecker::CheckIfAvailableExpr(IfAvailableExpr& iae, const PluginCustomAnnoInfo& scopeAnnoInfo)
{
    if (!iae.desugarExpr || iae.desugarExpr->astKind != ASTKind::IF_EXPR) {
        return;
    }
    auto ifExpr = StaticCast<IfExpr>(iae.desugarExpr.get());
    Ptr<FuncArg> arg = iae.GetArg();
    if (parseNameParam.count(arg->name.Val()) <= 0) {
        return;
    }
    auto ifscopeAnnoInfo = PluginCustomAnnoInfo();
    parseNameParam[arg->name.Val()](*ifExpr->condExpr, ifscopeAnnoInfo, diag);
    if (ifscopeAnnoInfo.since != 0 && ifscopeAnnoInfo.since < IFAVAILABLE_LOWER_LIMITLEVEL) {
        diag.DiagnoseRefactor(DiagKindRefactor::sema_ifavailable_level_limit, *arg);
        return;
    }
    // if branch.
    auto checkerIf = [this, &ifscopeAnnoInfo, &scopeAnnoInfo](Ptr<Node> node) -> VisitAction {
        if (auto e = DynamicCast<IfAvailableExpr>(node)) {
            CheckIfAvailableExpr(*e, ifscopeAnnoInfo);
            return VisitAction::SKIP_CHILDREN;
        }
        // If the reference meets the 'IfAvaliable' condition but does not meet the global APILevel configuration, set
        // linkage to 'EXTERNAL_WEAK'.
        auto ret = CheckNode(node, ifscopeAnnoInfo);
        if (ret && !CheckNode(node, scopeAnnoInfo, false)) {
            MarkTargetAsExternalWeak(node);
        }
        if (!ret) {
            return VisitAction::SKIP_CHILDREN;
        }
        return VisitAction::WALK_CHILDREN;
    };
    Walker(ifExpr->thenBody.get(), checkerIf).Walk();
    // else branch.
    auto checkerElse = [this, &scopeAnnoInfo](Ptr<Node> node) -> VisitAction {
        if (auto e = DynamicCast<IfAvailableExpr>(node)) {
            CheckIfAvailableExpr(*e, scopeAnnoInfo);
            return VisitAction::SKIP_CHILDREN;
        }
        if (!CheckNode(node, scopeAnnoInfo)) {
            return VisitAction::SKIP_CHILDREN;
        }
        return VisitAction::WALK_CHILDREN;
    };
    Walker(ifExpr->elseBody.get(), checkerElse).Walk();
}

void PluginCustomAnnoChecker::CheckAnnoBeforeMacro(Package& pkg)
{
    const std::vector<std::string> annoName = {std::string(APILEVEL_ANNO_NAME), std::string(HIDE_ANNO_NAME)};
    auto checker = [this, &annoName](Ptr<Node> node) -> VisitAction {
        if (node->astKind != ASTKind::MACRO_EXPAND_DECL) {
            return VisitAction::WALK_CHILDREN;
        }
        auto med = StaticCast<MacroExpandDecl>(node);
        if (!Utils::In(med->identifier.Val(), annoName)) {
            return VisitAction::WALK_CHILDREN;
        }
        auto subDecl = med->invocation.decl.get();
        if (subDecl->astKind != ASTKind::MACRO_EXPAND_DECL) {
            return VisitAction::WALK_CHILDREN;
        }
        auto subMed = StaticCast<MacroExpandDecl>(subDecl);
        if (!Utils::In(subMed->identifier.Val(), annoName)) {
            diag.DiagnoseRefactor(DiagKindRefactor::sema_hide_must_at_end, med->begin, med->identifier);
            return VisitAction::SKIP_CHILDREN;
        }
        return VisitAction::WALK_CHILDREN;
    };
    for (auto& file : pkg.files) {
        for (auto& node : file->originalMacroCallNodes) {
            Walker(node, checker).Walk();
        }
    }
}

void PluginCustomAnnoChecker::Check(Package& pkg)
{
    ctx = ci.GetASTContextByPackage(&pkg);
    curModuleName = GetModuleName(pkg.fullPackageName);
    CheckAnnoBeforeMacro(pkg);
    std::vector<Ptr<Decl>> scopeDecl;
    auto checker = [this, &scopeDecl](Ptr<Node> node) -> VisitAction {
        if (auto decl = DynamicCast<Decl>(node)) {
            if (decl->astKind == ASTKind::PRIMARY_CTOR_DECL) {
                return VisitAction::SKIP_CHILDREN;
            }
            scopeDecl.emplace_back(decl);
            return VisitAction::WALK_CHILDREN;
        }
        PluginCustomAnnoInfo scopeAnnoInfo;
        for (auto it = scopeDecl.rbegin(); it != scopeDecl.rend(); ++it) {
            Parse(**it, scopeAnnoInfo);
        }
        if (auto iae = DynamicCast<IfAvailableExpr>(node)) {
            scopeAnnoInfo.since = scopeAnnoInfo.since == 0 ? globalLevel : scopeAnnoInfo.since;
            CheckIfAvailableExpr(*iae, scopeAnnoInfo);
            return VisitAction::SKIP_CHILDREN;
        }
        if (!CheckNode(node, scopeAnnoInfo)) {
            return VisitAction::SKIP_CHILDREN;
        }
        return VisitAction::WALK_CHILDREN;
    };
    auto popScope = [&scopeDecl](Ptr<Node> node) -> VisitAction {
        if (!scopeDecl.empty() && scopeDecl.back() == node) {
            scopeDecl.pop_back();
        }
        return VisitAction::WALK_CHILDREN;
    };
    Walker(&pkg, checker, popScope).Walk();
    // Clear the annotation information of the dependency package to avoid chir failure.
    // In the LSP scenario, annotation information still needs to be saved after SEMA.
    if (!ci.invocation.globalOptions.enableMacroInLSP) {
        ClearAnnoInfoOfDepPkg(importManager);
    }
}
