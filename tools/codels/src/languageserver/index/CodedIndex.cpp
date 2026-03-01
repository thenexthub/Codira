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

#include <fstream>
#include "CodedIndex.h"

namespace Codira {

std::string DCompilerInstance::Denoising(std::string candidate)
{
    return ark::lsp::CodedIndexer::GetInstance()->GetPkgMap().count(candidate) ? candidate : "";
}

void DCompilerInstance::ImportCodeoToManager(const std::unique_ptr<ark::CodeoManager> &codeoManager,
                                           const std::unique_ptr<ark::DependencyGraph> &graph)
{
    // Import stdlib codeo, priority is low.
    for (const auto &codeoCache: codeoFileCacheMap) {
        importManager.SetPackageCodeoCache(codeoCache.first, codeoCache.second);
    }

    auto allDependencies = graph->FindAllDependencies(pkgNameForPath);
    for (auto &package: allDependencies) {
        for (auto &item: usrCodeoFileCacheMap) {
            if (item.second.count(package)) {
                importManager.SetPackageCodeoCache(package, item.second[package]);
            }
        }
    }
}
} // namespace Codira

namespace ark {
namespace  lsp {
CodedIndexer *CodedIndexer::instance = nullptr;

void CodedIndexer::InitInstance(Callbacks *cb, const std::string& stdCodedPathOption,
                              const std::string& ohosCodedPathOption, const std::string& codedCachePathOption)
{
    if (instance == nullptr) {
        instance = new(std::nothrow) CodedIndexer(cb, stdCodedPathOption,
                                                ohosCodedPathOption, codedCachePathOption);
        if (instance == nullptr) {
            Logger::Instance().LogMessage(MessageType::MSG_WARNING, "CodedIndexer::InitInstance fail.");
        }
    }
}

CodedIndexer *CodedIndexer::GetInstance()
{
    return instance;
}

void CodedIndexer::LoadAllCODEDResource()
{
    // 1. load all code.d file, construct package info
    Trace::Log("LoadAllCODEDResource start");
    std::vector<std::string> codedPaths;
    codedPaths.emplace_back(stdCodedPath);
    codedPaths.emplace_back(ohosCodedPath);
    std::map<int, std::vector<std::string>> fileMap;
    for (auto &codedPath: codedPaths) {
        if (!enablePackaged) {
            for (auto &modulePath: FileUtil::GetAllDirsUnderCurrentPath(codedPath)) {
                // just handle the level-1 subdirectory
                if (IsFirstSubDir(codedPath, modulePath)) {
                    ReadCODEDSource(modulePath, modulePath, fileMap);
                }
            }
        } else {
            for (const auto &packagedCodedFile :
                FileUtil::GetAllFilesUnderCurrentPath(codedPath, "d")) {
                ReadPackagedCodedResource(codedPath, packagedCodedFile, fileMap);
            }
        }
    }
    if (CompilerCodiraProject::GetUseDB()) {
        CompilerCodiraProject::GetInstance()->GetBgIndexDB()->UpdateFile(fileMap);
    }
    Trace::Log("LoadAllCODEDResource end");
}

void CodedIndexer::ParsePackageDependencies()
{
    // 2. parse package dependencies
    Trace::Log("ParsePackageDependencies start");
    for (auto &item: pkgMap) {
        auto ci = std::make_unique<DCompilerInstance>(
                callback, *item.second->compilerInvocation, *item.second->diag, item.first);
        ci->cangjieHome = CompilerCodiraProject::GetInstance()->GetModulesHome();
        ci->loadSrcFilesFromCache = true;
        ci->bufferCache = item.second->bufferCache;
        ci->PreCompileProcess();
        std::string fullPackageName = item.first;
        ci->UpdateDepGraph(graph, item.first);
        ciMap[fullPackageName] = std::move(ci);
    }
    Trace::Log("ParsePackageDependencies end");
}

void CodedIndexer::BuildCODEDIndex()
{
    // 3. compiler all packages, build code.d index
    Trace::Log("BuildCODEDIndex start");
    auto sortResult = graph->TopologicalSort(true);
    for (auto &package: sortResult) {
        auto taskId = GenTaskId(package);
        std::unordered_set<uint64_t> dependencies;
        auto allDependencies = graph->FindAllDependencies(package);
        auto task = [this, package, taskId]() {
            Trace::Log("start execute task ", package);
            (void) ciMap[package]->ImportCodeoToManager(codeoManager, graph);
            (void) ciMap[package]->ImportPackage();
            (void) ciMap[package]->MacroExpand();
            (void) ciMap[package]->Sema();
            (void) ExecuteCompilerApi("DeleteASTLoaders", &ImportManager::DeleteASTLoaders,
                                      ciMap[package]->importManager);
            auto packages = ciMap[package]->GetSourcePackages();
            std::vector<uint8_t> data;
            (void) ciMap[package]->ExportAST(false, data, *packages[0]);
            codeoManager->SetData(package, {data, DataStatus::FRESH});
            lsp::SymbolCollector sc = lsp::SymbolCollector(*ciMap[package]->typeManager,
                                                           ciMap[package]->importManager, false);
            sc.Build(*packages[0]);
            pkgSymsMap.insert_or_assign(package, *sc.GetSymbolMap());
            auto shardIdentifier = "coded";
            auto shard = lsp::IndexFileOut();
            shard.symbols = sc.GetSymbolMap();
            shard.refs = sc.GetReferenceMap();
            shard.relations = sc.GetRelations();
            shard.extends = sc.GetSymbolExtendMap();
            shard.crossSymbos = sc.GetCrossSymbolMap();
            cacheManager->StoreIndexShard(package, shardIdentifier, shard);
            thrdPool->TaskCompleted(taskId);
            Trace::Log("finish execute task ", package);
        };
        thrdPool->AddTask(taskId, dependencies, task);
    }
    thrdPool->WaitUntilAllTasksComplete();
    Trace::Log("BuildCODEDIndex end");
}

SymbolLocation CodedIndexer::GetSymbolDeclaration(SymbolID id, const std::string& fullPkgName)
{
    SymbolLocation loc;
    if (auto found = pkgSymsMap.find(fullPkgName); found != pkgSymsMap.end()) {
        for (auto &sym: found->second) {
            if (sym.id == id) {
                loc = sym.location;
                break;
            }
        }
    }
    return loc;
}

CommentGroups CodedIndexer::GetSymbolComments(SymbolID id, const std::string& fullPkgName)
{
    CommentGroups comments;
    if (auto found = pkgSymsMap.find(fullPkgName); found != pkgSymsMap.end()) {
        for (auto &sym: found->second) {
            if (sym.id == id) {
                comments = sym.comments;
                break;
            }
        }
    }
    return comments;
}

void CodedIndexer::ReadCODEDSource(const std::string &rootPath, const std::string &modulePath,
                               std::map<int, std::vector<std::string>> &fileMap, const std::string &parentPkg)
{
    std::string dirName = FileUtil::GetDirName(rootPath);
    std::string currentPkg = parentPkg.empty() ? dirName : parentPkg + "." + dirName;
    pkgMap[currentPkg] =
            std::make_unique<DPkgInfo>(rootPath, modulePath,
                                       FileUtil::GetDirName(modulePath), callback);
    auto allFiles = GetAllFilesUnderCurrentPath(rootPath, "d");
    for (auto &file: allFiles) {
        auto filePath = NormalizePath(JoinPath(rootPath, file));
        LowFileName(filePath);
        pkgMap[currentPkg]->bufferCache.emplace(filePath, GetFileContents(filePath));

        auto id = GetFileIdForDB(filePath);
        std::vector<std::string> fileInfo;
        fileInfo.emplace_back(filePath);
        fileInfo.emplace_back("");
        fileInfo.emplace_back("");
        auto digest = Digest(filePath);
        fileInfo.emplace_back(digest);
        fileMap.insert(std::make_pair(id, fileInfo));
    }
    for (auto &childPath: FileUtil::GetAllDirsUnderCurrentPath(rootPath)) {
        if (FileUtil::IsDir(childPath)) {
            ReadCODEDSource(childPath, modulePath, fileMap, currentPkg);
        }
    }
}

void CodedIndexer::ReadPackagedCodedResource(const std::string& rootPath, const std::string& filePath,
    std::map<int, std::vector<std::string>> &fileMap)
{
    auto pkgName = FileUtil::GetFileBase(FileUtil::GetFileBase(filePath));
    auto normalizedPath = NormalizePath(JoinPath(rootPath, filePath));
    LowFileName(normalizedPath);
    auto ModuleName = pkgName.substr(0, pkgName.find_first_of('.'));
    pkgMap[pkgName] = std::make_unique<DPkgInfo>(rootPath, rootPath, ModuleName, callback);
    pkgMap[pkgName]->bufferCache.emplace(normalizedPath, GetFileContents(normalizedPath));

    auto id = GetFileIdForDB(normalizedPath);
    std::vector<std::string> fileInfo;
    fileInfo.emplace_back(normalizedPath);
    fileInfo.emplace_back("");
    fileInfo.emplace_back("");
    auto digest = Digest(normalizedPath);
    fileInfo.emplace_back(digest);
    fileMap.insert(std::make_pair(id, fileInfo));
}

void CodedIndexer::BuildIndexFromCache()
{
    Trace::Log("BuildIndexFromCache Start");
    std::string codedIndexDir = JoinPath(JoinPath(codedCachePath, ".cache"), "index");
    std::map<int, std::vector<std::string>> fileMap;
    for (auto& idxFile:
            FileUtil::GetAllFilesUnderCurrentPath(codedIndexDir, "idx")) {
        auto package = FileUtil::GetFileBase(FileUtil::GetFileBase(idxFile));
        std::string shardIdentifier = "coded";
        auto indexCache = cacheManager->LoadIndexShard(package, shardIdentifier);
        if (!indexCache.has_value()) {
            Trace::Log("BuildIndexFromCache failed", package);
            return;
        }
        {
            std::unique_lock<std::mutex> indexLock(mtx);
            (void) pkgSymsMap.insert_or_assign(package, indexCache->get()->symbols);
        }

        // update db file table
        if (!CompilerCodiraProject::GetUseDB() || indexCache->get()->symbols.empty()) {
            continue;
        }
        const auto& sym = indexCache->get()->symbols[0];
        std::string absName = FileStore::NormalizePath(sym.location.fileUri);
        std::string curDigest = Digest(absName);
        auto id = GetFileIdForDB(absName);
        std::string oldDigest = CompilerCodiraProject::GetInstance()->GetBgIndexDB()->GetFileDigest(id);
        if (curDigest == oldDigest) {
            continue;
        }
        std::vector<std::string> fileInfo;
        fileInfo.emplace_back(absName);
        fileInfo.emplace_back("");
        fileInfo.emplace_back("");
        fileInfo.emplace_back(curDigest);
        fileMap.insert(std::make_pair(id, fileInfo));

        // check is contain macrocall file
        std::string macroCallFile = FileStore::NormalizePath(JoinPath(sym.location.fileUri, "macrocall"));
        if (!FileExist(macroCallFile)) {
            continue;
        }
        std::string curMacroCallDigest = Digest(macroCallFile);
        auto macroCallId = GetFileIdForDB(absName);
        std::string oldMacroCallDigest =
            CompilerCodiraProject::GetInstance()->GetBgIndexDB()->GetFileDigest(macroCallId);
        if (curMacroCallDigest == oldMacroCallDigest) {
            continue;
        }
        std::vector<std::string> macroCallFileInfo;
        macroCallFileInfo.emplace_back(macroCallFile);
        macroCallFileInfo.emplace_back("");
        macroCallFileInfo.emplace_back("");
        macroCallFileInfo.emplace_back(curMacroCallDigest);
        fileMap.insert(std::make_pair(macroCallId, macroCallFileInfo));
    }
    if (CompilerCodiraProject::GetUseDB()) {
        CompilerCodiraProject::GetInstance()->GetBgIndexDB()->UpdateFile(fileMap);
    }
    Trace::Log("BuildIndexFromCache End");
}

void CodedIndexer::Build()
{
    if (CheckCodedCache()) {
        BuildIndexFromCache();
        return;
    }
    isIndexing = true;
    LoadAllCODEDResource();
    ParsePackageDependencies();
    BuildCODEDIndex();
    GenerateValidFile();
    isIndexing = false;
}

bool CodedIndexer::CheckCodedCache()
{
    const std::string validFile = JoinPath(codedCachePath, "valid.txt");
    std::string reason;
    if (FileExist(validFile) && ReadFileContent(validFile, reason).value_or("") == GetValidCode()) {
        return true;
    }
    return false;
}

std::string CodedIndexer::GetValidCode()
{
    std::string contents;
    std::string reason;
    for (auto& file: FileUtil::GetAllFilesUnderCurrentPath(codedCachePath, "idx")) {
        contents += file + FileUtil::ReadFileContent(file, reason).value_or("");
    }
    return std::to_string(std::hash<std::string>{}(contents));
}

void CodedIndexer::GenerateValidFile()
{
    Trace::Log("Generate Coded Index Valid Files Start");
    std::ofstream validFile;
    validFile.open(Normalize(JoinPath(codedCachePath, "valid.txt")));
    if (!validFile.is_open()) {
        Trace::Log("Create coded index files valid file failed");
    }
    validFile << GetValidCode();
    if (validFile.fail()) {
        Trace::Log("Write coded index files valid file failed");
    }
    validFile.close();
    Trace::Log("Generate Coded Index Valid Files End");
}
} // namespace lsp
} // namespace ark
