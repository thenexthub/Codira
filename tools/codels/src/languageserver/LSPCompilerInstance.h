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

#ifndef CODIRA_FRONTEND_LSPCOMPILERINSTANCE_H
#define CODIRA_FRONTEND_LSPCOMPILERINSTANCE_H

#include <utility>

#include "../json-rpc/StdioTransport.h"
#include "CodeoManager.h"
#include "DependencyGraph.h"
#include "Codira/Frontend/CompilerInstance.h"
#include "Codira/Macro/MacroExpansion.h"
#include "capabilities/diagnostic/LSPDiagObserver.h"
#include "common/multiModule/ModuleManager.h"
#include "index/SymbolCollector.h"
#include "logger/Logger.h"

namespace Codira {
template <typename Func, typename... Args>
bool ExecuteCompilerApi(const std::string &name, Func func, Args &&...args)
{
    ark::Logger::Instance().CollectKernelLog(std::this_thread::get_id(), name, "start");
    try {
        Trace::Wlog("#### execute " + name + " start ####");
        std::invoke(func, std::forward<Args>(args)...);
        Trace::Wlog("#### execute " + name + " end ####");
    } catch (std::exception &e) {
        ark::Logger::Instance().LogMessage(ark::MessageType::MSG_ERROR,
            "Func " + name + e.what());
        return false;
    } catch (...) {
        ark::Logger::Instance().LogMessage(ark::MessageType::MSG_ERROR,
            "Func " + name + "Caught an unknown exception");
        return false;
    }
    ark::Logger::Instance().CollectKernelLog(std::this_thread::get_id(), name, "end");
    return true;
}

class PackageMapNode {
public:
    PackageMapNode() : inDegree(0) {}

    std::set<std::string> importPackages;
    std::set<std::string> downstreamPkgs;
    size_t inDegree;
    bool isInModule = true;
};

class LSPCompilerInstance : public CompilerInstance {
public:
    using PackageMap = std::unordered_map<std::string, PackageMapNode>;
    using CodeoCacheMap = std::unordered_map<std::string, std::vector<uint8_t>>;
    LSPCompilerInstance(ark::Callbacks *cb,
                        CompilerInvocation &invocation,
                        DiagnosticEngine &diag,
                        std::string realPkgName,
                        const std::unique_ptr<ark::ModuleManager> &moduleManger);

    virtual ~LSPCompilerInstance() { callback = nullptr; }

    void PreCompileProcess();

    void CompilePassForComplete(const std::unique_ptr<ark::CodeoManager> &codeoManager,
        const std::unique_ptr<ark::DependencyGraph> &graph,
        Position pos = INVALID_POSITION, const std::string &name = "");

    bool MacroExpand()
    {
        std::lock_guard<std::mutex> lock(ark::StdioTransport::Instance().stdoutMutex);
        const bool ret =
            ExecuteCompilerApi("PerformMacroExpand", &CompilerInstance::PerformMacroExpand, this);
        return ret;
    }

    static std::vector<std::string> GetTopologySort();

    static void SetCodeoPathInModules(const std::string &cangjieHome, const std::string &cangjiePath);

    static void ReadCodeoFileOneModule(const std::string &modulePath);

    static void ReadCodeoFileOneModuleExternal(const std::string &modulesPath);

    static void UpdateUsrCodeoFileCacheMap(
        std::string &moduleName, std::unordered_map<std::string, std::string> &CodeoRequiresMap);

    static void InitCacheFileCacheMap();

    std::unordered_set<std::string> GetAllImportedCodeo(
        const std::string &pkgName, std::unordered_map<std::string, bool> &isVisited);

    bool ToImportPackage(const std::string &curModuleName, const std::string &codeoPackage);

    bool Parse()
    {
        return ExecuteCompilerApi("PerformParse", &CompilerInstance::PerformParse, this);
    }

    bool ConditionCompile()
    {
        return ExecuteCompilerApi("PerformConditionCompile",
                                  &CompilerInstance::PerformConditionCompile, this);
    }

    bool ImportPackage()
    {
        return ExecuteCompilerApi("PerformImportPackage", &CompilerInstance::PerformImportPackage,
                                  this);
    }

    bool Sema() { return ExecuteCompilerApi("PerformSema", &CompilerInstance::PerformSema, this); }

    bool ExportAST(
        bool saveFileWithAbsPath,
        std::vector<uint8_t> &astData,
        const AST::Package &pkg,
        const std::function<void(ASTWriter &)> additionalSerializations = [](ASTWriter &) {})
    {
        return ExecuteCompilerApi("ExportAST", &ImportManager::ExportAST, this->importManager,
                                  saveFileWithAbsPath, astData, pkg, additionalSerializations);
    }

    virtual std::string Denoising(std::string candidate);

    Ptr<AST::File> GetFileByPath(const std::string& filePath) override;

    void ImportUsrPackage(const std::string &curModuleName);

    void ImportUsrCodeo(const std::string &curModuleName, std::unordered_set<std::string> &visitedPackages);

    void ImportAllUsrCodeo(const std::string &curModuleName);

    virtual void ImportCodeoToManager(
        const std::unique_ptr<ark::CodeoManager> &codeoManager, const std::unique_ptr<ark::DependencyGraph> &graph);

    void IndexCodeoToManager(
        const std::unique_ptr<ark::CodeoManager> &codeoManager, const std::unique_ptr<ark::DependencyGraph> &graph);

    bool CompileAfterParse(
        const std::unique_ptr<ark::CodeoManager> &codeoManager, const std::unique_ptr<ark::DependencyGraph> &graph);

    std::unordered_map<std::string, ark::EdgeType> UpdateUpstreamPkgs();

    void UpdateDepGraph(bool isIncrement = true, const std::string &prePkgName = "");

    void UpdateDepGraph(const std::unique_ptr<ark::DependencyGraph> &graph, const std::string &prePkgName);

    ark::Callbacks *callback = nullptr;
    std::string pkgNameForPath; // Real Package Name
    std::string pkgNameForCode;
    bool macroExpandSuccess = false;
    std::set<std::string> upstreamPkgs;
    const std::unique_ptr<ark::ModuleManager> &moduleManger;

    static inline std::shared_mutex mtx;
    static inline PackageMap dependentPackageMap;
    static inline std::unordered_map<std::string, std::pair<std::vector<uint8_t>, bool>> astDataMap;
    static inline std::vector<std::string> codeoPathInModules;
    static inline CodeoCacheMap codeoFileCacheMap;
    static inline std::unordered_map<std::string, std::vector<std::string>> codeoLibraryMap;
    // key: moduleName, value: CodeoCacheMap
    static inline std::unordered_map<std::string, CodeoCacheMap> usrCodeoFileCacheMap;
    static inline std::unordered_set<std::string> codeoPathSet;

private:
    static void MarkBrokenDecls(AST::Package &pkg);
};
} // namespace Codira
#endif // CODIRA_FRONTEND_LSPCOMPILERINSTANCE_H
