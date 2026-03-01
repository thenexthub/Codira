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

#ifndef LSPSERVER_INDEX_SYMBOLCOLLECTOR_H
#define LSPSERVER_INDEX_SYMBOLCOLLECTOR_H

#include <string>
#include <utility>
#include <vector>
#include "../ArkAST.h"
#include "Ref.h"
#include "Relation.h"
#include "Symbol.h"
#include "Codira/AST/Node.h"
#include "Codira/AST/ASTCasting.h"
#include "Codira/Utils/Utils.h"
#include "Codira/Sema/TypeManager.h"
#include "../common/Utils.h"

namespace ark {
namespace lsp {
using namespace Codira;
using namespace AST;

class SymbolCollector {
public:
    SymbolCollector(TypeManager& typeManager, ImportManager& importManager, bool isCodeoPkg)
        : tyMgr(typeManager), importMgr(importManager), isCodeoPkg(isCodeoPkg)
    {
    }

    void Preamble(const Package& package);

    void Build(const Package& package);

    const std::vector<Symbol>* GetSymbolMap() const
    {
        return &pkgSymsMap;
    }

    const std::map<SymbolID, std::vector<Ref>>* GetReferenceMap() const
    {
        return &symbolRefMap;
    }

    const std::map<SymbolID, std::vector<ExtendItem>>* GetSymbolExtendMap() const
    {
        return &symbolExtendMap;
    }

    const std::vector<CrossSymbol>* GetCrossSymbolMap() const
    {
        return &crsSymsMap;
    }

    const std::vector<Relation>* GetRelations() const
    {
        return &relations;
    }

    void SetArkAstMap(std::map<std::string, std::unique_ptr<ArkAST>> arkAstMap)
    {
        astMap = std::move(arkAstMap);
    }

private:
    enum class CrossRegisterType: uint8_t {
        GLOBAL_FUNC_REGISTER,
        MEMBER_FUNC_REGISTER,
        CLASS_REGISTER,
        MODULE_REGISTER
    };

    void UpdateScope(const Decl& decl)
    {
        std::string name;
        if (decl.astKind == ASTKind::EXTEND_DECL) {
            name = "extend";
        } else {
            name = decl.identifier;
        }
        (void)scopes.emplace_back(std::make_pair(&decl, name + "."));
    }

    void UpdateCrossScope(const Node &node, const CrossRegisterType type, const std::string &registerName)
    {
        (void)crossRegisterScopes.emplace_back(&node, std::make_pair(type, registerName));
    }

    void RestoreScope(const Node& node)
    {
        if (scopes.empty()) {
            return;
        }
        if (auto [scopeNode, _] = scopes.back(); scopeNode == &node) {
            scopes.pop_back();
        }
    }

    void RestoreCrossScope(const Node &node)
    {
        if (crossRegisterScopes.empty()) {
            return;
        }
        if (auto [scopeNode, _] = crossRegisterScopes.back(); scopeNode == &node) {
            crossRegisterScopes.pop_back();
        }
    }

    void CollectCrossScopes(Ptr<Node> node);

    bool ShouldPassInCodedIndexing(Ptr<Node> node);

    void CollectRelations(const std::unordered_set<Ptr<InheritableDecl>>& inheritableDecls);

    void CollectNamedParam(Ptr<const Decl> parent, Ptr<const Decl> member);

    // Only for global or member decl.
    void CreateBaseSymbol(const Decl& decl, const std::string& filePath, AccessLevel pkgAccess);

    void CreateBaseOrExtendSymbol(const Decl &decl, const std::string &filePath, AccessLevel pkgAccess);

    void CreateRef(const NameReferenceExpr& ref, const std::string& filePath);

    void CreateTypeRef(const Type& type, const std::string& filePath);

    void CreateMacroRef(const Node& node, const MacroInvocation& invocation);

    void CreateNamedArgRef(const CallExpr& ce);

    void CreateImportRef(const File &fileNode);

    void CreateExtend(const Decl& decl, const std::string& filePath);

    void CreateCrossSymbolByInterop(const Decl &decl);

    void CreateCrossSymbolByRegister(const NameReferenceExpr &ref, const SrcIdentifier &identifier);

    void DealRegisterModule(const CallExpr &callExpr);

    void DealRegisterClass(const FuncArg &registerIdentify, const FuncArg &registerTarget);

    void DealRegisterFunc(const FuncArg &registerIdentify, const FuncArg &registerTarget);

    void  ResloveBlock(const Block &block, const std::string &registerIdentify);

    void DealCrossSymbol(const NameReferenceExpr &ref, const Decl &target, const SrcIdentifier &identifier);

    void DealAddCrossMethodSymbol(const NameReferenceExpr &ref);

    void DealCrossFunctionSymbol(const NameReferenceExpr &functionRef, const SrcIdentifier &identifier);

    void DealFunctionSymbolInRegisterClass(const NameReferenceExpr &functionRef, const SrcIdentifier &identifier);

    void DealFunctionSymbolInRegisterModule(const NameReferenceExpr &functionRef, const SrcIdentifier &identifier);

    void DealFunctionSymbolInFunc(const NameReferenceExpr &functionRef, const SrcIdentifier &identifier);

    void DealCrossClassSymbol(const NameReferenceExpr &clazzRef, const SrcIdentifier &identifier);

    void DealClassSymbolInRegisterModule(const NameReferenceExpr &clazzRef, const SrcIdentifier &identifier);

    void DealClassSymbolInFunc(const Decl &decl, const NameReferenceExpr &clazzRef, const SrcIdentifier &identifier);

    void DealExportsRegisterSymbol(const NameReferenceExpr &clazzRef);

