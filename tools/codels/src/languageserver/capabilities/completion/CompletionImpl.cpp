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

#include "CompletionImpl.h"

#include <algorithm>
#include <vector>
#include "DotCompleterByParse.h"
#include "KeywordCompleter.h"
#include "NormalCompleterByParse.h"
#include "../../common/SyscapCheck.h"
#include "Codira/AST/Node.h"

#undef THIS
using namespace Codira;
namespace {
std::vector<Codira::Token> GetLineTokens(const std::vector<Codira::Token> &inputTokens, int line)
{
    std::vector<Codira::Token> res;
    for (auto &token : inputTokens) {
        if (token.Begin().line == line) {
            res.push_back(token);
        }
    }
    return res;
}

// For case: import std.xxx.{}
bool IsMultiImport(const std::vector<Codira::Token> &inputTokens)
{
    if (inputTokens.empty()) {
        return false;
    }
    if (inputTokens.front().kind == Codira::TokenKind::IMPORT) {
        auto preTokenKind = Codira::TokenKind::INIT;
        for (auto &token : inputTokens) {
            if (token.kind == Codira::TokenKind::LCURL && preTokenKind == Codira::TokenKind::DOT) {
                return true;
            }
            preTokenKind = token.kind;
        }
    }
    return false;
}

std::string GetMultiImportPrefix(const std::vector<Codira::Token> &inputTokens)
{
    std::string prefix;
    std::vector<std::string> items;
    for (size_t i = 0; inputTokens[i].kind != Codira::TokenKind::LCURL; ++i) {
        if (inputTokens[i].kind == Codira::TokenKind::IMPORT) {
            continue;
        }
        if (inputTokens[i].kind == Codira::TokenKind::IDENTIFIER) {
            items.push_back(inputTokens[i].Value());
        }
    }

    if (items.empty()) {
        return {};
    }

    prefix = items[0];
    for (size_t i = 1; i < items.size(); ++i) {
        prefix += "." + items[i];
    }
    return prefix;
}

bool InterpolationString(
    const ark::ArkAST &input, ark::DotCompleterByParse &dotCompleter, Codira::Position &pos, std::string &prefix)
{
    if (prefix.size() > 1 && prefix.find('.') != std::string::npos) {
        auto found = prefix.find_last_of('.');
        auto chainedName = prefix.substr(0, found);
        if (prefix.back() == '.') {
            prefix = ".";
        } else {
            pos.column = pos.column - static_cast<int>((prefix.size() - (found + 1)));
            prefix = prefix.substr(found + 1, (prefix.size() - (found - 1)));
        }
        dotCompleter.Complete(input, pos, chainedName);
        return true;
    }
    return false;
}

std::string GetInterpolationPrefix(const ark::ArkAST &input, const StringPart &str, Position &pos)
{
    std::string interpolation = "${";
    size_t size = interpolation.size();
    Position exprPos = {str.begin.line, str.begin.column + static_cast<int>(interpolation.size())};
    std::string value = str.value.substr(size, str.value.length() - size - 1);
    Lexer lexer = Lexer(value, input.packageInstance->diag, *input.sourceManager, exprPos);
    auto newTokens = lexer.GetTokens();
    std::vector<Token> tokenV;
    for (size_t i = 0; i < newTokens.size(); i ++) {
        auto nt = newTokens[i];
        bool isValidTy = nt.kind == TokenKind::IDENTIFIER || nt.kind == TokenKind::DOT;
        if (isValidTy) {
            tokenV.push_back(nt);
        } else {
            tokenV.clear();
        }
        if (nt.Begin() == pos && nt.kind == TokenKind::DOT) {
            pos.column = pos.column + 1;
            break;
        }
        if (nt.End() == pos) {
            if (i + 1 < newTokens.size() - 1 && newTokens[i + 1].kind == TokenKind::DOT) {
                tokenV.push_back(newTokens[i + 1]);
                pos.column = pos.column + 1;
            }
            break;
        }
    }
    std::string newPrefix;
    std::for_each(tokenV.begin(), tokenV.end(), [&newPrefix](Token t) { newPrefix += t.Value(); });
    std::cerr << "newPrefix is " << newPrefix << std::endl;
    return newPrefix;
}
}

