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

#include "Codira/CHIR/AST2CHIR/TranslateASTNode/Translator.h"

using namespace Codira::CHIR;
using namespace Codira;

static void CollectInstantiatedFuncNodes(
    std::vector<Ptr<AST::Node>>& nodes, const AST::FuncDecl& genericFunc, const GenericInstantiationManager& gim)
{
    auto decls = gim.GetInstantiatedDecls(genericFunc);
    if (decls.empty() && genericFunc.genericDecl) {
        // 'genericFunc' may be partially instantiated decl which cannot be found in map.
        decls = gim.GetInstantiatedDecls(*genericFunc.genericDecl);
    }
    for (auto instantiatedDecl : decls) {
        (void)nodes.emplace_back(instantiatedDecl);
    }
}

static bool IsUnnecessarySuperCall(const AST::Node& node)
{
    auto ce = DynamicCast<AST::CallExpr>(&node);
    if (!ce || !ce->baseFunc) {
        return false;
    }
    if (auto re = DynamicCast<AST::RefExpr>(ce->baseFunc.get()); re && re->isSuper) {
        // Super call of 'Object' can be ignored.
        return ce->ty->IsObject();
    }
    return false;
}

#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
static std::vector<Ptr<AST::Node>> CollectBlockBodyNodes(
    const AST::Block& block, const GenericInstantiationManager* gim)
{
    std::vector<Ptr<AST::Node>> nodes;
    for (const auto& body : block.body) {
        // If we find a generic local function here, then we must
        // retrieve all the instantiated functions for the translation.
        auto funcDecl = DynamicCast<AST::FuncDecl*>(body.get());
        if (funcDecl != nullptr && funcDecl->TestAttr(AST::Attribute::GENERIC)) {
            if (gim) {
                CollectInstantiatedFuncNodes(nodes, *funcDecl, *gim);
            }
            nodes.emplace_back(funcDecl);
        } else {
            if (IsUnnecessarySuperCall(*body)) {
                continue;
            }
            nodes.emplace_back(body.get());
        }
    }
    return nodes;
}
#endif

Ptr<Value> Translator::Visit(const AST::Block& b)
{
    CODEC_ASSERT(!blockGroupStack.empty());
    auto block = CreateBlock();
    // Current block may be changed during translation,
    // store AST block 'b' related CHIR block for caller to generate goto or branch.
    SetSymbolTable(b, *block);

    currentBlock = block;
    // Since codira's block has value, return the value of last block node.
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    std::vector<Ptr<AST::Node>> nodes = CollectBlockBodyNodes(b, gim);
#endif
    for (size_t i = 0; i < nodes.size(); ++i) {
        if (i == nodes.size() - 1) {
            return TranslateSubExprAsValue(*nodes[i]);
        } else {
            TranslateSubExprToDiscarded(*nodes[i]);
        }
    }
    return nullptr;
}
