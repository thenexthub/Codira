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

#ifndef CODIRA_INCREMENTAL_COMPILATION_CACHED_MANGLE_MAP
#define CODIRA_INCREMENTAL_COMPILATION_CACHED_MANGLE_MAP

#include <string>
#include <unordered_set>
#include "Codira/AST/Node.h"
#include "Codira/IncrementalCompilation/IncrementalCompilationLogger.h"

/// Describes which decls are new or removed compared to last compilation. The decls are recorded by their mangled
/// names.
struct CachedMangleMap {
    // Stored mangledName of decls which need be removed from IR.
    // NOTE: mangled names should be CodeGen recognizable.
    std::unordered_set<std::string> incrRemovedDecls;
    // imported inline decls, to be set external and non-dso_local
    std::unordered_set<std::string> importedInlineDecls;
    std::unordered_set<std::string> newExternalDecls;
    void EmplaceImportedInlineDeclPtr(const Codira::AST::Decl &decl)
    {
        importedInlineDeclsPtr.emplace(&decl);
    };
    void Clear()
    {
        incrRemovedDecls.clear();
        importedInlineDecls.clear();
        newExternalDecls.clear();
        importedInlineDeclsPtr.clear();
    }
    void UpdateImportedInlineDeclsMangle()
    {
        importedInlineDecls.clear();
        for (const auto& p : importedInlineDeclsPtr) {
            CODEC_NULLPTR_CHECK(p);
            if (p) {
                importedInlineDecls.emplace(p->mangledName);
            }
        }
    }
    void Dump() const
    {
        auto& logger = IncrementalCompilationLogger::GetInstance();
        if (!logger.IsEnable()) {
            return;
        }
        if (incrRemovedDecls.empty() && importedInlineDecls.empty() && newExternalDecls.empty()) {
            logger.LogLn("[CachedMangleMap] empty");
            return;
        }
        logger.LogLn("[CachedMangleMap] START");
        if (!incrRemovedDecls.empty()) {
            logger.LogLn("[incrRemovedDecls]:");
            for (auto incrRemovedDecl : incrRemovedDecls) {
                logger.LogLn(incrRemovedDecl);
            }
        }
        if (!importedInlineDecls.empty()) {
            logger.LogLn("[importedInlineDecls]:");
            for (auto importedInlineDecl : importedInlineDecls) {
                logger.LogLn(importedInlineDecl);
            }
        }
        if (!newExternalDecls.empty()) {
            logger.LogLn("[newExternalDecls]:");
            for (auto newExternalDecl : newExternalDecls) {
                logger.LogLn(newExternalDecl);
            }
        }
        logger.LogLn("[CachedMangleMap] END");
    }
private:
    std::unordered_set<const Codira::AST::Decl*> importedInlineDeclsPtr;
};

#endif // CODIRA_INCREMENTAL_COMPILATION_CACHED_MANGLE_MAP
