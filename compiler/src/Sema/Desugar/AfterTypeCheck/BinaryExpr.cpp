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

#include "Desugar/AfterTypeCheck.h"

#include "Codira/AST/Create.h"
#include "Codira/AST/Utils.h"

using namespace Codira;
using namespace AST;

namespace {
/**
 * *************** before desugar ****************
 * e1 |> e2
 * *************** after desugar  ****************
 * e2(e1)
 * *************** after blockify ****************
 * {
 *     let v = e1
 *     e2(v)
 * }
 */
void BlockifyFlowExpr(BinaryExpr& be)
{
    CODEC_ASSERT(be.op == TokenKind::PIPELINE && be.desugarExpr);
    // Get the inner most `desugarExpr`.
    Ptr<CallExpr> innerCe = StaticCast<CallExpr*>(be.desugarExpr.get());
    while (innerCe->desugarExpr) {
        innerCe = StaticCast<CallExpr*>(innerCe->desugarExpr.get());
    }
    CODEC_ASSERT(innerCe->args.size() == 1 && innerCe->args.front() != nullptr);
    // Create `let v = e1`.
    auto vd = CreateVarDecl(V_COMPILER, std::move(innerCe->args.front()->expr));
    vd->fullPackageName = be.GetFullPackageName();
    CopyBasicInfo(vd->initializer.get(), vd.get());
    // Create the reference `v` and replace the argument with this `RefExpr`.
    auto re = CreateRefExpr(*vd);
    CopyBasicInfo(vd->initializer.get(), re.get());
    innerCe->args.front()->expr = std::move(re);
    // Create the block.
    std::vector<OwnedPtr<Node>> nodes;
    nodes.emplace_back(std::move(vd));
    nodes.emplace_back(std::move(be.desugarExpr));
    auto block = CreateBlock(std::move(nodes), be.ty);
    CopyBasicInfo(&be, block.get());
    AddCurFile(*block, be.curFile);
    be.desugarExpr = std::move(block);
}
} // namespace

namespace Codira::Sema::Desugar::AfterTypeCheck {
void DesugarBinaryExpr(BinaryExpr& be)
{
    if (!Ty::IsTyCorrect(be.ty)) {
        return;
    }
    if (be.op == TokenKind::PIPELINE) {
        BlockifyFlowExpr(be);
    }
}
} // namespace Codira::Sema::Desugar::AfterTypeCheck
