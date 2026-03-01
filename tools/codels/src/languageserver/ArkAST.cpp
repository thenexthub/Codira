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

#include "ArkAST.h"
#include "logger/Logger.h"
#include "common/Utils.h"

#include <string>
#include <vector>

using namespace std;
using namespace Codira;
using namespace Codira::AST;
using namespace Codira::Meta;
namespace {
    const int STRING_LITERAL_SIZE = 2; // STRING_LITERAL SIZ
}

namespace ark {
#ifdef THIS
#define LIB_THIS THIS
#undef THIS
#endif
const std::unordered_set<TokenKind> CHECK_TOKEN_KIND = {TokenKind::IDENTIFIER, TokenKind::STRING_LITERAL,
                                                        TokenKind::MULTILINE_STRING, TokenKind::DOLLAR_IDENTIFIER,
                                                        TokenKind::INIT, TokenKind::SUPER, TokenKind::THIS,
                                                        TokenKind::MAIN, TokenKind::PUBLIC, TokenKind::PRIVATE,
                                                        TokenKind::PROTECTED, TokenKind::OVERRIDE,
                                                        TokenKind::ABSTRACT, TokenKind::OPEN, TokenKind::REDEF,
                                                        TokenKind::SEALED};
// "[]","!","**","*","%","/","+","-","<<",">>","<","<=",">",">=","==","!=","&","^","|"
const std::unordered_set<TokenKind> OPERATOR_TO_OVERLOAD = {TokenKind::LSQUARE, TokenKind::RSQUARE, TokenKind::NOT,
                                                            TokenKind::EXP, TokenKind::MUL, TokenKind::MOD,
                                                            TokenKind::DIV, TokenKind::ADD, TokenKind::SUB,
                                                            TokenKind::LSHIFT, TokenKind::RSHIFT, TokenKind::LT,
                                                            TokenKind::LE, TokenKind::GT, TokenKind::GE,
                                                            TokenKind::EQUAL, TokenKind::NOTEQ, TokenKind::BITAND,
                                                            TokenKind::BITXOR, TokenKind::BITOR};

Ptr<Decl> ArkAST::FindDeclByNode(Ptr<Node> node) const
{
    Ptr<Decl> tmp = nullptr;
    if (!node) { return tmp; }
    return match(*node) (
        [](const MacroExpandDecl &macroExpandDecl) {
            return macroExpandDecl.invocation.target;
        },
        [](Decl &decl) {
            return Ptr(&decl);
        },
        [](const RefType &refType) {
            return refType.ref.target;
        },
        [](const RefExpr &refExpr) {
            return refExpr.ref.target;
        },
        [](const CallExpr &callExpr) {
            return Ptr(RawStaticCast<Decl*>(callExpr.resolvedFunction));
        },
        [](const VarPattern &varPattern) {
            return Ptr(RawStaticCast<Decl*>(varPattern.varDecl.get()));
        },
        [](const MemberAccess &mAccess) {
            return mAccess.target;
        },
        [this](const IfExpr &ifExpr) {
            return FindDeclByNode(ifExpr.condExpr.get());
        },
        [this](const FuncBody &funcBody) {
            return FindDeclByNode(funcBody.funcDecl);
        },
        [](const QualifiedType &qualifiedType) {
            return qualifiedType.target;
        },
        [](const MacroExpandExpr &macroExpandExpr) {
            return macroExpandExpr.invocation.target;
        },
        [tmp](const OverloadableExpr &overloadableExpr) {
            auto callExpr = dynamic_cast<CallExpr*>(overloadableExpr.desugarExpr.get().get());
            if (callExpr != nullptr) {
                return Ptr(RawStaticCast<Decl*>(callExpr->resolvedFunction));
            }
            return tmp;
        },
        // ----------- Match nothing --------------------
        [tmp]() { return Ptr(tmp); }
    );
}

void ArkAST::DoLexer(const std::string &contents, const std::string &fileName)
{
    if (sourceManager == nullptr || sourceManager->GetFileID(fileName) < 0) {
        return;
    }
    const unsigned int curFileID = static_cast<unsigned int>(sourceManager->GetFileID(fileName));
    Lexer lexer(curFileID, contents, diag, *sourceManager);
    for (;;) {
        Token tok = lexer.Next();
        if (tok.kind == TokenKind::END) {
            break;
        }
        tokens.push_back(tok);
    }
}

bool ArkAST::CheckTokenKind(TokenKind tokenKind, bool isForRename) const
{
    if (isForRename) {
        return CheckTokenKindWhenRenamed(tokenKind);
    }
    if (CHECK_TOKEN_KIND.count(tokenKind)) {
        return true;
    }
    return OPERATOR_TO_OVERLOAD.find(tokenKind) != OPERATOR_TO_OVERLOAD.end();
}

bool ArkAST::CheckTokenKindWhenRenamed(TokenKind tokenKind) const
{
    return tokenKind == TokenKind::IDENTIFIER || tokenKind == TokenKind::STRING_LITERAL ||
           tokenKind == TokenKind::MULTILINE_STRING || tokenKind == TokenKind::DOLLAR_IDENTIFIER ||
           tokenKind == TokenKind::PUBLIC || tokenKind == TokenKind::PRIVATE ||
           tokenKind == TokenKind::PROTECTED || tokenKind == TokenKind::OVERRIDE ||
           tokenKind == TokenKind::ABSTRACT || tokenKind == TokenKind::OPEN || tokenKind == TokenKind::REDEF ||
           tokenKind == TokenKind::SEALED;
}

int ArkAST::GetCurToken(const Codira::Position &pos, int start, int end) const
{
    if (start < 0 || start > end || end >= static_cast<int>(tokens.size())) {
        return -1;
    }

    int midIndex = (start + end) / 2;
    Token midToken = tokens[static_cast<unsigned int>(midIndex)];
    Position midTokenBegin = midToken.Begin();
    // case: a[pos]b, pos point to b, and either a or b can be highlighted,
    // so we need to care midTokenEndNextPos(midTokenEnd.column + 1)
    Position midTokenEndNextPos = midToken.End();

    if (midToken.kind == Codira::TokenKind::MULTILINE_STRING) {
        std::string s = "";
        int multiLineEnd = LineOfCommentEnd(midToken, s);
        midTokenEndNextPos.line = multiLineEnd;
    }

    if (pos <= midTokenBegin) {
        return GetCurToken(pos, start, midIndex - 1);
    } else if (pos <= midTokenEndNextPos) {
        return midIndex;
    } else {
        return GetCurToken(pos, midIndex + 1, end);
    }
}

int ArkAST::GetCurTokenByStartColumn(const Codira::Position &pos, int start, int end) const
{
    int idx = GetCurToken(pos, start, end);
    if (idx == -1) {
        Position nextColumnPos = {pos.fileID, pos.line, pos.column + 1};
        int newIdx = GetCurToken(nextColumnPos, start, end);
        if (newIdx == -1) {
            return newIdx;
        }        
        Token curToken = tokens[static_cast<unsigned int>(newIdx)];
        if (nextColumnPos >= curToken.Begin() && nextColumnPos <= curToken.End()) {
            return newIdx;
        }
        return -1;
    }
    Token curToken = tokens[static_cast<unsigned int>(idx)];
    if (pos < curToken.Begin() && idx - 1 >= 0) {
        auto preToken = tokens[static_cast<size_t>(idx - 1)];
        if (preToken.Begin() <= pos && preToken.End() > pos) {
            return idx - 1;
        }
    }
    if (pos >= curToken.End() && static_cast<size_t>(idx + 1) < tokens.size()) {
        auto nextToken = tokens[static_cast<size_t>(idx + 1)];
        if (nextToken.Begin() <= pos && nextToken.End() > pos) {
            return idx + 1;
        }
    }
    return idx;
}

int ArkAST::GetCurTokenSkipSpace(const Codira::Position &pos, int start, int end, int lastEnd) const
{
    if (start > end) {
        return -1;
    }

    int midIndex = (start + end) / 2;
    Token midToken = tokens[static_cast<unsigned int>(midIndex)];
    Position midTokenBegin = midToken.Begin();
    // case: a  [pos] b, we can skip the space to find a's index
    Position midTokenEndNextPos = midToken.End();

    if (midIndex + 1 <= lastEnd) {
        Token behindToken = tokens[static_cast<unsigned int>(midIndex + 1)];
        midTokenEndNextPos = behindToken.Begin();
    }
    if (pos <= midTokenBegin) {
        return GetCurTokenSkipSpace(pos, start, midIndex - 1, lastEnd);
    } else if (pos <= midTokenEndNextPos) {
        return midIndex;
    } else {
        return GetCurTokenSkipSpace(pos, midIndex + 1, end, lastEnd);
    }
}

int ArkAST::GetCurTokenByPos(const Position &pos, int start, int end, bool isForRename) const
{
    if (start > end) {
        return -1;
    }

    int midIndex = (start + end) / 2;
    Token midToken = tokens[static_cast<unsigned int>(midIndex)];
    Position midTokenBegin = midToken.Begin();
    // case: a[pos]b, pos point to b, and either a or b can be highlighted,
    // so we need to care midTokenEndNextPos(midTokenEnd.column + 1)
    std::string lastString;
    int line = LineOfCommentEnd(midToken, lastString);
    Position midTokenEndNextPos;
    midTokenEndNextPos.fileID = midToken.Begin().fileID;
    midTokenEndNextPos.line = line;
    /**
     * Two Scenarios.
     * Scenario 1: the start and end of the token are in the same line.
     * Scenario 2: the start and end of the token are in different line.
     */
    if (midToken.kind == Codira::TokenKind::MULTILINE_STRING) {
        midTokenEndNextPos.column = static_cast<int>(lastString.length());
    } else {
        midTokenEndNextPos.column = midToken.Begin().column + static_cast<int>(lastString.length());
    }

    if (pos < midTokenBegin) {
        return GetCurTokenByPos(pos, start, midIndex - 1, isForRename);
    } else if (pos == midTokenBegin) {
        // case: init([pos]r: uint8) {}, pos between '(' and 'r', tok at 'r'
        if (CheckTokenKind(midToken.kind, isForRename)) {
            return midIndex;
        }
        // case: init(r[pos]: uint8) {}, pos between 'r' and ':', tok at ':'
        if ((midIndex - 1) >= 0 && CheckTokenKind(tokens[static_cast<unsigned int>(midIndex - 1)].kind, isForRename)) {
            return midIndex - 1;
        }
        return -1;
    } else if (pos <= midTokenEndNextPos) {
        // case: this.c[pos][whitespace]= cc, in lexer the whitespace will be discarded,
        // so pos don't belong to any token, when midToken at 'c', need look forward
        if (CheckTokenKind(midToken.kind, isForRename)) {
            return midIndex;
        }
        return GetCurTokenByPos(pos, midIndex + 1, end, isForRename);
    } else {
        return GetCurTokenByPos(pos, midIndex + 1, end, isForRename);
    }
}

bool ArkAST::IsFilterToken(const Position &pos) const
{
    int idx = GetCurTokenByPos(pos, 0, static_cast<int>(tokens.size()) - 1);
    if (idx == -1) {
        return true;
    }
    Token curToken = tokens[static_cast<size_t>(idx)];
    return curToken.kind == TokenKind::THIS || curToken.kind == TokenKind::SUPER;
}

bool ArkAST::IsFilterTokenInHighlight(const Position &pos) const
{
    int idx = GetCurTokenByPos(pos, 0, static_cast<int>(tokens.size()) - 1);
    if (idx == -1) {
        return true;
    }
    Token curToken = tokens[static_cast<size_t>(idx)];
    if (curToken.kind == TokenKind::THIS || curToken.kind == TokenKind::SUPER) {
        return true;
    }
    if (idx - 1 >= 0) {
        Token preToken = tokens[static_cast<size_t>(idx - 1)];
        return NAME_TO_ANNO_KIND.count(curToken.Value()) && preToken.kind == TokenKind::AT;
    }
    return false;
}

Ptr<Decl> ArkAST::GetDeclByPosition(const Codira::Position& originPos, std::vector<Codira::AST::Symbol*>& syms,
    std::vector<Ptr<Decl>>& decls, const pair<bool, bool>& isMacroOrAlias) const
{
    Logger &logger = Logger::Instance();
    // check current token is the kind required in function CheckTokenKind(TokenKind)
    int index = GetCurTokenByPos(originPos, 0, static_cast<int>(tokens.size()) - 1);
    if (index < 0 || tokens[static_cast<size_t>(index)].kind == TokenKind::MAIN) {
        return nullptr;
    }
    auto curToken = tokens[static_cast<size_t>(index)];
    // filter pos search err result
    if (curToken.Begin().line == originPos.line && (curToken.Begin().column > originPos.column ||
                                                curToken.Value().length() <
                                                static_cast<size_t>(originPos.column - curToken.Begin().column))) {
        return nullptr;
    }
    Position pos = originPos;
    if (tokens[static_cast<size_t>(index)].kind != TokenKind::MULTILINE_STRING &&
        tokens[static_cast<size_t>(index)].kind != TokenKind::MULTILINE_RAW_STRING &&
        tokens[static_cast<size_t>(index)].kind != TokenKind::STRING_LITERAL &&
        OPERATOR_TO_OVERLOAD.find(tokens[index].kind) == OPERATOR_TO_OVERLOAD.end()) {
        pos = tokens[static_cast<size_t>(index)].Begin();
    }
    std::string query = "_ = (" + std::to_string(pos.fileID) + ", "
                        + std::to_string(pos.line) + ", " + std::to_string(pos.column) + ")";

    syms = SearchContext(packageInstance->ctx, query);
    if (syms.empty()) {
        logger.LogMessage(MessageType::MSG_WARNING, "the result of search is empty.");
        return nullptr;
    }
    if (!syms[0] || IsMarkPos(syms[0]->node, pos)) {
        return nullptr;
    }
    decls = FindRealDecl(*this, syms, query, pos, isMacroOrAlias);
    // filters modifiers before definition
    bool invalid = decls.empty() || IsModifierBeforeDecl(decls[0], pos);
    if (invalid) {
        logger.LogMessage(MessageType::MSG_WARNING, "the decl is null.");
        return nullptr;
    }
    return decls[0];
}

Ptr<Decl> ArkAST::GetDeclByPosition(const Codira::Position &pos) const
{
    std::vector<Codira::AST::Symbol *> syms;
    std::vector<Ptr<Decl> > decls;
    return GetDeclByPosition(pos, syms, decls);
}

std::vector<Ptr<Decl>> ArkAST::GetOverloadDecls(const ark::Token token) const
{
    std::vector<Ptr<Decl>> myVector;
    std::string funcName = token.Value();
    Position pos = token.Begin();
    std::string query = "_ = (" + std::to_string(pos.fileID) + ", "
                        + std::to_string(pos.line) + ", " + std::to_string(pos.column) + ")";
    auto posSyms = SearchContext(packageInstance->ctx, query);
    if (posSyms.empty() || funcName.empty()) {
        return myVector;
    }
    std::string curScopeName = ScopeManagerApi::GetScopeNameWithoutTail(posSyms[0]->scopeName);
    for (; !curScopeName.empty(); curScopeName = ScopeManagerApi::GetParentScopeName(curScopeName)) {
        query = "scope_name: " + curScopeName + "&& name: " + funcName;
        auto syms = SearchContext(packageInstance->ctx, query);
        for (auto it:syms) {
            if (it && !it->name.empty() && it->name == funcName) {
                Node &node = *it->node;
                auto *funcDecl = dynamic_cast<Codira::AST::FuncDecl*>(&node);
                myVector.push_back(funcDecl);
            }
        }
    }
    return myVector;
}

Ptr<Node> ArkAST::GetNodeBySymbols(const ark::ArkAST& nowAst, Ptr<Node> node, const std::vector<Symbol*>& syms,
    const std::string& query, const size_t index) const
{
    if (index >= syms.size()) {
        return node;
    }
    if (syms[index]->astKind == ASTKind::FUNC_ARG) {
        // deal named Parameter
        node = syms[index]->target;
        return node;
    }
    if (!query.empty() && node != nullptr && node->astKind == ASTKind::FUNC_PARAM) {
        // deal ENUM_DECL  like enumFunc(Set<int32>)   Hover and DocumentHlight
        auto *funcParam = dynamic_cast<FuncParam*>(node.get());
        if (funcParam != nullptr && funcParam->isIdentifierCompilerAdd) {
            std::string enumQuery = query + "&& scope_level : 0";
            auto syms1 = SearchContext(nowAst.packageInstance->ctx, enumQuery);
            if (!syms1.empty() && syms1[0] != nullptr && syms1[0]->astKind == ASTKind::ENUM_DECL) {
                node = funcParam->type.get();
            }
        }
    }
    return node;
}

std::vector<Ptr<Decl>> ArkAST::FindRealDecl(const ark::ArkAST& nowAst, const std::vector<Symbol*>& syms,
    const std::string& query, const Codira::Position& macroPos, const pair<bool, bool>& isMacroOrAlias) const
{
    Ptr<Node> node = nullptr;
    Ptr<Node> firstNode = nullptr;
    Ptr<Decl> decl = nullptr;
    bool hasFirstCompilerAdd = false;
    bool isMacro = isMacroOrAlias.first;
    bool isAlias = isMacroOrAlias.second;
    std::vector<Ptr<Decl> > decls;
    for (size_t i = 0; i < syms.size(); i++) {
        if (i == 0 && syms[i]) { firstNode = syms[i]->node; }
        // deal all right position deSugar node
        bool isSugar = !firstNode || !syms[i] || !syms[i]->node || syms[i]->node->astKind == ASTKind::FILE ||
                       hasFirstCompilerAdd && (syms[i]->node->begin != firstNode->begin ||
                                               syms[i]->node->end != firstNode->end);
        if (isSugar) { continue; }
        node = syms[i]->node;
        node = GetNodeBySymbols(nowAst, node, syms, query, i);
        auto IsAnnoRef = [](const Ptr<Node>& node) {
            if (auto refNode = DynamicCast<RefExpr*>(node.get())) {
                return refNode->ref.target && refNode->ref.target->IsFunc() &&
                        refNode->ref.target->identifier == "init" && refNode->ref.target->outerDecl &&
                        refNode->ref.target->outerDecl->TestAttr(Attribute::IS_ANNOTATION);
            }
            return false;
        };
        bool isInvalid = !node || node->TestAttr(Attribute::COMPILER_ADD) &&
                                   !node->TestAttr(Attribute::IS_CLONED_SOURCE_CODE) && !IsAnnoRef(node);
        if (isInvalid) { continue; }
        if (isAlias) {
            auto ref = dynamic_cast<NameReferenceExpr*>(node.get());
            if (ref && ref->aliasTarget) {
                (void) decls.emplace_back(ref->aliasTarget);
                return decls;
            }
        }
        hasFirstCompilerAdd = true;
        bool isDeclInMacroCall = isMacro && (node->astKind == ASTKind::MACRO_EXPAND_DECL || node->isInMacroCall) &&
                                 !node->GetMacroCallNewPos(macroPos).IsZero();
        if (isDeclInMacroCall) {
            auto pos = node->GetMacroCallNewPos(macroPos);
            std::string newQuery = "_ = (" + std::to_string(pos.fileID) + ", "
                                   + std::to_string(pos.line) + ", " + std::to_string(pos.column) + ")";
            auto newSyms = SearchContext(nowAst.packageInstance->ctx, newQuery);
            if (newSyms.empty()) {
                return decls;
            }
            decls = nowAst.FindRealDecl(nowAst, newSyms, newQuery);
            if (!decls.empty()) { return decls; }
        }
        if (i < syms.size() - 1 && node->isInMacroCall && syms[i]->name == syms[i + 1]->name && !syms[i]->target) {
            node = syms[i + 1]->node;
        }
        if (CheckInQuote(node, macroPos)) {
            return decls;
        }
        decl = nowAst.FindDeclByNode(node);
        if (decl) {
            (void) decls.emplace_back(decl);
        }
    }
    return decls;
}

Ptr<Decl> ArkAST::GetDelFromType(Ptr<const Codira::AST::Type> type) const
{
    if (type == nullptr || type->ty == nullptr) {
        return nullptr;
    }
    Ptr<Decl> decl = nullptr;
    return Meta::match(*(type->ty))(
        [&](const ClassTy &classTy) {
            decl = classTy.decl;
            return decl;
        },
        [&](const InterfaceTy &interfaceTy) {
            decl = interfaceTy.decl;
            return decl;
        },
        [&](const StructTy &structTy) {
            decl = structTy.decl;
            return decl;
        },
        [&]() { return decl; }
    );
}

Ptr<Decl> ArkAST::FindRealGenericParamDeclForExtend(
    const std::string& genericParamName, const std::vector<Codira::AST::Symbol*> syms) const
{
    for (auto &sym : syms) {
        if (sym == nullptr || sym->node == nullptr || sym->node->astKind != ASTKind::EXTEND_DECL) {
            continue;
        }
        auto *extendDecl = dynamic_cast<ExtendDecl*>(sym->node.get());
        if (extendDecl == nullptr || extendDecl->extendedType.get() == nullptr) {
            continue;
        }
        Ptr<Decl> decl = GetDelFromType(extendDecl->extendedType.get());
        if (decl == nullptr || decl->generic == nullptr || decl->generic->typeParameters.empty()) {
            continue;
        }
        for (auto &param : decl->generic->typeParameters) {
            if (param->identifier == genericParamName) {
                return param.get();
            }
        }
    }
    return nullptr;
}

bool ArkAST::CheckInQuote(Ptr<const Node> node, const Codira::Position& pos) const
{
    if (node && node->astKind == ASTKind::MACRO_EXPAND_DECL) {
        Ptr<const MacroExpandDecl> macroDecl = dynamic_cast<const MacroExpandDecl*>(node.get());
        if (!macroDecl)  {
            return false;
        }
        auto macroNamePos = macroDecl->GetIdentifierPos();
        if (macroDecl->invocation.fullNameDotPos.size()) {
            macroNamePos = macroDecl->invocation.fullNameDotPos.back();
            macroNamePos.column++;
        }
        if (macroNamePos > pos) {
            return true;
        }
        macroNamePos.column += static_cast<int>(macroDecl->identifier.Val().size()) + 1;
        if (macroNamePos < pos) {
            return true;
        }
    }
    return false;
}
} // namespace ark
