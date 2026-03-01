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
 * This file implements logics related to parsing in Objective-C FFI in Codira Native
 */

#include "OCFFIParserImpl.h"
#include "../../ParserImpl.h"
#include "Codira/AST/Utils.h"

using namespace Codira;
using namespace AST;

void OCFFIParserImpl::CheckAnnotation(const Annotation& anno, ScopeKind scopeKind) const
{
    if (anno.kind == AnnotationKind::OBJ_C_MIRROR) {
        CheckMirrorAnnoArgs(anno);
        CheckMirrorAnnoTarget(anno, scopeKind);
    } else {
        CODEC_ASSERT(anno.kind == AnnotationKind::OBJ_C_IMPL);
        CheckImplAnnoArgs(anno);
        CheckImplAnnoTarget(anno);
    }
}

void OCFFIParserImpl::CheckMirrorSignature(ClassLikeDecl& decl, const PtrVector<Annotation>& annos) const
{
    CODEC_ASSERT(p.HasAnnotation(annos, AnnotationKind::OBJ_C_MIRROR));
    decl.EnableAttr(Attribute::OBJ_C_MIRROR);

    if (decl.TestAttr(Attribute::SEALED)) {
        DiagObjCMirrorCannotBeSealed(decl);
        decl.EnableAttr(Attribute::IS_BROKEN);
    }
}

void OCFFIParserImpl::CheckMirrorSignature(FuncDecl& decl, const PtrVector<Annotation>& annos) const
{
    CODEC_ASSERT(p.HasAnnotation(annos, AnnotationKind::OBJ_C_MIRROR));
    decl.EnableAttr(Attribute::OBJ_C_MIRROR);

    if (decl.TestAttr(Attribute::FOREIGN)) {
        DiagObjCMirrorFuncCannotBeForeign(decl);
        decl.EnableAttr(Attribute::IS_BROKEN);
    }

    if (decl.GetGeneric() != nullptr) {
        DiagObjCMirrorFuncCannotBeGeneric(decl);
        decl.EnableAttr(Attribute::IS_BROKEN);
    }

    if (decl.funcBody->body != nullptr) {
        DiagObjCMirrorFuncCannotHaveBody(decl);
        decl.EnableAttr(Attribute::IS_BROKEN);
    }

    if (decl.funcBody->retType == nullptr) {
        DiagObjCMirrorFuncMustHaveExplicitType(decl);
        decl.EnableAttr(Attribute::IS_BROKEN);
    }

    if (decl.isConst || p.HasModifier(decl.modifiers, TokenKind::CONST)) {
        DiagObjCMirrorFuncCannotBeConst(decl);
        decl.EnableAttr(Attribute::IS_BROKEN);
    }

    if (decl.outerDecl != nullptr) {
        DiagObjCMirrorFuncMustBeTopLevel(decl);
        decl.EnableAttr(Attribute::IS_BROKEN);
    }
}

void OCFFIParserImpl::CheckImplSignature(ClassLikeDecl& decl, const PtrVector<Annotation>& annos) const
{
    CODEC_ASSERT(p.HasAnnotation(annos, AnnotationKind::OBJ_C_IMPL));
    decl.EnableAttr(Attribute::OBJ_C_MIRROR_SUBTYPE);

    if (decl.GetGeneric() != nullptr) {
        DiagObjCImplCannotBeGeneric(decl);
        decl.EnableAttr(Attribute::IS_BROKEN);
    }

    if (decl.TestAttr(Attribute::ABSTRACT)) {
        DiagObjCImplCannotBeAbstract(decl);
        decl.EnableAttr(Attribute::IS_BROKEN);
    }

    if (decl.TestAttr(Attribute::SEALED)) {
        DiagObjCImplCannotBeSealed(decl);
        decl.EnableAttr(Attribute::IS_BROKEN);
    }

    if (decl.astKind == ASTKind::CLASS_DECL && decl.TestAttr(Attribute::OPEN)) {
        DiagObjCImplCannotBeOpen(decl);
        decl.EnableAttr(Attribute::IS_BROKEN);
    }

    if (decl.astKind == ASTKind::INTERFACE_DECL) {
        DiagObjCImplCannotBeInterface(decl);
        decl.EnableAttr(Attribute::IS_BROKEN);
    }
}

void OCFFIParserImpl::CheckMirrorAnnoArgs(const Annotation& anno) const
{
    static const std::string OBJC_MIRROR_NAME = "@ObjCMirror";
    p.ffiParser->CheckZeroOrSingleStringLitArgAnnotation(anno, OBJC_MIRROR_NAME);
}

void OCFFIParserImpl::CheckImplAnnoArgs(const Annotation& anno) const
{
    static const std::string OBJC_IMPL_NAME = "@ObjCImpl";

    p.ffiParser->CheckZeroOrSingleStringLitArgAnnotation(anno, OBJC_IMPL_NAME);
}

void OCFFIParserImpl::CheckMirrorAnnoTarget(const Annotation& anno, ScopeKind scopeKind) const
{
    if (p.SeeingAny({TokenKind::CLASS, TokenKind::INTERFACE})) {
        return;
    }
    if (p.Seeing(TokenKind::FUNC) && scopeKind == ScopeKind::TOPLEVEL) {
        return;
    }

    if (anno.kind == AnnotationKind::OBJ_C_MIRROR) {
        auto& lah = p.lookahead;
        auto qualifier = (scopeKind == ScopeKind::TOPLEVEL)? "": "member or local ";
        p.DiagUnexpectedAnnoOn(anno, lah.Begin(), anno.identifier, qualifier + lah.Value());
    }
}

void OCFFIParserImpl::CheckImplAnnoTarget(const Annotation& anno) const
{
    if (p.SeeingAny({TokenKind::CLASS, TokenKind::INTERFACE})) {
        return;
    }

    if (anno.kind == AnnotationKind::OBJ_C_IMPL) {
        auto& lah = p.lookahead;
        p.DiagUnexpectedAnnoOn(anno, lah.Begin(), anno.identifier, lah.Value());
    }
}