namespace ark {
Token CompletionImpl::curToken = Token(TokenKind::INIT);
bool CompletionImpl::needImport = false;
std::unordered_set<std::string> CompletionImpl::externalImportSym = {"ohos.ark_interop_macro:Interop"};

CompletionItem CodeCompletion::Render(const std::string &sortText, const std::string &prefix) const
{
    CompletionItem item;
    item.label = label;
    item.kind = kind;
    item.detail = detail;
    item.insertText = insertText;
    item.insertTextFormat = InsertTextFormat::SNIPPET;
    if (item.kind == CompletionItemKind::CIK_MODULE || item.insertText == name || item.insertText == name + "()") {
        item.insertTextFormat = InsertTextFormat::PLAIN_TEXT;
    }
    item.filterText = GetFilterText(name, prefix);
    item.additionalTextEdits = additionalTextEdits;
    item.deprecated = deprecated;
    item.sortText = sortText;
    return item;
}

void CompletionImpl::CodeComplete(const ArkAST &input,
                                  Codira::Position pos,
                                  CompletionResult &result,
                                  std::string &prefix)
{
    Trace::Log("CodeComplete in.");

    // update pos fileID
    pos.fileID = input.fileID;
    // adjust position from IDE to AST
    pos = PosFromIDE2Char(pos);
    // set value for whether to execute import package
    needImport = ark::InImportSpec(*input.file, pos);
    PositionIDEToUTF8ForC(input, pos);

    auto index = input.GetCurToken(pos, 0, static_cast<int>(input.tokens.size()) - 1);
    if (index == -1) {
        return;
    }

    // Get completion prefix
    Token tok = input.tokens[static_cast<unsigned int>(index)];
    curToken = tok;
    prefix = tok.Value().substr(0, static_cast<unsigned int>(pos.column - tok.Begin().column));
    // For STRING_LITERAL, caculate accurate completino prefix.
    // If cursor isn't in interpolation, no code completion support.
    if (tok.kind == TokenKind::STRING_LITERAL || tok.kind == TokenKind::MULTILINE_STRING) {
        Lexer lexer = Lexer(input.tokens, input.packageInstance->diag, *input.sourceManager);
        auto stringParts = lexer.GetStrParts(tok);
        bool isInExpr = false;
        for (const auto &str : stringParts) {
            if (str.strKind == Codira::StringPart::EXPR) {
                if (str.begin <= pos && pos <= str.end) {
                    isInExpr = true;
                    prefix = GetInterpolationPrefix(input, str, pos);
                    break;
                }
            }
        }
        if (!isInExpr) {
            return;
        }
    }

    auto curLineTokens = GetLineTokens(input.tokens, pos.line);
    if (IsMultiImport(curLineTokens)) {
        prefix = '.';
    }

    // If input.semaCache is not nullptr, it's parse complete
    if (input.semaCache && input.semaCache->packageInstance && input.semaCache->packageInstance->ctx) {
        FasterComplete(input, pos, result, index, prefix);
    }
}

void CompletionImpl::FasterComplete(
    const ArkAST &input, Codira::Position pos, CompletionResult &result, int index, std::string &prefix)
{
    if (!input.packageInstance) { return; }
    auto &importManager =
        needImport ? input.packageInstance->importManager : input.semaCache->packageInstance->importManager;
    DotCompleterByParse dotCompleter(*(input.semaCache->packageInstance->ctx), result, importManager,
        input.semaCache->file->filePath);

    // Completion action implement by DotCompleter.
    // Interpolation String: "${obja.b}"
    if (InterpolationString(input, dotCompleter, pos, prefix)) {
        return;
    }

    // Named parameter completion
    // Only attempt when not in dot completion mode (i.e., prefix is not "." and previous token is not DOT)
    bool isDotContext = (prefix == ".") ||
                        (index > 0 && input.tokens[static_cast<unsigned int>(index - 1)].kind == Codira::TokenKind::DOT);

    if (!isDotContext) {
        size_t originalSize = result.completions.size();
        NamedParameterComplete(input, pos, result, index, prefix);

        // If named parameter completions were successfully generated, return directly without proceeding
        // to normal completion, giving higher priority to named parameters
        if (result.completions.size() > originalSize) {
            return;
        }
    }

    int firstTokIdxInLine = GetFirstTokenOnCurLine(input.tokens, pos.line);
    Token prevTokOfPrefix = Token(TokenKind::INIT);
    Token firstTokInLine = Token(TokenKind::INIT);
    if (firstTokIdxInLine != -1) {
        firstTokInLine = input.tokens[firstTokIdxInLine];
    }
    if (index > 0) {
        prevTokOfPrefix = input.tokens[static_cast<unsigned int>(index - 1)];
    }
    int dotIndex = GetDotIndexPos(input.tokens, index, firstTokIdxInLine);
    if (dotIndex != -1 && IsPreambleComplete(input, firstTokInLine.kind, firstTokIdxInLine)) {
        if (prevTokOfPrefix.kind == TokenKind::AS) { return; }
        auto beforeKind = prevTokOfPrefix.kind != TokenKind::COMMA && prevTokOfPrefix.kind != TokenKind::LCURL &&
                          prevTokOfPrefix.kind != TokenKind::DOT && prefix != ".";
        if (beforeKind) {
            CodeCompletion asToken;
            asToken.name = asToken.label = asToken.insertText = "as";
            asToken.kind = CompletionItemKind::CIK_KEYWORD;
            result.completions.push_back(asToken);
            return;
        }
    }

    // If the prevToken is INTEGER_LITERAL or FLOAT_LITERAL, no completion is provided.
    // If a letter is entered after the dot, completion is provided.
    // let a = 100.     <--  no completion provided
    //            ^
    // let a = 100.p    <--  completion provided
    //             ^
    if (Utils::In(prevTokOfPrefix.kind, {TokenKind::INTEGER_LITERAL, TokenKind::FLOAT_LITERAL})) {
        return;
    }

    // Import Spec: import pkg.[item]
    // Fully qualified Type: xxx.[item]
    if (prefix == ".") {
        auto chainedName = GetChainedName(input, pos, index, firstTokIdxInLine);
        dotCompleter.Complete(input, pos, chainedName);
        return;
    }

    // Import Spec: import pkg.v[item]
    // Fully qualified Type: xxx.v[item]
    if (prevTokOfPrefix.kind == TokenKind::DOT) {
        if (index > 1) {
            pos.column = prevTokOfPrefix.Begin().column + 1;
            pos.line = prevTokOfPrefix.Begin().line;
            auto chainedName = GetChainedNameComplex(input, firstTokIdxInLine, index - 2);
            dotCompleter.Complete(input, pos, chainedName);
            return;
        }
    }

    // For word wrap dot completion
    // class A {
    //     func foo() {}
    // }
    // eg: let a = A().\n <- this is Token NL
    //     fo <- this is completion position
    int offset = 2; // Offset between prefix and dot
    if (prevTokOfPrefix.kind == TokenKind::NL && index > offset) {
        size_t prevTokenIdxOfNL = static_cast<unsigned int>(index - offset);
        Token prevTokenOfNL = input.tokens[prevTokenIdxOfNL];
        if (prevTokenOfNL.kind == TokenKind::DOT) {
            pos.column = prevTokenOfNL.Begin().column + 1;
            pos.line = prevTokenOfNL.Begin().line;
            auto chainedName = GetChainedNameComplex(input, firstTokIdxInLine, index - offset);
            dotCompleter.Complete(input, pos, chainedName);
            return;
        }
    }

    // Completion action implement by NormalCompleter.
    NormalParseImpl(input, pos, result, index, prefix);
}

std::string CompletionImpl::GetChainedName(const ArkAST &input, const Codira::Position &pos,
                                            int index, int firstTokIdxInLine)
{
    int possibleChainedBegin = GetChainedPossibleBegin(input, firstTokIdxInLine);
    auto chainedName = GetChainedNameComplex(input, possibleChainedBegin, index - 1);
    auto curLineTokens = GetLineTokens(input.tokens, pos.line);
    if (IsMultiImport(curLineTokens)) {
        chainedName = chainedName.empty() ? GetMultiImportPrefix(curLineTokens)
                                            : GetMultiImportPrefix(curLineTokens) + CONSTANTS::DOT + chainedName;
    }
    return chainedName;
}

void CompletionImpl::AutoImportPackageComplete(const ArkAST &input, CompletionResult &result, const std::string &prefix)
{
    auto index = CompilerCodiraProject::GetInstance()->GetIndex();
    if (!index) {
        return;
    }
    if (!input.file || !input.file->package || !input.file->curPackage) {
        return;
    }
    auto pkgName = input.file->curPackage->fullPackageName;
    // get import's pos
    int lastImportLine = 0;
    for (const auto &import : input.file->imports) {
        if (!import) {
            continue;
        }
        lastImportLine = std::max(import->content.rightCurlPos.line, std::max(import->importPos.line, lastImportLine));
    }
    Position pkgPos = input.file->package->packagePos;
    if (lastImportLine == 0 && pkgPos.line > 0) {
        lastImportLine = pkgPos.line;
    }
    Position textEditStart = {input.fileID, lastImportLine, 0};
    Range textEditRange{textEditStart, textEditStart};
    auto curModule = SplitFullPackage(input.file->curPackage->fullPackageName).first;
    SyscapCheck syscap(curModule);
    index->FindImportSymsOnCompletion(std::make_pair(result.normalCompleteSymID, result.importDeclsSymID),
        pkgName, curModule, prefix,
        [&result, &textEditRange, &syscap](const std::string &pkg,
            const lsp::Symbol &sym, const lsp::CompletionItem &completionItem) {
            if (!sym.syscap.empty() && !syscap.CheckSysCap(sym.syscap)) {
                return;
            }
            CodeCompletion completion;
            std::string fullSymName = pkg + ":" + sym.name;
            if (externalImportSym.count(fullSymName)) {
                CompletionImpl::HandleExternalSymAutoImport(result, pkg, sym, completionItem, textEditRange);
                return;
            }
            auto astKind = sym.kind;
            completion.deprecated = sym.isDeprecated;
            completion.kind = ItemResolverUtil::ResolveKindByASTKind(astKind);
            completion.name = sym.name;
            completion.label = completionItem.label;
            completion.insertText = completionItem.insertText;
            completion.detail = "import " + pkg;
            TextEdit textEdit;
            textEdit.range = textEditRange;
            textEdit.newText = "import " + pkg + "." + sym.name + "\n";
            completion.additionalTextEdits = std::vector<TextEdit>{textEdit};
            completion.sortType = SortType::AUTO_IMPORT_SYM;
            result.completions.push_back(completion);
        });
}

void CompletionImpl::GenerateNamedArgumentCompletion(ark::CompletionResult &result, const std::string &prefix, std::unordered_set<std::string> usedNamedParams, int positionalsUsed, std::unordered_set<std::string> suggestedParamNames, const std::vector<OwnedPtr<FuncParamList>> &paramLists, int paramIndex)
{
    for (const auto &paramList : paramLists) {
        if (!paramList) {
            continue;
        }

        for (const auto &param : paramList->params) {
            if (!param) {
                paramIndex++;
                continue;
            }

            std::string paramName = param->identifier;

            // 1. Check '!' (named parameter definition flag)
            bool isNamedParamDef = (param->notMarkPos.line != 0 || param->notMarkPos.column != 0);

            // 2. Check if already satisfied by positional parameters (only for non-named parameters)
            if (!isNamedParamDef && paramIndex < positionalsUsed) {
                // If it's a non-named parameter (no !) and has been consumed by previous positional parameters, skip
                paramIndex++;
                continue;
            }

            // 3. Filter out already used as named parameters or already suggested by other overloads
            if (usedNamedParams.count(paramName) || suggestedParamNames.count(paramName)) {
                paramIndex++;
                continue;
            }

            // 4. Filter by prefix
            if (paramName.size() < prefix.size() || paramName.find(prefix) != 0) {
                paramIndex++;
                continue;
            }

            // 5. Construct completion item (and record suggested names to prevent duplicate additions from other overloads)
            suggestedParamNames.insert(paramName);

            ark::CodeCompletion completion;
            completion.name = paramName;
            completion.label = paramName;
            completion.kind = ark::CompletionItemKind::CIK_VARIABLE;
            completion.detail = "Named Argument";
            completion.insertText = paramName + ": ";
            completion.sortType=SortType::KEYWORD;

            if (isNamedParamDef) {
                result.completions.push_back(completion);
            }
            paramIndex++;
        }
    }
}

void CompletionImpl::NamedParameterComplete(const ark::ArkAST &input, const Codira::Position &pos,
                                            ark::CompletionResult &result, int index, const std::string &prefix)
{
    int lparenIndex = -1;
    if (!CheckNamedParameter(input, index, lparenIndex)) {
        return;
    }

    // 3. Get all overloads of the function
    int funcNameIndex = lparenIndex - 1;
    ark::Token funcToken = input.tokens[funcNameIndex];

    std::vector<Codira::AST::FuncDecl*> funcDeclsWithOverride;

    auto decls = input.semaCache->GetOverloadDecls(funcToken);
    for (auto &decl : decls) {
        auto *funcDecl = dynamic_cast<Codira::AST::FuncDecl*>(decl.get());
        if (funcDecl) {
            funcDeclsWithOverride.push_back(funcDecl);
        }
    }

    if (funcDeclsWithOverride.empty()) {
        return;
    }

    // 4. Analyze current parameter input status
    // 4.1 Collect already used named parameters
    std::unordered_set<std::string> usedNamedParams;
    for (int i = lparenIndex + 1; i < index; ++i) {
        // Heuristic rule: Identifier followed immediately by Colon (:)
        if (input.tokens[i].kind == Codira::TokenKind::IDENTIFIER &&
            (i + 1 < input.tokens.size()) &&
            input.tokens[i+1].kind == Codira::TokenKind::COLON) {
            usedNamedParams.insert(input.tokens[i].Value());
        }
    }

    // 4.2 Count consumed positional arguments
    int positionalsUsed = 0;
    for (int i = lparenIndex + 1; i < index; ++i) {
        if (input.tokens[i].kind == Codira::TokenKind::COMMA) {
            positionalsUsed++;
        }
    }
    // If the cursor is after the left parenthesis and the token before the cursor is not a comma,
    // then the currently typed argument also counts as a consumed position.
    if (index > lparenIndex + 1 && input.tokens[index - 1].kind != Codira::TokenKind::COMMA) {
        positionalsUsed++;
    }

    // Used for deduplication across overloads
    std::unordered_set<std::string> suggestedParamNames;

    // 5. Iterate through all function overloads and their parameters
    for (auto *funcDecl : funcDeclsWithOverride) {
        if (!funcDecl->funcBody) {
            continue;
        }

        // Calculate the total number of parameters for the current overload
        const auto &paramLists = funcDecl->funcBody->paramLists;
        int totalParamCount = 0;
        for (const auto &pList : paramLists) {
            if (pList) {
                totalParamCount += pList->params.size();
            }
        }

        // If the number of consumed positional arguments exceeds the total number of parameters
        // in this overload, this overload does not match, skip it.
        if (totalParamCount < positionalsUsed) {
            continue;
        }

        int paramIndex = 0;

        GenerateNamedArgumentCompletion(result, prefix, usedNamedParams, positionalsUsed, suggestedParamNames, paramLists, paramIndex);
    }
}

bool CompletionImpl::CheckNamedParameter(const ark::ArkAST &input, const int index, int &lparenIndex)
{
    // 1. Must have semantic cache
    if (!input.semaCache) {
        return false;
    }

    // 2. Backtrack to find the current function call's left parenthesis '('
    int balance = 0;

    // Start scanning from the previous token of the current token
    int startIndex = index - 1;
    if (startIndex < 0) {
        return false;
    }

    for (int i = startIndex; i >= 0; --i) {
        auto kind = input.tokens[i].kind;
        if (kind == Codira::TokenKind::RPAREN || kind == Codira::TokenKind::RSQUARE || kind == Codira::TokenKind::RCURL) {
            balance++;
        } else if (kind == Codira::TokenKind::LPAREN || kind == Codira::TokenKind::LSQUARE || kind == Codira::TokenKind::LCURL) {
            if (balance > 0) {
                balance--;
            } else if (kind == Codira::TokenKind::LPAREN) {
                // Found the left parenthesis of the current level
                lparenIndex = i;
                break;
            } else {
                // Found other opening brackets, stop
                return false;
            }
        } else if (kind == Codira::TokenKind::SEMI) {
            // Encountered semicolon, exceeded function call scope
            return false;
        }
    }

    if (lparenIndex <= 0) {
        return false; // Not found or left parenthesis is the first token
    }
    return true;
}

void CompletionImpl::HandleExternalSymAutoImport(CompletionResult &result, const std::string &pkg,
    const lsp::Symbol &sym, const lsp::CompletionItem &completionItem, Range textEditRange)
{
    if (sym.name == "Interop" && pkg == "ohos.ark_interop_macro") {
        CodeCompletion completion;
        auto astKind = sym.kind;
        completion.deprecated = sym.isDeprecated;
        completion.kind = ItemResolverUtil::ResolveKindByASTKind(astKind);
        completion.name = sym.name;
        completion.label = completionItem.label;
        completion.insertText = completionItem.insertText;
        completion.detail = "import " + pkg;
        ark::TextEdit textEdit;
        textEdit.range = textEditRange;
        textEdit.newText = "import ohos.ark_interop_macro.*\nimport ohos.ark_interop.*\n";
        completion.additionalTextEdits = std::vector<TextEdit>{textEdit};
        completion.sortType = SortType::AUTO_IMPORT_SYM;
        result.completions.push_back(completion);
        return;
    }
}

void CompletionImpl::NormalParseImpl(
    const ArkAST &input, const Codira::Position &pos, CompletionResult &result, int index, std::string &prefix)
{
    TokenKind beforePrefixKind = TokenKind::INIT;
    if (index > 0) {
        beforePrefixKind = input.tokens[static_cast<unsigned int>(index - 1)].kind;
    }

    bool afterDoubleColon = false;
    // if package name has org name, check beforePrefixKind and change token position
    if (beforePrefixKind == TokenKind::DOUBLE_COLON && index >= 3) { // package org :: is three tokens
        beforePrefixKind = input.tokens[static_cast<unsigned int>(index - 3)].kind; // package org :: is three tokens
        afterDoubleColon = true;
    }

    auto &importManager =
        needImport ? input.packageInstance->importManager : input.semaCache->packageInstance->importManager;
    NormalCompleterByParse normalCompleter(result, &importManager, *(input.semaCache->packageInstance->ctx), prefix);

    // Complete package name.
    // package [name]
    // macro package [name]
    if (beforePrefixKind == TokenKind::PACKAGE) {
        normalCompleter.CompletePackageSpec(input, afterDoubleColon);
        return;
    }

    // NormalComplete only need supply module name in Import Spec.
    // modifier? import [module]
    // modifier? import {[module] }
    // modifier? import {std.collection.ArrayList, [module]}
    if (IsPreamble(input, pos)) {
        auto curModule = SplitFullPackage(input.file->curPackage->fullPackageName).first;
        afterDoubleColon = afterDoubleColon | IsImportHasOrg(input, pos);
        normalCompleter.CompleteModuleName(curModule, afterDoubleColon);
        return;
    }

    // recognize typing identifier[just same as keyword] of toplevel like decls which shouldn't to be completed
    // e.g. func is, class case, enum in, ...
    // expect cases like : func param decl, enum constructor
    if (KeywordCompleter::keyWordKinds.count(curToken.kind) &&
        KeywordCompleter::declKeyWordKinds.count(FindPreFirstValidTokenKind(input, index))) {
        return;
    }

    // No code completion needed in identifier of Decl.
    if (!normalCompleter.Complete(input, pos)) {
        return;
    }

    // Complete all keywords and build-in snippets.
    KeywordCompleter::Complete(result);

    if (Options::GetInstance().IsOptionSet("disableAutoImport")) {
        return;
    }

    AutoImportPackageComplete(input, result, prefix);
}

int CompletionImpl::GetChainedPossibleBegin(const ArkAST &input, int firstTokIdxInLine)
{
    if (firstTokIdxInLine >= input.tokens.size() || firstTokIdxInLine < 0) {
        return firstTokIdxInLine;
    }
    if (input.tokens[firstTokIdxInLine].kind != TokenKind::DOT) {
        return firstTokIdxInLine;
    }
    int qualifyPreTokenIdx = firstTokIdxInLine - 1;
    if (qualifyPreTokenIdx < 0) {
        return firstTokIdxInLine;
    }
    int preLineFirstIdx = GetFirstTokenOnCurLine(input.tokens, input.tokens[qualifyPreTokenIdx].Begin().line);
    if (preLineFirstIdx == -1) {
        return firstTokIdxInLine;
    }
    Token preLineFirstToken = input.tokens[preLineFirstIdx];
    while (preLineFirstIdx >= 0 && preLineFirstIdx - 1 >= 0
        && (input.tokens[preLineFirstIdx - 1].kind == TokenKind::NL
            || preLineFirstToken.kind == TokenKind::DOT)) {
        preLineFirstIdx--;
        preLineFirstIdx = GetFirstTokenOnCurLine(input.tokens, input.tokens[preLineFirstIdx].Begin().line);
        if (preLineFirstIdx == -1) {
            break;
        }
        preLineFirstToken = input.tokens[preLineFirstIdx];
    }
    int begin = GetFirstTokenOnCurLine(input.tokens, input.tokens[preLineFirstIdx].Begin().line);
    if (begin == -1) {
        return firstTokIdxInLine;
    }
    return begin;
}

std::string CompletionImpl::GetChainedNameComplex(const ArkAST &input, int start, int end)
{
    bool isInvalid = start > end || start < 0 || static_cast<unsigned int>(end) >= input.tokens.size();
    if (isInvalid) {
        return "";
    }

    std::string chainedName;
    std::stack<TokenKind> quoteStack = {};
    std::set<TokenKind> identifier = {TokenKind::IDENTIFIER, TokenKind::SUPER, TokenKind::THIS, TokenKind::QUEST,
                                      TokenKind::PUBLIC, TokenKind::PRIVATE, TokenKind::PROTECTED, TokenKind::OVERRIDE,
                                      TokenKind::ABSTRACT, TokenKind::OPEN, TokenKind::REDEF, TokenKind::SEALED};
    std::set<TokenKind> leftPunct = {TokenKind::LSQUARE, TokenKind::LT, TokenKind::LPAREN};
    std::set<TokenKind> rightPunct = {TokenKind::RSQUARE, TokenKind::GT, TokenKind::RPAREN};
    std::map<TokenKind, TokenKind> quoteMap = {{TokenKind::RPAREN, TokenKind::LPAREN},
                                               {TokenKind::GT, TokenKind::LT},
                                               {TokenKind::RSQUARE, TokenKind::LSQUARE}};
    size_t i;
    size_t zeroPos = 0;
    bool hasDot = true;
    bool skipQuest = false;
    for (i = static_cast<unsigned int>(end); i >= static_cast<unsigned int>(start); --i) {
        if (input.tokens[i].kind == TokenKind::NL) {
            continue;
        }
        if (skipQuest) {
            skipQuest = false;
            continue;
        }
        if (rightPunct.find(input.tokens[i].kind) != rightPunct.end()) {
            quoteStack.push(input.tokens[i].kind);
        } else if (leftPunct.find(input.tokens[i].kind) != leftPunct.end()) {
            if (quoteStack.empty() || quoteMap[quoteStack.top()] != input.tokens[i].kind) {
                continue;
            }
            quoteStack.pop();
            continue;
        }
        if (!quoteStack.empty()) {
            continue;
        }
        if (!hasDot && (input.tokens[i].kind == TokenKind::DOT || input.tokens[i].kind == TokenKind::DOUBLE_COLON)) {
            hasDot = true;
            (void)chainedName.insert(zeroPos, input.tokens[i].Value());
        } else if (hasDot && identifier.find(input.tokens[i].kind) != identifier.end()) {
            (void)chainedName.insert(zeroPos, input.tokens[i].Value());
            if (input.tokens[i].kind == TokenKind::QUEST && i - 1 >= 0) {
                (void)chainedName.insert(zeroPos, input.tokens[i - 1].Value());
                skipQuest = true;
            }
            hasDot = false;
        } else {
            break;
        }
    }
    if (!quoteStack.empty()) {
        return "";
    }
    return chainedName;
}

bool CompletionImpl::IsPreambleComplete(const ArkAST &input,
                                        const TokenKind firstTokenKind,
                                        const int firstTokenIndexOnLine)
{
    bool keywordFlag = firstTokenKind == TokenKind::IMPORT || firstTokenKind == TokenKind::PACKAGE;
    if (keywordFlag) {
        return true;
    }
    if (firstTokenKind == TokenKind::PUBLIC &&
        static_cast<unsigned int>(firstTokenIndexOnLine + 1) < input.tokens.size() &&
        input.tokens[firstTokenIndexOnLine + 1].kind == TokenKind::IMPORT) {
        return true;
    }
    return false;
}

bool CompletionImpl::IsPreamble(const ArkAST &input, Codira::Position pos)
{
    for (const auto &im : input.file->imports) {
        if (pos <= im->end) {
            return true;
        }
    }
    return false;
}

bool CompletionImpl::IsImportHasOrg(const ArkAST &input, Codira::Position pos)
{
    for (const auto &im : input.file->imports) {
        if (pos <= im->end) {
            return im->content.kind == Codira::AST::ImportKind::IMPORT_MULTI
                && im->content.hasDoubleColon && pos > im->content.leftCurlPos;
        }
    }
    return false;
}
} // namespace ark
