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

#ifndef LSPSERVER_COMPLETIONENV_H
#define LSPSERVER_COMPLETIONENV_H

#include <cangjie/AST/Node.h>
#include <cangjie/AST/Symbol.h>
#include <cstdint>
#include <vector>

#include "../../../json-rpc/Protocol.h"
#include "../../common/SyscapCheck.h"
#include "../../CompilerCodiraProject.h"
#include "CompletionImpl.h"

namespace ark {
enum class FILTER {
    IS_THIS = 0,
    IS_SUPER,
    IS_STATIC,
    IGNORE_STATIC,
    INSIDE_STATIC_FUNC, // inside static func, must inside func
    INSIDE_STRUCT_INIT, // inside struct init, must inside func
    INSIDE_STRUCT_FUNC, // inside struct func, must inside func
    INSIDE_LAMBDA,      // inside lambda, must inside func
    INSIDE_VARDECL,
    INSIDE_FUNC,
};

class CompletionEnv {
public:
    explicit CompletionEnv();

    bool GetValue(FILTER kind) const;

    void SetValue(FILTER kind, bool value);

    void DotAccessible(Codira::AST::Decl &decl, const Codira::AST::Decl &parentDecl, bool isSuperOrThis = false);

    void InvokedAccessible(
        Ptr<Codira::AST::Node> node, bool insideFunction, bool deepestFunction, bool isImport = false);

    void CompleteNode(Ptr<Node> node,
        bool isImport = false,
        bool isInScope = false,
        bool isSameName = false,
        const std::string &container = "");

    static ark::lsp::SymbolID GetDeclSymbolID(const Decl& decl);

    void DealTypeAlias(Ptr<Node> node);

    void DeepComplete(Ptr<Node> node, const Position pos);

    void SemaCacheComplete(Ptr<Node> node, const Position pos);

    bool CompleteInParseCache(const std::string &parentClassLikeName);

    void CompleteInSemaCache(const std::string &parentClassLikeName);

    void CompleteAliasItem(Ptr<Codira::AST::Node> node,
                           const std::string &aliasName,
                           bool isImport = false);

    void AccessibleByString(const std::string &str, const std::string &completionDetail = "");

    void DotPkgName(const std::string &fullPkgName, const std::string &prePkgName = "");

    template <typename T>
    void CompleteClassOrStructDecl(const T &decl,
                                   const std::string &aliasName = "",
                                   bool isType = false,
                                   bool isInScope = false);

    void CompleteInheritedTypes(Ptr<Node> decl);

    ~CompletionEnv() = default;

    void OutputResult(CompletionResult &result) const;

    bool CheckParentAccessible(const Codira::AST::Decl &decl,
                               const Codira::AST::Decl &parent,
                               bool isSuperOrThis);

    bool CanCompleteStaticMember(const Codira::AST::Decl &decl) const;

    void AddDirectScope(const std::string &scope, Ptr<Codira::AST::Node> node)
    {
        directScope[scope] = node;
    }

    void AddVArrayItem();

    void AddBuiltInItem(Decl &decl)
    {
        std::string signature = ItemResolverUtil::ResolveSignatureByNode(decl);
        AddItem(decl, signature, true);
    }

    static void MarkDeprecated(Decl &decl, CodeCompletion &completion)
    {
        if (decl.HasAnno(AnnotationKind::DEPRECATED)) {
            completion.deprecated = true;
        }
    }

    std::string curPkgName;

    const ArkAST *parserAst = nullptr;

    const ArkAST *cache = nullptr;

    bool isFromNormal = false;

    bool isInPackage = false;

    bool isAfterAT = false;

    std::string matchSelector;

    std::string prefix;

    uint8_t scopeDepth = 0;

    // map<ID,vector<Ptr<Decl>>>
    std::map<std::string, std::vector<Ptr<Decl>>> idMap = {};

    // map<name, <decl, bool>>
    std::unordered_map<std::string, std::pair<Ptr<Decl>, bool>> importedExternalDeclMap = {};

    std::map<std::string, Ptr<Decl>> aliasMap = {};

    std::unordered_set<Ptr<Node>> visitedNodes;

    void AddCompletionItem(
        const std::string &name, const std::string &signature, const CodeCompletion &completion, bool overwrite = true);

    void SetSyscap(const std::string &moduleName);

private:
    static bool Contain(Ptr<Node> node, const Position pos)
    {
        return node && node->GetBegin().fileID == pos.fileID && node->GetBegin() <= pos && pos < node->GetEnd();
    };

    void DealFuncDecl(Ptr<Node> node, const Position pos);

    void DealMainDecl(Ptr<Node> node, const Position pos);

    void DealMacroDecl(Ptr<Node> node, const Position pos);

    void DealMacroExpandDecl(Ptr<Node> node, const Position pos);

    void DealMacroExpandExpr(Ptr<Node> node, const Position pos);

    void DealPrimaryCtorDecl(Ptr<Node> node, const Position pos);

    void DealLambdaExpr(Ptr<Node> node, const Position pos);

    void DealClassDecl(Ptr<Node> node, const Position pos);

    void DealStructDecl(Ptr<Node> node, const Position pos);

