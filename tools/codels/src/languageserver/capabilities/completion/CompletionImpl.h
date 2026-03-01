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

#ifndef LSPSERVER_COMPLETION_H
#define LSPSERVER_COMPLETION_H

#include <cstdint>
#include <iostream>
#include <vector>
#include "../../../json-rpc/Protocol.h"
#include "../../../json-rpc/CompletionType.h"
#include "../../common/Utils.h"
#include "../../logger/Logger.h"
#include "Codira/Lex/Token.h"
#include "Codira/Basic/DiagnosticEngine.h"
#include "Codira/Parse/Parser.h"
#include "Codira/AST/ASTContext.h"
#include "Codira/Basic/Match.h"
#include "Codira/AST/Match.h"
#include "Codira/AST/Symbol.h"
#include "Codira/Utils/FileUtil.h"
#include "Codira/Modules/ImportManager.h"
#include "../../ArkAST.h"

namespace ark {

enum class SortType : uint8_t {
    NORMAL_SYM,
    AUTO_IMPORT_SYM,
    KEYWORD,
};

struct CodeCompletion {
    bool show = true;
    bool isEnumCtor = false; // Special for enum constructor in different match expr
    bool deprecated = false;

    SortType sortType = SortType::NORMAL_SYM; // for completion list sort
    CompletionItemKind kind = CompletionItemKind::CIK_MISSING;
    std::string name;
    std::string label;
    std::string detail;
    std::string insertText;
    std::string container;
    uint8_t itemDepth = 0;
    std::optional<std::vector<TextEdit>> additionalTextEdits;
    ark::lsp::SymbolID id = 0;

    [[nodiscard]] CompletionItem Render(const std::string &sortText, const std::string &prefix) const;
};

struct CompletionResult {
    std::vector<CodeCompletion> completions {};
    uint8_t cursorDepth = 0;
    std::unordered_set<ark::lsp::SymbolID> normalCompleteSymID {};
    std::unordered_set<ark::lsp::SymbolID> importDeclsSymID {};
};

class CompletionImpl {
public:
    static void CodeComplete(const ArkAST &input, Codira::Position pos,
                             CompletionResult &result, std::string &prefix);

    static int GetChainedPossibleBegin(const ArkAST &input, int firstTokIdxInLine);

    static std::string GetChainedNameComplex(const ArkAST &input, int start, int end);

    static bool IsPreambleComplete(const ArkAST &input, const TokenKind firstTokenKind,
                                   const int firstTokenIndexOnLine);

    static bool IsPreamble(const ArkAST &input, Codira::Position pos);

    static bool IsImportHasOrg(const ArkAST &input, Codira::Position pos);

    static Token curToken;

    static bool needImport;

    static std::unordered_set<std::string> externalImportSym;

private:
    static void NamedParameterComplete(const ark::ArkAST &input, const Codira::Position &pos,
                            ark::CompletionResult &result, int index, const std::string &prefix);

    static void FasterComplete(const ArkAST &input, Codira::Position pos,
                              CompletionResult &result, int index, std::string &prefix);

    static void NormalParseImpl(const ArkAST &input, const Codira::Position &pos,
                                CompletionResult &result, int index, std::string &prefix);

    static void AutoImportPackageComplete(const ArkAST &input, CompletionResult &result, const std::string &prefix);

    static void GenerateNamedArgumentCompletion(ark::CompletionResult &result, const std::string &prefix,
                        std::unordered_set<std::string> usedNamedParams, int positionalsUsed,
                        std::unordered_set<std::string> suggestedParamNames, const std::vector<OwnedPtr<FuncParamList>> &paramLists,
                        int paramIndex);

    static void HandleExternalSymAutoImport(CompletionResult &result, const std::string &pkg, const lsp::Symbol &sym,
                                            const lsp::CompletionItem &completionItem, Range textEditRange);

    static std::string GetChainedName(const ArkAST &input, const Codira::Position &pos, int index, int firstTokIdxInLine);

    static bool CheckNamedParameter(const ark::ArkAST &input, const int index, int &lparenIndex);
};
} // namespace ark

#endif // LSPSERVER_COMPLETION_H
