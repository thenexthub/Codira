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

#include "CompletionEnv.h"
#include <cangjie/Modules/ModulesUtils.h>

using namespace Codira;
using namespace Codira::AST;

namespace {
std::string TrimSubPkgName(const std::string &subPkgName)
{
    auto found = subPkgName.find_first_of(CONSTANTS::DOT);
    if (found != std::string::npos) {
        return subPkgName.substr(0, found);
    }
    return subPkgName;
}

bool IsAllParamInitial(FuncDecl &fd)
{
    if (!fd.funcBody || fd.funcBody->paramLists.empty()) {
        return false;
    }
    auto paramNum = fd.funcBody->paramLists[0]->params.size();
    if (paramNum == 0) {
        return false;
    }
    size_t count = 0;
    for (auto &param : fd.funcBody->paramLists[0]->params) {
        if (param->TestAttr(Attribute::HAS_INITIAL) || param->assignment != nullptr) {
            count++;
        }
    }
    return paramNum == count;
}

bool noComplete(Ptr<Codira::AST::Node> node, const ark::SyscapCheck& syscap, bool isCompleteFunction)
{
    if (node == nullptr) {
        return true;
    }
    bool hasAPILevel = false;
    bool result = syscap.CheckSysCap(node, hasAPILevel);
    if (hasAPILevel) {
        return !result;
    }
    return (isCompleteFunction && node->astKind != ASTKind::FUNC_DECL);
};
}

