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
 * This file declares the AST Serialization related classes, which provides AST serialization capabilities.
 */

#ifndef CODIRA_COMPILATION_CACHE_SERIALIZATION_H
#define CODIRA_COMPILATION_CACHE_SERIALIZATION_H

#include <cstdint>
#include <flatbuffers/flatbuffers.h>
#include <string>
#include <unordered_map>
#include <vector>

#include "Codira/IncrementalCompilation/CompilationCache.h"
#include "Codira/Modules/ASTSerialization.h"

namespace Codira {
using TVirtualDepOffset = flatbuffers::Offset<CachedASTFormat::VirtualDep>;
class HashedASTWriter {
public:
    HashedASTWriter() = default;
    ~HashedASTWriter() = default;

    // Export external decls of a package AST to a buffer.
    // NOTE: should notice bep.
    void SetImportSpecs(const AST::Package& package);
    void SetLambdaCounter(uint64_t counter);
    void SetEnvClassCounter(uint64_t counter);
    void SetStringLiteralCounter(uint64_t counter);
    void SetCompileArgs(const std::vector<std::string>& args);
    void SetVarAndFuncDependency(
        const std::vector<std::pair<Ptr<const AST::Decl>, std::vector<Ptr<const AST::Decl>>>>& varAndFuncDep);
    void SetCHIROptInfo(const OptEffectStrMap& optInfo);
    void SetVirtualFuncDep(const VirtualWrapperDepMap& depMap);
    void SetVarInitDep(const VarInitDepMap& depMap);
    void SetCCOutFuncs(const std::set<std::string>& funcs);
    void SetBitcodeFilesName(const std::vector<std::string>& bitcodeFiles);
    void SetSemanticInfo(const SemanticInfo& info);
    void WriteAllDecls(ASTCache&& ast, ASTCache&& imports, std::vector<const AST::Decl*>&& order);
    std::vector<uint8_t> AST2FB(const std::string& pkgName);

private:
    flatbuffers::FlatBufferBuilder builder{INITIAL_FILE_SIZE};
    std::vector<TStringOffset> bitcodeFilesName;
    std::vector<TStringOffset> compileArgs;
    std::vector<TDeclDepOffset> varAndFunc;
    std::vector<TEffectMapOffset> chirOptInfo;
    std::vector<TVirtualDepOffset> virtualFuncDep;
    std::vector<TVirtualDepOffset> varInitDep;
    std::vector<TStringOffset> ccOutFuncs;
    // NOTE: For incremental compilation 2.0. Above members will be removed later.
    flatbuffers::Offset<CachedASTFormat::SemanticInfo> semaUsages;
    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<CachedASTFormat::TopDecl>>> allAST;
    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<CachedASTFormat::TopDecl>>> importedDecls;
    uint64_t lambdaCounter = 0;
    uint64_t envClassCounter = 0;
    uint64_t stringLiteralCounter = 0;
    uint64_t specs = 0;
};

class HashedASTLoader {
public:
    explicit HashedASTLoader(std::vector<uint8_t>&& astData) : serializedData(std::move(astData))
    {
    }
    ~HashedASTLoader() = default;
    std::pair<bool, CompilationCache> DeserializeData(const RawMangled2DeclMap& mangledName2DeclMap);

private:
    bool VerifyData();
    ASTCache LoadCachedAST(const CachedASTFormat::HashedPackage& p);
    std::unordered_map<RawMangledName, TopLevelDeclCache> LoadImported(const CachedASTFormat::HashedPackage& p);
    static SemanticInfo LoadSemanticInfos(
        const CachedASTFormat::HashedPackage& hasedPackage, const RawMangled2DeclMap& mangledName2DeclMap);
    MemberDeclCache Load(const CachedASTFormat::MemberDecl& decl);
    TopLevelDeclCache Load(const CachedASTFormat::TopDecl& decl, bool srcPkg);

    std::vector<uint8_t> serializedData;
    // pair of mangledname and gvid, to be sorted by gvid
    std::unordered_map<std::string, std::vector<std::pair<RawMangledName, int>>> fileMap{};
};

} // namespace Codira
#endif // CODIRA_COMPILATION_CACHE_SERIALIZATION_H
