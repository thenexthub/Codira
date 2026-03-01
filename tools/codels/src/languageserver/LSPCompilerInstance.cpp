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

#include "LSPCompilerInstance.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "CodeoManager.h"
#include "CompilerCodiraProject.h"
#include "DependencyGraph.h"
#include "Codira/Driver/TempFileManager.h"
#include "common/Utils.h"
#ifdef __linux__
#include <malloc.h>
#endif

using namespace Codira;
using namespace AST;
using namespace Codira::FileUtil;
using namespace Codira::Utils;

namespace  {
inline bool ShouldSkipDecl(const Decl &decl)
{
    auto md = DynamicCast<const MacroDecl *>(&decl);
    const Decl &beCheckedDecl = md && md->desugarDecl ? *md->desugarDecl : decl;
    return beCheckedDecl.TestAnyAttr(Attribute::HAS_BROKEN, Attribute::IS_BROKEN) || !Ty::IsTyCorrect(beCheckedDecl.ty);
}

std::tuple<std::string, std::string> GetFullPackageNames(const ImportSpec& import)
{
    // Multi-imports are desugared after parser which should not be used for get package name.
    CODEC_ASSERT(import.content.kind != ImportKind::IMPORT_MULTI);
    if (import.content.prefixPaths.empty()) {
        return std::tuple(import.content.identifier, "");
    }
    std::string fullPackageName = import.content.GetPrefixPath();
    if (import.content.kind == ImportKind::IMPORT_ALL) {
        return std::tuple(fullPackageName, "");
    }
    [[maybe_unused]] std::string maybePackageName = fullPackageName + "." + import.content.identifier;
    return std::tuple(maybePackageName, fullPackageName);
}
}

LSPCompilerInstance::LSPCompilerInstance(ark::Callbacks *cb, CompilerInvocation &invocation,
                                         DiagnosticEngine &diag, std::string realPkgName,
                                         const std::unique_ptr<ark::ModuleManager> &moduleManger)
    : CompilerInstance(invocation, diag), callback(cb), pkgNameForPath(std::move(realPkgName)),
      moduleManger(moduleManger)
{
    (void)ExecuteCompilerApi("SetSourceCodeImportStatus", &ImportManager::SetSourceCodeImportStatus,
                             &importManager, false);
}

std::unordered_map<std::string, ark::EdgeType> LSPCompilerInstance::UpdateUpstreamPkgs()
{
    const auto packages = GetSourcePackages();
    if (packages.empty()) {
        return {};
    }

    std::string curModule;
    std::unordered_set<std::string> depends;
    if (!pkgNameForPath.empty()) {
        curModule = SplitQualifiedName(pkgNameForPath).front();
        depends = ark::CompilerCodiraProject::GetInstance()->GetOneModuleDeps(curModule);
    }

    std::set<std::string> depPkgs;
    std::unordered_map<std::string, ark::EdgeType> depPkgsEdges;
    std::unordered_map<TokenKind, ark::EdgeType> edgeKindMap = {
        {TokenKind::PUBLIC, ark::EdgeType::PUBLIC},
        {TokenKind::PROTECTED, ark::EdgeType::PROTECTED},
        {TokenKind::INTERNAL, ark::EdgeType::INTERNEL},
        {TokenKind::PRIVATE, ark::EdgeType::PRIVATE}
    };
    for (const auto &file : packages[0]->files) {
        for (auto &import : file->imports) {
            if (import->IsImportMulti()) {
                continue;
            }
            TokenKind modifier = import->modifier ? import->modifier->modifier : TokenKind::PRIVATE;
            // Get real dependent package.
            auto [package, orPackage] = ::GetFullPackageNames(*import);
            package = Denoising(package);
            orPackage = Denoising(orPackage);
            if (package.empty() && orPackage.empty()) {
                continue;
            }
            std::string realDep = (package.size() > orPackage.size()) ? package : orPackage;
            if (pkgNameForPath.empty() || realDep.empty()) {
                if (depPkgsEdges.find(realDep) == depPkgsEdges.end() || depPkgsEdges[realDep] < edgeKindMap[modifier]) {
                    depPkgsEdges[realDep] = edgeKindMap[modifier];
                }
                depPkgs.insert(realDep);
                continue;
            }
            auto realModule = SplitQualifiedName(realDep).front();
            if (curModule != realModule && depends.count(realModule) == 0) {
                continue;
            }
            if (depPkgsEdges.find(realDep) == depPkgsEdges.end() || depPkgsEdges[realDep] < edgeKindMap[modifier]) {
                depPkgsEdges[realDep] = edgeKindMap[modifier];
            }
            depPkgs.insert(realDep);
        }
    }
    upstreamPkgs = depPkgs;
    return depPkgsEdges;
}

