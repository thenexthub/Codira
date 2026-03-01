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

#ifndef LSPSERVER_DOTCOMPLETERBYPARSE_H
#define LSPSERVER_DOTCOMPLETERBYPARSE_H

#include <cangjie/AST/ASTContext.h>
#include <cangjie/AST/Node.h>
#include <cangjie/AST/ScopeManagerApi.h>
#include <cangjie/AST/Symbol.h>
#include <cstdint>
#include "../../CompilerCodiraProject.h"
#include "../../common/ItemResolverUtil.h"
#include "CompletionEnv.h"
#include "CompletionImpl.h"
#include "Codira/AST/Walker.h"

namespace ark {
class DotCompleterByParse {
public:
    DotCompleterByParse(Codira::ASTContext &ctx, CompletionResult &res, Codira::ImportManager &importManager,
    const std::string& curFilePath)
        : context(&ctx), result(res), importManager(&importManager), curFilePath(curFilePath)
    {
        if (context && context->curPackage && !context->curPackage->files.empty() && context->curPackage->files[0]) {
            packageNameForPath = GetPkgNameFromNode(context->curPackage->files[0].get());
            auto moduleName = Utils::SplitQualifiedName(packageNameForPath).front();
            syscap.SetIntersectionSet(moduleName);
        }
        InitMap();
    }

    ~DotCompleterByParse()= default;

    void Complete(const ArkAST &input, const Codira::Position &pos, const std::string &prefix = "");

    void CompleteFromType(const std::string &identifier,
                          const Codira::Position &pos,
                          Codira::AST::Ty *type,
                          CompletionEnv &env) const;

private:
    void CompleteClassDecl(Ptr<Codira::AST::Ty> ty,
                           const Codira::Position &pos,
                           CompletionEnv &env,
                           bool isSuperOrThis = false) const;

    void CompleteInterfaceDecl(Ptr<Codira::AST::InterfaceDecl> interfaceDecl,
                               const Codira::Position &pos,
                               CompletionEnv &env) const;

    void CompleteEnumDecl(Ptr<Codira::AST::Ty> ty, const Codira::Position &pos, CompletionEnv &env) const;

    void CompleteStructDecl(Ptr<Codira::AST::Ty> ty, const Codira::Position &pos, CompletionEnv &env) const;

    void CompleteBuiltInType(Ty *type, CompletionEnv &env) const;

    void FillingDeclsInPackageDot(std::pair<std::string, CompletionEnv &> pkgNameAndEnv,
                                  Codira::AST::Node &curNode,
                                  std::vector<OwnedPtr<Codira::AST::File>> &files,
                                  std::set<std::string> lastDeclSet = {"*"},
                                  const std::pair<bool, bool> openFillAndIsImport = {false, false});

    std::set<std::string> FindDeclSetByFiles(const std::string &packageName,
                                             std::vector<OwnedPtr<Codira::AST::File>> &files) const;

    void AddExtendDeclFromIndex(Ptr<Ty> &extendTy, CompletionEnv &env, const Position &pos) const;

    std::string QueryByPos(Ptr<Node> node, const Position pos);

    static bool Contain(Ptr<Node> node, const Position pos)
    {
        return node && node->begin.fileID == pos.fileID && node->begin <= pos && pos < node->end;
    };

    bool CheckHasLocalDecl(const std::string &beforePrefix, const std::string &scopeName,
                           Ptr<Expr> expr, const Position &pos) const;

