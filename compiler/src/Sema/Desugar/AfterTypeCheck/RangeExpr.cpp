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

#include "TypeCheckUtil.h"

#include "Codira/AST/Create.h"
#include "Codira/AST/Match.h"
#include "Codira/AST/Utils.h"

using namespace Codira;
using namespace AST;
using namespace TypeCheckUtil;

namespace {
std::vector<OwnedPtr<FuncArg>> CreateRangeExprArgs(const RangeExpr& re)
{
    std::vector<OwnedPtr<FuncArg>> args;
    if (re.startExpr != nullptr) {
        args.push_back(CreateFuncArg(ASTCloner::Clone(re.startExpr.get())));
    } else {
        // If startExpr does not exist, set LitConst "0" as default value.
        auto startExpr = CreateLitConstExpr(LitConstKind::INTEGER, "0", re.ty->typeArgs[0]);
        args.push_back(CreateFuncArg(std::move(startExpr)));
    }
    if (re.stopExpr != nullptr) {
        args.push_back(CreateFuncArg(ASTCloner::Clone(re.stopExpr.get())));
    } else {
        // If stopExpr does not exist, set LitConst "0" as default value.
        auto stopExpr = CreateLitConstExpr(
            LitConstKind::INTEGER, std::to_string(std::numeric_limits<int64_t>::max()), re.ty->typeArgs[0]);
        args.push_back(CreateFuncArg(std::move(stopExpr)));
    }
    if (re.stepExpr != nullptr) {
        args.push_back(CreateFuncArg(ASTCloner::Clone(re.stepExpr.get())));
    } else {
        // If stepExpr does not exist, set LitConst "1" as default value.
        auto stepExpr =
            CreateLitConstExpr(LitConstKind::INTEGER, "1", TypeManager::GetPrimitiveTy(TypeKind::TYPE_INT64));
        args.push_back(CreateFuncArg(std::move(stepExpr)));
    }
    std::string hasStart = re.startExpr ? "true" : "false";
    std::string hasStop = re.stopExpr ? "true" : "false";
    std::string isClosed = re.isClosed ? "true" : "false";
    auto hasStartExpr =
        CreateLitConstExpr(LitConstKind::BOOL, hasStart, TypeManager::GetPrimitiveTy(TypeKind::TYPE_BOOLEAN));
    auto hasStopExpr =
        CreateLitConstExpr(LitConstKind::BOOL, hasStop, TypeManager::GetPrimitiveTy(TypeKind::TYPE_BOOLEAN));
    auto isClosedExpr =
        CreateLitConstExpr(LitConstKind::BOOL, isClosed, TypeManager::GetPrimitiveTy(TypeKind::TYPE_BOOLEAN));
    args.push_back(CreateFuncArg(std::move(hasStartExpr)));
    args.push_back(CreateFuncArg(std::move(hasStopExpr)));
    args.push_back(CreateFuncArg(std::move(isClosedExpr)));
    return args;
}
} // namespace

namespace Codira::Sema::Desugar::AfterTypeCheck {
void DesugarRangeExpr(TypeManager& typeManager, RangeExpr& re)
{
    // Desugar of RangeExpr is done after typeCheck.
    if (!re.decl) {
        // RangeExpr in for-in expr does not have decl set.
        return;
    }
    CODEC_NULLPTR_CHECK(re.decl->generic);
    CODEC_NULLPTR_CHECK(re.ty);
    if (re.desugarExpr) {
        return;
    }
    if (re.ty->typeArgs.empty() || re.ty->typeArgs.size() != re.decl->generic->typeParameters.size() ||
        !Ty::IsTyCorrect(re.decl->generic->typeParameters[0]->ty)) {
        return;
    }
    auto rangeFunc = CreateRefExpr(re.decl->identifier);
    CopyBasicInfo(&re, rangeFunc.get());
    (void)rangeFunc->instTys.emplace_back(re.ty->typeArgs[0]);
    TypeSubst typeMapping;
    typeMapping.emplace(StaticCast<GenericsTy*>(re.decl->generic->typeParameters[0]->ty), re.ty->typeArgs[0]);

    std::vector<OwnedPtr<FuncArg>> args = CreateRangeExprArgs(re);
    auto ce = CreateCallExpr(std::move(rangeFunc), std::move(args));
    ce->ty = re.ty;
    for (auto& initFn : re.decl->body->decls) {
        if (auto fd = AST::As<ASTKind::FUNC_DECL>(initFn.get()); fd && IsInstanceConstructor(*fd)) {
            if (auto refExpr = AST::As<ASTKind::REF_EXPR>(ce->baseFunc.get()); refExpr) {
                ReplaceTarget(refExpr, fd, false);
                CODEC_NULLPTR_CHECK(fd->ty);
                refExpr->ty = typeManager.GetInstantiatedTy(fd->ty, typeMapping);
                ce->resolvedFunction = fd;
                ce->callKind = CallKind::CALL_OBJECT_CREATION;
            }
            break;
        }
    }
    CopyBasicInfo(&re, ce.get());
    AddCurFile(*ce, re.curFile);
    re.desugarExpr = std::move(ce);
}
} // namespace Codira::Sema::Desugar::AfterTypeCheck
