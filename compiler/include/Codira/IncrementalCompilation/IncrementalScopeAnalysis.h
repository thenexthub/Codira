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

#ifndef CODIRA_INCRE_SCOPE_ANALYSIS_H
#define CODIRA_INCRE_SCOPE_ANALYSIS_H

#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Codira/AST/Node.h"
#include "Codira/IncrementalCompilation/CompilationCache.h"
#include "Codira/Modules/ImportManager.h"

namespace Codira {

enum class IncreKind { NO_CHANGE, INCR, ROLLBACK, EMPTY_PKG, INVALID };

struct IncreResult {
    IncreKind kind;
    std::unordered_set<Ptr<AST::Decl>> declsToRecompile;
    std::list<RawMangledName> deleted;
    std::list<std::string> deletedMangleNames; // deleted mangle names for codegen
    CompilationCache cacheInfo;
    RawMangled2DeclMap mangle2decl;
    std::list<RawMangledName> reBoxedTypes;
    void Dump() const;
};

// A helper struct to organize the args of incremental scope analysis entry function
struct IncrementalScopeAnalysisArgs {
    const RawMangled2DeclMap rawMangleName2DeclMap;
    ASTCache&& astCacheInfo;
    const AST::Package& srcPackage;
    const GlobalOptions& op;
    const ImportManager& importer;
    const CompilationCache& cachedInfo;
    const FileMap& fileMap;
    std::unordered_map<RawMangledName, std::list<std::pair<Ptr<AST::ExtendDecl>, int>>>&& directExtends;
};

// Entry function of incremental scope analysis
IncreResult IncrementalScopeAnalysis(IncrementalScopeAnalysisArgs&& args);
} // namespace Codira
#endif // CODIRA_INCRE_SCOPE_ANALYSIS_H
