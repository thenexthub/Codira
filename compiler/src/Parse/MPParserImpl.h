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
 * This file declares class MPParserImpl.
 */

#ifndef CODIRA_PARSE_MPPARSERIMPL_H
#define CODIRA_PARSE_MPPARSERIMPL_H

#include "Codira/Parse/Parser.h"

namespace Codira {
using namespace AST;
class MPParserImpl final {
public:
    explicit MPParserImpl(const ParserImpl& parserImpl): ref(&parserImpl)
    {
    }
    ~MPParserImpl() = default;

    // set compile options for codemp.
    void SetCompileOptions(const GlobalOptions& opts);
    // Check CODEMP modifier rules.
    bool CheckCODEMPModifiers(const std::set<AST::Modifier> &modifiers) const;
    // The entry of checking CODEMP decl rules.
    void CheckCODEMPDecl(AST::Decl& decl) const;
    // Check whether the given modifier is a CODEMP modifier (COMMON or PLATFORM).
    bool HasCODEMPModifiers(const AST::Modifier& modifier) const;
    // Check presence of ctor in common class/struct
    void CheckCODEMPCtorPresence(const AST::Decl& decl) const;

private:
    bool CheckCODEMPModifiersOf(const AST::Decl& decl) const;
    bool CheckCODEMPModifiersBetween(const AST::Decl& inner, const AST::Decl& outer) const;
    void CheckCODEMPFuncParams(AST::Decl& decl, const Ptr<AST::FuncBody> funcBody) const;
    void CheckPlatformInterface(const AST::InterfaceDecl& decl) const;
    // Diag report
    void DiagOuterDeclMissMatch(const AST::Node& node,
        const std::string& p0, const std::string& p1, const std::string& p2, const std::string& p3) const;
private:
    const ParserImpl* ref;
    bool compileCommon{false}; // true if compiling common part
    bool compilePlatform{false}; // true if compiling platform part
};

} // namespace Codira
#endif