    std::string GetTypeString(const Decl& decl)
    {
        if (auto fd = DynamicCast<const FuncDecl*>(&decl)) {
            if ((fd->TestAttr(Attribute::PRIMARY_CONSTRUCTOR) ||
            fd->TestAttr(Attribute::CONSTRUCTOR)) && fd->outerDecl) {
                return fd->outerDecl->identifier.GetRawText();
            }
            CODEC_NULLPTR_CHECK(fd->funcBody);
            return fd->funcBody->retType && fd->funcBody->retType->ty ? fd->funcBody->retType->ty->String() : "Invalid";
        } else {
            CODEC_NULLPTR_CHECK(decl.ty);
            return decl.ty->String();
        }
    }

    inline SymbolID GetContextID()
    {
        SymbolID containerId = INVALID_SYMBOL_ID;
        CODEC_ASSERT(!scopes.empty());
        Ptr<const Decl> decl;
        for (size_t i = scopes.size() - 1; i >= 0; i--) {
            decl = DynamicCast<const Decl*>(scopes[i].first);
            if (decl && decl->astKind == Codira::AST::ASTKind::VAR_DECL) {
                continue;
            }
            break;
        }
        if (decl) {
            containerId = declToSymIdMap[decl];
        }
        return containerId;
    }

    SymbolID GetDeclSymbolID(const Decl& decl)
    {
        auto found = declToSymIdMap.find(&decl);
        if (found != declToSymIdMap.end()) {
            return found->second;
        }
        auto identifier = decl.exportId;
        auto isSupport = (decl.astKind == ASTKind::FUNC_PARAM) || (identifier.empty() && IsLocalFuncOrLambda(decl));
        if (isSupport) {
            if (!decl.outerDecl) {
                return INVALID_SYMBOL_ID;
            }
            identifier = decl.outerDecl->exportId + "$" + decl.identifier;
        }
        if (identifier.empty()) {
            return INVALID_SYMBOL_ID;
        }
        size_t ret = 0;
        ret = hash_combine<std::string>(ret, identifier);
        (void)declToSymIdMap.emplace(&decl, ret);
        return ret;
    }

    Ptr<Decl> FindOverriddenMember(const Decl& member, const InheritableDecl& id, const TypeSubst& typeMapping,
        const std::function<bool(const Decl&)>& satisfy);

    Ptr<Decl> FindOverriddenMemberFromSuperClass(const Decl& member, const ClassDecl& cd);

    std::vector<Ptr<Decl>> FindImplMemberFromInterface(const Decl& member, const InheritableDecl& id);

    void UpdatePos(SymbolLocation &location, const Node& node, const std::string& filePath);

    static void CollectCompletionItem(const Decl &decl, Symbol &declSym);

    void CollectNode(Ptr<Node> node, const std::string& filePath, AccessLevel pkgAccess);

    // Only toplevel and member decls (except extend decl).
    std::unordered_map<Ptr<const Decl>, SymbolID> declToSymIdMap;

    std::vector<std::pair<Ptr<const Node>, std::string>> scopes;

    std::vector<std::pair<Ptr<const Node>, std::pair<CrossRegisterType, std::string>>> crossRegisterScopes;

    std::unordered_map<SymbolID, std::vector<std::tuple<std::string, SymbolLocation, SymbolID, SymbolLocation>>>
        crossRegisterDecls;

    /**
     * import pkg.item as alias
     * Pair is [item, alias]
     */
    std::unordered_map<std::string, std::string> aliasMap;

    TypeManager& tyMgr;

    ImportManager& importMgr;

    std::vector<Symbol> pkgSymsMap;

    std::map<std::string, std::unique_ptr<ArkAST>> astMap;

    std::map<SymbolID, std::vector<Ref>> symbolRefMap;

    std::vector<Relation> relations;

    std::map<SymbolID, std::vector<ExtendItem>> symbolExtendMap;

    std::vector<CrossSymbol> crsSymsMap;

    bool isCodeoPkg;

    const std::string CROSS_ARK_TS_WITH_INTEROP_NAME = "Interop";

    const std::string CROSS_ARK_TS_WITH_INTEROP_INVISIBLE_NAME = "Invisible";

    const std::string CROSS_ARK_TS_WITH_REGISTER_PACKAGE = "ohos.ark_interop";

    const std::string CROSS_ARK_TS_WITH_REGISTER_MODULE = "registerModule";

    const std::string CROSS_ARK_TS_WITH_REGISTER_CLASS = "registerClass";

    const std::string CROSS_ARK_TS_WITH_REGISTER_FUNC = "registerFunc";

    const std::set<std::string> CROSS_ARK_TS_WITH_REGISTER_NAMES = {
        CROSS_ARK_TS_WITH_REGISTER_MODULE, CROSS_ARK_TS_WITH_REGISTER_CLASS, CROSS_ARK_TS_WITH_REGISTER_FUNC
    };

    const std::string FUNCTION_REGISTER_SYMBOL = "function";

    const std::string CLAZZ_REGISTER_SYMBOL = "clazz";

    const std::string ADD_METHOD = "addMethod";

    const std::string JS_OBJECT_BASE_TY = "JSObjectBase";

    const std::string JS_LAMBDA_TY = "(Class-JSContext, Struct-JSCallInfo) -> Struct-JSValue";

    const std::string FUNC_REGISTER_TY = "(Class-JSContext) -> Class-JSFunction";

    const std::string CLASS_REGISTER_TY = "(Class-JSContext) -> Class-JSClass";

    const std::string MODULE_REGISTER_TY = "(Class-JSContext, Class-JSObject) -> Unit";
};
} // namespace lsp
} // namespace ark

#endif // LSPSERVER_INDEX_SYMBOLCOLLECTOR_H
