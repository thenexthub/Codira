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

#include "HoverImpl.h"
#include "../definition/LocateDefinition4Import.h"
#include "Codira/Lex/Token.h"

namespace {
    const int NUMBER_FOR_LINE_COMMENT = 2;       // length of "//"
    const int NUMBER_FOR_BLOCK_COMMENT = 2;      // length of "/*" or "*/"
    const int NUMBER_FOR_DOC_COMMENT = 3;        // length of "/**"
}

namespace ark {
using namespace Codira;
using namespace Codira::AST;
using namespace CONSTANTS;

std::string HoverImpl::curFilePath = "";

Decl* HoverImpl::GetRealDecl(const std::vector<Ptr<Decl>> &decls)
{
    Decl *decl = decls[0];
    if (decl->astKind == Codira::AST::ASTKind::FUNC_PARAM) {
        for (auto D : decls) {
            if (D->astKind == Codira::AST::ASTKind::VAR_DECL) {
                return D;
            }
        }
    }

    return decl;
}

void HoverImpl::TrimSpaceAndTab(std::string &s)
{
    if (s.empty()) {
        return;
    }
    int begin = 0;
    int end = static_cast<int>(s.size()) - 1;
    // resolve spaces in full-width mode
    std::string fullWidthSpace = "\u3000";
    int fullWidthSpaceLen = static_cast<int>(fullWidthSpace.size());
    bool isInvalidHead = true;
    bool isInvalidTail = true;
    while (begin <= end && (isInvalidHead || isInvalidTail)) {
        if (isInvalidHead) {
            if (begin + fullWidthSpaceLen - 1 <= end && s.substr(begin, fullWidthSpaceLen) == fullWidthSpace) {
                begin += fullWidthSpaceLen;
            } else if (s[begin] == ' ' || s[begin] == '\t' || s[begin] == '\r') {
                begin++;
            } else {
                isInvalidHead = false;
            }
        }
        if (isInvalidTail) {
            if ((end - fullWidthSpaceLen) + 1 >= begin &&
                s.substr((end - fullWidthSpaceLen) + 1, fullWidthSpaceLen) == fullWidthSpace) {
                end -= fullWidthSpaceLen;
            } else if (s[end] == ' ' || s[end] == '\t' || s[end] == '\r') {
                end--;
            } else {
                isInvalidTail = false;
            }
        }
    }
    s = begin > end ? "" : s.substr(begin, (end - begin) + 1);
}

std::string ReplaceNewlines(const std::string &input)
{
    std::string result;
    for (size_t i = 0; i < input.size(); ++i) {
        if (input[i] == '\n') {
            if (i == 0 || input[i - 1] != '\r') {
                result += "\r\n";
            } else {
                result += '\n';
            }
        } else {
            result += input[i];
        }
    }
    return result;
}

void HoverImpl::RemoveStar(const std::string& content, std::string &result)
{
    std::istringstream iss(content);
    std::string line;

    while (std::getline(iss, line)) {
        size_t starPos = line.find('*');
        if (starPos != std::string::npos) {
            line = line.substr(starPos + 1);
            if (!line.empty() && line[0] == ' ') {
                line = line.substr(1);
            }
        }

        result += line;
        if (!iss.eof() && iss.peek() != EOF) {
            result += "\n";
        }
    }
}


void HoverImpl::RemoveAboveBlank(const std::string& content, std::vector<std::string> &lines, size_t &minIndent)
{
    std::istringstream iss(content);
    std::string line;
    minIndent = std::string::npos;
    // First collect all lines and calculate the minimum indentation
    while (std::getline(iss, line)) {
        const size_t nonSpace = line.find_first_not_of(" \t\r");
        if (nonSpace != std::string::npos) {
            if (minIndent == std::string::npos || nonSpace < minIndent) {
                minIndent = nonSpace;
            }
        }
        lines.push_back(line);
    }
}

void HoverImpl::RemoveBlankAndStar(const std::string &content, std::string &result)
    {
        std::vector<std::string> lines;
        size_t minIndent;
        RemoveAboveBlank(content, lines, minIndent);

        for (size_t i = 0; i < lines.size(); ++i) {
            auto &line = lines[i];
            if (minIndent != std::string::npos && line.size() >= minIndent) {
                line = line.substr(minIndent);
            }
            // remove trailing spaces and tabs from the last line
            if (i == lines.size() - 1) {
                line.erase(line.find_last_not_of(" \t\r") + 1);
            }
            RemoveStar(line, result);
        }
    }

std::string HoverImpl::ResolveComment(const std::string &comment, const CommentKind kind)
{
    if (comment.empty()) {
        return "";
    }
    std::string retComment;
    switch (kind) {
        case CommentKind::NO_COMMENT:
            break;
        case CommentKind::LINE_COMMENT: {
            size_t pos = comment.find("//");
            if (pos != std::string::npos) {
                retComment = comment.substr(pos + NUMBER_FOR_LINE_COMMENT);
            } else {
                retComment = comment;
            }
            TrimSpaceAndTab(retComment);
            break;
        }
        case CommentKind::BLOCK_COMMENT: {
            size_t start = comment.find("/*");
            size_t end = comment.rfind("*/");
            if (start == std::string::npos || end == std::string::npos || end <= start + NUMBER_FOR_BLOCK_COMMENT) {
                retComment = comment;
                break;
            }
            std::string content =
                comment.substr(start + NUMBER_FOR_BLOCK_COMMENT, end - start - NUMBER_FOR_BLOCK_COMMENT);
            std::string result;
            RemoveBlankAndStar(content, result);
            retComment = result;
            break;
        }
        case CommentKind::DOC_COMMENT: {
            size_t start = comment.find("/**");
            size_t end = comment.rfind("*/");
            if (start == std::string::npos || end == std::string::npos || end <= start + NUMBER_FOR_DOC_COMMENT) {
                retComment = comment;
                break;
            }

            std::string content =
                comment.substr(start + NUMBER_FOR_DOC_COMMENT, end - start - NUMBER_FOR_DOC_COMMENT);
            std::string result;
            RemoveBlankAndStar(content, result);
            retComment = result;
            break;
        }
        default:
            break;
    }
    return retComment;
}

bool HoverImpl::IsAnnoAPILevel(Ptr<Annotation> anno, Ptr<ASTContext> ctx)
{
    if (ctx && ctx->curPackage && ctx->curPackage->fullPackageName == PKG_NAME_WHERE_APILEVEL_AT) {
        return anno->identifier == APILEVEL_ANNO_NAME;
    }
    if (!anno || anno->identifier != APILEVEL_ANNO_NAME) {
        return false;
    }
    auto target = anno->baseExpr ? anno->baseExpr->GetTarget() : nullptr;
    if (target && target->curFile && target->curFile->curPackage &&
        target->curFile->curPackage->fullPackageName != PKG_NAME_WHERE_APILEVEL_AT) {
        return false;
    }
    return true;
}

std::string MapToApiLevelString(const std::map<std::string, std::string> &map)
{
    std::string result;
    const std::string indent = "    ";
    result += "@!APILevel[\n";
    for (const auto &[key, value] : map) {
        if (key == "") {
            result += indent + value + ",\n";
        } else if (key == SYSCAP_IDENTGIFIER) {
            result += indent + key + ": " + "\"" + value + "\"" + "\n";
        } else {
            result += indent + key + ": " + value + ",\n";
        }
    }
    result.append("]\n");
    return result;
}

std::string HoverImpl::GetDeclApiLevelAnnoInfo(Decl &decl, const ArkAST &ast)
{
    std::map<std::string, std::string> map;
    for (auto& anno : decl.annotations) {
        if (!anno || !IsAnnoAPILevel(anno.get(), ast.packageInstance->ctx) || anno->args.size() < 1) {
            continue;
        }

        for (const auto& arg : anno->args) {
            if (!arg) {
                continue;
            }
            if (auto lce = DynamicCast<LitConstExpr>(arg->expr.get())) {
                map[arg->name.Val()] = lce->stringValue;
            }
        }
    }
    if (!map.empty()) {
        return MapToApiLevelString(map);
    }
    return "";
}

std::string HoverImpl::GetHoverMessageByOuterDecl(const Decl &node)
{
    std::string detail;
    return Meta::match(node)(
        [&detail](const Codira::AST::ClassDecl &decl) {
            detail = "// In class " + decl.identifier + CONSTANTS::BLANK;
            return detail;
        },
        [&detail](const Codira::AST::InterfaceDecl &decl) {
            detail = "// In interface " + decl.identifier + CONSTANTS::BLANK;
            return detail;
        },
        [&detail](const Codira::AST::EnumDecl &decl) {
            detail = "// In enum " + decl.identifier + CONSTANTS::BLANK;
            return detail;
        },
        [&detail](const Codira::AST::StructDecl &decl) {
            detail = "// In struct " + decl.identifier + CONSTANTS::BLANK;
            return detail;
        },
        [&detail](const Codira::AST::ExtendDecl &decl) {
            if (decl.ty == nullptr) {
                return detail;
            }
            Ptr<Decl> realDecl = ItemResolverUtil::GetDeclByTy(decl.ty);
            if (realDecl == nullptr) {
                return detail;
            }
            detail = GetHoverMessageByOuterDecl(*realDecl);
            return detail;
        },
        // ----------- Match nothing --------------------
        [&detail]() { return detail; });
}

int HoverImpl::GetHoverMessage(Ptr<Decl> decl, Hover &result, const ArkAST &ast)
{
    auto instance = CompilerCodiraProject::GetInstance();
    std::string path = instance->GetPathBySource(curFilePath, decl->identifier.Begin().fileID);
    // get source declared
    std::string source = ItemResolverUtil::ResolveSourceByNode(decl, path);
    source = (source.empty()) ? source : (source + CONSTANTS::BLANK + CONSTANTS::BLANK);
    result.markedString.push_back(source);

    // get outer message
    if (decl->outerDecl != nullptr) {
        result.markedString.push_back(GetHoverMessageByOuterDecl(*decl->outerDecl));
    }

    // get signature
    std::string detail = ItemResolverUtil::ResolveDetailByNode(*decl, ast.sourceManager);
    detail = (detail.empty()) ? detail : (detail + CONSTANTS::BLANK);
    result.markedString.push_back(detail);
    // get apiKey
    if (MessageHeaderEndOfLine::GetIsDeveco()) {
        result.markedString.push_back(GetDeclApiKey(decl));
    }
    // get Annotations
    if (!decl->annotations.empty()) {
        result.markedString.push_back(GetDeclApiLevelAnnoInfo(*decl, ast));
    }

    // get decl arkAST
    if (!decl) {
        return 0;
    }
    std::vector<std::string> comments;

    auto index = CompilerCodiraProject::GetInstance()->GetIndex();
    if (!index) {
        return 1;
    }
    auto symFromIndex = index->GetAimSymbol(*decl);
    if (symFromIndex.id == lsp::INVALID_SYMBOL_ID) {
        return 1;
    }
    index->FindComment(symFromIndex, comments);
    for (size_t i = 0; i < comments.size(); ++i) {
        CommentKind kind = GetCommentKind(comments[i]);
        const std::string comment = ReplaceNewlines(comments[i]);
        std::string resolveComment = ResolveComment(comment, kind);
        // config Vscode and DevEco(without \r)
        if (i == comments.size() - 1) {
            result.markedString.push_back(resolveComment + "\n");
        } else {
            result.markedString.push_back(resolveComment + "\r\n");
        }
    }
    return 1;
}

std::string HoverImpl::GetDeclApiKey(const Ptr<Decl> &decl)
{
    std::string apiKey = "apiKey:";
    apiKey += decl->fullPackageName + '/';
    if (decl->outerDecl) {
        apiKey += decl->outerDecl->identifier.Val();
        if (decl->outerDecl->generic) {
            apiKey += "<";
            bool firstGeneric = true;
            for (const auto &type : decl->outerDecl->generic->typeParameters) {
                if (!firstGeneric) {
                    apiKey += ", ";
                }
                apiKey += type->identifier;
                firstGeneric = false;
            }
            apiKey += ">";
        }
        if (!decl->outerDecl->identifier.Val().empty()) {
            apiKey += ".";
        }
    }
    std::string signature;
    if (auto fd = DynamicCast<FuncDecl>(decl)) {
        signature += fd->identifier.Val();
        if (!fd->funcBody) { return ""; }
        if (fd->funcBody->generic) {
            signature += "<";
            bool firstGeneric = true;
            for (const auto &type : fd->funcBody->generic->typeParameters) {
                if (!firstGeneric) {
                    signature += ", ";
                }
                signature += type->identifier;
                firstGeneric = false;
            }
            signature += ">";
        }
        signature += '(';
        bool firstTy = true;
        for (const auto &param : fd->funcBody->paramLists[0]->params) {
            if (!firstTy) {
                signature += ", ";
            }
            signature += GetString(*param->ty);
            firstTy = false;
        }
        signature += ')';
        apiKey += signature + "\r\n";
        return apiKey;
    }
    // return empty apiKey:
    return "apiKey:\r\n";
}

int HoverImpl::FindHover(const ArkAST &ast, Hover &result, Codira::Position pos)
{
    Logger &logger = Logger::Instance();
    logger.LogMessage(MessageType::MSG_LOG, "HoverImpl::FindHover in.");

    // update pos fileID
    pos.fileID = ast.fileID;

    // check current token is the kind which required in function CheckTokenKind(TokenKind)
    pos = PosFromIDE2Char(pos);
    PositionIDEToUTF8(ast.tokens, pos, *ast.file);

    // get curFilePath
    curFilePath = ast.file ? ast.file->filePath : "";
    int index = ast.GetCurTokenByPos(pos, 0, static_cast<int>(ast.tokens.size()) - 1);
    bool incorrectIndex = !ast.packageInstance || index < 0 ||
                        ast.tokens[static_cast<size_t>(index)].kind == TokenKind::MAIN;
    if (incorrectIndex) {
        logger.LogMessage(MessageType::MSG_INFO, "this token does not need to hover.");
        return -1;
    }
    if (index - 1 >= 0) {
        Token preToken = ast.tokens[static_cast<size_t>(index - 1)];
        if (NAME_TO_ANNO_KIND.count(ast.tokens[static_cast<size_t>(index)].Value())
            && preToken.kind == TokenKind::AT) {
            return -1;
        }
    }

    std::string query = "_ = (" + std::to_string(pos.fileID) + ", "
                        + std::to_string(pos.line) + ", " + std::to_string(pos.column) + ")";
    auto syms = SearchContext(ast.packageInstance->ctx, query);
    if (syms.empty()) {
        logger.LogMessage(MessageType::MSG_WARNING, "the result of search is empty.");
        return -1;
    }
    Token curToken = ast.tokens[index];
    if (auto mee = DynamicCast<MacroExpandExpr*>(syms[0]->node)) {
        bool isInvalid = mee->invocation.leftParenPos <= curToken.Begin() &&
            curToken.End() <= mee->invocation.rightParenPos;
        if (isInvalid) {
            Trace::Wlog("no hover info in macro expand expr");
            return -1;
        }
    }
    bool isMarkPosition = !syms[0] || IsMarkPos(syms[0]->node, pos);
    if (isMarkPosition) { return -1; }
    std::vector<Ptr<Decl> > decls = ast.FindRealDecl(ast, syms, query, pos, {true, false});
    bool isDeclEmpty = decls.empty() || decls[0] == nullptr;
    Ptr<Decl> decl;
    if (isDeclEmpty) {
        LocateDefinition4Import::getImportDecl(syms, ast, pos, decl);
        if (!decl) {
            logger.LogMessage(MessageType::MSG_WARNING, "the decl is null.");
            return -1;
        }
    } else {
        decl = GetRealDecl(decls);
    }
    if (decl->TestAttr(Attribute::DEFAULT, Attribute::COMPILER_ADD)) {
        query = "_ = (" + std::to_string(decl->identifier.Begin().fileID) +
            ", " + std::to_string(decl->identifier.Begin().line) +
            ", " + std::to_string(decl->identifier.Begin().column) + ")";
        auto curSyms = SearchContext(ast.packageInstance->ctx, query);
        for (const auto &sym: curSyms) {
            if (sym->name != decl->identifier.Val()) { continue; }
            if (auto realDecl = DynamicCast<Decl*>(sym->node)) {
                decl = realDecl;
            }
        }
    }
    if (IsModifierBeforeDecl(decl, curToken.Begin())) {
        logger.LogMessage(MessageType::MSG_WARNING, "this token does not need to hover");
        return -1;
    }
    Range range = {
        curToken.Begin(),
        {
            curToken.Begin().fileID,
            curToken.Begin().line,
            curToken.Begin().column + static_cast<int>(CountUnicodeCharacters(curToken.Value()))
        }
    };
    Ptr<Codira::AST::Node> node = syms[0]->node;
    SetRangForInterpolatedString(curToken, node, range);
    if (curToken.Value().substr(0, 1) == "`" && curToken.Value().substr(curToken.Value().size() - 1, 1) == "`") {
        range.start = range.start + 1;
        range.end = range.end - 1;
    }
    UpdateRange(ast.tokens, range, *node);
    result.range = TransformFromChar2IDE(range);
    int ret = GetHoverMessage(decl, result, ast);

    return ret;
}
} // namespace ark
