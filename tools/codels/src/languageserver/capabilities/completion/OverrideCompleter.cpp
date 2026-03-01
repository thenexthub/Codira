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

#include "OverrideCompleter.h"
#include "../../common/Utils.h"
namespace ark {
using namespace Codira::AST;

void OverrideCompleter::FindOvrrideFunction()
{
    // find all super class
    std::vector<Ptr<ClassLikeDecl>> canSuperCall = GetSuperCallDecls(topLevelDecl);
    if (canSuperCall.empty()) {
        return;
    }
    // get generic name
    ExtractReplace(topLevelDecl);
    for (auto superDecl: canSuperCall) {
        if (superDecl == nullptr) {
            continue;
        }
        ExtractReplace(superDecl);
    }
    // get super class function member
    for (auto superDecl: canSuperCall) {
        if (superDecl == nullptr || superDecl == topLevelDecl || IsHiddenDecl(superDecl)) {
            continue;
        }
        std::optional<std::unordered_map<std::string, std::string>> replace;
        if (genericTypeMap.find(superDecl) != genericTypeMap.end()) {
            replace = genericTypeMap.at(superDecl);
        }
        for (auto& memberDecl : superDecl->GetMemberDeclPtrs()) {
            auto decl = DynamicCast<FuncDecl>(memberDecl);
            if (decl == nullptr || IsHiddenDecl(decl)) {
                continue;
            }
            if (modifier.has_value() && !decl->TestAttr(modifier.value())) {
                continue;
            }
            // if open and not constructor function, store in funcMap
            if (decl->TestAttr(Attribute::OPEN) && !decl->TestAttr(Attribute::CONSTRUCTOR) &&
                !CheckRepeated(decl)) {
                CompleteFuncDecl(decl, replace);
            }
            implementedMethods.emplace_back(decl);
        }
    }
}

void OverrideCompleter::CompleteFuncDecl(Ptr<FuncDecl> decl,
        const std::optional<std::unordered_map<std::string, std::string>>& replace)
{
    if (decl == nullptr) {
        return;
    }
    if (!IsMatchingCompletion(prefix, decl->identifier)) {
        return;
    }
    auto funcDetail = ResolveFuncDetail(decl);
    if (replace.has_value()) {
        for (auto& rule: replace.value()) {
            funcDetail.SetGenericType(rule.first, rule.second);
        }
    }
    functionDetails.emplace_back(std::move(funcDetail));
}

std::vector<CodeCompletion> OverrideCompleter::ExportItems()
{
    std::vector<CodeCompletion> result;
    std::unordered_set<std::string> signatureSet;
    for (auto& func : functionDetails) {
        auto signature = func.GetFunctionName();
        CodeCompletion completion;
        completion.label = "override " + GetFuncLabel(func);
        completion.name = signature;
        completion.kind = CompletionItemKind::CIK_FUNCTION;
        completion.detail = "(function) override " + GetFuncLabel(func) + " {}";
        completion.insertText = func.GetFunctionWithRet() + " {\n\t$0\n}";
        completion.show = true;
        TextEdit textEdit;
        textEdit.range.start = additionalPos;
        textEdit.range.end = additionalPos;
        textEdit.newText = GetTextString(func.modifiers);
        completion.additionalTextEdits = {textEdit};
        if (signatureSet.find(signature) == signatureSet.end()) {
            signatureSet.insert(signature);
            result.push_back(completion);
        }
    }
    return result;
}

bool OverrideCompleter::SetCompletionConfig(Ptr<Decl> decl, const Position& pos)
{
    FilterModifiers();
    if (decl == nullptr || decl->astKind != ASTKind::FUNC_DECL) {
        additionalPos = pos;
        additionalPos.column = pos.column - prefix.size() - 1;
        additionalPos.line = pos.line - 1;
        hasFuncWord = false;
        return true;
    }
    if (!CheckConfilctModifer(decl)) {
        return false;
    }
    additionalPos = decl->GetBegin();
    additionalPos.column--;
    additionalPos.line--;
    // get modifer
    if (decl->TestAttr(Attribute::PUBLIC)) {
        modifier = Attribute::PUBLIC;
    } else if (decl->TestAttr(Attribute::PROTECTED)) {
        modifier = Attribute::PROTECTED;
    } else if (decl->TestAttr(Attribute::PRIVATE)) {
        modifier = Attribute::PRIVATE;
    }
    // get func decl modifer
    ResolveDeclModifiers(decl);
    return true;
}

void OverrideCompleter::ResolveDeclModifiers(const Ptr<Decl> &decl)
{
    for (const auto& item: attr2str) {
        if (decl->TestAttr(item.first)) {
            curModifierSet.insert(item.second);
        }
    }
}

std::string OverrideCompleter::GetModifierString(const std::vector<std::string>& strs,
                                                 const std::unordered_set<std::string>& filter)
{
    const std::string delimiter = " ";
    std::string s;
    for (unsigned int i = 0; i < strs.size(); ++i) {
        if (filter.find(strs[i]) != filter.end()) {
            continue;
        }
        s += strs[i] + delimiter;
    }
    return s;
}

std::string OverrideCompleter::GetTextString(const std::vector<std::string>& strs)
{
    auto modifierText = GetModifierString(strs, curModifierSet);
    modifierText = "override " + modifierText;
    if (!hasFuncWord) {
        modifierText += "func ";
    }
    return modifierText;
}

void OverrideCompleter::FilterModifiers()
{
    if (topLevelDecl == nullptr) {
        return;
    }
    modifierSet.insert("abstract");
    modifierSet.insert("open");
    if (topLevelDecl->astKind == ASTKind::INTERFACE_DECL) {
        modifierSet.insert("public");
    }
    curModifierSet = modifierSet;
}

std::vector<Ptr<ClassLikeDecl>> OverrideCompleter::GetSuperCallDecls(Ptr<Decl> decl)
{
    if (auto classDecl = DynamicCast<InheritableDecl>(decl)) {
        return classDecl->GetAllSuperDecls();
    }
    return {};
}

void OverrideCompleter::ExtractReplace(Ptr<Decl> decl)
{
    auto inheritableDecl = DynamicCast<InheritableDecl>(decl);
    if (!inheritableDecl) {
        return;
    }
    const auto& inheritedTypes = inheritableDecl->inheritedTypes;
    std::unordered_map<std::string, std::string> replace{};
    if (genericTypeMap.find(inheritableDecl) != genericTypeMap.end()) {
        replace = genericTypeMap[inheritableDecl];
    }
    for (const auto& inheritedType: inheritedTypes) {
        Ptr<ClassLikeDecl> inheritedDecl = nullptr;
        if (auto clsTy = DynamicCast<ClassTy*>(inheritedType->ty)) {
            inheritedDecl = clsTy->declPtr;
        } else if (auto ifTy = DynamicCast<InterfaceTy*>(inheritedType->ty)) {
            inheritedDecl = ifTy->declPtr;
        }
        if (!inheritedDecl || !inheritedType->ty) {
            continue;
        }
        auto originalDetail = ResolveType(inheritedDecl->ty);
        if (!originalDetail) {
            return;
        }
        auto newDetail = ResolveType(inheritedType->ty);
        for (auto& item: replace) {
            newDetail->SetIdentifier(item.first, item.second);
        }
        std::unordered_map<std::string, std::string> myReplace;
        originalDetail->Diff(newDetail, myReplace);
        genericTypeMap[inheritedDecl] = myReplace;
    }
}

bool OverrideCompleter::CheckConfilctModifer(Ptr<Decl> decl)
{
    if (decl == nullptr) {
        return true;
    }
    if (decl->TestAttr(Codira::AST::Attribute::STATIC)) {
        return false;
    }
    return true;
}

bool OverrideCompleter::CheckRepeated(Ptr<FuncDecl> decl)
{
    return std::any_of(implementedMethods.begin(), implementedMethods.end(),
                        [decl](Ptr<FuncDecl> cmpDecl)-> bool {
        if (!decl || !cmpDecl) {
            return false;
        }
        return IsFuncSignatureIdentical(*cmpDecl, *decl);
    });
}

std::string OverrideCompleter::GetFuncLabel(const FuncDetail& funcDetail)
{
    std::string detail = GetModifierString(funcDetail.modifiers, modifierSet);
    detail += "func ";
    detail += funcDetail.identifier;
    detail += "(";
    detail += funcDetail.params.ToString();
    detail += "): ";
    if (funcDetail.retType) {
        detail += funcDetail.retType->ToString();
    }
    return detail;
}
} // namespace ark
