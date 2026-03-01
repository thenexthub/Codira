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

#include "SemanticHighlightImpl.h"
#include <algorithm>
#include "Codira/Utils/FileUtil.h"
#include "../../common/Utils.h"

using namespace Codira::Meta;

namespace ark {
using namespace Codira;
using namespace Codira::AST;
const std::vector<std::string> KEYWORD_IDENTIFIER = {"public", "private", "protected", "override", "abstract",
                                                     "open", "redef", "sealed"};

void AddAnnoToken(Ptr<Decl> node, std::vector<SemanticHighlightToken> &result,
                  const std::vector<Codira::Token> &tokens, Codira::SourceManager *sourceManager)
{
    for (auto& anno: node->annotations) {
        if (anno->baseExpr) {
            Position annoPos = anno->identifier.Begin();
            Range annoRange = {annoPos, {annoPos.fileID, annoPos.line,
                                         annoPos.column + CountUnicodeCharacters(anno->identifier)}};
            UpdateRange(tokens, annoRange, *anno);
            result.push_back({HighlightKind::CLASS_H, TransformFromChar2IDE(annoRange)});
        }
    }
}

void GetFuncDecl(Ptr<Node> node, std::vector<SemanticHighlightToken> &result, const std::vector<Codira::Token> &tokens,
                 Codira::SourceManager *sourceManager)
{
    if (node->TestAttr(Codira::AST::Attribute::PRIMARY_CONSTRUCTOR)) {
        return;
    }
    Ptr<Decl> decl = dynamic_cast<Decl*>(node.get());
    if (!decl || decl->identifier == "<invalid identifier>" || decl->identifier == "~init") { return; }
    Position pos = decl->GetIdentifierPos();

    HighlightKind kind = HighlightKind::FUNCTION_H;
    Range range = {pos, {pos.fileID, pos.line, pos.column + CountUnicodeCharacters(decl->identifier)}};
    UpdateRange(tokens, range, *decl);
    // Processes the set and get functions of the prop.
    if (!decl->identifierForLsp.empty()) {
        range = {pos, {pos.fileID, pos.line, pos.column + static_cast<int>(decl->identifierForLsp.length())}};
    }
    result.push_back({kind, TransformFromChar2IDE(range)});
    AddAnnoToken(decl, result, tokens, sourceManager);
}

void GetPrimaryDecl(
    Ptr<Node> node, std::vector<SemanticHighlightToken> &result, const std::vector<Codira::Token> &tokens,
    Codira::SourceManager *sourceManager)
{
    Ptr<Decl> decl = dynamic_cast<Decl*>(node.get());
    if (!decl) { return; }
    Position pos = decl->GetIdentifierPos();
    Range range = {pos, {pos.fileID, pos.line, pos.column + CountUnicodeCharacters(decl->identifier)}};
    UpdateRange(tokens, range, *decl);
    result.push_back({HighlightKind::CLASS_H, TransformFromChar2IDE(range)});
    AddAnnoToken(decl, result, tokens, sourceManager);
}

void GetVarDecl(Ptr<Node> node, std::vector<SemanticHighlightToken> &result, const std::vector<Codira::Token> &tokens,
                Codira::SourceManager *sourceManager)
{
    Ptr<Decl> decl = dynamic_cast<Decl*>(node.get());
    if (!decl) { return; }
    Position pos = decl->identifier.Begin();
    Range range = {pos, {pos.fileID, pos.line, pos.column + CountUnicodeCharacters(decl->identifier)}};
    UpdateRange(tokens, range, *decl);
    result.push_back({HighlightKind::VARIABLE_H, TransformFromChar2IDE(range)});
    AddAnnoToken(decl, result, tokens, sourceManager);
}

void GetPropDecl(Ptr<Node> node, std::vector<SemanticHighlightToken> &result, const std::vector<Codira::Token> &tokens,
                 Codira::SourceManager *sourceManager)
{
    Ptr<Decl> decl = dynamic_cast<Decl*>(node.get());
    if (!decl) { return; }
    Position pos = decl->identifier.Begin();
    Range range = {pos, {pos.fileID, pos.line, pos.column + CountUnicodeCharacters(decl->identifier)}};
    UpdateRange(tokens, range, *decl);
    result.push_back({HighlightKind::VARIABLE_H, TransformFromChar2IDE(range)});
    AddAnnoToken(decl, result, tokens, sourceManager);
}

void GetCallExpr(Ptr<Node> node, std::vector<SemanticHighlightToken> &result, const std::vector<Codira::Token> &tokens,
                 Codira::SourceManager *sourceManager)
{
    auto callExpr = dynamic_cast<CallExpr*>(node.get());
    if (!callExpr || !callExpr->symbol || !callExpr->resolvedFunction || !callExpr->baseFunc) {
        return;
    }
    Ptr<Decl> decl = callExpr->resolvedFunction;
    // case: value.add(1,2),"value" will be add in RefExpr, "add" will be add in memberaccess, so here return
    if ((ark::Is<MemberAccess>(callExpr->baseFunc.get().get()))) {
        return;
    }
    Position pos = node->GetBegin();
    if ((callExpr->leftParenPos.column - 1 - callExpr->GetBegin().column - 1) == static_cast<int>(
                                                                                     callExpr->symbol->name.size())) {
        pos.column = pos.column + 1;
    }
    Range range = {pos, {pos.fileID, pos.line, pos.column + static_cast<int>(
                                                                CountUnicodeCharacters(node->symbol->name))}};
    UpdateRange(tokens, range, *decl);
    if ((decl && decl->identifier != "init") || callExpr->callKind == CallKind::CALL_INVALID) {
        result.push_back({HighlightKind::FUNCTION_H, TransformFromChar2IDE(range)});
    }
}

void GetMemberAccess(
    Ptr<Node> node, std::vector<SemanticHighlightToken> &result, const std::vector<Codira::Token> &tokens,
    Codira::SourceManager *sourceManager)
{
    auto mAccess = dynamic_cast<MemberAccess*>(node.get());
    if (!mAccess || mAccess->field == "<invalid identifier>") {
        return;
    }

    Position pos = mAccess->GetEnd();
    Position fieldPos = mAccess->GetFieldPos();
    PositionUTF8ToIDE(tokens, pos, *node);
    PositionUTF8ToIDE(tokens, fieldPos, *node);
    Range leftRange = {{pos.fileID, pos.line, pos.column - static_cast<int>(CountUnicodeCharacters(mAccess->field))},
                       pos};
    Range rightRange = {fieldPos, {fieldPos.line, fieldPos.column + static_cast<int>(
                                                                        CountUnicodeCharacters(mAccess->field))}};

    if (mAccess->target != nullptr && mAccess->target->identifier == "init") {
        result.push_back({HighlightKind::CLASS_H, TransformFromChar2IDE(leftRange)});
    } else if (ark::Is<ClassLikeDecl>(mAccess->target.get()) || ark::Is<EnumDecl>(mAccess->target.get()) ||
               ark::Is<StructDecl>(mAccess->target.get()) || ark::Is<TypeAliasDecl>(mAccess->target.get())) {
        result.push_back({HighlightKind::CLASS_H, TransformFromChar2IDE(leftRange)});
    } else if (ark::Is<PackageDecl>(mAccess->target.get())) {
        leftRange = {node->GetBegin(), node->end};
        result.push_back({HighlightKind::PACKAGE_H, TransformFromChar2IDE(leftRange)});
    } else if (ark::Is<FuncDecl>(mAccess->target.get()) || (!mAccess->field.Empty() && !(mAccess->isAlone))) {
        /** the if condition contains two cases:
         * case1: access an existing function;
         * case2: access the function that modified externally.
         */
        if (ark::Is<VarDecl>(mAccess->target.get())) {
            result.push_back({HighlightKind::VARIABLE_H, TransformFromChar2IDE(rightRange)});
        } else {
            result.push_back({HighlightKind::FUNCTION_H, TransformFromChar2IDE(rightRange)});
        }
    } else {
        result.push_back({HighlightKind::FIELD_H, TransformFromChar2IDE(rightRange)});
    }
}

void GetFuncArg(Ptr<Node> node, std::vector<SemanticHighlightToken> &result, const std::vector<Codira::Token> &tokens,
                Codira::SourceManager *sourceManager)
{
    auto funcArg = dynamic_cast<FuncArg*>(node.get());
    if (!funcArg) { return; }
    Position pos = funcArg->name.Begin();
    if (pos.fileID == 0 && pos.line == 0 && pos.column == 0) {
        return;
    }
    Range range = {pos, {pos.fileID, pos.line, pos.column + static_cast<int>(CountUnicodeCharacters(funcArg->name))}};
    UpdateRange(tokens, range, *node);
    result.push_back({HighlightKind::VARIABLE_H, TransformFromChar2IDE(range)});
}

bool RefTargetEmpty(const Ptr<Node> node)
{
    auto refExpr = dynamic_cast<RefExpr*>(node.get());
    if (!refExpr) {
        return true;
    }
    Ptr<Decl> decl = refExpr->ref.target;

    return decl == nullptr;
}

bool SpecialTarget(const Ptr<Node> node)
{
    auto refExpr = dynamic_cast<RefExpr*>(node.get());
    Ptr<Decl> decl = refExpr->ref.target;
    bool isSpecialDecl = (ark::Is<ClassLikeDecl>(decl.get()) || ark::Is<EnumDecl>(decl.get()) ||
                          ark::Is<StructDecl>(decl.get()) || ark::Is<BuiltInDecl>(decl.get()) ||
                          ark::Is<TypeAliasDecl>(decl.get()));
    if (!isSpecialDecl) {
        return ark::Is<FuncDecl>(decl.get()) && decl->identifier == "init";
    }

    return isSpecialDecl;
}

/**
 * handle '$' expression's highlight
 *
 * ex: `$variable`
 * highlight `variable`
 *
 * @param range
 * @param sourceManager
 */
void HandleInterpolationExpr(Range &range, Codira::SourceManager *sourceManager)
{
    if (!sourceManager) {
        return;
    }
    if (range.start.column + 1 > range.end.column) {
        return;
    }
    Position end(range.start.fileID, range.start.line, range.start.column + 1);
    std::string result = sourceManager->GetContentBetween(range.start.fileID, range.start, end);
    if (result == "$") {
        range.start.column = range.start.column + 1;
        range.end.column = range.end.column + 1;
    }
}

void GetRefExpr(Ptr<Node> node, std::vector<SemanticHighlightToken> &result, const std::vector<Codira::Token> &tokens,
                Codira::SourceManager *sourceManager)
{
    auto refExpr = dynamic_cast<RefExpr*>(node.get());
    Ptr<Decl> decl = nullptr;
    if (!refExpr) {
        return;
    }
    decl = refExpr->ref.target;
    if (decl == nullptr) {
        return;
    }
    Position pos = refExpr->GetIdentifierPos();
    Range range = {pos, {pos.fileID, pos.line,
        pos.column + static_cast<int>(CountUnicodeCharacters(refExpr->ToString()))}};
    UpdateRange(tokens, range, *refExpr);
    bool isSpecialDecl = (ark::Is<ClassLikeDecl>(decl.get()) || ark::Is<EnumDecl>(decl.get()) ||
        ark::Is<StructDecl>(decl.get()) || ark::Is<BuiltInDecl>(decl.get()) || ark::Is<TypeAliasDecl>(decl.get()));
    if (isSpecialDecl) {
        result.push_back({HighlightKind::CLASS_H, TransformFromChar2IDE(range)});
    } else if (ark::Is<FuncDecl>(decl.get())) {
        if (decl->identifier == "init") {
            result.push_back({HighlightKind::CLASS_H, TransformFromChar2IDE(range)});
        } else {
            result.push_back({HighlightKind::FUNCTION_H, TransformFromChar2IDE(range)});
        }
    } else if (ark::Is<PackageDecl>(decl.get())) {
        result.push_back({HighlightKind::PACKAGE_H, TransformFromChar2IDE(range)});
    } else if (refExpr->isBaseFunc) {
        if (ark::Is<VarDecl>(decl.get())) {
            result.push_back({HighlightKind::VARIABLE_H, TransformFromChar2IDE(range)});
        } else {
            // undefined function highlight
            result.push_back({HighlightKind::FUNCTION_H, TransformFromChar2IDE(range)});
        }
    } else {
        HandleInterpolationExpr(range, sourceManager);
        result.push_back({HighlightKind::VARIABLE_H, TransformFromChar2IDE(range)});
    }
}

void GetClassDecl(
    Ptr<Node> node, std::vector<SemanticHighlightToken> &result, const std::vector<Codira::Token> &tokens,
    Codira::SourceManager *sourceManager)
{
    auto decl = dynamic_cast<Decl*>(node.get());
    // 2021.11.10 if it is a wrong class name, identifier is "<invalid identifier>" and server don't highlight it
    if (!decl || decl->identifier == "<invalid identifier>") {
        return;
    }
    Position pos = decl->GetIdentifierPos();
    Range range = {pos, {pos.fileID, pos.line, pos.column + CountUnicodeCharacters(decl->identifier)}};
    UpdateRange(tokens, range, *decl);
    result.push_back({HighlightKind::CLASS_H, TransformFromChar2IDE(range) });
    AddAnnoToken(decl, result, tokens, sourceManager);
}

void GetRefType(Ptr<Node> node, std::vector<SemanticHighlightToken> &result, const std::vector<Codira::Token> &tokens,
                Codira::SourceManager *sourceManager)
{
    auto refType = dynamic_cast<RefType*>(node.get());
    if (!refType) {
        return;
    }

    Position pos = refType->ref.identifier.Begin();
    if (pos.IsZero() && refType->symbol && refType->symbol->name == "Option") {
        return;
    }
    Range range = {pos, {pos.fileID, pos.line,
        pos.column + static_cast<int>(CountUnicodeCharacters(refType->ref.identifier))}};
    UpdateRange(tokens, range, *node);
    if (refType->ref.target == nullptr) {
        return;
    }

    // target isn't null, but target.symbol is null.  for example: systemlib
    if (refType->ref.target->astKind == ASTKind::PACKAGE_DECL) {
        result.push_back({HighlightKind::PACKAGE_H, TransformFromChar2IDE(range)});
    }
    // AuthorizeSuccessObj is a class of Systemlib, but AuthorizeSuccessObj is used separately
    // for example: success: Option<(AuthorizeSuccessObj)
    Ptr<Decl> decl = refType->ref.target;
    if (decl == nullptr) {
        return;
    }

    bool isValidDecl = decl->astKind == ASTKind::CLASS_DECL || decl->astKind == ASTKind::ENUM_DECL ||
                       decl->astKind == ASTKind::BUILTIN_DECL || decl->astKind == ASTKind::TYPE_ALIAS_DECL;
    if (isValidDecl) {
        result.push_back({HighlightKind::CLASS_H, TransformFromChar2IDE(range)});
    } else if (decl->astKind == ASTKind::INTERFACE_DECL) {
        result.push_back({HighlightKind::INTERFACE_H, TransformFromChar2IDE(range)});
    } else if (decl->astKind == ASTKind::STRUCT_DECL) {
        result.push_back({HighlightKind::RECORD_H, TransformFromChar2IDE(range)});
    } else {
        result.push_back({HighlightKind::VARIABLE_H, TransformFromChar2IDE(range)});
    }
}

void GetFuncParam(
    Ptr<Node> node, std::vector<SemanticHighlightToken> &result, const std::vector<Codira::Token> &tokens,
    Codira::SourceManager *sourceManager)
{
    auto funcParam = dynamic_cast<FuncParam*>(node.get());
    if (!funcParam || funcParam->isIdentifierCompilerAdd) { return; }
    Position pos = funcParam->identifier.Begin();
    Range range = {pos, {pos.fileID, pos.line, pos.column + CountUnicodeCharacters(funcParam->identifier)}};
    UpdateRange(tokens, range, *node);
    result.push_back({HighlightKind::VARIABLE_H, TransformFromChar2IDE(range)});
    AddAnnoToken(funcParam, result, tokens, sourceManager);
}

void GetInterfaceDecl(Ptr<Node> node, std::vector<SemanticHighlightToken> &result,
                      const std::vector<Codira::Token> &tokens, Codira::SourceManager *sourceManager)
{
    Ptr<Decl> decl = dynamic_cast<Decl*>(node.get());
    if (!decl || decl->identifier == "<invalid identifier>") { return; }
    Position pos = decl->identifier.Begin();
    Range range = {pos, {pos.fileID, pos.line, pos.column + CountUnicodeCharacters(decl->identifier)}};
    UpdateRange(tokens, range, *decl);
    result.push_back({HighlightKind::INTERFACE_H, TransformFromChar2IDE(range)});
    AddAnnoToken(decl, result, tokens, sourceManager);
}

void GetStructDecl(
    Ptr<Node> node, std::vector<SemanticHighlightToken> &result, const std::vector<Codira::Token> &tokens,
    Codira::SourceManager *sourceManager)
{
    Ptr<Decl> decl = dynamic_cast<Decl*>(node.get());
    if (!decl || decl->identifier == "<invalid identifier>") { return; }
    Position pos = decl->identifier.Begin();
    Range range = {pos, {pos.fileID, pos.line, pos.column + CountUnicodeCharacters(decl->identifier)}};
    UpdateRange(tokens, range, *decl);
    result.push_back({HighlightKind::CLASS_H, TransformFromChar2IDE(range)});
    AddAnnoToken(decl, result, tokens, sourceManager);
}

void GetEnumDecl(Ptr<Node> node, std::vector<SemanticHighlightToken> &result, const std::vector<Codira::Token> &tokens,
                 Codira::SourceManager *sourceManager)
{
    Ptr<Decl> decl = dynamic_cast<Decl*>(node.get());
    if (!decl || decl->identifier == "<invalid identifier>") { return; }
    Position pos = decl->identifier.Begin();
    Range range = {pos, {pos.fileID, pos.line, pos.column + CountUnicodeCharacters(decl->identifier)}};
    UpdateRange(tokens, range, *decl);
    result.push_back({HighlightKind::CLASS_H, TransformFromChar2IDE(range)});
    AddAnnoToken(decl, result, tokens, sourceManager);
}

void GetGenericParam(
    Ptr<Node> node, std::vector<SemanticHighlightToken> &result, const std::vector<Codira::Token> &tokens,
    Codira::SourceManager *sourceManager)
{
    auto genericParam = dynamic_cast<GenericParamDecl*>(node.get());
    if (!genericParam) { return; }
    if (genericParam->identifier.Val().compare("<invalid identifier>") == 0) {
        return;
    }
    Position pos = genericParam->identifier.Begin();
    Range range = {pos, {pos.fileID, pos.line,
        pos.column + static_cast<int>(CountUnicodeCharacters(genericParam->identifier))}};
    UpdateRange(tokens, range, *node);
    result.push_back({HighlightKind::VARIABLE_H, TransformFromChar2IDE(range)});
    AddAnnoToken(genericParam, result, tokens, sourceManager);
}

void GetQualifiedType(Ptr<Node> node, std::vector<SemanticHighlightToken> &result,
                      const std::vector<Codira::Token> &tokens, Codira::SourceManager *sourceManager)
{
    auto qualifiedType = dynamic_cast<QualifiedType*>(node.get());
    if (!qualifiedType) { return; }

    Position pos = {qualifiedType->GetBegin().fileID, qualifiedType->GetBegin().line,
        qualifiedType->end.column - static_cast<int>(qualifiedType->field.Val().size())};
    Range range = {pos, {pos.fileID, pos.line,
        pos.column + static_cast<int>(CountUnicodeCharacters(qualifiedType->field))}};
    UpdateRange(tokens, range, *node);
    static std::unordered_map<ASTKind, HighlightKind> highlightMap = {
        {ASTKind::PACKAGE_DECL, HighlightKind::PACKAGE_H},
        {ASTKind::CLASS_LIKE_DECL, HighlightKind::CLASS_H},
        {ASTKind::CLASS_DECL, HighlightKind::CLASS_H},
        {ASTKind::ENUM_DECL, HighlightKind::CLASS_H},
        {ASTKind::TYPE_ALIAS_DECL, HighlightKind::CLASS_H},
        {ASTKind::INTERFACE_DECL, HighlightKind::INTERFACE_H},
        {ASTKind::STRUCT_DECL, HighlightKind::CLASS_H},
        {ASTKind::FUNC_DECL, HighlightKind::FUNCTION_H}
    };
    HighlightKind kind = HighlightKind::VARIABLE_H;
    if (qualifiedType->target != nullptr && highlightMap.count(qualifiedType->target->astKind)) {
        kind = highlightMap[qualifiedType->target->astKind];
    }
    result.push_back({kind, TransformFromChar2IDE(range)});
}

void GetMacroExtendDecl(Ptr<Node> node, std::vector<SemanticHighlightToken> &result,
                        const std::vector<Codira::Token> &tokens, Codira::SourceManager *sourceManager)
{
    Range range = GetMacroRange<MacroExpandDecl>(*node);
    UpdateRange(tokens, range, *node);
    result.push_back({HighlightKind::FUNCTION_H, TransformFromChar2IDE(range)});
    auto *actualType = dynamic_cast<Codira::AST::MacroExpandDecl*>(node.get());
    if (actualType != nullptr && actualType->invocation.fullNameDotPos.size()) {
        auto start = actualType->GetIdentifierPos();
        auto end = actualType->invocation.fullNameDotPos.back();
        Range packageRange = {start, end};
        UpdateRange(tokens, packageRange, *node);
        result.push_back({HighlightKind::PACKAGE_H, TransformFromChar2IDE(packageRange)});
    }
    AddAnnoToken(actualType, result, tokens, sourceManager);
}

void GetMacroExtendExpr(Ptr<Node> node, std::vector<SemanticHighlightToken> &result,
                        const std::vector<Codira::Token> &tokens, Codira::SourceManager *sourceManager)
{
    Range range = GetMacroRange<MacroExpandExpr>(*node);
    UpdateRange(tokens, range, *node);
    result.push_back({HighlightKind::FUNCTION_H, TransformFromChar2IDE(range)});
}

void GetTypeAliasDecl(Ptr<Node> node, std::vector<SemanticHighlightToken> &result,
                      const std::vector<Codira::Token> &tokens, Codira::SourceManager *sourceManager)
{
    Ptr<Decl> decl = dynamic_cast<Decl*>(node.get());
    if (!decl || decl->identifier == "<invalid identifier>") { return; }
    Position pos = decl->identifier.Begin();
    Range range = {pos, {pos.fileID, pos.line, pos.column + CountUnicodeCharacters(decl->identifier)}};
    UpdateRange(tokens, range, *decl);
    result.push_back({HighlightKind::CLASS_H, TransformFromChar2IDE(range)});
}

using Func = void(*)(Ptr<Node> node, std::vector<SemanticHighlightToken> &, const std::vector<Codira::Token> &,
                      Codira::SourceManager *sourceManager);

bool FindCharKeyWord(const std::string &tokenName)
{
    if (std::find(KEYWORD_IDENTIFIER.begin(), KEYWORD_IDENTIFIER.end(), tokenName) != KEYWORD_IDENTIFIER.end()) {
        return false;
    }
    unsigned int len = sizeof(Codira::TOKENS) / sizeof(Codira::TOKENS[0]);
    for (unsigned int i = 0; i < len; i++) {
        std::string token(Codira::TOKENS[i]);
        if (token.empty()) {
            continue;
        }
        if (isalpha(token[0]) || token == "!in") {
            if (token == tokenName) {
                return true;
            }
        }
    }
    return false;
}

bool SemanticHighlightImpl::NodeValid(const Ptr<Node> node, unsigned int fileID, const std::string &name)
{
    if (!node) {
        return false;
    }

    unsigned int symbolFileID = node->GetBegin().fileID;
    if (symbolFileID != fileID) {
        return false;
    }
    // primary constructor node name is init, but it need highlight
    if (!node->TestAttr(Codira::AST::Attribute::PRIMARY_CONSTRUCTOR) &&
        FindCharKeyWord(name)) {
        return false;
    }

    if (IsZeroPosition(node)) {
        return false;
    }
    bool isInvalid = node->TestAttr(Codira::AST::Attribute::COMPILER_ADD) &&
                     (!node->TestAttr(Codira::AST::Attribute::IS_CLONED_SOURCE_CODE) &&
                      !node->TestAttr(Codira::AST::Attribute::PRIMARY_CONSTRUCTOR));
    if (isInvalid) {
        return false;
    }

    return true;
}

void SemanticHighlightImpl::FindHighlightsTokens(const ArkAST &ast, std::vector<SemanticHighlightToken> &result,
                                                 unsigned int fileID)
{
    Logger& logger = Logger::Instance();
    logger.LogMessage(MessageType::MSG_LOG, "ArkAST::FindHighlightsTokensV2 in.");

    std::map<ASTKind, Func> highlights;
    (void)highlights.emplace(std::pair<ASTKind, Func>(ASTKind::FUNC_DECL, GetFuncDecl));
    (void)highlights.emplace(std::pair<ASTKind, Func>(ASTKind::PRIMARY_CTOR_DECL, GetPrimaryDecl));
    (void)highlights.emplace(std::pair<ASTKind, Func>(ASTKind::VAR_DECL, GetVarDecl));
    (void)highlights.emplace(std::pair<ASTKind, Func>(ASTKind::PROP_DECL, GetPropDecl));
    (void)highlights.emplace(std::pair<ASTKind, Func>(ASTKind::CALL_EXPR, GetCallExpr));
    (void)highlights.emplace(std::pair<ASTKind, Func>(ASTKind::MEMBER_ACCESS, GetMemberAccess));
    (void)highlights.emplace(std::pair<ASTKind, Func>(ASTKind::FUNC_ARG, GetFuncArg));
    (void)highlights.emplace(std::pair<ASTKind, Func>(ASTKind::REF_EXPR, GetRefExpr));
    (void)highlights.emplace(std::pair<ASTKind, Func>(ASTKind::CLASS_DECL, GetClassDecl));
    (void)highlights.emplace(std::pair<ASTKind, Func>(ASTKind::REF_TYPE, GetRefType));
    (void)highlights.emplace(std::pair<ASTKind, Func>(ASTKind::FUNC_PARAM, GetFuncParam));
    (void)highlights.emplace(std::pair<ASTKind, Func>(ASTKind::INTERFACE_DECL, GetInterfaceDecl));
    (void)highlights.emplace(std::pair<ASTKind, Func>(ASTKind::STRUCT_DECL, GetStructDecl));
    (void)highlights.emplace(std::pair<ASTKind, Func>(ASTKind::ENUM_DECL, GetEnumDecl));
    (void)highlights.emplace(std::pair<ASTKind, Func>(ASTKind::GENERIC_PARAM_DECL, GetGenericParam));
    (void)highlights.emplace(std::pair<ASTKind, Func>(ASTKind::QUALIFIED_TYPE, GetQualifiedType));
    (void)highlights.emplace(std::pair<ASTKind, Func>(ASTKind::MACRO_EXPAND_DECL, GetMacroExtendDecl));
    (void)highlights.emplace(std::pair<ASTKind, Func>(ASTKind::MACRO_EXPAND_EXPR, GetMacroExtendExpr));
    (void)highlights.emplace(std::pair<ASTKind, Func>(ASTKind::TYPE_ALIAS_DECL, GetTypeAliasDecl));

    for (const auto& symbol : ast.packageInstance->ctx->symbolTable) {
        bool symbolInValid = !symbol || !symbol->node || symbol->invertedIndexBeenDeleted;
        if (symbolInValid || !NodeValid(symbol->node, fileID, symbol->name)) {
            continue;
        }
        Ptr<Node> node = symbol->node;
        if (node->TestAttr(Codira::AST::Attribute::MACRO_EXPANDED_NODE)) {
            if (symbol->astKind != ASTKind::REF_EXPR && symbol->astKind != ASTKind::MEMBER_ACCESS &&
                symbol->astKind != ASTKind::CALL_EXPR) {
                continue;
            }
            auto func = highlights.find(symbol->astKind);
            if (!RefTargetEmpty(node) && SpecialTarget(node)) {
                (func->second)(node, result, ast.tokens, ast.sourceManager);
            } else if (symbol->astKind == ASTKind::MEMBER_ACCESS && NeedHightlight(ast, node)) {
                (func->second)(node, result, ast.tokens, ast.sourceManager);
            } else if (symbol->astKind == ASTKind::CALL_EXPR) {
                (func->second)(node, result, ast.tokens, ast.sourceManager);
            }
            continue;
        }
        auto func = highlights.find(symbol->astKind);
        if (func != highlights.end()) {
            (func->second)(node, result, ast.tokens, ast.sourceManager);
        }
    }
}
bool SemanticHighlightImpl::NeedHightlight(const ArkAST &ast, const Ptr<Node> &node)
{
    auto mAccess = dynamic_cast<MemberAccess *>(node.get());
    if (!mAccess || mAccess->field == "<invalid identifier>") {
        return false;
    }

    Position pos = mAccess->GetEnd();
    auto index = ast.GetCurToken(pos, 0, static_cast<int>(ast.tokens.size()) - 1);
    if (index == -1) {
        return false;
    }
    Token tok = ast.tokens[static_cast<unsigned int>(index)];
    if (tok.kind != TokenKind::IDENTIFIER ||
        tok.Begin().column + static_cast<int>(tok.Value().size()) != pos.column) {
        return false;
    }
    return true;
}
} // namespace ark
