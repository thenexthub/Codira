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
 * This file implements typecheck apis for annotations.
 */

#include "TypeCheckerImpl.h"

#include "Diags.h"
#include "Codira/AST/Clone.h"
#include "Codira/AST/Create.h"
#include "Codira/AST/RecoverDesugar.h"
#include "NativeFFI/Java/TypeCheck/TypeCheckAnnotation.h"

using namespace Codira;
using namespace AST;
using namespace Sema;

namespace {
void DiagAnnotationArgTarget(DiagnosticEngine& diag, const Node& node)
{
    diag.DiagnoseRefactor(DiagKindRefactor::sema_annotation_arg_target, node);
}

OwnedPtr<CallExpr> DesugarCustomAnnotation(Annotation& ann)
{
    if (!ann.baseExpr) {
        return nullptr;
    }
    auto callExpr = CreateCallExpr(std::move(ann.baseExpr), std::move(ann.args));
    callExpr->callKind = CallKind::CALL_ANNOTATION;
    CopyBasicInfo(&ann, callExpr.get());
    AddCurFile(*callExpr, ann.curFile);
    return callExpr;
}

void RecoverToCustomAnnotation(Annotation& ann, CallExpr& callExpr)
{
    RecoverToCallExpr(callExpr);
    if (callExpr.baseFunc->astKind == ASTKind::REF_EXPR) {
        auto re = StaticAs<ASTKind::REF_EXPR>(callExpr.baseFunc.get());
        if (re->callOrPattern == &callExpr) {
            re->callOrPattern = nullptr;
        }
    }
    ann.baseExpr = std::move(callExpr.baseFunc);
    ann.args = std::move(callExpr.args);
}

void DesugarAnnotationsArray(ImportManager& importManager, TypeManager& typeManager, Decl& decl,
    const std::vector<std::reference_wrapper<Annotation>>& annotations,
    std::vector<OwnedPtr<Expr>>&& annotationsArray)
{
    if (!annotationsArray.empty()) {
        auto objectClass = importManager.GetCoreDecl<ClassDecl>(OBJECT_NAME);
        auto arrayStruct = importManager.GetCoreDecl<StructDecl>(STD_LIB_ARRAY);
        if (objectClass != nullptr && arrayStruct != nullptr) {
            auto arrayTy = typeManager.GetStructTy(*arrayStruct, {objectClass->ty});
            decl.annotationsArray = CreateArrayLit(std::move(annotationsArray), arrayTy);
            decl.annotationsArray->EnableAttr(Attribute::IS_ANNOTATION);
        } else {
            CODEC_ASSERT(annotations.size() == annotationsArray.size());
            for (size_t i = 0; i < annotations.size(); ++i) {
                RecoverToCustomAnnotation(annotations[i], StaticCast<CallExpr&>(*annotationsArray[i]));
            }
        }
    }
}

bool CheckCustomAnnotationPlace(DiagnosticEngine& diag, const Decl& decl, const Annotation& ann)
{
    auto declsInType = [](const Decl& d) {
        return d.TestAnyAttr(Attribute::IN_CLASSLIKE, Attribute::IN_STRUCT, Attribute::IN_ENUM, Attribute::IN_EXTEND);
    };
    bool validCustomAnnotaionPlace = decl.IsNominalDecl() ||
        IsInstanceConstructor(decl) || decl.TestAttr(Attribute::ENUM_CONSTRUCTOR) || declsInType(decl) ||
        (decl.astKind == ASTKind::FUNC_PARAM && decl.outerDecl) ||
        (decl.TestAttr(Attribute::GLOBAL) && (Is<VarDeclAbstract>(decl) || Is<FuncDecl>(decl)));
    if (!validCustomAnnotaionPlace) {
        auto builder =
            diag.DiagnoseRefactor(DiagKindRefactor::sema_annotation_custom_place, MakeRangeForDeclIdentifier(decl));
        builder.AddHint(ann);
        builder.AddNote("custom annotations can only be used on non-local declarations");
    }
    return validCustomAnnotaionPlace;
}
} // namespace

