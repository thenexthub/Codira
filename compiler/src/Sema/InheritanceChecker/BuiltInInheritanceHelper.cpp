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

/**
 * @file
 *
 * This file implements inheritance checking of structure declarations.
 */
#include "StructInheritanceChecker.h"

#include "Codira/AST/Clone.h"
#include "Codira/AST/Match.h"
#include "Codira/AST/Node.h"
#include "Codira/AST/Types.h"
#include "Codira/AST/Utils.h"
#include "Codira/Sema/TypeManager.h"

#include "BuiltInOperatorUtil.h"
#include "TypeCheckUtil.h"

using namespace Codira;
using namespace AST;
using namespace TypeCheckUtil;

bool StructInheritanceChecker::IsBuiltInOperatorFuncInExtend(
    const MemberSignature& member, const Decl& structDecl) const
{
    if (structDecl.astKind != ASTKind::EXTEND_DECL || !member.decl->IsFunc() || !Ty::IsTyCorrect(member.decl->ty) ||
        !member.decl->TestAttr(Attribute::ABSTRACT, Attribute::OPERATOR)) {
        return false;
    }
    auto ed = RawStaticCast<const ExtendDecl*>(&structDecl);
    auto fd = RawStaticCast<const FuncDecl*>(member.decl);
    auto funcTy = RawStaticCast<FuncTy*>(member.ty);
    auto iFuncRetTy = funcTy->retTy;
    const std::vector<Ptr<Ty>>& paramTys = funcTy->paramTys;
    Ptr<Ty> thisTy = ed->extendedType->ty;
    if (paramTys.size() == 1 && thisTy && paramTys[0] && IsBuiltinBinaryExpr(fd->op, *thisTy, *paramTys[0])) {
        TypeKind returnTyKind = GetBuiltinBinaryExprReturnKind(fd->op, thisTy->kind);
        auto expectedRetTy = TypeManager::GetPrimitiveTy(returnTyKind);
        if (expectedRetTy == iFuncRetTy) {
            CreateBuiltInBinaryOperatorFunc(fd->op, paramTys[0], *const_cast<ExtendDecl*>(ed), returnTyKind);
        } else {
            diag.DiagnoseRefactor(DiagKindRefactor::sema_return_type_incompatible, structDecl, fd->identifier);
        }
        return true;
    } else if (paramTys.empty() && thisTy && IsBuiltinUnaryExpr(fd->op, *thisTy)) {
        TypeKind returnTyKind = GetBuiltinUnaryOpReturnKind(fd->op, ed->ty->kind);
        auto expectedRetTy = TypeManager::GetPrimitiveTy(returnTyKind);
        if (expectedRetTy == iFuncRetTy) {
            CreateBuiltInUnaryOperatorFunc(fd->op, *const_cast<ExtendDecl*>(ed));
        } else {
            diag.DiagnoseRefactor(DiagKindRefactor::sema_return_type_incompatible, structDecl, fd->identifier);
        }
        return true;
    }
    return false;
}

