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

#ifndef SELECTIONTREE_H
#define SELECTIONTREE_H
#include <cangjie/AST/ASTContext.h>
#include "../ArkAST.h"

namespace ark {
using namespace Codira;
using namespace AST;
class SelectionTree {
public:
    static bool CreateEach(const ArkAST &arkAST,
        const std::string &fileName,
        Position Begin,
        Position End,
        std::function<bool(SelectionTree)> Func);

    enum class Selection : unsigned char {
        Unselected,
        Partial,
        Complete,
    };

    // can only be refact within scope: GLOBAL_VAR/MEMBER_VAR/FUNC_BODY
    enum class Scope {
        UNKNOWN,
        GLOBAL_VAR,
        MEMBER_VAR,
        FUNC_BODY
    };

    struct SelectionTreeNode {
        Ptr<Node> node;
        Ptr<Node> Parent;
        std::vector<Ptr<SelectionTreeNode>> Children;
        Selection selected;
    };

    enum class WalkAction {
        WALK_CHILDREN,
        SKIP_CHILDREN,
        STOP_NOW
    };

    const SelectionTreeNode *CommonAncestor() const;

    const SelectionTreeNode *root() const
    {
        return Root ? &(*Root) : nullptr;
    }

    const Scope SelectedScope() const
    {
        return scope;
    }

    const Ptr<Codira::AST::Node> TargetDecl() const
    {
        return targetDecl;
    }

    const Ptr<Codira::AST::Node> TopDecl() const
    {
        return topDecl;
    }

    const Ptr<Codira::AST::Node> OuterInterpExpr() const
    {
        return outerInterpExpr;
    }

    using WalkCallBack = std::function<WalkAction(const SelectionTreeNode*)>;

    static void Walk(const SelectionTreeNode *node, WalkCallBack callBack) ;

    void printSelection(const SelectionTreeNode* node, int level = 0) const;

private:
    // Creates a selection tree for the given range in the main file.
    // The range includes bytes [start, end).
    SelectionTree(const ArkAST &arkAST, const std::string &fileName, Position start, Position end);

    bool FindSelectNode(Decl *decl, Position start, Position end);

    void MatchSelectedScope(Ptr<Node> node, Position start, Position end);

    void FindTopDecl(Codira::AST::Node &node);

    static void BuildTreeNode(SelectionTreeNode *rootTreeNode, Position start, Position end);

    static WalkAction WalkImpl(const SelectionTreeNode* node, WalkCallBack callBack);

    std::unique_ptr<SelectionTreeNode> Root;

    Codira::AST::Decl *topDecl = nullptr;

    Scope scope = Scope::UNKNOWN;

    Ptr<Codira::AST::Node> targetDecl = nullptr;

    Ptr<Codira::AST::Node> outerInterpExpr = nullptr;
};
} // namespace ark

#endif // SELECTIONTREE_H