void TypeChecker::TypeCheckerImpl::CheckAnnotationDecl(ASTContext& ctx, Annotation& ann)
{
    if (ann.TestAttr(Attribute::TOOL_ADD)) {
        return;
    }
    CODEC_ASSERT(ann.kind == AnnotationKind::ANNOTATION);
    if (ann.args.empty()) {
        return;
    }
    if (ann.args.size() > 1) {
        DiagAnnotationArgTarget(diag, ann);
        return;
    }
    CODEC_NULLPTR_CHECK(ann.args.front());
    auto& arg = *ann.args.front();
    if (arg.name != "target") {
        DiagAnnotationArgTarget(diag, arg);
        return;
    }
    CODEC_NULLPTR_CHECK(arg.expr);
    if (arg.expr->astKind != ASTKind::ARRAY_LIT) {
        diag.DiagnoseRefactor(DiagKindRefactor::sema_annotation_arg_target_array_lit, *arg.expr);
        return;
    }
    // Check arg.ty <: Array<AnnotationKind>
    auto arrayStruct = importManager.GetCoreDecl<StructDecl>("Array");
    auto annotationKindEnum = importManager.GetCoreDecl<EnumDecl>("AnnotationKind");
    if (arrayStruct == nullptr || annotationKindEnum == nullptr) {
        // Errors should have been reported if core package is not imported correctly.
        return;
    }
    auto targetTy = typeManager.GetStructTy(*arrayStruct, {typeManager.GetEnumTy(*annotationKindEnum)});
    (void)Check(ctx, targetTy, ann.args.front().get());
}


OwnedPtr<CallExpr> TypeChecker::TypeCheckerImpl::CheckCustomAnnotation(
    ASTContext& ctx, const Decl& decl, Annotation& ann)
{
    CODEC_ASSERT(ann.kind == AnnotationKind::CUSTOM);
    auto callExpr = DesugarCustomAnnotation(ann);
    if (!callExpr) {
        return nullptr;
    }
    if (Ty::IsTyCorrect(Synthesize(ctx, callExpr.get())) && CheckCustomAnnotationPlace(diag, decl, ann)) {
        CODEC_ASSERT(callExpr->ty->IsClass());
        // The args information needs to be save into codeo. The original node need to be recover.
        std::vector<OwnedPtr<FuncArg>> args = {};
        for (auto& arg : callExpr->args) {
            args.emplace_back(ASTCloner::Clone(arg.get()));
        }
        ann.baseExpr = ASTCloner::Clone(callExpr->baseFunc.get());
        ann.args = std::move(args);
        return callExpr;
    }
    RecoverToCustomAnnotation(ann, *callExpr);
    return nullptr;
}

void TypeChecker::TypeCheckerImpl::CheckAnnotations(ASTContext& ctx, Decl& decl)
{
    std::vector<std::reference_wrapper<Annotation>> annotations;
    std::vector<OwnedPtr<Expr>> annotationsArray;
    for (auto& anno : decl.annotations) {
        CODEC_NULLPTR_CHECK(anno);
        switch (anno->kind) {
            case AnnotationKind::ANNOTATION: {
                CheckAnnotationDecl(ctx, *anno);
                break;
            }
            case AnnotationKind::CUSTOM: {
                auto callExpr = CheckCustomAnnotation(ctx, decl, *anno);
                if (!callExpr) {
                    break;
                }
                CODEC_ASSERT(callExpr && Ty::IsTyCorrect(callExpr->ty));
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
                if (!anno->isCompileTimeVisible) {
                    // this special attribute is to tell CHIR that this attr isCompileTimeVisible when computing
                    // annotations not visible during compile time
                    // it should have name NO_COMPILE_TIME_INFO, but there is no such attribute. Virtually any
                    // attribute that cannot be used on const eval expr works here.
                    callExpr->EnableAttr(Attribute::NO_REFLECT_INFO);
                }
#endif
                annotationsArray.emplace_back(std::move(callExpr));
                annotations.emplace_back(*anno);
                break;
            }
            case AnnotationKind::C:
            case AnnotationKind::CALLING_CONV:
            case AnnotationKind::FASTNATIVE:
            case AnnotationKind::FROZEN: {
                break;
            }
            case AnnotationKind::JAVA_HAS_DEFAULT: {
                Interop::Java::CheckJavaHasDefaultAnnotation(diag, *anno, decl);
                break;
            }
            default: {
                for (auto& arg : anno->args) {
                    (void)Synthesize(ctx, arg.get());
                }
            }
        }
    }
    DesugarAnnotationsArray(importManager, typeManager, decl, annotations, std::move(annotationsArray));
}