    void DealInterfaceDecl(Ptr<Node> node, const Position pos);

    void DealExtendDecl(Ptr<Node> node, const Position pos);

    void DealEnumDecl(Ptr<Node> node, const Position pos);

    void DealIfExpr(Ptr<Node> node, const Position pos);

    void DealLetPatternDestructor(Ptr<Node> node, const Position pos);

    void DealTuplePattern(Ptr<Node> node, const Position pos);

    void DealVarPattern(Ptr<Node> node, const Position /* pos */);

    void DealVarOrEnumPattern(Ptr<Node> node, const Position /* pos */);

    void DealEnumPattern(Ptr<Node> node, const Position pos);

    void DealWhileExpr(Ptr<Node> node, const Position pos);

    void DealDoWhileExpr(Ptr<Node> node, const Position pos);

    void DealForInExpr(Ptr<Node> node, const Position pos);

    void DealSynchronizedExpr(Ptr<Node> node, const Position pos);

    void DealTryExpr(Ptr<Node> node, const Position pos);

    void DealReturnExpr(Ptr<Node> node, const Position pos);

    void DealFuncBody(Ptr<Node> node, const Position pos);

    void DealVarDecl(Ptr<Node> node, const Position pos);

    void DealVarWithPatternDecl(Ptr<Node> node, const Position pos);

    void DealPropDecl(Ptr<Node> node, const Position pos);

    void DealMatchExpr(Ptr<Node> node, const Position pos);

    void DealMatchCase(Ptr<Node> node, const Position pos);

    void DealTrailingClosureExpr(Ptr<Node> node, const Position pos);

    void DealFuncParamList(Ptr<Node> node, const Position pos);

    void DealGeneric(Ptr<Node> node, const Position pos);

    void DealBlock(Ptr<Node> node, const Position pos);

    void DealFuncArg(Ptr<Node> node, const Position pos);

    void DealParenExpr(Ptr<Node> node, const Position pos);

    void DealCallExpr(Ptr<Node> node, const Position pos);

    void DealBinaryExpr(Ptr<Node> node, const Position pos);

    void DealTupleLit(Ptr<Node> node, const Position pos);

    void DealSpawnExpr(Ptr<Node> node, const Position pos);

    void DealMemberAccess(Ptr<Node> node, const Position pos);

    void DealRefExpr(Ptr<Node> node, const Position pos);

    void DealPrimaryMemberParam(Ptr<Node> node, bool needCheck = false);

    template <typename T> void DealClassOrInterfaceDeclByName(T &decl);

    void InitMap() const;

    void AddExtraMemberDecl(const std::string &name);

    void CompleteInitFuncDecl(Ptr<Codira::AST::Node> node,
                              const std::string &aliasName = "",
                              bool isType = false,
                              bool isInScope = false);

    void CompleteFollowLambda(const Codira::AST::Node &node, Codira::SourceManager *sourceManager,
        CodeCompletion &completion, const std::string &initFuncReplace = "");

    void CompleteParamListFuncTypeVarDecl(const Codira::AST::Node &node, Codira::SourceManager *sourceManager,
        CodeCompletion &completion);

    bool CheckInsideVarDecl(const Codira::AST::Decl &decl) const;

    bool RefToDecl(Ptr<Codira::AST::Node> node, bool insideFunction, bool deepestFunction);

    bool VarDeclIsLet(Ptr<Codira::AST::Node> node) const;

    void DealParentDeclByName(std::string parentClassLikeName);

    void DeclVarWithTuple(Ptr<const Node> node, bool isImport, bool isInScope, bool isSameName);

    void AddItem(Decl &decl, const std::string &signature, bool show);

    void AddItemForMacro(Ptr<Node> node, CodeCompletion &completion);

    std::map<std::string, std::map<std::string, CodeCompletion>> items{};

    std::map<std::string, Ptr<Codira::AST::Node>> directScope{};

    std::vector<Ptr<Codira::AST::VarDecl>> insideVarDecl{};

    std::vector<Ptr<Codira::AST::FuncBody>> insideFunc{};

    std::set<std::string> visitedSet{};

    unsigned int filter = 0;

    bool isCompleteFunction = false;

    SyscapCheck syscap;

    bool IsSignatureInItems(const std::string &name, const std::string &signature);

    void CompleteFunctionName(CodeCompletion &completion, bool isRawIdentifier);

    bool IsInEnumConstructor(OwnedPtr<Decl>& constor, Position pos);
};

using StatusFunc = void (ark::CompletionEnv::*)(Ptr<Node>, Position);
class NormalMatcher {
public:
    static NormalMatcher &GetInstance()
    {
        static NormalMatcher instance {};
        return instance;
    }

    void RegFunc(ASTKind astKind, const StatusFunc &func)
    {
        container[astKind] = func;
    }

    StatusFunc GetFunc(ASTKind astKind)
    {
        auto item = container.find(astKind);
        if (item == container.end()) { return nullptr; }
        return item->second;
    }

private:
    NormalMatcher() = default;

    ~NormalMatcher() = default;

    std::map<ASTKind, StatusFunc> container{};
};
} // namespace ark

#endif // LSPSERVER_COMPLETIONENV_H