void StructInheritanceChecker::CreateBuiltInUnaryOperatorFunc(TokenKind op, ExtendDecl& ed) const
{
    TypeKind returnTyKind = GetBuiltinUnaryOpReturnKind(op, ed.ty->kind);
    auto returnTy = TypeManager::GetPrimitiveTy(returnTyKind);
    auto nothingTy = TypeManager::GetNothingTy();

    auto fd = MakeOwnedNode<FuncDecl>();
    fd->toBeCompiled = true; // For incremental compilation.
    fd->EnableAttr(Attribute::IN_EXTEND, Attribute::PUBLIC, Attribute::OPERATOR, Attribute::IMPLICIT_ADD);
    CopyBasicInfo(&ed, fd.get());
    fd->moduleName = ed.moduleName;
    fd->fullPackageName = ed.fullPackageName;
    fd->op = op;
    fd->identifier = SrcIdentifier{TOKENS[static_cast<int>(op)]};
    fd->ty = typeManager.GetFunctionTy({}, returnTy);
    fd->outerDecl = &ed;

    auto funcBody = MakeOwnedNode<FuncBody>();
    funcBody->paramLists.emplace_back(MakeOwnedNode<FuncParamList>());
    funcBody->funcDecl = fd.get();
    funcBody->ty = fd->ty;
    auto retType = MakeOwnedNode<PrimitiveType>();
    retType->kind = returnTyKind;
    retType->ty = returnTy;
    funcBody->retType = std::move(retType);

    auto block = MakeOwnedNode<Block>();
    block->ty = nothingTy;

    auto thisExpr = MakeOwnedNode<RefExpr>();
    thisExpr->isThis = true;
    thisExpr->ref.identifier = "this";
    thisExpr->ty = ed.ty;

    auto ue = MakeOwnedNode<UnaryExpr>();
    ue->op = op;
    ue->expr = std::move(thisExpr);
    ue->ty = returnTy;

    auto returnExpr = MakeOwnedNode<ReturnExpr>();
    returnExpr->expr = std::move(ue);
    returnExpr->ty = nothingTy;
    returnExpr->refFuncBody = funcBody.get();

    block->body.emplace_back(std::move(returnExpr));
    funcBody->body = std::move(block);
    fd->funcBody = std::move(funcBody);
    AddCurFile(*fd, ed.curFile);
    ed.members.push_back(std::move(fd));
}

void StructInheritanceChecker::CreateBuiltInBinaryOperatorFunc(
    TokenKind op, Ptr<Ty> rightTy, ExtendDecl& ed, TypeKind returnTyKind) const
{
    auto retTy = TypeManager::GetPrimitiveTy(returnTyKind);
    auto nothingTy = TypeManager::GetNothingTy();

    auto fd = MakeOwnedNode<FuncDecl>();
    fd->toBeCompiled = true; // For incremental compilation.
    fd->EnableAttr(Attribute::IN_EXTEND, Attribute::PUBLIC, Attribute::OPERATOR, Attribute::IMPLICIT_ADD);
    CopyBasicInfo(&ed, fd.get());
    fd->moduleName = ed.moduleName;
    fd->fullPackageName = ed.fullPackageName;
    fd->op = op;
    fd->identifier = SrcIdentifier{TOKENS[static_cast<int>(op)]};
    fd->ty = typeManager.GetFunctionTy({rightTy}, retTy);
    fd->outerDecl = &ed;

    auto funcBody = MakeOwnedNode<FuncBody>();
    funcBody->ty = fd->ty;
    funcBody->funcDecl = fd.get();
    auto retType = MakeOwnedNode<PrimitiveType>();
    retType->kind = returnTyKind;
    retType->ty = retTy;
    funcBody->retType = std::move(retType);

    auto rightParam = MakeOwnedNode<FuncParam>();
    rightParam->ty = rightTy;
    rightParam->identifier = "right";

    auto block = MakeOwnedNode<Block>();
    block->ty = nothingTy;

    auto leftExpr = MakeOwnedNode<RefExpr>();
    leftExpr->isThis = true;
    leftExpr->ref.identifier = "this";
    leftExpr->ty = ed.ty;

    auto rightExpr = MakeOwnedNode<RefExpr>();
    rightExpr->ref.identifier = "right";
    rightExpr->ref.target = rightParam.get();
    rightExpr->ty = rightTy;

    auto be = MakeOwnedNode<BinaryExpr>();
    be->op = op;
    be->leftExpr = std::move(leftExpr);
    be->rightExpr = std::move(rightExpr);
    be->ty = retTy;

    auto returnExpr = MakeOwnedNode<ReturnExpr>();
    returnExpr->expr = std::move(be);
    returnExpr->ty = nothingTy;
    returnExpr->refFuncBody = funcBody.get();

    auto paramList = MakeOwnedNode<FuncParamList>();
    paramList->params.emplace_back(std::move(rightParam));

    funcBody->paramLists.emplace_back(std::move(paramList));
    block->body.emplace_back(std::move(returnExpr));
    funcBody->body = std::move(block);
    fd->funcBody = std::move(funcBody);
    AddCurFile(*fd, ed.curFile);
    ed.members.push_back(std::move(fd));
}