void LSPCompilerInstance::UpdateDepGraph(bool isIncrement, const std::string &prePkgName)
{
    (void)UpdateUpstreamPkgs();
    {
        std::unique_lock<std::shared_mutex> lck(mtx);
        if (pkgNameForPath.empty() && !invocation.globalOptions.packagePaths.empty()) {
            pkgNameForPath = invocation.globalOptions.packagePaths[0];
            dependentPackageMap[pkgNameForPath].isInModule = false;
        }
        if (dependentPackageMap.count(pkgNameForPath)) {
            for (auto &item : dependentPackageMap[pkgNameForPath].importPackages) {
                if (!dependentPackageMap.count(item)) {
                    continue;
                }
                dependentPackageMap[item].downstreamPkgs.erase(pkgNameForPath);
            }
        }

        if (!prePkgName.empty() && prePkgName != pkgNameForPath) {
            dependentPackageMap[pkgNameForPath].downstreamPkgs =
                dependentPackageMap[prePkgName].downstreamPkgs;
            for (const auto &item : dependentPackageMap) {
                if (item.first == pkgNameForPath) {
                    continue;
                }
                if (dependentPackageMap[item.first].importPackages.count(pkgNameForPath)) {
                    dependentPackageMap[pkgNameForPath].downstreamPkgs.insert(item.first);
                }
            }
            if (dependentPackageMap.count(prePkgName)) {
                for (auto &iter : dependentPackageMap[prePkgName].importPackages) {
                    dependentPackageMap[iter].downstreamPkgs.erase(prePkgName);
                }
            }
            dependentPackageMap.erase(prePkgName);
        }

        dependentPackageMap[pkgNameForPath].importPackages = upstreamPkgs;
        dependentPackageMap[pkgNameForPath].inDegree = upstreamPkgs.size();
        if (isIncrement) {
            for (const auto &item : upstreamPkgs) {
                if (!dependentPackageMap.count(item)) {
                    dependentPackageMap[pkgNameForPath].importPackages.erase(item);
                    dependentPackageMap[pkgNameForPath].inDegree =
                        dependentPackageMap[pkgNameForPath].importPackages.size();
                    continue;
                }
                dependentPackageMap[item].downstreamPkgs.insert(pkgNameForPath);
            }
        }
    }
}

void LSPCompilerInstance::UpdateDepGraph(
    const std::unique_ptr<ark::DependencyGraph> &graph, const std::string &fullPkgName)
{
    auto edges = UpdateUpstreamPkgs();
    graph->UpdateDependencies(fullPkgName, upstreamPkgs, edges);
}

void LSPCompilerInstance::PreCompileProcess()
{
    diag.Reset();
    diag.SetSourceManager(&GetSourceManager());

    (void)Parse();
    // parse completion need condition compile
    (void)ConditionCompile();
    AddSourceToMember();
}

void LSPCompilerInstance::CompilePassForComplete(
    const std::unique_ptr<ark::CodeoManager> &codeoManager,
    const std::unique_ptr<ark::DependencyGraph> &graph, Position pos, const std::string &name)
{
    // Faster Completion needs pass: Parse, ConditionCompile and ImportPackage.
    diag.Reset();
    diag.SetSourceManager(&GetSourceManager());
    (void)Parse();
    (void)ConditionCompile();
    const auto filePath = GetSourceManager().GetSource(pos.fileID).path;
    auto file = GetFileByPath(filePath).get();
    // If the position is not in ImportSpec, do not need to ImportPackage.
    if (file && !ark::InImportSpec(*file, pos) && name != "SignatureHelp") {
        return;
    }
    ImportCodeoToManager(codeoManager, graph);
    (void)ImportPackage();
}

