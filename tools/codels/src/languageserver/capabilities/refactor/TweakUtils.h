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

#ifndef CODIRA_LSP_TWEAKUTILS_H
#define CODIRA_LSP_TWEAKUTILS_H

#include "Codira/AST/Node.h"
#include "Codira/AST/ASTContext.h"
#include "../../ArkAST.h"
#include "../../common/Utils.h"
#include "../../selection/SelectionTree.h"

namespace ark {
using namespace Codira;
using namespace AST;

class TweakUtils {
public:
    static Ptr<Block> FindOuterScopeNode(const ASTContext& ctx, const Expr& expr, bool &isGlobal, Range &range);

    static bool Contain(Codira::AST::Node &node, Range &range);

    static bool CheckValidExpr(const SelectionTree::SelectionTreeNode &treeNode);

    static Range FindGlobalInsertPos(const File &file, Range &range);
private:
    static Ptr<Block> GetSatisfiedBlock(const ASTContext &ctx, Range &range, const std::string &scopeName,
        bool &isGlobal);

    static Ptr<Block> GetSatisfiedBlockBySearch(const ASTContext &ctx, Range &range, const std::string &scopeName,
        bool &isGlobal);

    static Ptr<Block> GetSymbolBlock(AST::Symbol &symbol, Range &range);

    static Ptr<Block> DealTryExpr(TryExpr *tryExpr, Range &range);

    static Ptr<Block> DealBlock(Block *block, Range &range);

    static Ptr<Block> DealFuncBody(FuncBody *funcBody, Range &range);

    static Ptr<Block> DealIfExpr(IfExpr *ifExpr, Range &range);

    static Ptr<Block> DealWhileExpr(WhileExpr *whileExpr, Range &range);

    static Ptr<Block> DealDoWhileExpr(DoWhileExpr *doWhileExpr, Range &range);

    static Ptr<Block> DealForInExpr(ForInExpr *forInExpr, Range &range);

    static Ptr<Block> DealMatchCase(MatchCase *matchCase, Range &range);

    static Ptr<Block> DealFuncDecl(FuncDecl *funcDecl, Range &range);
};
} // namespace ark

#endif // CODIRA_LSP_TWEAKUTILS_H