namespace ark {
void CompletionEnv::DealFuncDecl(Ptr<Node> node, const Position pos)
{
    auto pFuncDecl = dynamic_cast<FuncDecl*>(node.get());
    if (!pFuncDecl) { return; }
    DeepComplete(pFuncDecl->funcBody.get(), pos);
}

void CompletionEnv::DealMainDecl(Ptr<Node> node, const Position pos)
{
    auto pMainDecl = dynamic_cast<MainDecl*>(node.get());
    if (!pMainDecl) { return; }
    DeepComplete(pMainDecl->funcBody.get(), pos);
}

void CompletionEnv::DealMacroDecl(Ptr<Node> node, const Position pos)
{
    auto pMacroDecl = dynamic_cast<MacroDecl*>(node.get());
    if (!pMacroDecl) { return; }
    DeepComplete(pMacroDecl->funcBody.get(), pos);
}

void CompletionEnv::DealMacroExpandDecl(Ptr<Node> node, const Position pos)
{
    auto pMacroDecl = dynamic_cast<MacroExpandDecl*>(node.get());
    if (!pMacroDecl) { return; }
    DeepComplete(pMacroDecl->invocation.decl.get(), pos);
}

void CompletionEnv::DealMacroExpandExpr(Ptr<Node> node, const Position pos)
{
    auto pMacroDecl = dynamic_cast<MacroExpandExpr*>(node.get());
    if (!pMacroDecl) { return; }
    DeepComplete(pMacroDecl->invocation.decl.get(), pos);
}

void CompletionEnv::DealPrimaryCtorDecl(Ptr<Node> node, const Position pos)
{
    auto pPrimaryCtorDecl = dynamic_cast<PrimaryCtorDecl*>(node.get());
    if (!pPrimaryCtorDecl) { return; }
    DeepComplete(pPrimaryCtorDecl->funcBody.get(), pos);
}

void CompletionEnv::DealLambdaExpr(Ptr<Node> node, const Position pos)
{
    auto pLambdaExpr = dynamic_cast<LambdaExpr*>(node.get());
    if (!pLambdaExpr) { return; }
    DeepComplete(pLambdaExpr->funcBody.get(), pos);
}

void CompletionEnv::DealPrimaryMemberParam(Ptr<Node> node, bool needCheck)
{
    if (node == nullptr) { return; }
    auto *primaryDecl = dynamic_cast<PrimaryCtorDecl*>(node.get());
    if (primaryDecl == nullptr ||
        primaryDecl->funcBody->paramLists.empty() ||
        primaryDecl->funcBody->paramLists[0] == nullptr) { return; }
    for (auto &param : primaryDecl->funcBody->paramLists[0]->params) {
        if (!param) {
            continue;
        }
        bool flag = param->isMemberParam && (!needCheck || !param->TestAttr(Codira::AST::Attribute::PRIVATE));
        if (flag) {
            CompleteNode(param.get());
        }
    }
}

void CompletionEnv::DealClassDecl(Ptr<Node> node, const Position pos)
{
    scopeDepth++;
    CompleteNode(node, false, true);
    auto *pClassDecl = dynamic_cast<ClassDecl*>(node.get());
    if (!pClassDecl) { return; }
    if (pClassDecl->generic != nullptr) {
        DeepComplete(pClassDecl->generic.get(), pos);
        if (Contain(pClassDecl->generic.get(), pos)) {
            return;
        }
    }
    for (auto &parentDecl : pClassDecl->inheritedTypes) {
        if (parentDecl != nullptr) {
            DealParentDeclByName(parentDecl->ToString());
        }
    }
    Ptr<Decl> innerDecl = nullptr;
    for (auto &classMember : pClassDecl->GetMemberDecls()) {
        if (!classMember) {
            continue;
        }
        if (classMember->astKind == ASTKind::MACRO_EXPAND_DECL &&
            (classMember->identifier == "APILevel" || classMember->identifier == "Hide")) {
            continue;
        }
        CompleteNode(classMember.get());
        if (Contain(classMember.get(), pos)) {
            innerDecl = classMember.get();
        }
        if (classMember->astKind == Codira::AST::ASTKind::PRIMARY_CTOR_DECL) {
            DealPrimaryMemberParam(classMember.get());
        }
    }
    if (innerDecl) { DeepComplete(innerDecl, pos); }
}

void CompletionEnv::DealStructDecl(Ptr<Node> node, const Position pos)
{
    scopeDepth++;
    CompleteNode(node, false, true);
    auto *pStructDecl = dynamic_cast<StructDecl*>(node.get());
    if (!pStructDecl) { return; }
    if (pStructDecl->generic != nullptr) {
        DeepComplete(pStructDecl->generic.get(), pos);
        if (Contain(pStructDecl->generic.get(), pos)) {
            return;
        }
    }
    for (auto &parentDecl : pStructDecl->inheritedTypes) {
        if (parentDecl != nullptr) {
            DealParentDeclByName(parentDecl->ToString());
        }
    }
    Ptr<Decl> innerDecl = nullptr;
    for (auto &structMember : pStructDecl->GetMemberDecls()) {
        if (!structMember) {
            continue;
        }
        if (structMember->astKind == ASTKind::MACRO_EXPAND_DECL &&
            (structMember->identifier == "APILevel" || structMember->identifier == "Hide")) {
            continue;
        }
        CompleteNode(structMember.get());
        if (Contain(structMember.get(), pos)) {
            innerDecl = structMember.get();
        }
        if (structMember->astKind == Codira::AST::ASTKind::PRIMARY_CTOR_DECL) {
            DealPrimaryMemberParam(structMember.get());
        }
    }
    if (innerDecl) { DeepComplete(innerDecl, pos); }
}

void CompletionEnv::DealInterfaceDecl(Ptr<Node> node, const Position pos)
{
    auto *pInterfaceDecl = dynamic_cast<InterfaceDecl*>(node.get());
    if (!pInterfaceDecl) { return; }
    if (pInterfaceDecl->generic != nullptr) {
        DeepComplete(pInterfaceDecl->generic.get(), pos);
        if (Contain(pInterfaceDecl->generic.get(), pos)) {
            return;
        }
    }
    for (auto &parentDecl : pInterfaceDecl->inheritedTypes) {
        if (parentDecl != nullptr) {
            DealParentDeclByName(parentDecl->ToString());
        }
    }
    Ptr<Decl> innerDecl = nullptr;
    for (auto &memberDecl : pInterfaceDecl->GetMemberDecls()) {
        if (!memberDecl || (memberDecl->astKind == ASTKind::MACRO_EXPAND_DECL &&
                            (memberDecl->identifier == "APILevel" || memberDecl->identifier == "Hide"))) {
            continue;
        }
        CompleteNode(memberDecl.get());
        if (Contain(memberDecl.get(), pos)) {
            innerDecl = memberDecl.get();
        }
    }
    if (innerDecl) { DeepComplete(innerDecl, pos); }
}

void CompletionEnv::DealExtendDecl(Ptr<Node> node, const Position pos)
{
    auto *pExtendDecl = dynamic_cast<ExtendDecl*>(node.get());
    if (!pExtendDecl) {
        return;
    }
    Ptr<Decl> innerDecl = nullptr;
    if (pExtendDecl->generic != nullptr) {
        DeepComplete(pExtendDecl->generic.get(), pos);
        if (Contain(pExtendDecl->generic.get(), pos)) {
            return;
        }
    }
    for (auto &parentDecl : pExtendDecl->inheritedTypes) {
        DealParentDeclByName(parentDecl->ToString());
    }
    auto declName = pExtendDecl->extendedType == nullptr ? "" : pExtendDecl->extendedType->ToString();
    auto posOfLeft = declName.find_first_of('<');
    if (posOfLeft != std::string::npos) {
        declName = declName.substr(0, posOfLeft);
    }
    AddExtraMemberDecl(declName);
    for (auto &memberDecl : pExtendDecl->GetMemberDecls()) {
        CompleteNode(memberDecl.get());
        if (Contain(memberDecl.get(), pos)) {
            innerDecl = memberDecl.get();
        }
    }
    if (innerDecl) { DeepComplete(innerDecl, pos); }
}

void CompletionEnv::DealEnumDecl(Ptr<Node> node, const Position pos)
{
    auto *pEnumDecl = dynamic_cast<EnumDecl*>(node.get());
    if (!pEnumDecl) { return; }
    if (pEnumDecl->generic != nullptr) {
        DeepComplete(pEnumDecl->generic.get(), pos);
        if (Contain(pEnumDecl->generic.get(), pos)) { return; }
    }
    Ptr<Decl> innerDecl = nullptr;
    AddExtraMemberDecl(pEnumDecl->identifier);
    for (auto &parentDecl : pEnumDecl->inheritedTypes) {
        if (!parentDecl) {
            continue;
        }
        DealParentDeclByName(parentDecl->ToString());
    }
    for (auto &memberDecl : pEnumDecl->constructors) {
        CompleteNode(memberDecl.get(), false, false, false, pEnumDecl->identifier.Val());
        if (IsInEnumConstructor(memberDecl, pos)) {
            items.clear();
            return;
        }
    }
    for (auto &memberDecl : pEnumDecl->GetMemberDecls()) {
        CompleteNode(memberDecl.get());
        if (Contain(memberDecl.get(), pos)) {
            innerDecl = memberDecl.get();
        }
    }
    if (innerDecl) { DeepComplete(innerDecl, pos); }
}

void CompletionEnv::DealIfExpr(Ptr<Node> node, const Position pos)
{
    auto *pIfExpr = dynamic_cast<IfExpr*>(node.get());
    if (!pIfExpr) { return; }

    // for letPatternDestructor
    if (pIfExpr->condExpr->astKind == ark::ASTKind::LET_PATTERN_DESTRUCTOR) {
        DeepComplete(pIfExpr->condExpr.get(), pos);
    }
    if (Contain(pIfExpr->thenBody.get(), pos)) {
        DeepComplete(pIfExpr->thenBody.get(), pos);
    } else if (Contain(pIfExpr->elseBody.get(), pos)) {
        DeepComplete(pIfExpr->elseBody.get(), pos);
    }
}

void CompletionEnv::DealWhileExpr(Ptr<Node> node, const Position pos)
{
    auto *pWhileExpr = dynamic_cast<WhileExpr*>(node.get());
    bool isInvalid = pWhileExpr == nullptr || pWhileExpr->body == nullptr;
    if (isInvalid) { return; }
    DeepComplete(pWhileExpr->body.get(), pos);
}

void CompletionEnv::DealDoWhileExpr(Ptr<Node> node, const Position pos)
{
    auto *pDoWhileExpr = dynamic_cast<DoWhileExpr*>(node.get());
    bool isInvalid = pDoWhileExpr == nullptr || pDoWhileExpr->body == nullptr;
    if (isInvalid) { return; }
    DeepComplete(pDoWhileExpr->body.get(), pos);
}

void CompletionEnv::DealForInExpr(Ptr<Node> node, const Position pos)
{
    auto *pForInExpr = dynamic_cast<ForInExpr*>(node.get());
    bool isInvalid = pForInExpr == nullptr || pForInExpr->body == nullptr;
    if (isInvalid) { return; }
    DeepComplete(pForInExpr->pattern.get(), pos);
    DeepComplete(pForInExpr->body.get(), pos);
}

void CompletionEnv::DealSynchronizedExpr(Ptr<Node> node, const Position pos)
{
    auto *pSynchronizedExpr = dynamic_cast<SynchronizedExpr*>(node.get());
    bool isInvalid = pSynchronizedExpr == nullptr || pSynchronizedExpr->body == nullptr;
    if (isInvalid) { return; }
    DeepComplete(pSynchronizedExpr->body.get(), pos);
}

void CompletionEnv::DealReturnExpr(Ptr<Node> node, const Position pos)
{
    auto pRE = dynamic_cast<ReturnExpr*>(node.get());
    if (!pRE) { return; }
    if (pRE->expr && Contain(pRE->expr, pos)) {
        DeepComplete(pRE->expr, pos);
    }
}

void CompletionEnv::DealTryExpr(Ptr<Node> node, const Position pos)
{
    auto *pTryExpr = dynamic_cast<TryExpr*>(node.get());
    if (!pTryExpr) { return; }
    if (Contain(pTryExpr->tryBlock.get(), pos)) {
        DeepComplete(pTryExpr->tryBlock.get(), pos);
    } else if (Contain(pTryExpr->finallyBlock.get(), pos)) {
        DeepComplete(pTryExpr->finallyBlock.get(), pos);
    } else {
        int count = 0;
        for (auto &catchBlock : pTryExpr->catchBlocks) {
            if (Contain(catchBlock.get(), pos)) {
                auto exceptTypePattern = RawStaticCast<const ExceptTypePattern*>(pTryExpr->catchPatterns[count].get());
                DeepComplete(exceptTypePattern->pattern.get(), pos);
                DeepComplete(catchBlock.get(), pos);
                return;
            }
            count += 1;
        }
    }
}

void CompletionEnv::DealFuncBody(Ptr<Node> node, const Position pos)
{
    auto pFuncBody = dynamic_cast<FuncBody*>(node.get());
    if (!pFuncBody) { return; }
    if (!pFuncBody->paramLists.empty()) {
        DeepComplete(pFuncBody->paramLists[0].get(), pos);
    }
    if (pFuncBody->generic != nullptr) {
        DeepComplete(pFuncBody->generic.get(), pos);
    }
    if (Contain(pFuncBody->body.get(), pos)) {
        DeepComplete(pFuncBody->body.get(), pos);
    }
}

void CompletionEnv::DealVarDecl(Ptr<Node> node, const Position pos)
{
    auto pVarDecl = dynamic_cast<VarDecl*>(node.get());
    if (!pVarDecl) { return; }
    if (Contain(pVarDecl->initializer.get(), pos)) {
        DeepComplete(pVarDecl->initializer.get(), pos);
    }
}

void CompletionEnv::DealVarWithPatternDecl(Ptr<Node> node, const Position pos)
{
    auto pVarWithPatternDecl = dynamic_cast<VarWithPatternDecl*>(node.get());
    if (!pVarWithPatternDecl) { return; }
    if (Contain(pVarWithPatternDecl->initializer.get(), pos)) {
        DeepComplete(pVarWithPatternDecl->initializer.get(), pos);
    }
}

void CompletionEnv::DealPropDecl(Ptr<Codira::AST::Node> node, const Codira::Position pos)
{
    auto pPropDecl = dynamic_cast<PropDecl*>(node.get());
    if (!pPropDecl) { return; }
    CodeCompletion getter;
    getter.label = "get";
    getter.name = "get";
    getter.kind = CompletionItemKind::CIK_METHOD;
    getter.detail = "(function) func get()";
    getter.insertText = "get() {\n\t$0\n}";
    getter.show = true;
    AddCompletionItem("get", "get", getter);
    if (pPropDecl->isVar) {
        CodeCompletion setter;
        setter.label = "set";
        setter.name = "set";
        setter.kind = CompletionItemKind::CIK_METHOD;
        setter.detail = "(function) func set(value)";
        setter.show = true;
        setter.insertText = "set(${1:value}) {\n\t$0\n}";
        AddCompletionItem("set", "set", setter);
    }

    if (!pPropDecl->setters.empty()) {
        DeepComplete(pPropDecl->setters[0].get(), pos);
    }
}

void CompletionEnv::DealLetPatternDestructor(Ptr<Codira::AST::Node> node, const Codira::Position pos)
{
    auto pLetPatternDestructor = dynamic_cast<LetPatternDestructor*>(node.get());
    if (!pLetPatternDestructor) { return; }
    auto& letPatterns = pLetPatternDestructor->patterns;
    for (const auto& letPattern : letPatterns) {
        if (letPattern->astKind == ark::ASTKind::ENUM_PATTERN) {
            auto pEnumPattern = dynamic_cast<EnumPattern*>(letPattern.get().get());
            if (!pEnumPattern) {
                return;
            }
            for (auto &pattern : pEnumPattern->patterns) {
                DeepComplete(pattern.get(), pos);
            }
        }
    }
}

void CompletionEnv::DealMatchExpr(Ptr<Node> node, const Position pos)
{
    auto pMatchExpr = dynamic_cast<MatchExpr *>(node.get());
    if (!pMatchExpr) {
        return;
    }
    if (pMatchExpr->selector) {
        auto selPosition = pMatchExpr->selector->GetBegin();
        std::string query = "_ = (" + std::to_string(pos.fileID) + ", " + std::to_string(selPosition.line) + ", " +
                            std::to_string(selPosition.column) + ")";
        auto symbols = SearchContext(cache->packageInstance->ctx, query);
        if (auto target = Ty::GetDeclPtrOfTy(symbols[0]->node->ty)) {
            matchSelector = target->identifier;
        }

        if (Contain(pMatchExpr->selector.get(), pos)) {
            DeepComplete(pMatchExpr->selector.get(), pos);
        }
    }
    for (auto &matchCase : pMatchExpr->matchCases) {
        if (Contain(matchCase.get(), pos)) {
            DeepComplete(matchCase.get(), pos);
        }
    }
}

void CompletionEnv::DealMatchCase(Ptr<Node> node, const Position pos)
{
    auto pMatchCase = dynamic_cast<MatchCase*>(node.get());
    if (!pMatchCase) { return; }
    for (auto &pattern : pMatchCase->patterns) {
        if (!pattern) {
            continue;
        }
        // not support Const_Pattern | Tuple_Pattern
        if (pattern->astKind == ASTKind::CONST_PATTERN || pattern->astKind == ASTKind::TUPLE_PATTERN) { continue; }
        if (pattern->astKind == ASTKind::ENUM_PATTERN) {
            DealEnumPattern(pattern.get(), pos);
        } else {
            std::string str = parserAst->sourceManager->GetContentBetween(parserAst->fileID,
                                                                          {pattern->GetBegin().line,
                                                                           pattern->GetBegin().column},
                                                                          {pattern->GetEnd().line,
                                                                           pattern->GetEnd().column});
            std::string detail = "(variable) let " + str;
            if (pattern->begin <= pos && pos <= pattern->end) { continue; }
            AccessibleByString(str, detail);
        }
    }
    if (Contain(pMatchCase->exprOrDecls.get(), pos)) {
        DeepComplete(pMatchCase->exprOrDecls.get(), pos);
    }
}

void CompletionEnv::DealTrailingClosureExpr(Ptr<Node> node, const Position pos)
{
    auto pTrailingClosureExpr = dynamic_cast<TrailingClosureExpr*>(node.get());
    if (!pTrailingClosureExpr) { return; }
    if (pTrailingClosureExpr->expr && Contain(pTrailingClosureExpr->expr.get(), pos)) {
        DeepComplete(pTrailingClosureExpr->expr.get(), pos);
    }
    if (pTrailingClosureExpr->lambda.get() && Contain(pTrailingClosureExpr->lambda.get(), pos)) {
        DeepComplete(pTrailingClosureExpr->lambda.get(), pos);
    }
}

void CompletionEnv::DealFuncParamList(Ptr<Node> node, const Position pos)
{
    auto *pFuncParamList = dynamic_cast<FuncParamList*>(node.get());
    if (!pFuncParamList) { return; }
    Ptr<Node> innerDecl = nullptr;
    if (pFuncParamList->GetBegin() > pos || pos > pFuncParamList->GetEnd()) {
        for (auto &memberDecl : pFuncParamList->params) {
            if (!memberDecl) { continue; }
            CompleteNode(memberDecl.get());
        }
        return;
    }
    for (auto &memberDecl : pFuncParamList->params) {
        if (!memberDecl) { continue; }
        if (memberDecl->commaPos.IsZero()) { continue; }
        CompleteNode(memberDecl.get());
        if (Contain(memberDecl.get(), pos)) {
            innerDecl = memberDecl.get();
        }
    }
    DeepComplete(innerDecl, pos);
}

void CompletionEnv::DealGeneric(Ptr<Node> node, const Position pos)
{
    auto *pGeneric = dynamic_cast<Generic*>(node.get());
    if (!pGeneric) { return; }
    Ptr<Node> innerDecl = nullptr;
    for (auto &memberDecl : pGeneric->typeParameters) {
        if (memberDecl == nullptr) { continue; }
        CompleteNode(memberDecl.get());
        if (Contain(memberDecl.get(), pos)) {
            innerDecl = memberDecl.get();
        }
    }
    for (auto &memberDecl : pGeneric->genericConstraints) {
        if (memberDecl == nullptr) { continue; }
        CompleteNode(memberDecl.get());
        if (Contain(memberDecl.get(), pos)) {
            innerDecl = memberDecl.get();
        }
    }
    if (innerDecl) { DeepComplete(innerDecl, pos); }
}

void CompletionEnv::DealBlock(Ptr<Node> node, const Position pos)
{
    scopeDepth++;
    auto *pBlock = dynamic_cast<Block*>(node.get());
    if (!pBlock) { return; }
    Ptr<Node> innerDecl = nullptr;
    bool isAfterInnerDecl = false;
    for (auto &memberDecl : pBlock->body) {
        if (memberDecl == nullptr) { continue; }
        CompleteNode(memberDecl.get(), false, false, isAfterInnerDecl);
        if (memberDecl->GetBegin() <= pos && pos <= memberDecl->GetEnd()) {
            innerDecl = memberDecl.get();
            isAfterInnerDecl = true;
        }
    }
    if (innerDecl) { DeepComplete(innerDecl, pos); }
}

void CompletionEnv::DealTuplePattern(Ptr<Node> node, const Position pos)
{
    auto pTuplePattern = dynamic_cast<TuplePattern*>(node.get());
    if (!pTuplePattern) { return; }
    for (auto &pattern : pTuplePattern->patterns) {
        if (!pattern) {
            continue;
        }
        DeepComplete(pattern.get(), pos);
    }
}

void CompletionEnv::DealVarPattern(Ptr<Node> node, const Position /* pos */)
{
    auto pVarPattern = dynamic_cast<VarPattern*>(node.get());
    std::string str = parserAst->sourceManager->GetContentBetween(parserAst->fileID,
                                                                  {pVarPattern->GetBegin().line,
                                                                   pVarPattern->GetBegin().column},
                                                                  {pVarPattern->GetEnd().line,
                                                                   pVarPattern->GetEnd().column});
    std::string detail = "(variable) let " + str;
    AccessibleByString(str, detail);
}

void CompletionEnv::DealVarOrEnumPattern(Ptr<Node> node, const Position /* pos */)
{
    auto pVarOrEnumPattern = dynamic_cast<VarOrEnumPattern*>(node.get());
    std::string str = parserAst->sourceManager->GetContentBetween(parserAst->fileID,
                                                                  {pVarOrEnumPattern->GetBegin().line,
                                                                   pVarOrEnumPattern->GetBegin().column},
                                                                  {pVarOrEnumPattern->GetEnd().line,
                                                                   pVarOrEnumPattern->GetEnd().column});
    std::string detail = "(variable) let " + str;
    AccessibleByString(str, detail);
}

void CompletionEnv::DealEnumPattern(Ptr<Node> node, const Position pos)
{
    auto pEnumPattern = dynamic_cast<EnumPattern*>(node.get());
    if (!pEnumPattern) { return; }
    for (auto &ptn : pEnumPattern->patterns) {
        if (!ptn) {
            continue;
        }
        if (ptn->astKind == ASTKind::VAR_OR_ENUM_PATTERN) {
            DealVarOrEnumPattern(ptn.get(), pos);
        }
    }
}

void CompletionEnv::DealParenExpr(Ptr<Node> node, const Position pos)
{
    auto pParenExpr = dynamic_cast<ParenExpr*>(node.get());
    if (!pParenExpr) { return; }
    DeepComplete(pParenExpr->expr.get(), pos);
}

void CompletionEnv::DealFuncArg(Ptr<Node> node, const Position pos)
{
    auto pFuncArg = dynamic_cast<FuncArg*>(node.get());
    if (!pFuncArg) { return; }
    DeepComplete(pFuncArg->expr.get(), pos);
}

void CompletionEnv::DealCallExpr(Ptr<Node> node, const Position pos)
{
    auto pCallExpr = dynamic_cast<CallExpr*>(node.get());
    if (!pCallExpr) { return; }
    DeepComplete(pCallExpr->baseFunc, pos);
    for (auto const &arg : pCallExpr->args) {
        if (!arg) {
            continue;
        }
        DeepComplete(arg.get(), pos);
    }
}

void CompletionEnv::DealBinaryExpr(Ptr<Node> node, const Position pos)
{
    auto pBinaryExpr = dynamic_cast<BinaryExpr*>(node.get());
    if (!pBinaryExpr) {
        return;
    }
    if (pBinaryExpr->leftExpr && Contain(pBinaryExpr->leftExpr, pos)) {
        DeepComplete(pBinaryExpr->leftExpr, pos);
        return;
    }
    if (pBinaryExpr->rightExpr && Contain(pBinaryExpr->rightExpr, pos)) {
        DeepComplete(pBinaryExpr->rightExpr, pos);
    }
}

void CompletionEnv::DealMemberAccess(Ptr<Node> node, const Position pos)
{
    auto ma = dynamic_cast<MemberAccess*>(node.get());
    if (!ma) { return; }
    DeepComplete(ma->baseExpr, pos);
}

void CompletionEnv::DealRefExpr(Ptr<Node> node, const Position pos)
{
    auto re = dynamic_cast<RefExpr*>(node.get());
    if (!re) { return; }
    DeepComplete(re->ref.target, pos);
}

void CompletionEnv::DealTupleLit(Ptr<Node> node, const Position pos)
{
    auto pTupleLit = dynamic_cast<TupleLit*>(node.get());
    if (!pTupleLit) { return; }
    for (auto const &ch : pTupleLit->children) {
        if (!ch) { continue; }
        DeepComplete(ch.get(), pos);
    }
}

void CompletionEnv::DealSpawnExpr(Ptr<Node> node, const Position pos)
{
    auto spawnExpr = dynamic_cast<SpawnExpr*>(node.get());
    if (!spawnExpr) { return; }
    DeepComplete(spawnExpr->task.get(), pos);
}

void CompletionEnv::InitMap() const
{
    NormalMatcher::GetInstance().RegFunc(ASTKind::FUNC_DECL, &ark::CompletionEnv::DealFuncDecl);
    NormalMatcher::GetInstance().RegFunc(ASTKind::MAIN_DECL, &ark::CompletionEnv::DealMainDecl);
    NormalMatcher::GetInstance().RegFunc(ASTKind::MACRO_DECL, &ark::CompletionEnv::DealMacroDecl);
    NormalMatcher::GetInstance().RegFunc(ASTKind::MACRO_EXPAND_DECL, &ark::CompletionEnv::DealMacroExpandDecl);
    NormalMatcher::GetInstance().RegFunc(ASTKind::MACRO_EXPAND_EXPR, &ark::CompletionEnv::DealMacroExpandExpr);
    NormalMatcher::GetInstance().RegFunc(ASTKind::PRIMARY_CTOR_DECL, &ark::CompletionEnv::DealPrimaryCtorDecl);
    NormalMatcher::GetInstance().RegFunc(ASTKind::LAMBDA_EXPR, &ark::CompletionEnv::DealLambdaExpr);
    NormalMatcher::GetInstance().RegFunc(ASTKind::CLASS_DECL, &ark::CompletionEnv::DealClassDecl);
    NormalMatcher::GetInstance().RegFunc(ASTKind::STRUCT_DECL, &ark::CompletionEnv::DealStructDecl);
    NormalMatcher::GetInstance().RegFunc(ASTKind::INTERFACE_DECL, &ark::CompletionEnv::DealInterfaceDecl);
    NormalMatcher::GetInstance().RegFunc(ASTKind::EXTEND_DECL, &ark::CompletionEnv::DealExtendDecl);
    NormalMatcher::GetInstance().RegFunc(ASTKind::ENUM_DECL, &ark::CompletionEnv::DealEnumDecl);
    NormalMatcher::GetInstance().RegFunc(ASTKind::VAR_DECL, &ark::CompletionEnv::DealVarDecl);
    NormalMatcher::GetInstance().RegFunc(
        ASTKind::VAR_WITH_PATTERN_DECL, &ark::CompletionEnv::DealVarWithPatternDecl);
    NormalMatcher::GetInstance().RegFunc(ASTKind::PROP_DECL, &ark::CompletionEnv::DealPropDecl);
    NormalMatcher::GetInstance().RegFunc(
        ASTKind::LET_PATTERN_DESTRUCTOR, &ark::CompletionEnv::DealLetPatternDestructor);
    NormalMatcher::GetInstance().RegFunc(ASTKind::MATCH_EXPR, &ark::CompletionEnv::DealMatchExpr);
    NormalMatcher::GetInstance().RegFunc(ASTKind::MATCH_CASE, &ark::CompletionEnv::DealMatchCase);
    NormalMatcher::GetInstance().RegFunc(ASTKind::IF_EXPR, &ark::CompletionEnv::DealIfExpr);
    NormalMatcher::GetInstance().RegFunc(ASTKind::WHILE_EXPR, &ark::CompletionEnv::DealWhileExpr);
    NormalMatcher::GetInstance().RegFunc(ASTKind::DO_WHILE_EXPR, &ark::CompletionEnv::DealDoWhileExpr);
    NormalMatcher::GetInstance().RegFunc(ASTKind::FOR_IN_EXPR, &ark::CompletionEnv::DealForInExpr);
    NormalMatcher::GetInstance().RegFunc(ASTKind::SYNCHRONIZED_EXPR, &ark::CompletionEnv::DealSynchronizedExpr);
    NormalMatcher::GetInstance().RegFunc(ASTKind::RETURN_EXPR, &ark::CompletionEnv::DealReturnExpr);
    NormalMatcher::GetInstance().RegFunc(ASTKind::TRY_EXPR, &ark::CompletionEnv::DealTryExpr);
    NormalMatcher::GetInstance().RegFunc(ASTKind::FUNC_BODY, &ark::CompletionEnv::DealFuncBody);
    NormalMatcher::GetInstance().RegFunc(ASTKind::TRAIL_CLOSURE_EXPR, &ark::CompletionEnv::DealTrailingClosureExpr);
    NormalMatcher::GetInstance().RegFunc(ASTKind::FUNC_PARAM_LIST, &ark::CompletionEnv::DealFuncParamList);
    NormalMatcher::GetInstance().RegFunc(ASTKind::GENERIC, &ark::CompletionEnv::DealGeneric);
    NormalMatcher::GetInstance().RegFunc(ASTKind::BLOCK, &ark::CompletionEnv::DealBlock);
    NormalMatcher::GetInstance().RegFunc(ASTKind::TUPLE_PATTERN, &ark::CompletionEnv::DealTuplePattern);
    NormalMatcher::GetInstance().RegFunc(ASTKind::VAR_PATTERN, &ark::CompletionEnv::DealVarPattern);
    NormalMatcher::GetInstance().RegFunc(ASTKind::VAR_OR_ENUM_PATTERN, &ark::CompletionEnv::DealVarOrEnumPattern);
    NormalMatcher::GetInstance().RegFunc(ASTKind::ENUM_PATTERN, &ark::CompletionEnv::DealEnumPattern);
    NormalMatcher::GetInstance().RegFunc(ASTKind::PAREN_EXPR, &ark::CompletionEnv::DealParenExpr);
    NormalMatcher::GetInstance().RegFunc(ASTKind::FUNC_ARG, &ark::CompletionEnv::DealFuncArg);
    NormalMatcher::GetInstance().RegFunc(ASTKind::CALL_EXPR, &ark::CompletionEnv::DealCallExpr);
    NormalMatcher::GetInstance().RegFunc(ASTKind::TUPLE_LIT, &ark::CompletionEnv::DealTupleLit);
    NormalMatcher::GetInstance().RegFunc(ASTKind::SPAWN_EXPR, &ark::CompletionEnv::DealSpawnExpr);
    NormalMatcher::GetInstance().RegFunc(ASTKind::MEMBER_ACCESS, &ark::CompletionEnv::DealMemberAccess);
    NormalMatcher::GetInstance().RegFunc(ASTKind::REF_EXPR, &ark::CompletionEnv::DealRefExpr);
    NormalMatcher::GetInstance().RegFunc(ASTKind::BINARY_EXPR, &ark::CompletionEnv::DealBinaryExpr);
}

CompletionEnv::CompletionEnv()
{
    InitMap();
}

bool CompletionEnv::GetValue(FILTER kind) const
{
    return (filter & (1U << static_cast<unsigned int>(kind))) != 0;
}

void CompletionEnv::SetValue(FILTER kind, bool value)
{
    filter |= (1U << static_cast<unsigned int>(kind));
    if (!value) {
        filter -= (1U << static_cast<unsigned int>(kind));
    }
}

void CompletionEnv::DotAccessible(Decl &decl, const Decl &parentDecl, bool isSuperOrThis)
{
    if (!syscap.CheckSysCap(decl) || IsHiddenDecl(&decl)) {
        return;
    }
    // use signature to unique
    SourceManager *temp = parserAst == nullptr ? nullptr : parserAst->sourceManager;
    std::string signature = ItemResolverUtil::ResolveSignatureByNode(decl, temp);
    if (decl.TestAttr(Codira::AST::Attribute::MACRO_INVOKE_FUNC) || signature.empty() ||
            IsSignatureInItems(signature, signature)) {
        return;
    }
    if (isFromNormal && decl.TestAttr(Codira::AST::Attribute::CONSTRUCTOR)) {
        std::string replaceString = ItemResolverUtil::ResolveSignatureByNode(parentDecl);
        std::string insertReplaceString =
                parentDecl.identifier + ItemResolverUtil::GetGenericInsertByDecl(parentDecl.GetGeneric());
        signature = signature.replace(signature.begin(),
                                      signature.begin() + static_cast<long long>(std::string("init").size()),
                                      replaceString);
        CodeCompletion initCompletion;
        initCompletion.label = signature;
        initCompletion.name = replaceString;
        initCompletion.kind = CompletionItemKind::CIK_FUNCTION;
        initCompletion.detail = ItemResolverUtil::ResolveDetailByNode(decl);
        std::string insertText = ItemResolverUtil::ResolveInsertByNode(decl);
        insertText = insertText.replace(insertText.begin(),
                                        insertText.begin() + static_cast<long long>(std::string("init").size()),
                                        insertReplaceString);
        initCompletion.insertText = insertText;
        initCompletion.show = CheckParentAccessible(decl, parentDecl, isSuperOrThis);
        // if decl is deprecated
        MarkDeprecated(decl, initCompletion);
        AddCompletionItem(signature, signature, initCompletion);
        CompleteFollowLambda(decl, parserAst->sourceManager, initCompletion, replaceString);
        return;
    }
    bool show = true;
    // dot completion need check accessible between parent and child type.
    // dot completion can't provide results of constructor and operator function item.
    if (!CheckParentAccessible(decl, parentDecl, isSuperOrThis) || !CheckInsideVarDecl(decl) ||
        decl.TestAttr(Codira::AST::Attribute::CONSTRUCTOR) || decl.TestAttr(Codira::AST::Attribute::OPERATOR)) {
        show = false;
    }
    AddItem(decl, signature, show);
}

void CompletionEnv::AddItem(Decl &decl, const std::string &signature, bool show)
{
    CodeCompletion completion;
    completion.name = decl.identifier;
    // if decl is deprecated
    MarkDeprecated(decl, completion);
    auto med = DynamicCast<MacroExpandDecl *>(&decl);
    while (med) {
        Ptr<Decl> realDecl = med->invocation.decl;
        if (realDecl == nullptr) {
            break;
        }
        completion.name = realDecl->identifier;
        med = DynamicCast<MacroExpandDecl *>(realDecl.get());
    }
    completion.kind = ItemResolverUtil::ResolveKindByNode(decl);
    if (show && completion.kind == CompletionItemKind::CIK_METHOD && !decl.TestAttr(Attribute::ENUM_CONSTRUCTOR)) {
        completion.label = completion.insertText = completion.name;
        completion.detail = "";
        completion.show = show;
        AddCompletionItem(completion.name, completion.name, completion);
        // If all arguments to the function have default values, provide a completion with the empty function list.
        if (auto fd = DynamicCast<FuncDecl *>(&decl); fd && ::IsAllParamInitial(*fd)) {
            auto defaultFuncName = completion.name + "()";
            completion.label = completion.insertText = completion.name = defaultFuncName;
            completion.detail = ItemResolverUtil::ResolveDetailByNode(decl);
            completion.show = show;
            AddCompletionItem(completion.name, completion.name, completion);
        }
    }
    completion.label = signature;
    completion.detail = ItemResolverUtil::ResolveDetailByNode(decl);
    completion.insertText = ItemResolverUtil::ResolveInsertByNode(decl);
    completion.show = show;
    AddCompletionItem(signature, signature, completion);
    if (completion.kind == CompletionItemKind::CIK_VARIABLE) {
        CompleteParamListFuncTypeVarDecl(decl, parserAst->sourceManager, completion);
    }
    CompleteFollowLambda(decl, parserAst->sourceManager, completion);
}

void CompletionEnv::AddVArrayItem()
{
    CodeCompletion completion;
    completion.name = "size";
    completion.kind = CompletionItemKind::CIK_VARIABLE;

    completion.label = "size";
    completion.detail = "(variable) let size: Int64";
    completion.insertText = "size";
    completion.show = true;
    AddCompletionItem("size", "size", completion);
}

void CompletionEnv::InvokedAccessible(Ptr<Node> node,
                                      bool insideFunction,
                                      bool deepestFunction,
                                      bool isImport)
{
    if (node == nullptr || node->TestAnyAttr(Codira::AST::Attribute::CONSTRUCTOR,
                                             Codira::AST::Attribute::MACRO_INVOKE_FUNC) ||
                                            IsHiddenDecl(node)) {
        return;
    }

    // remove ord/chr etc.
    if (IsZeroPosition(node) && node->astKind != ASTKind::PACKAGE &&
        node->astKind != ASTKind::GENERIC_PARAM_DECL && node->astKind != ASTKind::BUILTIN_DECL) {
        return;
    }
    if (RefToDecl(node, insideFunction, deepestFunction)) {
        return;
    }
    std::string signature = ItemResolverUtil::ResolveSignatureByNode(*node);
    if (signature.empty() || IsSignatureInItems(signature, signature)) {
        return;
    }
    bool show = true;
    if (!VarDeclIsLet(node) && insideFunction && !deepestFunction) {
        show = false;
    }
    CodeCompletion completion;
    completion.name = ItemResolverUtil::ResolveNameByNode(*node);
    completion.kind = ItemResolverUtil::ResolveKindByNode(*node);
    completion.detail = ItemResolverUtil::ResolveDetailByNode(*node);
    // if decl is deprecated
    if (auto decl = dynamic_cast<Decl *>(node.get())) {
        MarkDeprecated(*decl, completion);
    }
    // if isImport complete simple
    if (isImport) {
        completion.label = completion.insertText = completion.name;
        completion.show = show;
        AddCompletionItem(signature, signature, completion);
        return;
    }
    completion.label = signature;
    completion.insertText = ItemResolverUtil::ResolveInsertByNode(*node);
    completion.show = show;
    if (node->TestAttr(Codira::AST::Attribute::MACRO_FUNC)) {
        AddItemForMacro(node, completion);
        return;
    }
    AddCompletionItem(signature, signature, completion);
    if (completion.kind == CompletionItemKind::CIK_METHOD) {
        completion.label = completion.insertText = completion.name;
        completion.detail = "";
        AddCompletionItem(completion.name, completion.name, completion);
    }
    // complete class_decl and struct_decl init func_decl
    if (completion.kind == CompletionItemKind::CIK_CLASS || completion.kind == CompletionItemKind::CIK_STRUCT) {
        CompleteInitFuncDecl(node, "");
    }
}

void CompletionEnv::DealParentDeclByName(std::string parentClassLikeName)
{
    if (parserAst == nullptr || parserAst->file == nullptr || parserAst->file->curPackage == nullptr) { return; }
    // format the classLike Name if name is ClassA<T> ->  ClassA
    auto posOfLeft = parentClassLikeName.find_first_of('<');
    if (posOfLeft != std::string::npos) {
        parentClassLikeName = parentClassLikeName.substr(0, posOfLeft);
    }

    if (visitedSet.find(parentClassLikeName) != visitedSet.end()) { return; }
    (void)visitedSet.insert(parentClassLikeName);

    // find the ParentClassLikeName in this package
    if (idMap.find(parentClassLikeName) != idMap.end()) {
        if (!CompleteInParseCache(parentClassLikeName)) {
            return;
        }
    }

    // find the ParentClassLikeName in the last semaCache
    CompleteInSemaCache(parentClassLikeName);
}

bool CompletionEnv::CompleteInParseCache(const std::string &parentClassLikeName)
{
    bool hasAPILevel = false;
    for (auto item : idMap[parentClassLikeName]) {
        if (item == nullptr) { continue; }
        for (auto &memberDecl : item->GetMemberDecls()) {
            if (!memberDecl) {
                continue;
            }
            if (memberDecl->astKind == ASTKind::MACRO_EXPAND_DECL && memberDecl->identifier == "APILevel") {
                hasAPILevel = true;
                continue;
            }
            if (memberDecl->astKind == ASTKind::MACRO_EXPAND_DECL && memberDecl->identifier == "Hide") {
                continue;
            }
            if (!memberDecl->TestAttr(Codira::AST::Attribute::PRIVATE)) {
                CompleteNode(memberDecl.get());
            }
            if (memberDecl->astKind == Codira::AST::ASTKind::PRIMARY_CTOR_DECL) {
                DealPrimaryMemberParam(memberDecl.get(), true);
            }
            CompleteInheritedTypes(memberDecl.get());
        }
    }
    return hasAPILevel;
}

void CompletionEnv::CompleteInSemaCache(const std::string &parentClassLikeName)
{
    auto matchDecl = cache->packageInstance->importManager.GetPackageMembersByName(
        *cache->packageInstance->ctx->curPackage, parentClassLikeName);
    for (auto item : matchDecl) {
        if (item == nullptr) {
            continue;
        }
        if (item->astKind == Codira::AST::ASTKind::CLASS_DECL) {
            auto decl = dynamic_cast<ClassDecl *>(item.get());
            DealClassOrInterfaceDeclByName(decl);
        } else if (item->astKind == Codira::AST::ASTKind::INTERFACE_DECL) {
            auto decl = dynamic_cast<InterfaceDecl *>(item.get());
            DealClassOrInterfaceDeclByName(decl);
        }
    }
}

void CompletionEnv::CompleteInheritedTypes(Ptr<Node> decl)
{
    if (!decl) { return; }
    if (decl->astKind == Codira::AST::ASTKind::CLASS_DECL) {
        auto *classDecl = dynamic_cast<ClassDecl*>(decl.get());
        if (!classDecl) {
            return;
        }
        for (auto &parentDecl : classDecl->inheritedTypes) {
            if (!parentDecl) {
                continue;
            }
            DealParentDeclByName(parentDecl->ToString());
        }
    } else if (decl->astKind == Codira::AST::ASTKind::INTERFACE_DECL) {
        auto *interfaceDecl = dynamic_cast<InterfaceDecl*>(decl.get());
        if (!interfaceDecl) {
            return;
        }
        for (auto &parentDecl : interfaceDecl->inheritedTypes) {
            if (!parentDecl) {
                continue;
            }
            DealParentDeclByName(parentDecl->ToString());
        }
    }
}

template<typename T>
void CompletionEnv::DealClassOrInterfaceDeclByName(T &decl)
{
    if (!decl) { return; }
    for (auto &memberDecl : decl->GetMemberDecls()) {
        if (!memberDecl) {
            continue;
        }
        if (memberDecl->TestAttr(Codira::AST::Attribute::PUBLIC) ||
            memberDecl->TestAttr(Codira::AST::Attribute::PROTECTED)) {
            CompleteNode(memberDecl.get());
        }
        for (auto &parentDecl : decl->inheritedTypes) {
            DealParentDeclByName(parentDecl->ToString());
        }
    }
}

void CompletionEnv::CompleteNode(
    Ptr<Node> node, bool isImport, bool isInScope, bool isSameName, const std::string &container)
{
    if (noComplete(node, syscap, isCompleteFunction) || IsHiddenDecl(node)) {
        return;
    }
    DeclVarWithTuple(node, isImport, isInScope, isSameName);
    if (!IsMatchingCompletion(prefix, ItemResolverUtil::ResolveNameByNode(*node)) && !CheckIsRawIdentifier(node)) {
        return;
    }
    if (parserAst == nullptr) { return; }
    std::string signature = ItemResolverUtil::ResolveSignatureByNode(*node, parserAst->sourceManager);
    if (signature.empty()) { return; }
    auto itemSignature = signature;
    (void) itemSignature.erase(remove_if(itemSignature.begin(), itemSignature.end(), ::isspace), itemSignature.end());
    bool abort = isSameName && node->astKind != ASTKind::FUNC_DECL && IsSignatureInItems(itemSignature, itemSignature);
    if (abort) {
        return;
    }
    bool show = true;
    CodeCompletion completion;
    completion.itemDepth = scopeDepth;
    if (auto decl = dynamic_cast<Decl *>(node.get())) {
        completion.id = GetDeclSymbolID(*decl);
        MarkDeprecated(*decl, completion);
    }
    completion.name = ItemResolverUtil::ResolveNameByNode(*node);
    completion.kind = ItemResolverUtil::ResolveKindByNode(*node);
    completion.detail = ItemResolverUtil::ResolveDetailByNode(*node, parserAst->sourceManager);
    if (node->TestAttr(Attribute::ENUM_CONSTRUCTOR)) {
        completion.isEnumCtor = true;
        completion.container = container;
    }
    // if isImport complete simple
    if (isImport) {
        completion.label = completion.insertText = completion.name;
        completion.show = show;
        AddCompletionItem(itemSignature, itemSignature, completion);
        return;
    }
    // skip VArray type
    if (node->ty->kind == TypeKind::TYPE_VARRAY) {
        return;
    }
    completion.label = signature;
    if (signature == "CFunc<T>") {
        completion.insertText = std::string("CFunc<${1:()} -> ${2:Unit}>");
    } else {
        completion.insertText = ItemResolverUtil::ResolveInsertByNode(*node, parserAst->sourceManager);
    }
    completion.show = show;
    if (node->TestAttr(Codira::AST::Attribute::MACRO_FUNC)) {
        AddItemForMacro(node, completion);
        return;
    }
    AddCompletionItem(itemSignature, itemSignature, completion);
    // complete follow lambda for func_decl
    CompleteFollowLambda(*node, parserAst->sourceManager, completion);
    bool isRawIdentifier = CheckIsRawIdentifier(node);
    if (isRawIdentifier) {
        auto rawCompletion = completion;
        auto rawName = "`" + completion.name + "`";
        auto rawSignature = itemSignature;
        rawSignature.replace(rawSignature.find(completion.name), completion.name.length(), rawName);
        rawCompletion.label.replace(rawCompletion.label.find(completion.name), completion.name.length(), rawName);
        rawCompletion.insertText.replace(rawCompletion.insertText.find(completion.name),
                                         completion.name.length(), rawName);
        rawCompletion.detail.replace(rawCompletion.detail.find(completion.name), completion.name.length(), rawName);
        rawCompletion.name = rawName;
        AddCompletionItem(rawSignature, rawSignature, rawCompletion);
    }
    // complete function name
    CompleteFunctionName(completion, isRawIdentifier);
    // complete class_decl and struct_decl init func_decl
    if (completion.kind == CompletionItemKind::CIK_CLASS || completion.kind == CompletionItemKind::CIK_STRUCT) {
        CompleteInitFuncDecl(node, "", false, isInScope);
        // for curPackage typeAlias
        if (node->astKind == ASTKind::TYPE_ALIAS_DECL) {
            DealTypeAlias(node);
        }
    }
    if (completion.kind == CompletionItemKind::CIK_VARIABLE) {
        CompleteParamListFuncTypeVarDecl(*node, parserAst->sourceManager, completion);
    }
}

void CompletionEnv::CompleteFunctionName(CodeCompletion &completion, bool isRawIdentifier)
{
    if (completion.kind == CompletionItemKind::CIK_METHOD) {
        completion.label = completion.insertText = completion.name;
        completion.detail = "";
        AddCompletionItem(completion.name, completion.name, completion);
        if (isRawIdentifier) {
            auto rawCompletion = completion;
            auto rawName = "`" + completion.name + "`";
            rawCompletion.label.replace(rawCompletion.label.find(completion.name), completion.name.length(), rawName);
            rawCompletion.insertText.replace(rawCompletion.insertText.find(completion.name),
                                             completion.name.length(), rawName);
            rawCompletion.detail = "";
            rawCompletion.name = rawName;
            AddCompletionItem(rawName, rawName, rawCompletion);
        }
    }
}

void CompletionEnv::DeclVarWithTuple(Ptr<const Node> node, bool isImport, bool isInScope, bool isSameName)
{
    if (!node || node->astKind != ASTKind::VAR_WITH_PATTERN_DECL && node->astKind != ASTKind::TUPLE_PATTERN) { return; }
    if (node->astKind == ASTKind::VAR_WITH_PATTERN_DECL) {
        auto varWithPatternDecl = dynamic_cast<const VarWithPatternDecl*>(node.get());
        if (varWithPatternDecl && varWithPatternDecl->irrefutablePattern &&
            varWithPatternDecl->irrefutablePattern->astKind == ASTKind::TUPLE_PATTERN) {
            CompleteNode(varWithPatternDecl->irrefutablePattern.get(), isImport, isInScope, isSameName);
        }
        return;
    }
    // decl node->astKind == ASTKind::TUPLE_PATTERN
    auto tuplePattern = dynamic_cast<const TuplePattern*>(node.get());
    if (!tuplePattern) { return; }
    for (auto &pattern: tuplePattern->patterns) {
        if (pattern->astKind == ASTKind::TUPLE_PATTERN) {
            CompleteNode(pattern.get(), isImport, isInScope, isSameName);
            continue;
        }
        auto varPattern = dynamic_cast<VarPattern*>(pattern.get().get());
        if (!varPattern) { continue; }
        CompleteNode(varPattern->varDecl.get(), isImport, isInScope, isSameName);
    }
}

void CompletionEnv::DealTypeAlias(Ptr<Codira::AST::Node> node)
{
    auto *typeAlias = dynamic_cast<TypeAliasDecl*>(node.get());
    if (typeAlias == nullptr) { return; }
    std::string aliasName = typeAlias->identifier;
    if (typeAlias->type == nullptr) { return; }
    std::string declName = typeAlias->type->ToString();
    auto posOfLeft = declName.find_first_of('<');
    if (posOfLeft != std::string::npos) {
        declName = declName.substr(0, posOfLeft);
    }
    if (importedExternalDeclMap.find(declName) != importedExternalDeclMap.end() &&
        importedExternalDeclMap[declName].first) {
        auto targetDecl = importedExternalDeclMap[declName].first;
        if (targetDecl->astKind == ASTKind::TYPE_ALIAS_DECL) {
            if (aliasMap.find(declName) != aliasMap.end()) {
                targetDecl = aliasMap[declName];
            }
        }
        aliasMap[aliasName] = targetDecl;
        CompleteInitFuncDecl(targetDecl, aliasName, true);
    }
}

void CompletionEnv::DeepComplete(Ptr<Node> node, const Position pos)
{
    if (node == nullptr || visitedNodes.count(node)) { return; }
    visitedNodes.insert(node);
    auto func = NormalMatcher::GetInstance().GetFunc(node->astKind);
    if (func != nullptr) {
        (this->*(func))(node, pos);
    }
}

void CompletionEnv::SemaCacheComplete(Ptr<Node> node, const Position pos)
{
    if (node == nullptr) {
        return;
    }
    isCompleteFunction = true;
    DeepComplete(node, pos);
    isCompleteFunction = false;
}

void CompletionEnv::CompleteAliasItem(Ptr<Node> node, const std::string &aliasName, bool isImport)
{
    if (node == nullptr || !syscap.CheckSysCap(node)) {
        return;
    }
    if (auto ED = DynamicCast<EnumDecl *>(node)) {
        for (auto &ctor : ED->constructors) {
            CompleteNode(ctor.get());
        }
    }
    std::string signature = ItemResolverUtil::ResolveSignatureByNode(*node);
    std::string name = ItemResolverUtil::ResolveNameByNode(  *node);
    bool flag = signature.empty() || name.empty() || items.count(aliasName);
    if (flag) {
        return;
    }
    CodeCompletion completion;
    completion.name = aliasName;
    completion.label =
        signature.replace(signature.begin(), signature.begin() + static_cast<long long>(name.size()), aliasName);
    completion.kind = ItemResolverUtil::ResolveKindByNode(*node);
    completion.detail = ItemResolverUtil::ResolveDetailByNode(*node);
    completion.show = true;
    // if isImport complete simple
    if (auto decl = dynamic_cast<Decl *>(node.get())) {
        MarkDeprecated(*decl, completion);
    }
    if (isImport) {
        std::string originLabel = completion.label;
        completion.label = completion.insertText = completion.name;
        AddCompletionItem(originLabel, originLabel, completion);
        return;
    }
    std::string insertText = ItemResolverUtil::ResolveInsertByNode(*node);
    insertText =
        insertText.replace(insertText.begin(), insertText.begin() + static_cast<long long>(name.size()), aliasName);
    completion.insertText = insertText;
    if (node->TestAttr(Codira::AST::Attribute::MACRO_FUNC)) {
        std::string label = ItemResolverUtil::ResolveSignatureByNode(*node, parserAst->sourceManager, true);
        label = label.replace(label.begin(), label.begin() + static_cast<long long>(name.size()), aliasName);
        completion.label = label;
        completion.name = aliasName;
        AddCompletionItem(completion.label, completion.label, completion);
        CodeCompletion macroCompletion = completion;
        macroCompletion.label += "(input: Tokens)";
        macroCompletion.insertText += "(${2:input: Tokens})";
        AddCompletionItem(macroCompletion.label, macroCompletion.label, macroCompletion);
        return;
    }
    AddCompletionItem(completion.label, completion.label, completion);
    // complete class_decl and struct_decl init func_decl
    if (completion.kind == CompletionItemKind::CIK_CLASS || completion.kind == CompletionItemKind::CIK_STRUCT) {
        CompleteInitFuncDecl(node, aliasName);
    }
}

void CompletionEnv::CompleteInitFuncDecl(Ptr<Node> node, const std::string &aliasName, bool isType, bool isInScope)
{
    if (!node) { return; }
    if (auto md = DynamicCast<MacroExpandDecl*>(node)) {
        if (auto i = md->GetInvocation()) {
            auto className = i->decl->identifier;
            std::string query = "name: " + className + " && " + "ast_kind: class_decl";
            auto syms = SearchContext(cache->packageInstance->ctx, query);
            for (const auto &sym : syms) {
                if (sym->node->TestAttr(Codira::AST::Attribute::MACRO_EXPANDED_NODE)) {
                    CompleteInitFuncDecl(sym->node, aliasName, isInScope, isType);
                }
            }
        }
    }
    if (node->astKind == ASTKind::CLASS_DECL) {
        auto *decl = dynamic_cast<ClassDecl*>(node.get());
        bool isInvalid = !decl || !decl->body || !decl->body.get() || decl->body->decls.empty();
        if (isInvalid) {
            return;
        }
        CompleteClassOrStructDecl(*decl, aliasName, isType, isInScope);
    } else if (node->astKind == ASTKind::STRUCT_DECL) {
        auto *decl = dynamic_cast<StructDecl*>(node.get());
        bool isInvalid = !decl || !decl->body || !decl->body.get() || decl->body->decls.empty();
        if (isInvalid) {
            return;
        }
        CompleteClassOrStructDecl(*decl, aliasName, isType, isInScope);
    } else if (node->astKind == ASTKind::TYPE_ALIAS_DECL) {
        auto *decl = dynamic_cast<TypeAliasDecl*>(node.get());
        bool isInvalid = !decl || !decl->type || !decl->type->GetTarget();
        if (isInvalid) { return; }
        auto targetDecl = decl->type->GetTarget();
        if (targetDecl->IsStructOrClassDecl()) {
            CompleteInitFuncDecl(targetDecl, aliasName, true);
        }
    }
}

void CompletionEnv::CompleteFollowLambda(const Codira::AST::Node &node,
    Codira::SourceManager *sourceManager, CodeCompletion &completion, const std::string &initFuncReplace)
{
    std::string signature = ItemResolverUtil::ResolveFollowLambdaSignature(node, sourceManager, initFuncReplace);
    if (signature.empty()) {
        return;
    }
    std::string insert = ItemResolverUtil::ResolveFollowLambdaInsert(node, sourceManager, initFuncReplace);
    if (insert.empty()) {
        return;
    }
    completion.label = signature;
    completion.insertText = insert;
    AddCompletionItem(signature, signature, completion);
}

void CompletionEnv::CompleteParamListFuncTypeVarDecl(const Codira::AST::Node &node,
    Codira::SourceManager *sourceManager, CodeCompletion &completion)
{
    std::string signature;
    std::string insertText;
    ItemResolverUtil::ResolveParamListFuncTypeVarDecl(node, signature, insertText, sourceManager);
    if (signature.empty() || insertText.empty()) {
        return;
    }
    completion.label = signature;
    completion.insertText = insertText;
    AddCompletionItem(signature, signature, completion);
}

template<typename T>
void CompletionEnv::CompleteClassOrStructDecl(const T &decl, const std::string &aliasName, bool isType,
                                              bool isInScope)
{
    SourceManager *temp = parserAst == nullptr ? nullptr : parserAst->sourceManager;
    std::string replaceString = ItemResolverUtil::ResolveSignatureByNode(decl, temp);
    std::string insertReplaceString = decl.identifier + ItemResolverUtil::GetGenericInsertByDecl(decl.GetGeneric());
    if (!aliasName.empty()) {
        replaceString = isType ? aliasName : replaceString.replace(replaceString.begin(),
            replaceString.begin() + static_cast<long long>(decl.identifier.Val().size()),
            aliasName);
        insertReplaceString = isType ? aliasName : aliasName +
                                                   ItemResolverUtil::GetGenericInsertByDecl(decl.GetGeneric());
    }

    for (auto &it : decl.body->decls) {
        // if decl is imported primaryctor, the ty will be parsed as invalid
        if (!it) {
            continue;
        }
        bool isImportedPrimaryCtor = false;
        if (it->astKind == ASTKind::PRIMARY_CTOR_DECL) {
            auto got = importedExternalDeclMap.find(it->identifier);
            if (got != importedExternalDeclMap.end()) {
                isImportedPrimaryCtor = got->second.second;
            }
        }

        if (it && (((it->identifier.Val().compare("init") == 0 && it->astKind == ASTKind::FUNC_DECL)) ||
                it->astKind == ASTKind::PRIMARY_CTOR_DECL && !isImportedPrimaryCtor)) {
            // out package ensure it's PUBLIC  || in package ensure it's not Private
            bool flag = !isInScope && !(isInPackage && !it->TestAttr(Codira::AST::Attribute::PRIVATE)) &&
                        !((decl.fullPackageName != curPkgName && it->TestAttr(Codira::AST::Attribute::PUBLIC)) ||
                          (decl.fullPackageName == curPkgName && !it->TestAttr(Codira::AST::Attribute::PRIVATE)));
            if (flag) { continue; }
            std::string signature = ItemResolverUtil::ResolveSignatureByNode(*it.get(), temp, false, isAfterAT);
            auto len = static_cast<long long>(it->identifier.Val().size());
            signature = signature.replace(signature.begin(), signature.begin() + len, replaceString);
            if (!isInScope && IsSignatureInItems(replaceString, signature)) { continue; }
            CodeCompletion completion;
            if (auto declPtr = dynamic_cast<Decl*>(&(*it))) {
                MarkDeprecated(*declPtr, completion);
            }
            completion.label = signature;
            completion.name = replaceString;
            completion.kind = CompletionItemKind::CIK_FUNCTION;
            completion.detail = ItemResolverUtil::ResolveDetailByNode(*it.get(), temp);
            std::string insertText = ItemResolverUtil::ResolveInsertByNode(*it.get(), temp, isAfterAT);
            insertText = insertText.replace(insertText.begin(), insertText.begin() + len, insertReplaceString);
            completion.insertText = insertText;
            completion.show = true;
            AddCompletionItem(replaceString, signature, completion, false);
            // complete follow lambda for func_decl
            CompleteFollowLambda(*it.get(), parserAst->sourceManager, completion, replaceString);
        }
    }
}

void CompletionEnv::AccessibleByString(const std::string &str, const std::string &completionDetail)
{
    if (IsSignatureInItems(str, str)) {
        return;
    }
    std::string signature = str;
    CodeCompletion completion;
    completion.label = str;
    completion.name = str;
    completion.kind = CompletionItemKind::CIK_VARIABLE;
    if (completionDetail == "packageName" || completionDetail == "moduleName") {
        completion.kind = CompletionItemKind::CIK_MODULE;
    }
    completion.detail = completionDetail;
    completion.insertText = str;
    completion.show = true;
    AddCompletionItem(signature, signature, completion);
}

void CompletionEnv::DotPkgName(const std::string &fullPkgName, const std::string &prePkgName)
{
    std::string prefixWithDot = prePkgName + CONSTANTS::DOT;
    bool isInvalid = fullPkgName.empty() || fullPkgName.find(prefixWithDot) != 0 ||
                     prePkgName.length() >= fullPkgName.length() || items.count(fullPkgName) > 0;
    if (isInvalid) {
        return;
    }
    auto singlePkgName = ::TrimSubPkgName(fullPkgName.substr(prefixWithDot.size()));
    CodeCompletion completion;
    completion.label = singlePkgName;
    completion.name = singlePkgName;
    completion.kind = CompletionItemKind::CIK_MODULE;
    completion.detail = "packageName";
    completion.insertText = singlePkgName;
    completion.show = true;
    AddCompletionItem(singlePkgName, singlePkgName, completion);
}

bool CompletionEnv::CanCompleteStaticMember(const Codira::AST::Decl &decl) const
{
    return ark::Is<ClassDecl>(&decl) || ark::Is<InterfaceDecl>(&decl) || ark::Is<StructDecl>(&decl) ||
           ark::Is<EnumDecl>(&decl);
}

bool CompletionEnv::CheckParentAccessible(const Decl &decl, const Decl &parent, bool isSuperOrThis)
{
    if (!CanCompleteStaticMember(parent)) {
        return true;
    }
    // normal complete can complete protect and public
    bool flag = isFromNormal && (decl.TestAttr(Codira::AST::Attribute::PROTECTED) ||
                                     decl.TestAttr(Codira::AST::Attribute::PUBLIC));
    if (flag) {
        return true;
    }
    // super and this can completion protect && !static
    flag = isSuperOrThis && decl.TestAttr(Codira::AST::Attribute::PROTECTED) &&
                !decl.TestAttr(Codira::AST::Attribute::STATIC);
    if (flag) {
        return true;
    }
    // non-toplevel decl visible check
    auto relation = Modules::GetPackageRelation(curPkgName, decl.fullPackageName);
    if (decl.fullPackageName != curPkgName && !Modules::IsVisible(decl, relation)) {
        return false;
    }
    // about static
    flag = GetValue(FILTER::IS_SUPER) && GetValue(FILTER::IS_STATIC) && decl.TestAttr(Codira::AST::Attribute::STATIC);
    if (flag) {
        if (decl.TestAttr(Codira::AST::Attribute::PRIVATE)) {
            return false;
        }
        return true;
    }
    flag = !GetValue(FILTER::IGNORE_STATIC) && !decl.TestAttr(Codira::AST::Attribute::ENUM_CONSTRUCTOR) &&
           GetValue(FILTER::IS_STATIC) != decl.TestAttr(Codira::AST::Attribute::STATIC);
    if (flag) {
        return false;
    }
    if (directScope.count(parent.identifier)) {
        return true;
    }
    if (decl.TestAttr(Codira::AST::Attribute::PRIVATE)) {
        return false;
    }
    return true;
}

bool CompletionEnv::CheckInsideVarDecl(const Decl &decl) const
{
    if (!ark::Is<VarDecl>(&decl)) {
        return true;
    }
    if (decl.ty != nullptr && decl.ty->kind == Codira::AST::TypeKind::TYPE_ENUM) {
        return true;
    }
    if (GetValue(FILTER::INSIDE_VARDECL)) {
        for (auto var : insideVarDecl) {
            if (var->identifier == decl.identifier && var->GetBegin() == decl.GetBegin()) {
                return false;
            }
        }
    }
    return true;
}

void CompletionEnv::AddExtraMemberDecl(const std::string &name)
{
    if (idMap.find(name) == idMap.end()) { return; }
    for (auto item : idMap[name]) {
        if (item == nullptr) { continue; }
        for (auto &memberDecl : item->GetMemberDecls()) {
            if (!memberDecl->TestAttr(Codira::AST::Attribute::PRIVATE)) {
                CompleteNode(memberDecl.get());
            }
        }
    }
    auto importedExtendDecls = cache->packageInstance->importManager.GetImportedDeclsByName(*cache->file, name);
    for (auto decl : importedExtendDecls) {
        if (decl == nullptr) { continue; }
        for (auto &memberDecl : decl->GetMemberDecls()) {
            CompleteNode(memberDecl.get());
        }
    }
}

void CompletionEnv::OutputResult(CompletionResult &result) const
{
    result.cursorDepth = scopeDepth;
    for (const auto& outerPair : items) {
        const auto& innerMap = outerPair.second;
        for (const auto& item : innerMap) {
            if (!matchSelector.empty() && item.second.isEnumCtor && matchSelector != item.second.container) {
                continue;
            }
            if (item.second.show) {
                result.completions.push_back(item.second);
                result.normalCompleteSymID.insert(item.second.id);
            }
        }
    }
}

bool CompletionEnv::RefToDecl(Ptr<Node> node, bool insideFunction, bool deepestFunction)
{
    if (node == nullptr) {
        return false;
    }
    if (node->astKind == Codira::AST::ASTKind::REF_EXPR && ark::Is<RefExpr>(node.get())) {
        auto ref = dynamic_cast<RefExpr*>(node.get());
        bool isInvalid = ref == nullptr || ref->ref.target == nullptr || ref->ref.target->symbol == nullptr;
        if (isInvalid) {
            return true;
        }
        InvokedAccessible(ref->ref.target, insideFunction, deepestFunction);
        return true;
    }
    if (node->astKind == Codira::AST::ASTKind::REF_TYPE && ark::Is<RefType>(node.get())) {
        auto ref = dynamic_cast<RefType*>(node.get());
        bool isInvalid = ref == nullptr || ref->ref.target == nullptr || ref->ref.target->symbol == nullptr;
        if (isInvalid) {
            return true;
        }
        InvokedAccessible(ref->ref.target, insideFunction, deepestFunction);
        return true;
    }
    return false;
}

bool CompletionEnv::VarDeclIsLet(Ptr<Node> node) const
{
    bool flag = !node || node->astKind != Codira::AST::ASTKind::VAR_DECL || !ark::Is<VarDecl>(node.get());
    if (flag) {
        return true;
    }
    auto varDecl = dynamic_cast<VarDecl*>(node.get());
    return !varDecl->isVar;
}

void CompletionEnv::AddItemForMacro(Ptr<Node> node, CodeCompletion &completion)
{
    completion.label = ItemResolverUtil::ResolveSignatureByNode(*node, parserAst->sourceManager, true);
    AddCompletionItem(completion.label, completion.label, completion);
    CodeCompletion macroCompletion = completion;
    if (auto decl = dynamic_cast<Decl *>(node.get())) {
        MarkDeprecated(*decl, completion);
    }
    macroCompletion.label += "(input: Tokens)";
    macroCompletion.insertText += "(${2:input: Tokens})";
    AddCompletionItem(macroCompletion.label, macroCompletion.label, macroCompletion);
}

ark::lsp::SymbolID CompletionEnv::GetDeclSymbolID(const Decl& decl)
{
    auto identifier = decl.exportId;
    if (decl.astKind == ASTKind::FUNC_PARAM) {
        if (!decl.outerDecl) {
            return ark::lsp::INVALID_SYMBOL_ID;
        }
        identifier = decl.outerDecl->exportId + "$" + decl.identifier;
    }
    if (identifier.empty()) {
        return ark::lsp::INVALID_SYMBOL_ID;
    }
    size_t ret = 0;
    ret = hash_combine<std::string>(ret, identifier);
    return ret;
}

bool CompletionEnv::IsSignatureInItems(const std::string &name, const std::string &signature)
{
    auto signatureItems = items.find(name);
    if (signatureItems != items.end()) {
        auto item = signatureItems->second.find(signature);
        return item != signatureItems->second.end();
    }
    return false;
}

void CompletionEnv::AddCompletionItem(
    const std::string &name, const std::string &signature, const CodeCompletion &completion, bool overwrite)
{
    if (overwrite) {
        // Overwrite all values under name and insert a new signature.
        items[name] = {{signature, completion}};
    } else {
        // Check whether signature exists in the map of name.
        auto &signatureMap = items[name];
        if (signatureMap.find(signature) == signatureMap.end()) {
            // If signature does not exist, add signature.
            signatureMap[signature] = completion;
        }
    }
}

void CompletionEnv::SetSyscap(const std::string &moduleName)
{
    this->syscap.SetIntersectionSet(moduleName);
}

bool CompletionEnv::IsInEnumConstructor(OwnedPtr<Decl> &decl, Position pos)
{
    if (decl->GetBegin() > pos || decl->GetEnd() <= pos) {
        return false;
    }

    if (auto cstor = DynamicCast<FuncDecl>(decl.get())) {
        return !cstor->funcBody || cstor->funcBody->GetBegin() > pos || cstor->funcBody->GetEnd() <= pos;
    }
    return false;
}
}
