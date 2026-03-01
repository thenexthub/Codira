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

#include "TypeCheckerImpl.h"

#include "DiagSuppressor.h"
#include "Diags.h"

using namespace Codira;
using namespace Sema;

bool TypeChecker::TypeCheckerImpl::SynthesizeAndReplaceIdealTy(ASTContext& ctx, Node& node)
{
    // Call `Synthesize` on declares containing invalid types may return valid types.
    // Therefore, we need to know if there are any errors during the inference process.
    auto ds = DiagSuppressor(diag);
    bool valid = Ty::IsTyCorrect(Synthesize(ctx, &node)) && ReplaceIdealTy(node) && !ds.HasError();
    ds.ReportDiag();
    return valid;
}

Ptr<Ty> TypeChecker::TypeCheckerImpl::SynBlock(ASTContext& ctx, Block& b)
{
    if (b.body.empty()) {
        b.ty = TypeManager::GetPrimitiveTy(TypeKind::TYPE_UNIT);
    } else {
        bool existInvalid = false;
        for (auto& node : b.body) {
            existInvalid = !SynthesizeAndReplaceIdealTy(ctx, *node) || existInvalid;
        }
        Ptr<Node> lastNode = b.body[b.body.size() - 1].get();
        CODEC_ASSERT(lastNode != nullptr);
        if (existInvalid) {
            b.ty = TypeManager::GetInvalidTy();
        } else if (lastNode->IsDecl()) {
            b.ty = TypeManager::GetPrimitiveTy(TypeKind::TYPE_UNIT);
        } else {
            b.ty = lastNode->ty;
        }
    }
    return b.ty;
}

bool TypeChecker::TypeCheckerImpl::ChkBlock(ASTContext& ctx, Ty& target, Block& b)
{
    Ptr<Ty> unitTy = TypeManager::GetPrimitiveTy(TypeKind::TYPE_UNIT);
    if (b.body.empty()) {
        b.ty = unitTy;
        // NOTE: This function may return false, the caller should handle diagnostics.
        // Only unsafe block is allowed to exist on its own, and needs to diagnose here.
        auto ret = typeManager.IsSubtype(b.ty, &target);
        if (!ret && b.TestAttr(Attribute::UNSAFE)) {
            auto builder = diag.DiagnoseRefactor(DiagKindRefactor::sema_mismatched_types, b);
            builder.AddMainHintArguments(target.String(), b.ty->String());
        }
        return ret;
    }
    // Synthesize the first N - 1 nodes.
    bool isWellTyped = true;
    for (size_t i = 0; i < b.body.size() - 1; i++) {
        CODEC_ASSERT(b.body[i]);
        isWellTyped = SynthesizeAndReplaceIdealTy(ctx, *b.body[i]) && isWellTyped;
    }
    Ptr<Node> lastNode = b.body[b.body.size() - 1].get();
    CODEC_ASSERT(lastNode != nullptr);
    // If lastNode is compiler added return, just check inner expr.
    if (!b.TestAttr(AST::Attribute::COMPILER_ADD) && lastNode->TestAttr(AST::Attribute::COMPILER_ADD) &&
        lastNode->astKind == ASTKind::RETURN_EXPR) {
        lastNode = StaticCast<ReturnExpr>(lastNode)->expr.get();
    }
    if (lastNode->IsDecl()) {
        bool typeMatched = typeManager.IsSubtype(unitTy, &target);
        isWellTyped = SynthesizeAndReplaceIdealTy(ctx, *lastNode) && typeMatched && isWellTyped;
        if (isWellTyped) {
            b.ty = unitTy;
            return true;
        } else {
            b.ty = TypeManager::GetInvalidTy();
            if (!typeMatched) {
                DiagMismatchedTypesWithFoundTy(
                    diag, *lastNode, target, *unitTy, "definitions and declarations are always of type 'Unit'");
            }
            return false;
        }
    } else {
        isWellTyped = Check(ctx, &target, lastNode) && isWellTyped;
        if (isWellTyped) {
            b.ty = lastNode->ty;
            return true;
        }
        b.ty = TypeManager::GetInvalidTy();
        return false;
    }
}
