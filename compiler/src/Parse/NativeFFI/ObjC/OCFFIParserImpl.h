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
 * This file declares class NativeFFIObjCParserImpl.
 */

#ifndef CODIRA_PARSE_NATIVEFFIOBJCPARSERIMPL_H
#define CODIRA_PARSE_NATIVEFFIOBJCPARSERIMPL_H

#include "Codira/Parse/Parser.h"

namespace Codira {
class OCFFIParserImpl final {
public:
    explicit OCFFIParserImpl(ParserImpl& parserImpl) : p(parserImpl)
    {
    }
    ~OCFFIParserImpl() = default;

    void CheckAnnotation(const AST::Annotation& anno, ScopeKind scopeKind) const;
    void CheckMirrorSignature(AST::ClassLikeDecl& decl, const PtrVector<AST::Annotation>& annos) const;
    void CheckImplSignature(AST::ClassLikeDecl& decl, const PtrVector<AST::Annotation>& annos) const;
    void CheckMirrorSignature(AST::FuncDecl& decl, const PtrVector<AST::Annotation>& annos) const; 

    // Make private, when checking @ObjCMirror/@ObjCImpl decl members moved to this class
    void DiagObjCMirrorCannotHaveFinalizer(const AST::Node& node) const;
    void DiagObjCMirrorMethodMustHaveForeignName(const AST::Node& node) const;
    void DiagObjCMirrorCtorMustHaveForeignName(const AST::Node& node) const;
    void DiagObjCMirrorCannotHavePrivateMember(const AST::Node& node) const;
    void DiagObjCMirrorCannotHaveConstMember(const AST::Node& node) const;
    void DiagObjCMirrorCannotHaveStaticInit(const AST::Node& node) const;
    void DiagObjCMirrorFieldCannotHaveInitializer(const AST::Node& node) const;
    void DiagObjCMirrorCannotHavePrimaryCtor(const AST::Node& node) const;
    void DiagObjCMirrorFieldCannotBeStatic(const AST::Node& node) const;

    void DiagObjCImplCannotHaveStaticInit(const AST::Node& node) const;
    void DiagObjCImplCannotBeGeneric(const AST::Node& node) const;

    void DiagObjCMirrorFuncCannotBeForeign(const AST::FuncDecl& node) const;
    void DiagObjCMirrorFuncCannotBeC(const AST::FuncDecl& node) const;
    void DiagObjCMirrorFuncCannotBeGeneric(const AST::FuncDecl& node) const;
    void DiagObjCMirrorFuncCannotHaveBody(const AST::FuncDecl& node) const;
    void DiagObjCMirrorFuncMustHaveExplicitType(const AST::FuncDecl& node) const;
    void DiagObjCMirrorFuncCannotBeConst(const AST::FuncDecl& node) const;
    void DiagObjCMirrorFuncMustBeTopLevel(const AST::FuncDecl& node) const;

private:
    void CheckMirrorAnnoArgs(const AST::Annotation& anno) const;
    void CheckImplAnnoArgs(const AST::Annotation& anno) const;

    void CheckMirrorAnnoTarget(const AST::Annotation& anno, ScopeKind scopeKind) const;
    void CheckImplAnnoTarget(const AST::Annotation& anno) const;

    void DiagObjCMirrorCannotBeSealed(const AST::Node& node) const;

    void DiagObjCImplCannotBeOpen(const AST::Node& node) const;
    void DiagObjCImplCannotBeInterface(const AST::Node& node) const;
    void DiagObjCImplCannotBeAbstract(const AST::Node& node) const;
    void DiagObjCImplCannotBeSealed(const AST::Node& node) const;

    ParserImpl& p;
};
} // namespace Codira

#endif // CODIRA_PARSE_NATIVEFFIOBJCPARSERIMPL_H
