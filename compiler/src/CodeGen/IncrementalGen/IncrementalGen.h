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

#ifndef CODIRA_INCREMENTALGEN2_H
#define CODIRA_INCREMENTALGEN2_H

#include <map>
#include <unordered_set>

#include "llvm/Transforms/Utils/ValueMapper.h"

#include "Codira/IncrementalCompilation/CachedMangleMap.h"

namespace Codira::CodeGen {
class IncrementalGen {
public:
    explicit IncrementalGen(bool cgParallelEnabled);
    ~IncrementalGen() = default;

    bool Init(const std::string& cachedIRPath, llvm::LLVMContext& llvmContext);
    llvm::Module* LinkModules(llvm::Module* incremental, const CachedMangleMap& cachedMangles = {});
    std::vector<std::string> GetIncrLLVMUsedNames();
    std::vector<std::string> GetIncrCachedStaticGINames();

private:
    void InitCodeGenAddedCachedMap();
    void UpdateCachedDeclsFromInjectedModule(const CachedMangleMap& cachedMangles);
    void CopyDeclarationsToInjectedModule();
    void FillValueMap(llvm::ValueToValueMapTy& valueMap);
    void UpdateInitializationsOfGlobalVariables(llvm::ValueToValueMapTy& valueMap);
    void UpdateDefinitionsOfFunction(llvm::ValueToValueMapTy& valueMap);
    void UpdateBodyOfKeepTypesFunction(llvm::ValueToValueMapTy& valueMap);
    void CollectUselessFunctions();
    void EraseUselessFunctions();
    void CollectUselessDefinitions(llvm::GlobalObject* uselessDefinition);
    void UpdateReflectionMetadata();
    void UpdateCodeGenAddedMetadata();
    void UpdateIncrLLVMUsedNames();

    const bool cgParallelEnabled;
    std::unique_ptr<llvm::Module> incrementalModule;
    std::unique_ptr<llvm::Module> injectedModule;
    // Key: decl name from CHIR
    // Value: codegen added variable name or codegen added function name for specific decl name
    std::unordered_map<std::string, std::unordered_set<std::string>> codegenAddedCachedMap;
    std::unordered_set<llvm::GlobalObject*> uselessDefinitions;
    std::unordered_set<llvm::GlobalObject*> deferErase;
    std::vector<std::string> llvmUsedGVNames;
    std::vector<std::string> staticGINames;
};
} // namespace Codira::CodeGen

#endif // CODIRA_INCREMENTALGEN2_H
