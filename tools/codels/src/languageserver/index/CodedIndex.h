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

#ifndef LSPSERVER_INDEX_CODEDINDEXER_H
#define LSPSERVER_INDEX_CODEDINDEXER_H

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include "../CompilerCodiraProject.h"
#include "Symbol.h"
#include "Codira/Basic/Version.h"

namespace Codira {
class DCompilerInstance final : public LSPCompilerInstance {
public:
    explicit DCompilerInstance(
            ark::Callbacks *cb, CompilerInvocation &invocation, DiagnosticEngine &diag, const std::string& pkgName)
        : LSPCompilerInstance(cb, invocation, diag, pkgName, moduleMgr)
    {
    }

    void ImportCodeoToManager(const std::unique_ptr<ark::CodeoManager> &codeoManager,
                            const std::unique_ptr<ark::DependencyGraph> &graph) override;

    std::string Denoising(std::string candidate) override;

private:
    std::unique_ptr<ark::ModuleManager> moduleMgr = nullptr;
};
} // namespace Codira

namespace ark {
namespace lsp {
using namespace Codira::FileUtil;

struct DPkgInfo : public PkgInfo {
    explicit DPkgInfo(const std::string &pkgPath,
                      const std::string &curModulePath,
                      const std::string &curModuleName,
                      Callbacks *callback)
        : PkgInfo(pkgPath, curModulePath, curModuleName, callback)
    {
        compilerInvocation->globalOptions.compileCoded = true;
        compilerInvocation->globalOptions.enableAddCommentToAst = true;
    }
};

class CodedIndexer {
public:
    explicit CodedIndexer(Callbacks *cb,
                        const std::string& stdCodedPath,
                        const std::string& ohosCodedPath,
                        const std::string& codedCachePath,
                        bool enablePackaged = true)
        : callback(cb),
          stdCodedPath(stdCodedPath),
          ohosCodedPath(ohosCodedPath),
          codedCachePath(JoinPath(codedCachePath, CODIRA_VERSION)),
          enablePackaged(enablePackaged)
    {
        if (CreateDirs(this->codedCachePath) == -1) {
            Trace::Log("coded cache dir build failed");
        }
        cacheManager = std::make_unique<CacheManager>(this->codedCachePath);
        cacheManager->InitDir();
    }

    ~CodedIndexer() = default;

    static void InitInstance(Callbacks *cb, const std::string& stdCodedPathOption,
                             const std::string& ohosCodedPathOption, const std::string& codedCachePathOption);

    static void DeleteInstance()
    {
        if (instance) {
            delete instance;
            instance = nullptr;
        }
    }

    static CodedIndexer *GetInstance();

    SymbolLocation GetSymbolDeclaration(SymbolID id, const std::string& fullPkgName);

    CommentGroups GetSymbolComments(SymbolID id, const std::string& fullPkgName);

    std::unordered_map<std::string, std::unique_ptr<DPkgInfo>> &GetPkgMap()
    {
        return pkgMap;
    }

    bool CheckCodedCache();

    void Build();

    void BuildIndexFromCache();

    bool GetRunningState() const
    {
        return isIndexing;
    }

private:
    void ReadCODEDSource(const std::string &rootPath, const std::string &modulePath,
                       std::map<int, std::vector<std::string>> &fileMap, const std::string &parentPkg = "");

    void LoadAllCODEDResource();

    void ReadPackagedCodedResource(const std::string& rootPath, const std::string& filePath,
        std::map<int, std::vector<std::string>> &fileMap);

    void ParsePackageDependencies();

    void BuildCODEDIndex();

    void GenerateValidFile();

    std::string GetValidCode();

    static CodedIndexer *instance;
    std::mutex mtx;
    bool enablePackaged = true;
    bool isIndexing = false;
    std::string cangjieHome;
    std::string stdCodedPath;
    std::string ohosCodedPath;
    std::string codedCachePath;
    Callbacks *callback = nullptr;
    std::unique_ptr<DependencyGraph> graph = std::make_unique<DependencyGraph>();
    std::unique_ptr<CodeoManager> codeoManager = std::make_unique<CodeoManager>();
    std::unique_ptr<ThrdPool> thrdPool = std::make_unique<ThrdPool>(6);
    std::unique_ptr<CacheManager> cacheManager;

    std::map<std::string, SymbolSlab> pkgSymsMap{};
    std::unordered_map<std::string, std::unique_ptr<DPkgInfo>> pkgMap;
    std::unordered_map<std::string, std::unique_ptr<DCompilerInstance>> ciMap;
};
} // namespace lsp
} // namespace ark
#endif // LSPSERVER_INDEX_CODEDINDEXER_H
