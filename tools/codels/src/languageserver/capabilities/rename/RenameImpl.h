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

#ifndef LSPSERVER_RENAMEIMPL_H
#define LSPSERVER_RENAMEIMPL_H

#include <algorithm>
#include <vector>
#include "../../../json-rpc/Protocol.h"
#include "../../ArkAST.h"
#include "../../ArkASTWorker.h"
#include "../../common/Utils.h"
#include "../../logger/Logger.h"
#include "Codira/AST/Match.h"
#include "Codira/AST/Searcher.h"

namespace ark {
using EditMap = std::unordered_map<std::string, std::set<TextEdit>>;
struct DocumentChanges {
    EditMap defineEditMap;
    EditMap usersEditMap;
};

class RenameImpl {
public:
    static std::string Rename(const ArkAST &ast, std::vector<TextDocumentEdit> &result, Codira::Position pos,
                              const std::string &newName, Callbacks &callback);

    static void RenameByIndex(lsp::SymbolID id, DocumentChanges &documentChanges, const std::string &newName);

    static std::string GetRealName(std::vector<TextDocumentEdit> &result, Callbacks &cb,
                                   ark::DocumentChanges &documentChanges);

    static void GetLocalVarUesage(Ptr<Decl> decl, const ArkAST &ast, ark::DocumentChanges &documentChanges,
                                  const std::string &newName);

    static void UpdateDefineMap(ark::DocumentChanges &documentChanges, std::string file, TextEdit t);

    static void UpdateUserMap(ark::DocumentChanges &documentChanges, std::string file, TextEdit t);

    static void HandleGeneric(Ptr<Decl> defineDecl, const ArkAST &ast, DocumentChanges &documentChanges,
                              const std::string &newName, const std::vector<Symbol *> &syms);

    static std::string curFilePath;
};
} // namespace ark

#endif // LSPSERVER_RENAMEIMPL_H