Ptr<File> LSPCompilerInstance::GetFileByPath(const std::string& filePath)
{
    const auto package = GetSourcePackages()[0];
    for (auto &file : package->files) {
        if (file->filePath == filePath) {
            return file;
        }
    }
    return nullptr;
}

std::unordered_set<std::string> LSPCompilerInstance::GetAllImportedCodeo(
    const std::string &pkgName, std::unordered_map<std::string, bool> &isVisited)
{
    std::unordered_set<std::string> res;
    if (isVisited[pkgName]) {
        return res;
    }
    isVisited[pkgName] = true;
    if (!dependentPackageMap.count(pkgName)) {
        return res;
    }
    auto deps = dependentPackageMap[pkgName].importPackages;

    if (deps.empty()) {
        return res;
    }
    for (const auto &dependent : dependentPackageMap[pkgName].importPackages) {
        auto temp = GetAllImportedCodeo(dependent, isVisited);
        res.insert(temp.begin(), temp.end());
    }
    res.insert(deps.begin(), deps.end());
    return res;
}

bool LSPCompilerInstance::ToImportPackage(const std::string &curModuleName,
                                          const std::string &codeoPackage)
{
    std::string codeoModuleName = ark::SplitFullPackage(codeoPackage).first;
    if (astDataMap.find(codeoPackage) == astDataMap.end() || astDataMap[codeoPackage].first.empty() ||
        codeoModuleName.empty()) {
        return false;
    }
    if (curModuleName.empty()) {
        return true;
    }
    auto found = moduleManger->requireAllPackages.find(curModuleName);
    if (found != moduleManger->requireAllPackages.end() && found->second.count(codeoModuleName)) {
        return true;
    }
    return false;
}

void LSPCompilerInstance::ImportUsrPackage(const std::string &curModuleName)
{
    std::unordered_map<std::string, bool> isVisited;
    const std::unordered_set<std::string> codeoPackageAll = GetAllImportedCodeo(pkgNameForPath, isVisited);
    for (const auto &codeoPackage : codeoPackageAll) {
        if (ToImportPackage(curModuleName, codeoPackage)) {
            importManager.SetPackageCodeoCache(codeoPackage, astDataMap[codeoPackage].first);
        }
    }
}

// LCOV_EXCL_STOP
void LSPCompilerInstance::ImportUsrCodeo(const std::string &curModuleName,
    std::unordered_set<std::string> &visitedPackages)
{
    if (usrCodeoFileCacheMap.count(curModuleName) != 0) {
        CodeoCacheMap &codeoCacheMap = usrCodeoFileCacheMap[curModuleName];
        for (auto &item : codeoCacheMap) {
            if (visitedPackages.count(item.first) > 0) {
                continue;
            }
            visitedPackages.insert(item.first);
            importManager.SetPackageCodeoCache(item.first, item.second);
        }
    }
}

void LSPCompilerInstance::ImportAllUsrCodeo(const std::string &curModuleName)
{
    std::unordered_set<std::string> visitedPackages;
    auto deps = ark::CompilerCodiraProject::GetInstance()->GetOneModuleDeps(curModuleName);
    for (auto& module: deps) {
        ImportUsrCodeo(module, visitedPackages);
    }
}

void LSPCompilerInstance::ImportCodeoToManager(
    const std::unique_ptr<ark::CodeoManager> &codeoManager, const std::unique_ptr<ark::DependencyGraph> &graph)
{
    std::string curModuleName = invocation.globalOptions.moduleName;

    // Import stdlib codeo, priority is low.
    for (const auto &codeoCache : codeoFileCacheMap) {
        importManager.SetPackageCodeoCache(codeoCache.first, codeoCache.second);
    }

    // import codeo for bin dependencies in codepm.toml
    ImportAllUsrCodeo(curModuleName);

    // Import user's source code, priority is high.
    const auto allDependencies = graph->FindAllDependencies(pkgNameForPath);
    for (auto &package : allDependencies) {
        auto codeoCache = codeoManager->GetData(package);
        if (!codeoCache) {
            continue;
        }
        importManager.SetPackageCodeoCache(package, *codeoCache);
    }
}

