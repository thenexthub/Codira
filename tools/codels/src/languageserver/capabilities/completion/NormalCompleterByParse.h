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

#ifndef LSPSERVER_NORMALCOMPLETERBYPARSE_H
#define LSPSERVER_NORMALCOMPLETERBYPARSE_H

#include <cangjie/AST/ASTContext.h>
#include <string>
#include "CompletionEnv.h"
#include "CompletionImpl.h"

namespace ark {
class NormalCompleterByParse {
public:
    NormalCompleterByParse(CompletionResult &res,
        Codira::ImportManager *importManager,
        const Codira::ASTContext &ctx,
        const std::string &prefix)
        : result(res), importManager(importManager), context(&ctx), prefix(prefix)
    {
    }

    ~NormalCompleterByParse() = default;

    bool Complete(const ArkAST &input, Codira::Position pos);

    void CompletePackageSpec(const ArkAST &input, bool afterDoubleColon);

    void CompleteModuleName(const std::string &curModule, bool afterDoubleColon);

private:

    std::pair<Ptr<Decl>, Ptr<Decl>> CompleteCurrentPackages(const ArkAST &input, const Position pos,
                                                            CompletionEnv &env);

    Ptr<Decl> CompleteCurrentPackagesOnSema(const ArkAST &input, const Position pos, CompletionEnv &env);                                                            

    void FillingDeclsInPackage(const std::string &packageName,
                               CompletionEnv &env,
                               Ptr<const Codira::AST::PackageDecl> pkgDecl);

    bool DealDeclInCurrentPackage(Ptr<Decl> decl, CompletionEnv &env);

    void AddImportPkgDecl(const ArkAST &input, CompletionEnv &env);

    bool CheckCompletionInParse(Ptr<Decl> decl);

    bool CheckIfOverrideComplete(Ptr<Decl> topLevelDecl, Ptr<Decl>& decl, const Position& pos, TokenKind kind);

    void GetOverrideComplete(Ptr<Codira::AST::Decl> semaCacheDecl, const std::string& prefixContent,
                                Ptr<Decl> decl, const Position& pos);

    CompletionResult &result;

    Codira::ImportManager *importManager = nullptr;

    const Codira::ASTContext *context = nullptr;

    std::set<std::string> usedPkg = {};

    std::unordered_map<std::string, std::string> pkgAliasMap;

    std::string prefix;
};
} // namespace ark

#endif // LSPSERVER_NORMALCOMPLETERBYPARSE_H
