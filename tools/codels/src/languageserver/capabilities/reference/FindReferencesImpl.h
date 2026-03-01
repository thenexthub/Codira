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

#ifndef LSPSERVER_FINDREFERENCESIMPL_H
#define LSPSERVER_FINDREFERENCESIMPL_H

#include "../../../json-rpc/Protocol.h"
#include "../../ArkAST.h"
#include "../../common/Utils.h"
#include "../../index/Ref.h"
#include "../../logger/Logger.h"
#include "Codira/AST/ASTContext.h"
#include "Codira/AST/Match.h"
#include "Codira/AST/Symbol.h"
#include "Codira/Basic/DiagnosticEngine.h"
#include "Codira/Basic/Match.h"
#include "Codira/Lex/Token.h"
#include "Codira/Modules/ImportManager.h"
#include "Codira/Parse/Parser.h"
#include "Codira/Utils/FileUtil.h"

namespace ark {
struct ReferencesResult {
    std::set<Location> References{};
};

class FindReferencesImpl {
public:
    static std::string curFilePath;

    static void FindReferences(const ArkAST &ast, ReferencesResult &result, Codira::Position pos);

    static void FindFileReferences(const ArkAST &ast, ReferencesResult &result);

    static void GetCurPkgUesage(Ptr<Decl> decl, const ArkAST &ast, ReferencesResult &result);

    static std::unordered_set<std::string> GetSelectedUesScopeNames(Ptr<Decl> decl, const ArkAST &ast, Range &range);

    static void DealGenericParamDecl(
        const ArkAST &ast, ReferencesResult &result, Ptr<Decl> oldDecl, std::vector<Symbol *> &syms);

    static bool IsInvalidRef(const lsp::Ref& ref, Position pos, int curIdx, const ArkAST &ast);

    static void CompileDownStreamPackage(const std::vector<Ptr<Codira::AST::Decl>> &decls);
};
} // namespace ark

#endif // LSPSERVER_FINDREFERENCESIMPL_H