void LSPCompilerInstance::IndexCodeoToManager(
    const std::unique_ptr<ark::CodeoManager> &codeoManager, const std::unique_ptr<ark::DependencyGraph> &graph)
{
    // Import stdlib's codeo, priority is low.
    for (const auto &codeoCache : codeoFileCacheMap) {
        importManager.SetPackageCodeoCache(codeoCache.first, codeoCache.second);
    }
    for (const auto &item : usrCodeoFileCacheMap) {
        for (const auto &codeoCache : item.second) {
            importManager.SetPackageCodeoCache(codeoCache.first, codeoCache.second);
        }
    }
}

/**
 * @brief The rest of the compilation process is performed, and the astdata data in the cache is updated and the
 * downstream package is marked as stale.
 *
 * @param codeoManager Read codeo cache and update codeo cache and state
 * @param graph
 * @return true
 * @return false
 */
bool LSPCompilerInstance::CompileAfterParse(
    const std::unique_ptr<ark::CodeoManager> &codeoManager, const std::unique_ptr<ark::DependencyGraph> &graph)
{
    ImportCodeoToManager(codeoManager, graph);
    (void)ImportPackage();
    if (!ark::CompilerCodiraProject::GetInstance()->isIdentical) {
        return false;
    }
    macroExpandSuccess = MacroExpand();
    (void)Sema();
    (void)ExecuteCompilerApi("DeleteASTLoaders", &ImportManager::DeleteASTLoaders, this->importManager);
    const auto packages = GetSourcePackages();
    if (packages.empty()) {
        return false;
    }
    if (pkgNameForPath.empty()) {
        return false;
    }
    ark::CodeoData codeoData;
    std::vector<uint8_t> data;
    MarkBrokenDecls(*packages[0]);
    (void)ExportAST(false, data, *packages[0]);
    auto oldData = codeoManager->GetData(pkgNameForPath);
    bool changed = codeoManager->CheckChanged(pkgNameForPath, data);
    codeoData.data = data;
    codeoData.status = ark::DataStatus::FRESH;
    codeoManager->SetData(pkgNameForPath, codeoData);
    return changed;
}

std::vector<std::string> LSPCompilerInstance::GetTopologySort()
{
    auto tempDependentPackageMap = dependentPackageMap;
    std::vector<std::string> result;
    std::queue<std::string> que;
    for (auto &iter : tempDependentPackageMap) {
        if (!iter.second.isInModule) {
            continue;
        }
        if (iter.second.inDegree == 0) {
            que.emplace(iter.first);
        }
    }
    while (!que.empty()) {
        std::string pkgName = que.front();
        result.push_back(pkgName);
        que.pop();
        if (!tempDependentPackageMap.count(pkgName)) {
            continue;
        }
        for (auto &outEdge : tempDependentPackageMap[pkgName].downstreamPkgs) {
            if (tempDependentPackageMap[outEdge].inDegree > 0) {
                tempDependentPackageMap[outEdge].inDegree--;
            }
            if (tempDependentPackageMap[outEdge].inDegree == 0) {
                que.emplace(outEdge);
            }
        }
    }
    return result;
}

void LSPCompilerInstance::SetCodeoPathInModules(const std::string &cangjieHome,
                                              const std::string &cangjiePath)
{
    auto invocation = CompilerInvocation();
#ifdef _WIN32
    const std::string separator = ";";
#else
    const std::string separator = ":";
#endif
    if (cangjieHome.empty()) {
        return;
    }
    if (!ark::Options::GetInstance().IsOptionSet("test") && ark::MessageHeaderEndOfLine::GetIsDeveco()) {
        invocation.globalOptions.target.arch = Triple::ArchType::AARCH64;
        invocation.globalOptions.target.os = Triple::OSType::LINUX;
        invocation.globalOptions.target.env = Triple::Environment::OHOS;
    }
    const auto libPathName = invocation.globalOptions.GetCodiraLibTargetPathName();
    if (libPathName.empty()) {
        return;
    }
    auto stdLibraryPath = JoinPath(JoinPath(cangjieHome, "modules"), libPathName);
    codeoPathInModules.emplace_back(stdLibraryPath);

    if (!cangjiePath.empty()) {
        auto tmpVec = SplitString(cangjiePath, separator);
        std::copy(tmpVec.begin(), tmpVec.end(), std::back_inserter(codeoPathInModules));
    }
}

