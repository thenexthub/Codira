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
 * This file declares the helper functions used for incremental semantic checking.
 */

#ifndef CODIRA_SEMA_INCREMENTAL_UTILS_H
#define CODIRA_SEMA_INCREMENTAL_UTILS_H

#include <unordered_set>

#include "Codira/AST/Node.h"
#include "Codira/IncrementalCompilation/CompilationCache.h"

namespace Codira::Sema {
void MarkIncrementalCheckForCtor(const std::unordered_set<Ptr<AST::Decl>>& declsToBeReCompiled);
std::unordered_set<Ptr<const AST::StructTy>> CollectChangedStructTypes(
    const AST::Package& pkg, const std::unordered_set<Ptr<AST::Decl>>& declsToBeReCompiled);
void HandleCtorForIncr(const AST::Package& pkg, std::map<std::string, Ptr<AST::Decl>>& mangledName2DeclMap,
    SemanticInfo& usageCache);
std::string GetTypeRawMangleName(const AST::Ty& ty);
void CollectCompilerAddedDeclUsage(const AST::Package& pkg, SemanticInfo& usageCache);
void CollectRemovedMangles(const std::string& removed, SemanticInfo& semaInfo,
    std::unordered_set<std::string>& removedMangles);
void CollectRemovedManglesForReCompile(
    const AST::Decl& changed, SemanticInfo& semaInfo, std::unordered_set<std::string>& removedMangles);
} // namespace Codira::Sema

#endif
