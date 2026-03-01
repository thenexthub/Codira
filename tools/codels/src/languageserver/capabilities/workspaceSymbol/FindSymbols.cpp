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

#include "FindSymbols.h"
#include "../../CompilerCodiraProject.h"

namespace ark {
namespace lsp {

std::vector<SymbolInformation> GetWorkspaceSymbols(const std::string &query)
{
    Logger &logger = Logger::Instance();
    logger.LogMessage(MessageType::MSG_LOG, "GetWorkspaceSymbols in.");
    auto index = ark::CompilerCodiraProject::GetInstance()->GetIndex();
    std::vector<SymbolInformation> result;
    if (!index || query.empty()) {
        return result;
    }
    FuzzyFindRequest req;
    req.query = query;
    index->FuzzyFind(req, [&result](const Symbol &sym) {
        if (Options::GetInstance().IsOptionSet("test") && sym.isCodeoSym) {
            return;
        }
        if (sym.kind == ASTKind::FUNC_PARAM) {
            return;
        }
        std::string realSig = sym.signature;
        if (sym.kind == ASTKind::FUNC_DECL) {
            auto lp = sym.signature.find_first_of('(');
            auto la = sym.signature.find_first_of('<');
            std::string funcName = sym.signature.substr(0, std::min(lp, la));
            if (funcName == "init") {
                realSig = sym.name + realSig.substr(std::min(lp, la));
            }
        }
        Location loc;
        loc.uri.file = URI::URIFromAbsolutePath(sym.location.fileUri).ToString();
        loc.range = TransformFromChar2IDE({sym.location.begin, sym.location.end});
        std::string containerName = sym.scope;
        std::replace(containerName.begin(), containerName.end(), ':', '.');
        SymbolInformation info{.name = realSig,
                               .kind = ark::GetSymbolKind(sym.kind),
                               .location = loc,
                               .containerName = containerName};
        result.push_back(info);
    });
    return result;
}

} // namespace lsp
} // namespace ark