void LSPCompilerInstance::ReadCodeoFileOneModule(const std::string &modulePath)
{
    std::string packageName;
    const std::string moduleName = GetDirName(modulePath);
    std::vector<std::string> packages = {};
    for (auto &file : GetAllFilesUnderCurrentPath(modulePath, "codeo")) {
        auto codeoPath = JoinPath(modulePath, file);
        if (!FileExist(codeoPath)) {
            continue;
        }
        std::vector<uint8_t> tmpAST;
        std::string failedReason;
        if (ReadBinaryFileToBuffer(codeoPath, tmpAST, failedReason)) {
            packageName = GetFileNameWithoutExtension(file);
            (void)codeoFileCacheMap.emplace(packageName, tmpAST);
            codeoPathSet.insert(codeoPath);
            packages.emplace_back(packageName);
        }
    }

    codeoLibraryMap[moduleName] = packages;
}

void LSPCompilerInstance::ReadCodeoFileOneModuleExternal(const std::string &modulesPath)
{
    std::string packageName;
    for (auto &file : GetAllFilesUnderCurrentPath(modulesPath, "codeo")) {
        auto codeoPath = JoinPath(modulesPath, file);
        if (!FileExist(codeoPath)) {
            continue;
        }
        std::vector<uint8_t> tmpAST;
        std::string failedReason;
        if (ReadBinaryFileToBuffer(codeoPath, tmpAST, failedReason)) {
            packageName = GetFileNameWithoutExtension(file);
            (void)codeoFileCacheMap.emplace(packageName, tmpAST);
            codeoPathSet.insert(codeoPath);
            size_t pos = packageName.find('.');
            std::string moduleName = (pos != std::string::npos) ? packageName.substr(0, pos) : packageName;
            auto it = codeoLibraryMap.find(moduleName);
            if (it != codeoLibraryMap.end()) {
                it->second.push_back(packageName);
            } else {
                codeoLibraryMap[moduleName] = {packageName};
            }
        }
    }
}

void LSPCompilerInstance::InitCacheFileCacheMap()
{
    for (const auto &dirPath : codeoPathInModules) {
        for (auto &modulePath : GetAllDirsUnderCurrentPath(dirPath)) {
            ReadCodeoFileOneModule(modulePath);
        }
        ReadCodeoFileOneModuleExternal(dirPath);
    }
}

void LSPCompilerInstance::UpdateUsrCodeoFileCacheMap(
    std::string &moduleName, std::unordered_map<std::string, std::string> &codeoRequiresMap)
{
    std::string packageName;
    std::string fullPkgName;
    std::string requireCodeoPath;
    CodeoCacheMap requiresMap = {};
    for (auto &require : codeoRequiresMap) {
        fullPkgName = require.first;
        requireCodeoPath = require.second;
        if (!FileExist(requireCodeoPath)) {
            continue;
        }
        std::vector<uint8_t> tmpAST;
        std::string failedReason;
        if (ReadBinaryFileToBuffer(requireCodeoPath, tmpAST, failedReason)) {
            (void)requiresMap.emplace(fullPkgName, tmpAST);
            codeoPathSet.insert(requireCodeoPath);
            auto curModuleName = ark::SplitFullPackage(fullPkgName).first;
            packageName = ark::SplitFullPackage(fullPkgName).second;
            codeoLibraryMap[curModuleName].emplace_back(packageName);
        }
    }
    usrCodeoFileCacheMap.emplace(moduleName, requiresMap);
}

void LSPCompilerInstance::MarkBrokenDecls(Package &pkg)
{
    Walker(&pkg, [](Ptr<Node> node) {
        if (In(node->astKind, {ASTKind::PACKAGE, ASTKind::FILE})) {
            return VisitAction::WALK_CHILDREN;
        }
        const auto decl = DynamicCast<Decl *>(node);
        if (!decl) {
            return VisitAction::SKIP_CHILDREN;
        }
        if (ShouldSkipDecl(*decl)) {
            decl->doNotExport = true;
        } else if (decl->IsNominalDecl()) {
            for (auto member : decl->GetMemberDeclPtrs()) {
                if (ShouldSkipDecl(*member)) {
                    member->doNotExport = true;
                }
            }
        }
        return VisitAction::SKIP_CHILDREN;
    }).Walk();
}

std::string LSPCompilerInstance::Denoising(std::string candidate)
{
    return ark::CompilerCodiraProject::GetInstance()->Denoising(candidate);
}