    void DeepFind(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void InitMap() const;

    void FindFuncDecl(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindFuncBody(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindBlock(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindVarDecl(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindVarWithPatternDecl(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindClassDecl(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindStructDecl(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindInterfaceDecl(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindEnumDecl(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindMainDecl(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindExtendDecl(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindIfExpr(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindWhileExpr(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindDoWhileExpr(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindForInExpr(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindLambdaExpr(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindMatchExpr(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindVarPattern(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindBinaryExpr(Ptr<Node> node, const Position &pos, std::string & /* scopeName */, bool &isInclude);

    void FindOptionalChainExpr(Ptr<Node> node, const Position &pos, std::string & /* scopeName */, bool &isInclude);

    void FindAssignExpr(Ptr<Node> node, const Position &pos, std::string & /* scopeName */, bool &isInclude);

    void FindIncOrDecExpr(Ptr<Node> node, const Position &pos, std::string & /* scopeName */, bool &isInclude);

    void FindMemberAccess(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindRefExpr(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindCallExpr(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindPrimaryCtorDecl(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindReturnExpr(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindTryExpr(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindMacroDecl(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindMacroExpandDecl(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindMacroExpandExpr(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindSynchronizedExpr(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindPropDecl(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindSpawnExpr(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindTrailingClosureExpr(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

    void FindIfAvailableExpr(Ptr<Node> node, const Position &pos,
                                std::string &scopeName, bool &isInclude);

    void CompleteQualifiedType(const std::string &beforePrefix, CompletionEnv &env) const;

    void FuzzyDotComplete(const ArkAST &input, const Position &pos, const std::string &prefix, CompletionEnv &env);

    void CompleteSuperInterface(Ptr<const InterfaceDecl> interfaceDecl, const Position &pos, CompletionEnv &env) const;

    void CompleteEnumInterface(Ptr<EnumDecl> enumDecl, const Position &pos, CompletionEnv &env) const;

    bool IsEnumCtorTy(const std::string &beforePrefix, Ty *const &ty) const;

    Ptr<Decl> FindTopDecl(const ArkAST &input, const std::string &prefix, CompletionEnv &env,
                          const Position &pos);

    void CompleteCandidate(const Position &pos, const std::string &prefix, CompletionEnv &env,
                           Candidate &declOrTy);

    Position GetMacroNodeNextPosition(
        const std::unique_ptr<ArkAST> &arkAst, const Ptr<NameReferenceExpr> &semaCacheExpr) const;

    /**
     * complete macro-modified field in macro node, ex:
     * @State var a: AA = AA()
     * a.[item]
     *
     * @param input ArkTs
     * @param pos Position
     * @param prefix std::string
     * @param env CompletionEnv
     * @param expr Expr
     */
    void NestedMacroComplete(const ArkAST &input, const Position &pos, const std::string &prefix,
                              CompletionEnv &env, Ptr<Expr> expr);

    void GetTyFromMacroCallNodes(Ptr<Expr> expr, std::unique_ptr<ArkAST> arkAst,
        Ptr<Ty> &ty, Ptr<NameReferenceExpr> &resExpr);

    void CompleteByReferenceTarget(const Position &pos, const std::string &prefix, CompletionEnv &env,
        const Ptr<Expr> &expr, const Ptr<NameReferenceExpr> &resExpr);

    Ptr<Ty> GetTyFromMacroCallNodes(Ptr<Expr> expr, std::unique_ptr<ArkAST> arkAst);

    bool CheckInIfAvailable(Ptr<Decl> decl, const Position &pos);

    bool CompleteEmptyPrefix(Ptr<Expr> expr, CompletionEnv &env, const std::string &prefix,
                                const std::string &scopeName, const Position &pos);

    void FindExprInTopDecl(Ptr<Decl> topDecl, Ptr<Expr>& expr, const ArkAST &input,
                            const Position &pos, OwnedPtr<Expr> &invocationEx);

    void WalkForMemberAccess(Ptr<Decl> topDecl, Ptr<Expr>& expr, const ArkAST &input,
                            const Position &pos, OwnedPtr<Expr> &invocationEx);

    void WalkForIfAvailable(Ptr<Decl> topDecl, Ptr<Expr>& expr, const ArkAST &input,
                            const Position &pos);

    std::string GetFullPrefix(const ArkAST &input, const Position &pos, const std::string &prefix);

    Codira::ASTContext *context = nullptr;

    CompletionResult &result;

    Codira::ImportManager *importManager = nullptr;

    std::string packageNameForPath;

    std::string curFilePath;

    std::set<std::string> usedPkg = {};

    bool isEnumCtor = false;

    SyscapCheck syscap;

    bool inIfAvailable = false;
};

using FindFunc = void (ark::DotCompleterByParse::*)(
    Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude);

class DotMatcher {
public:
    static DotMatcher &GetInstance()
    {
        static DotMatcher instance{};
        return instance;
    }

    void RegFunc(ASTKind astKind, const FindFunc &func) { container[astKind] = func; }

    FindFunc GetFunc(ASTKind astKind)
    {
        auto item = container.find(astKind);
        if (item == container.end()) {
            return nullptr;
        }
        return item->second;
    }

private:
    DotMatcher() = default;

    ~DotMatcher() = default;

    std::map<ASTKind, FindFunc> container{};
};
} // namespace ark

#endif // LSPSERVER_DOTCOMPLETERBYPARSE_H
