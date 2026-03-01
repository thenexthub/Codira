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
 * This file declares class NativeFFIJavaParserImpl
 */

#ifndef CODIRA_PARSE_NATIVEFFIJAVAPARSERIMPL_H
#define CODIRA_PARSE_NATIVEFFIJAVAPARSERIMPL_H

#include "Codira/Parse/Parser.h"

namespace Codira {
using namespace AST;

class JFFIParserImpl final {
public:
    explicit JFFIParserImpl(ParserImpl& parserImpl): p(parserImpl)
    {
    }
    ~JFFIParserImpl() = default;

    void CheckAnnotation(const Annotation& anno) const;
    void CheckMirrorSignature(ClassLikeDecl& decl, const PtrVector<Annotation>& annos) const;
    void CheckImplSignature(ClassLikeDecl& decl, const PtrVector<Annotation>& annos) const;
    void CheckJavaHasDefaultAnnotation(const Annotation& anno) const;
    bool IsAbstractFunction(const FuncDecl& fd, const Decl& outerDecl) const;

    void DiagJavaMirrorCannotHaveFinalizer(const Node& node) const;
    void DiagJavaMirrorCannotHavePrivateMember(const Node& node) const;
    void DiagJavaMirrorCannotHaveStaticInit(const Node& node) const;
    void DiagJavaMirrorCannotHaveConstMember(const Node& node) const;
    void DiagJavaImplCannotBeGeneric(const Node& node) const;
    void DiagJavaImplCannotBeAbstract(const Node& node) const;
    void DiagJavaImplCannotBeSealed(const Node& node) const;
    void DiagJavaMirrorCannotBeSealed(const Node& node) const;
    void DiagJavaImplCannotHaveStaticInit(const Node& node) const;

private:
    void CheckMirrorAnnoArgs(const Annotation& anno) const;
    void CheckImplAnnoArgs(const Annotation& anno) const;

    void CheckMirrorAnnoTarget(const Annotation& anno) const;
    void CheckImplAnnoTarget(const Annotation& anno) const;

    // Diag report
    void DiagOuterDeclMissMatch(const Node& node,
        const std::string& p0, const std::string& p1, const std::string& p2, const std::string& p3) const;
private:
    ParserImpl& p;
    bool compileCommon{false}; // true if compiling common part
    bool compilePlatform{false}; // true if compiling platform part
};

} // namespace Codira

#endif
