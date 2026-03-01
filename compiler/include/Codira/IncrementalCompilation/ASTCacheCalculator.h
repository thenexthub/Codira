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

#ifndef CODIRA_AST_CACHE_CALCULATOR_H
#define CODIRA_AST_CACHE_CALCULATOR_H

#include "Codira/AST/Node.h"
#include "Codira/IncrementalCompilation/CompilationCache.h"

namespace Codira::IncrementalCompilation {

/// a class that computes the information necessary to be kept for further incremental compilation, and also
/// data that may later be used to analyse changed decls since last compilation.
class ASTCacheCalculator {
public:
    ASTCacheCalculator(const AST::Package& p, const std::pair<bool, bool>& srcInfo);
    ~ASTCacheCalculator();

    void Walk() const;

    RawMangled2DeclMap mangled2Decl{}; // RawMangledName -> Ptr<AST::Decl> map
    ASTCache ret{};
    std::unordered_set<Ptr<const AST::Decl>> duplicatedMangleNames{}; // decls with duplicate RawMangledName
    
    // store direct extends temporarily: direct extends with the same extended type and constraints are the same,
    // collect them while traversing the ast, compute their cache after extracting all direct extends with the
    // same RawMangledName
    std::unordered_map<RawMangledName, std::list<std::pair<Ptr<AST::ExtendDecl>, int>>> directExtends;
    std::vector<const AST::Decl*> order; // the order of global decls by which decls are written to the cache.
        // order of members need not record as they are recorded in the MemberCache struct
    FileMap fileMap;

private:
    OwnedPtr<class ASTCacheCalculatorImpl> impl;
};
} // namespace Codira::IncrementalCompilation

#endif
