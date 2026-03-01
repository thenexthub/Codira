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
 * This file declares parser enhancement for Codira Native FFI with Java / Objective-C
 */

#ifndef CODIRA_PARSE_NATIVEFFIPARSERIMPL_H
#define CODIRA_PARSE_NATIVEFFIPARSERIMPL_H

#include "Codira/Parse/Parser.h"
#include "Java/JFFIParserImpl.h"
#include "ObjC/OCFFIParserImpl.h"

namespace Codira {
using namespace AST;

class FFIParserImpl final {
public:
    explicit FFIParserImpl(ParserImpl& parserImpl): p(parserImpl), jp(parserImpl), op(parserImpl)
    {
    }
    ~FFIParserImpl() = default;

    void CheckAnnotationsConflict(const PtrVector<Annotation>& annos) const;
    void CheckAnnotations(const PtrVector<Annotation>& annos, ScopeKind scopeKind) const;
    void CheckForeignNameAnnotation(Decl& decl) const;
    void CheckZeroOrSingleStringLitArgAnnotation(const AST::Annotation &anno, const std::string &annotationName) const;

    void CheckClassLikeSignature(AST::ClassLikeDecl& decl, const PtrVector<Annotation>& annos) const;
    void CheckFuncSignature(AST::FuncDecl& decl, const PtrVector<Annotation>& annos) const;

    JFFIParserImpl Java() const;
    OCFFIParserImpl ObjC() const;

private:
    // friend JFFIParserImpl;
    void CheckForeignNameAnnoArgs(const Annotation& anno) const;
    void CheckForeignNameAnnoTarget(const Annotation& anno) const;

    ParserImpl& p;
    JFFIParserImpl jp;
    OCFFIParserImpl op;
};

void DiagConflictingAnnos(DiagnosticEngine& diag, Annotation& first, Annotation& second);

const std::unordered_set<AnnotationKind> CONFLICTED_FFI_ANNOS {
    AnnotationKind::JAVA_MIRROR, AnnotationKind::JAVA_IMPL,
    AnnotationKind::OBJ_C_IMPL, AnnotationKind::OBJ_C_MIRROR,
    AnnotationKind::C, AnnotationKind::JAVA
};

namespace Native::FFI {

} // namespace Native::FFI
} // namespace Codira

#endif // CODIRA_PARSE_NATIVEFFIPARSERIMPL_H
