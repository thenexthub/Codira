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
 * This file contains logics related to Codira Native interoperation with Java, Objective-C
 */

#include "../ParserImpl.h"
#include "Codira/AST/Match.h"
#include "Codira/AST/Utils.h"

#include "FFIParserImpl.h"

namespace Codira {
using namespace AST;

namespace {
bool IsLitString(Ptr<Expr> expr)
{
    if (!expr || expr->astKind != ASTKind::LIT_CONST_EXPR) {
        return false;
    }
    auto lce = DynamicCast<LitConstExpr>(expr);
    if (!lce) {
        return false;
    }

    if (lce->kind != LitConstKind::STRING) {
        return false;
    }

    return true;
}

} // namespace

void DiagConflictingAnnos(DiagnosticEngine &diag, Annotation &first, Annotation &second)
{
    auto minPos = first.begin < second.begin ? first.begin : second.begin;
    auto maxPos = first.end > second.end ? first.end : second.end;

    diag.DiagnoseRefactor(
        DiagKindRefactor::parse_conflict_annotation,
        MakeRange(minPos, maxPos),
        first.identifier, second.identifier);
}

void FFIParserImpl::CheckAnnotationsConflict(const PtrVector<Annotation>& annos) const
{
    std::vector<Ptr<Annotation>> candidates;
    for (auto& it : annos) {
        if (CONFLICTED_FFI_ANNOS.find(it->kind) != CONFLICTED_FFI_ANNOS.end()) {
            candidates.push_back(it.get());
        }
    }

    if (candidates.size() < 2) {
        return;
    }

    // report diag only in relation with first annotation
    auto first = *candidates.begin();
    for (auto conflicted = std::next(candidates.begin()); conflicted != candidates.end(); ++conflicted) {
        DiagConflictingAnnos(p.diag, *first, **conflicted);
    }
}

void FFIParserImpl::CheckAnnotations(const PtrVector<Annotation>& annos, ScopeKind scopeKind) const
{
    for (auto& it : annos) {
        auto& anno = *it;
        switch (it->kind) {
            case AnnotationKind::JAVA_MIRROR:
            case AnnotationKind::JAVA_IMPL: {
                jp.CheckAnnotation(anno);
                break;
            }
            case AnnotationKind::FOREIGN_NAME: {
                CheckForeignNameAnnoArgs(anno);
                CheckForeignNameAnnoTarget(anno);
                break;
            }
            case AnnotationKind::JAVA_HAS_DEFAULT: {
                jp.CheckJavaHasDefaultAnnotation(anno);
                break;
            }
            case AnnotationKind::OBJ_C_MIRROR:
            case AnnotationKind::OBJ_C_IMPL: {
                op.CheckAnnotation(anno, scopeKind);
                break;
            }
            default: continue;
        }
    }

    CheckAnnotationsConflict(annos);
}

void FFIParserImpl::CheckForeignNameAnnoTarget(const Annotation& anno) const
{
    if (p.SeeingAny({TokenKind::FUNC, TokenKind::PROP, TokenKind::INIT, TokenKind::LET,
        TokenKind::VAR, TokenKind::CONST})) {
        return;
    }

    if (anno.kind == AnnotationKind::FOREIGN_NAME) {
        auto& lah = p.lookahead;
        p.DiagUnexpectedAnnoOn(anno, lah.Begin(), anno.identifier, lah.Value());
    }
}

void FFIParserImpl::CheckForeignNameAnnoArgs(const Annotation& anno) const
{
    static const std::string FOREIGN_NAME_NAME = "@ForeignName";

    if (anno.args.size() != 1 || !IsLitString(anno.args[0]->expr)) {
        p.DiagAnnotationExpectsOneArgument(anno, FOREIGN_NAME_NAME, "'String' literal");
        return;
    }
}

void FFIParserImpl::CheckForeignNameAnnotation(Decl& decl) const
{
    CODEC_ASSERT(decl.IsFuncOrProp() || decl.astKind == ASTKind::VAR_DECL);
    for (auto& it : decl.annotations) {
        if (it->kind != AnnotationKind::FOREIGN_NAME) {
            continue;
        }
        if (!decl.outerDecl || !decl.outerDecl->TestAnyAttr(Attribute::JAVA_MIRROR, Attribute::JAVA_MIRROR_SUBTYPE,
            Attribute::OBJ_C_MIRROR, Attribute::OBJ_C_MIRROR_SUBTYPE)) {
                p.diag.DiagnoseRefactor(DiagKindRefactor::parse_foreign_name_on_ffi_decl_member, decl);
                decl.EnableAttr(Attribute::IS_BROKEN);
                return;
        }
    }
}

JFFIParserImpl FFIParserImpl::Java() const
{
    return jp;
}

OCFFIParserImpl FFIParserImpl::ObjC() const
{
    return op;
}

void FFIParserImpl::CheckZeroOrSingleStringLitArgAnnotation(const AST::Annotation &anno,
                                                            const std::string &annotationName) const
{
    if (anno.args.size() == 0) {
        return;
    }

    if (anno.args.size() > 1 || !IsLitString(anno.args[0]->expr)) {
        p.DiagAnnotationMoreThanOneArgs(anno, annotationName, "'String' literal");
        return;
    }
}

void FFIParserImpl::CheckClassLikeSignature(AST::ClassLikeDecl& decl, const PtrVector<Annotation>& annos) const
{
    for (auto& anno : annos) {
        switch (anno->kind) {
            case AnnotationKind::JAVA_MIRROR:
                jp.CheckMirrorSignature(decl, annos);
                break;
            case AnnotationKind::JAVA_IMPL:
                jp.CheckImplSignature(decl, annos);
                break;
            case AnnotationKind::OBJ_C_MIRROR:
                op.CheckMirrorSignature(decl, annos);
                break;
            case AnnotationKind::OBJ_C_IMPL:
                op.CheckImplSignature(decl, annos);
                break;
            default: break;
        }
    }
}


void FFIParserImpl::CheckFuncSignature(AST::FuncDecl& decl, const PtrVector<Annotation>& annos) const
{
    for (auto& anno : annos) {
        switch (anno->kind) {
            case AnnotationKind::OBJ_C_MIRROR:
                op.CheckMirrorSignature(decl, annos);
                break;
            default: break;
        }
    }
}


namespace Native::FFI {

} // namespace Native::FFI
} // namespace Codira
