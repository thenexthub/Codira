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
 *  file implements functions to find all reference node of the specific decl.
 */
#include "FindDeclUsage.h"
#include "Codira/AST/Match.h"
using namespace CONSTANTS;
using namespace Codira;
using namespace AST;
namespace ark {
bool CheckDeclEqual(const Decl& source, const Decl& target);

bool CheckTypeEqual(Ty& src, Ty& target)
{
    bool basicInvalid = src.kind != target.kind || src.typeArgs.size() != target.typeArgs.size();
    if (basicInvalid) {
        return false;
    }
    auto srcDecl = Ty::GetDeclPtrOfTy(&src);
    auto targetDecl = Ty::GetDeclPtrOfTy(&target);
    if ((srcDecl && !targetDecl) || (targetDecl && !srcDecl)) {
        return false;
    }
    if (srcDecl && targetDecl) {
        if (!CheckDeclEqual(*srcDecl, *targetDecl)) {
            return false;
        }
    }
    if (ArrayTy* srcArrTy = dynamic_cast<ArrayTy*>(&src)) {
        auto targetArrTy = dynamic_cast<ArrayTy*>(&target);
        if (!targetArrTy || srcArrTy->dims != targetArrTy->dims) {
            return false;
        }
    } else if (src.kind == TypeKind::TYPE_FUNC) {
        auto srcFuncTy = dynamic_cast<FuncTy*>(&src);
        auto targetFuncTy = dynamic_cast<FuncTy*>(&target);
        if (!srcFuncTy || !targetFuncTy || srcFuncTy->isC != targetFuncTy->isC) {
            return false;
        }
    }
    for (size_t i = 0; i < src.typeArgs.size(); ++i) {
        if (!src.typeArgs[i] || !target.typeArgs[i]) {
            return false;
        }
        if (!CheckTypeEqual(*src.typeArgs[i], *target.typeArgs[i])) {
            return false;
        }
    }
    return true;
}

bool CheckParamListEqual(const FuncParamList& srcList, const FuncParamList& targetList)
{
    if (srcList.params.size() != targetList.params.size()) {
        return false;
    }

    for (size_t i = 0; i < srcList.params.size(); ++i) {
        auto& src = srcList.params[i];
        auto& target = targetList.params[i];
        bool invalid = !src || !src->ty || !target || !target->ty;
        if (invalid) {
            return false;
        }
        // Since type nodes in imported decls are wrapped refType, we can only compare sema type here.
        if (src->ty == nullptr || target->ty == nullptr || !CheckTypeEqual(*src->ty, *target->ty)) {
            return false;
        }
    }
    return true;
}

bool CheckFunctionEqual(const FuncDecl& srcFunc, const FuncDecl& targetFunc)
{
    if (!srcFunc.funcBody || !targetFunc.funcBody) {
        return false; // Broken functions are not equal.
    }
    if (srcFunc.funcBody->paramLists.size() != targetFunc.funcBody->paramLists.size()) {
        return false;
    }
    for (size_t i = 0; i < srcFunc.funcBody->paramLists.size(); ++i) {
        if (!CheckParamListEqual(*srcFunc.funcBody->paramLists[i], *targetFunc.funcBody->paramLists[i])) {
            return false;
        }
    }
    return true;
}

Ptr<const Decl> GetDefinedDecl(Ptr<const Decl> decl)
{
    auto fd = dynamic_cast<const FuncDecl*>(decl.get());
    if (fd && fd->propDecl) {
        return fd->propDecl;
    }
    return decl;
}

bool CheckDeclEqual(const Decl& source, const Decl& target)
{
    auto srcDecl = GetDefinedDecl(&source);
    auto targetDecl = GetDefinedDecl(&target);

    bool basicInvalid = srcDecl->fullPackageName != targetDecl->fullPackageName ||
                        srcDecl->identifier != targetDecl->identifier || srcDecl->astKind != targetDecl->astKind ||
                        srcDecl->TestAttr(Codira::AST::Attribute::GLOBAL) !=
                        targetDecl->TestAttr(Codira::AST::Attribute::GLOBAL);
    if (basicInvalid) {
        return false;
    }

    // Case 1: source decl is typeDecl.
    if (srcDecl->IsTypeDecl()) {
        // If the decl is type decl and usage is in different package with definition, two decls should be equal.
        return true;
    }
    // Case 2: source decl is extendDecl.
    if (srcDecl->astKind == ASTKind::EXTEND_DECL) {
        auto srcEd = dynamic_cast<const ExtendDecl*>(srcDecl.get());
        auto targetEd = dynamic_cast<const ExtendDecl*>(targetDecl.get());
        bool invalid = !srcEd || !srcEd->extendedType || !srcEd->extendedType->ty ||
                        !targetEd || !targetEd->extendedType || !targetEd->extendedType->ty;
        if (invalid) {
            return false;
        }
        return CheckTypeEqual(*srcEd->extendedType->ty, *targetEd->extendedType->ty);
    }
    // 'source' and 'target' must both have 'outerDecl' or both without 'outerDecl'.
    bool invalidContext = (srcDecl->outerDecl && !targetDecl->outerDecl) ||
                            (targetDecl->outerDecl && !srcDecl->outerDecl);
    if (invalidContext) {
        return false;
    }
    // Case 3: global or member none-function decl.
    if (srcDecl->astKind != ASTKind::FUNC_DECL) {
        bool equal = !srcDecl->outerDecl || CheckDeclEqual(*srcDecl->outerDecl, *targetDecl->outerDecl);
        return equal;
    }
    // Case 4: global or member functions that have same context.
    if (srcDecl->outerDecl) {
        auto outerEqual = CheckDeclEqual(*srcDecl->outerDecl, *targetDecl->outerDecl);
        if (!outerEqual) {
            return false;
        }
    }
    auto srcFuncDecl = dynamic_cast<const FuncDecl*>(srcDecl.get());
    auto targetFuncDecl = dynamic_cast<const FuncDecl*>(targetDecl.get());
    return srcFuncDecl && targetFuncDecl && CheckFunctionEqual(*srcFuncDecl, *targetFuncDecl);
}

Ptr<Node> GetRealNode(Ptr<Node> node)
{
    Ptr<Expr> expr = dynamic_cast<Expr*>(node.get());
    Ptr<MemberAccess> ma = dynamic_cast<MemberAccess*>(node.get());
    if (expr && expr->sourceExpr) {
        return expr->sourceExpr;
    } else if (ma && BUILTIN_OPERATORS.count(ma->field)) {
        if (auto ce = DynamicCast<CallExpr*>(ma->callOrPattern); ce && ce->sourceExpr) {
            return ce->sourceExpr;
        }
    }
    return node;
}

bool checkMacroFunc(const Decl &decl, Ptr<const Decl> target)
{
    return target &&
            decl.isInMacroCall &&
            decl.ty == target->ty &&
            decl.ty->kind == TypeKind::TYPE_FUNC &&
            decl.identifier.Val() == target->identifier.Val();
}

std::unordered_set<Ptr<Node>> FindUsage(const Decl& decl, Node& root, bool isRename = false)
{
    std::unordered_set<Ptr<Node>> results;
    // Two decls only can be considered in same package ast when package node are equal.
    auto searchingPkg = root.astKind == ASTKind::PACKAGE ? &root : (root.curFile ? root.curFile->curPackage : nullptr);
    bool inSamePkg = decl.curFile && decl.curFile->curPackage == searchingPkg;
    // When the decl is locally defined decl, it can only be found in same package.
    bool partialDecl = (!decl.outerDecl && !decl.TestAttr(Codira::AST::Attribute::GLOBAL)) ||
                       (decl.outerDecl && decl.outerDecl->astKind == ASTKind::FUNC_DECL);
    std::function<VisitAction(Ptr<Node>)> collector =
            [&results, &decl, &collector, inSamePkg, partialDecl, isRename](Ptr<Node> node) {
        if (auto fileNode = DynamicCast<File*>(node)) {
            for (auto &it : fileNode->originalMacroCallNodes) {
                Walker(it.get(), collector).Walk();
            }
            for (auto &it : fileNode->trashBin) {
                Walker(it.get(), collector).Walk();
            }
        }

        auto target = node->GetTarget();
        auto ref = dynamic_cast<NameReferenceExpr*>(node.get());
        bool isReNameTypeName = isRename && ref && ref->aliasTarget && ref->aliasTarget == &decl;
        bool isRealDecl = !isRename || !ref || !(ref->aliasTarget) || ref->aliasTarget == &decl;
        if (target == &decl || checkMacroFunc(decl, target) ||
                (target && target->begin == decl.begin && target->end == decl.end && target->curFile == decl.curFile)) {
            if (isRealDecl) {
                results.emplace(GetRealNode(node));
            }
        } else if (isReNameTypeName) {
            results.emplace(GetRealNode(node));
        }
        bool continueNext = !target || inSamePkg || partialDecl;
        if (continueNext) {
            return VisitAction::WALK_CHILDREN;
        }
        if (CheckDeclEqual(decl, *target)) {
            if (isRealDecl) {
                results.emplace(GetRealNode(node));
            }
        } else if (isReNameTypeName && CheckDeclEqual(decl, *ref->aliasTarget)) {
            results.emplace(GetRealNode(node));
        }
        return VisitAction::WALK_CHILDREN;
    };
    Walker(&root, collector).Walk();
    return results;
}

std::unordered_set<Ptr<Node>> FindNamedFuncParamUsage(const FuncParam& fp, Node& root)
{
    std::unordered_set<Ptr<Node>> results = FindUsage(fp, root);
    bool returnNow = !fp.isNamedParam || !fp.outerDecl || fp.outerDecl->astKind != ASTKind::FUNC_DECL;
    if (returnNow) {
        return results;
    }
    auto fdCandidates = FindUsage(*fp.outerDecl, root);
    for (auto expr: fdCandidates) {
        Ptr<CallExpr> ce = nullptr;
        if (auto ref = DynamicCast<NameReferenceExpr*>(expr.get())) {
            if (auto outer = DynamicCast<CallExpr*>(ref->callOrPattern)) {
                ce = outer;
            }
        }
        if (!ce) {
            continue;
        }
        for (auto &arg: ce->args) {
            if (arg && arg->name == fp.identifier) {
                results.emplace(arg.get());
            }
        }
    }
    return results;
}

std::unordered_set<Ptr<Node> > FindDeclUsage(const Decl &decl, Node &root, bool isRename)
{
    Ptr<const FuncParam> fp = dynamic_cast<const FuncParam*>(&decl);
    if (fp) {
        return FindNamedFuncParamUsage(*fp, root);
    } else {
        return FindUsage(decl, root, isRename);
    }
}
} // namespace ark
