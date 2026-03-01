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
 * This file implements logics related to parsing in java FFI in Codira Native
 */
#include "../../ParserImpl.h"
#include "Codira/AST/Utils.h"

#include "JFFIParserImpl.h"

using namespace Codira;
using namespace AST;

void JFFIParserImpl::CheckAnnotation(const Annotation& anno) const
{
    if (anno.kind == AnnotationKind::JAVA_MIRROR) {
        CheckMirrorAnnoArgs(anno);
        CheckMirrorAnnoTarget(anno);
    } else {
        CODEC_ASSERT(anno.kind == AnnotationKind::JAVA_IMPL);
        CheckImplAnnoArgs(anno);
        CheckImplAnnoTarget(anno);
    }
}

void JFFIParserImpl::CheckMirrorAnnoTarget(const Annotation& anno) const
{
    if (p.SeeingAny({TokenKind::CLASS, TokenKind::INTERFACE})) {
        return;
    }

    if (anno.kind == AnnotationKind::JAVA_MIRROR) {
        auto& lah = p.lookahead;
        p.DiagUnexpectedAnnoOn(anno, lah.Begin(), anno.identifier, lah.Value());
    }
}

void JFFIParserImpl::CheckImplAnnoTarget(const Annotation& anno) const
{
    if (p.SeeingAny({TokenKind::CLASS, TokenKind::INTERFACE})) {
        return;
    }

    if (anno.kind == AnnotationKind::JAVA_IMPL) {
        auto& lah = p.lookahead;
        p.DiagUnexpectedAnnoOn(anno, lah.Begin(), anno.identifier, lah.Value());
    }
}

void JFFIParserImpl::CheckMirrorAnnoArgs(const Annotation& anno) const
{
    static const std::string JAVA_MIRROR_NAME = "@JavaMirror";
    p.ffiParser->CheckZeroOrSingleStringLitArgAnnotation(anno, JAVA_MIRROR_NAME);
}

void JFFIParserImpl::CheckImplAnnoArgs(const Annotation& anno) const
{
    static const std::string JAVA_IMPL_NAME = "@JavaImpl";
    p.ffiParser->CheckZeroOrSingleStringLitArgAnnotation(anno, JAVA_IMPL_NAME);
}

void JFFIParserImpl::CheckMirrorSignature(AST::ClassLikeDecl& decl, const PtrVector<Annotation>& annos) const
{
    CODEC_ASSERT(p.HasAnnotation(annos, AnnotationKind::JAVA_MIRROR));
    decl.EnableAttr(Attribute::JAVA_MIRROR);
    if (!decl.inheritedTypes.empty()) {
        decl.EnableAttr(Attribute::JAVA_MIRROR_SUBTYPE);
    }
    if (decl.TestAttr(Attribute::SEALED)) {
        DiagJavaMirrorCannotBeSealed(decl);
        decl.EnableAttr(Attribute::IS_BROKEN);
    }
}

void JFFIParserImpl::CheckImplSignature(AST::ClassLikeDecl& decl, const PtrVector<Annotation>& annos) const
{
    CODEC_ASSERT(p.HasAnnotation(annos, AnnotationKind::JAVA_IMPL));
    decl.EnableAttr(Attribute::JAVA_MIRROR_SUBTYPE);

    if (decl.GetGeneric() != nullptr) {
        DiagJavaImplCannotBeGeneric(decl);
        decl.EnableAttr(Attribute::IS_BROKEN);
    }

    if (decl.TestAttr(Attribute::ABSTRACT)) {
        DiagJavaImplCannotBeAbstract(decl);
        decl.EnableAttr(Attribute::IS_BROKEN);
    }

    if (decl.TestAttr(Attribute::SEALED)) {
        DiagJavaImplCannotBeSealed(decl);
        decl.EnableAttr(Attribute::IS_BROKEN);
    }

    if (decl.astKind == ASTKind::CLASS_DECL && decl.TestAttr(Attribute::OPEN)) {
        p.diag.DiagnoseRefactor(DiagKindRefactor::parse_java_impl_cannot_be_open, decl);
        decl.EnableAttr(Attribute::IS_BROKEN);
    } else if (decl.astKind == ASTKind::INTERFACE_DECL) {
        p.diag.DiagnoseRefactor(DiagKindRefactor::parse_java_impl_cannot_be_interface, decl);
        decl.EnableAttr(Attribute::IS_BROKEN);
    }
}

void JFFIParserImpl::CheckJavaHasDefaultAnnotation(const Annotation& anno) const
{
    if (p.Seeing(TokenKind::FUNC)) {
        return;
    }
    auto& lah = p.lookahead;
    p.DiagUnexpectedAnnoOn(anno, lah.Begin(), anno.identifier, lah.Value());
}

bool JFFIParserImpl::IsAbstractFunction(const FuncDecl& fd, const Decl& outerDecl) const
{
    auto hasAbstractModifier = p.HasModifier(fd.modifiers, TokenKind::ABSTRACT);
    auto hasStaticModifier = p.HasModifier(fd.modifiers, TokenKind::STATIC);
    auto hasOuterDeclAbstractModifier = p.HasModifier(outerDecl.modifiers, TokenKind::ABSTRACT);
    auto isOuterDeclInterface = outerDecl.astKind == ASTKind::INTERFACE_DECL;
    auto isOuterDeclClass = outerDecl.astKind == ASTKind::CLASS_DECL;
    auto isOuterDeclJavaImpl = Interop::Java::IsImpl(outerDecl);
    auto isAbstractInsideAbsractClass = hasAbstractModifier && isOuterDeclClass && hasOuterDeclAbstractModifier;
    auto isAbstractInsideInterface = isOuterDeclInterface && !hasStaticModifier;

    return (isAbstractInsideAbsractClass || isAbstractInsideInterface) && !isOuterDeclJavaImpl;
}
