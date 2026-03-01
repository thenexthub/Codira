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

#ifndef LSPSERVER_ARKAST_H
#define LSPSERVER_ARKAST_H

#include <cstdint>
#include <sstream>
#include "../json-rpc/Protocol.h"
#include "../json-rpc/URI.h"
#include "Codira/AST/ASTContext.h"
#include "Codira/AST/Match.h"
#include "Codira/AST/Symbol.h"
#include "Codira/Basic/DiagnosticEngine.h"
#include "Codira/Basic/Match.h"
#include "Codira/Lex/Token.h"
#include "Codira/Modules/ImportManager.h"
#include "Codira/Parse/Parser.h"
#include "Codira/Utils/FileUtil.h"


#include "DocCache.h"

namespace ark {
using namespace Codira;
using namespace Codira::AST;
struct ParseInputs {
    std::string fileName;
    std::string contents;
    std::int64_t version = 0;
    bool forceRebuild = false;

    ParseInputs(std::string fileName, std::string ctent, std::int64_t v, bool forceRebuild = false)
        : fileName(std::move(fileName)), contents(std::move(ctent)), version(v),
          forceRebuild(forceRebuild)
    {
    }

    ParseInputs() {}
    ParseInputs &operator=(const ParseInputs &input)
    {
        if (this == &input) {
            return *this;
        }

        this->fileName = input.fileName;
        this->contents = input.contents;
        this->version = input.version;
        this->forceRebuild = input.forceRebuild;
        return *this;
    }

    ParseInputs(const ParseInputs &input)
    {
        this->fileName = input.fileName;
        this->contents = input.contents;
        this->version = input.version;
        this->forceRebuild = input.forceRebuild;
    }
};

struct PackageInstance {
    PackageInstance(Codira::DiagnosticEngine &d, Codira::ImportManager &imp)
        : package(nullptr), diag(d), importManager(imp), ctx(nullptr)
    {
    }

    ~PackageInstance() {}

    Ptr<const Codira::AST::Package> package;
    Codira::DiagnosticEngine &diag;
    Codira::ImportManager &importManager;
    Codira::ASTContext *ctx;
};

struct ArkAST {
    ArkAST(const std::pair<std::string, std::string> &paths,
           Ptr<const File> node,
           Codira::DiagnosticEngine &diagEngine,
           PackageInstance *pkgInstance,
           Codira::SourceManager *sm)
        : diag(diagEngine), file(node), packageInstance(pkgInstance), sourceManager(sm)
    {
        DoLexer(paths.second, paths.first);
    }

    ~ArkAST() {}

    int GetCurTokenByPos(const Codira::Position &pos,
                         int start,
                         int end,
                         bool isForRename = false) const;

    int GetCurToken(const Codira::Position &pos, int start, int end) const;

    int GetCurTokenByStartColumn(const Codira::Position &pos, int start, int end) const;

    int GetCurTokenSkipSpace(const Codira::Position &pos, int start, int end, int lastEnd) const;

    bool IsFilterToken(const Position &pos) const;

    bool IsFilterTokenInHighlight(const Position &pos) const;

    void DoLexer(const std::string &contents, const std::string &fileName);

    bool CheckTokenKind(Codira::TokenKind tokenKind, bool isForRename) const;

    bool CheckTokenKindWhenRenamed(Codira::TokenKind tokenKind) const;

    Ptr<Codira::AST::Decl> GetDeclByPosition(const Codira::Position &originPos,
                                              std::vector<Codira::AST::Symbol *> &syms,
                                              std::vector<Ptr<Decl>> &decls,
                                              const std::pair<bool, bool> &isMacroOrRename = {
                                                  false, false}) const;

    Ptr<Codira::AST::Decl> GetDeclByPosition(const Codira::Position &originPos) const;

    std::vector<Ptr<Decl>> GetOverloadDecls(const ark::Token token) const;

    Ptr<Codira::AST::Decl> FindDeclByNode(Ptr<Codira::AST::Node> node) const;

    Ptr<Codira::AST::Node> GetNodeBySymbols(const ark::ArkAST &,
                                             Ptr<Codira::AST::Node> node,
                                             const std::vector<Codira::AST::Symbol *> &,
                                             const std::string &query,
                                             const size_t) const;

    std::vector<Ptr<Decl>> FindRealDecl(const ark::ArkAST &nowAst,
                                        const std::vector<Codira::AST::Symbol *> &syms,
                                        const std::string &query = "",
                                        const Codira::Position &macroPos = {0, 0, 0},
                                        const std::pair<bool, bool> &isMacroOrRename = {
                                            false, false}) const;

    Ptr<Codira::AST::Decl> GetDelFromType(Ptr<const Codira::AST::Type> type) const;

    Ptr<Codira::AST::Decl> FindRealGenericParamDeclForExtend(
        const std::string &genericParamName, const std::vector<Codira::AST::Symbol *> syms) const;

    bool CheckInQuote(Ptr<const Node> node, const Codira::Position &pos) const;

    Codira::DiagnosticEngine &diag;
    std::vector<Codira::Token> tokens;
    Ptr<const Codira::AST::File> file = nullptr;
    PackageInstance *packageInstance;
    SourceManager *sourceManager{nullptr};
    ArkAST *semaCache = nullptr;
    unsigned int fileID = 0;
};
} // namespace ark

#endif // LSPSERVER_ARKAST_H
