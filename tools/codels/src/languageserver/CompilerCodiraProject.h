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

#ifndef LSPSERVER_COMPILERINSTANCE_H
#define LSPSERVER_COMPILERINSTANCE_H

#include <cangjie/Utils/FileUtil.h>
#include <cstdint>
#include <regex>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include "ArkAST.h"
#include "CodeoManager.h"
#include "DependencyGraph.h"
#include "LSPCompilerInstance.h"
#include "Options.h"
#include "ThrdPool.h"
#include "Codira/AST/Node.h"
#include "Codira/Frontend/CompilerInvocation.h"
#include "Codira/Modules/ImportManager.h"
#include "capabilities/completion/SortModel.h"
#include "capabilities/diagnostic/LSPDiagObserver.h"
#include "common/Callbacks.h"
#include "common/FileStore.h"
#include "common/LRUCache/LRUCache.h"
#include "index/BackgroundIndexDB.h"
#include "index/IndexDatabase.h"
#include "index/IndexStorage.h"
#include "index/MemIndex.h"
#include "logger/Logger.h"

namespace ark {
const unsigned int TEST_LRU_SIZE = 8;
const unsigned int LRU_SIZE = 3;

class CompilerCodiraProject;

struct SCCParam {
    std::unordered_map<std::string, size_t> dfn;
    std::unordered_map<std::string, size_t> low;
    std::unordered_map<std::string, bool> inSt;
    SCCParam() {};
    SCCParam(const std::unordered_map<std::string, size_t> &dfn,
             const std::unordered_map<std::string, size_t> &low,
             const std::unordered_map<std::string, bool> &inSt) : dfn(dfn), low(low), inSt(inSt) {};
};

struct PkgInfo {
public:
    std::string packagePath{};
    std::string packageName{};
    std::string modulePath{};
    std::string moduleName{};
    bool isSourceDir = false;
    bool needReCompile = false;

    std::mutex pkgInfoMutex;
    std::unique_ptr<CompilerInvocation> compilerInvocation;
    std::unordered_map<std::string, std::string> bufferCache;
    std::unique_ptr<DiagnosticEngine> diag;
    std::unique_ptr<DiagnosticEngine> diagTrash;

    explicit PkgInfo(const std::string &pkgPath, const std::string &curModulePath,
                        const std::string &curModuleName, Callbacks *callback);

    ~PkgInfo() = default;
};

enum class Modifier : uint8_t {
    UNDEFINED,
    PRIVATE,
    INTERNAL,
    PROTECTED,
    PUBLIC
};

class CompilerCodiraProject {
public:
    std::mutex mtx;
    std::mutex indexMtx;
    std::recursive_mutex fileCacheMtx;
    std::mutex fileMtx;
    std::atomic_bool isIdentical = true;
    static CompilerCodiraProject *GetInstance();
    static bool GetUseDB()
    {
        return useDB;
    }
    static void SetUseDB(const bool flag)
    {
        useDB = flag;
    }
    static void InitInstance(Callbacks *cb, lsp::IndexDatabase *arkIndexDB);
    void UpdateBuffCache(const std::string &file, bool isContentChange = false);
    void GetRealPath(std::string &path);
    std::string GetFilePathByID(const std::string &curFilePath, unsigned int fileID);
    std::string GetFilePathByID(const Node &node, unsigned int fileID);
    ~CompilerCodiraProject() noexcept
    {
        if (instance) {
            delete instance;
            instance = nullptr;
        }
    }

    ArkAST *GetArkAST(const std::string &fileName)
    {
        std::unique_lock<std::recursive_mutex> lock(fileCacheMtx);
        if (fileCache.find(fileName) != fileCache.end()) {
            return fileCache[fileName].get();
        }
        return nullptr;
    }

    ArkAST *GetParseArkAST(const std::string &fileName)
    {
        std::unique_lock<std::mutex> lock(fileMtx);
        if (fileCacheForParse.find(fileName) != fileCacheForParse.end()) {
            return fileCacheForParse[fileName].get();
        }
        return nullptr;
    }

    bool PkgIsFromSrcOrNoSrc(Ptr<const Codira::AST::Node> node) const
    {
        if (!node) { return false; }
        std::string fullPkgName = GetPkgNameFromNode(node);
        return CIMap.find(fullPkgName) != CIMap.end() || CIMapNotInSrc.find(fullPkgName) != CIMapNotInSrc.end();
    }

    bool PkgIsFromCIMap(const std::string &fullPkgName) const
    {
        return CIMap.find(fullPkgName)!=CIMap.end();
    }

    std::string GetStdLibPath()
    {
        return stdLibPath;
    }

    std::string GetPathFromPkg(const std::string &pkgName)
    {
        auto found = fullPkgNameToPath.find(pkgName);
        if (found != fullPkgNameToPath.end()) {
            return fullPkgNameToPath[pkgName];
        }
        return {};
    }

    bool PkgIsFromCIMapNotInSrc(const std::string &fullPkgName) const
    {
        return CIMapNotInSrc.find(fullPkgName)!=CIMapNotInSrc.end();
    }

    std::vector<std::string> GetFilesInPkg(const std::string &pkgPath) const
    {
        std::vector<std::string> ret;
        auto it = packageInstanceCache.find(pkgPath);
        if (it == packageInstanceCache.end() || !it->second ||!it->second->package) {
            return ret;
        }
        for (auto &file: it->second->package->files) {
            ret.push_back(file->fileName);
        }
        return ret;
    }

    void GetIncDegree(const std::string &pkgName, std::unordered_map<std::string, size_t>& inDegreeMap,
                      std::unordered_map<std::string, bool>& isVisited);

    std::vector<std::string> GetIncTopologySort(const std::string &pkgName);

    void ReportCircularDeps(const std::vector<std::vector<std::string>> &cycles);

    void ReportCombinedCycles();

    std::vector<std::vector<std::string>> ResolveDependence();

    void TarjanForSCC(SCCParam& sccParam,
                      std::stack <std::string>& st, size_t& index, const std::string& pkgName,
                      std::vector<std::vector<std::string>>& cycles);

    void InitPkgInfoAndParseInModule();

    void InitPkgInfoAndParseNotInModule();

    void InitPkgInfoAndParse();

    void InitOneModule(const ModuleInfo &moduleInfo);

    void InitNotInModule();

    bool Compiler(
        const std::string &moduleUri, const nlohmann::json &initializationOptions, const Environment &environment);

    void CheckPackageNameByAbsName(const Codira::AST::File& needCheckedFile, const std::string &fullPackageName);

    std::string GetFullPkgName(const std::string &filePath) const;

    std::string GetFullPkgByDir(const std::string &dirPath) const;

    Ptr<Package> GetSourcePackagesByPkg(const std::string &fullPkgName);

    std::string GetModuleSrcPath(const std::string &modulePath);

    void SetHead(const std::string &fullPkgName) const
    {
        if (pLRUCache == nullptr) { return; }
        if (pLRUCache->HasCache(fullPkgName)) {
            (void) pLRUCache->Get(fullPkgName);
        }
    };

    std::vector<std::string> GetCiMapList()
    {
        std::vector<std::string> ciMap = {};
        for (auto &item:CIMap) {
            ciMap.push_back(item.first);
        }
        return ciMap;
    };

    void InitLRU()
    {
        if (Options::GetInstance().IsOptionSet("test")) {
            pLRUCache = std::make_unique<LRUCache>(TEST_LRU_SIZE);
        } else {
            pLRUCache = std::make_unique<LRUCache>(LRU_SIZE);
        }
    }

    std::vector<std::string> GetCIMapNotInSrcList()
    {
        std::vector<std::string> ciMap = {};
        for (auto &item:CIMapNotInSrc) {
            ciMap.push_back(item.first);
        }
        return ciMap;
    };

    Ptr<Decl> GetDeclInPkgByNode(Ptr<InheritableDecl> curDecl, const std::string& aheadPath = "")
    {
        if (!curDecl) { return curDecl; }
        auto fullPkgName = curDecl->fullPackageName;
        if (fullPkgName.empty() || !IsFromSrcOrNoSrc(curDecl)) { return curDecl; }
        if (!PkgHasSemaCache(fullPkgName)) {
            if (IsFromCIMap(fullPkgName)) {
                IncrementTempPkgCompile(fullPkgName);
            } else {
                IncrementTempPkgCompileNotInSrc(fullPkgName);
            }
            SetHeadByFilePath(aheadPath);
        }
        if (!PkgHasSemaCache(fullPkgName) || !curDecl->curFile ||
            fullPkgNameToPath.find(fullPkgName) == fullPkgNameToPath.end()) { return curDecl; }
        auto fileName = curDecl->curFile->fileName;
        auto dirPath = fullPkgNameToPath[fullPkgName];
        if (packageInstanceCache.find(dirPath) == packageInstanceCache.end() ||
            !packageInstanceCache[dirPath]->package) {
            return curDecl;
        }
        for (auto &file : packageInstanceCache[dirPath]->package->files) {
            if (file->fileName != fileName) {
                continue;
            }
            for (auto &decl : file->decls) {
                if (curDecl->begin == decl->begin && decl->identifier == curDecl->identifier) {
                    return decl.get();
                }
            }
            break;
        }
        return curDecl;
    };

    std::set<Ptr<Codira::AST::ExtendDecl> >
    GetExtendDecls(const std::variant<Ptr<Codira::AST::Ty>, Ptr<Codira::AST::InheritableDecl> > &type,
                   const std::string& packageName)
    {
        if (!pLRUCache) { return {}; }
        if (pLRUCache->HasCache(packageName)) {
            if (!pLRUCache->Get(packageName)) {
                return {};
            }
            return pLRUCache->Get(packageName)->GetExtendDecls(type);
        }
        return {};
    };

    std::vector<Ptr<Codira::AST::Decl> >
    GetAllVisibleExtendMembers(const std::variant<Ptr<Codira::AST::Ty>, Ptr<Codira::AST::InheritableDecl> > &type,
                   const std::string& packageName, const AST::File& curFile)
    {
        if (!pLRUCache) { return {}; }
        if (pLRUCache->HasCache(packageName)) {
            if (!pLRUCache->Get(packageName)) {
                return {};
            }
            return pLRUCache->Get(packageName)->GetAllVisibleExtendMembers(type, curFile);
        }
        return {};
    };

    Candidate GetGivenReferenceTarget(ASTContext &ctx, const std::string& scopeName,
                                      Expr &expr, bool hasLocalDecl, const std::string &pkgForNoSrc)
    {
        if (!pLRUCache) { return {}; }
        if (pLRUCache->HasCache(ctx.fullPackageName)) {
            if (!pLRUCache->Get(ctx.fullPackageName)) {
                return {};
            }
            return pLRUCache->Get(ctx.fullPackageName)->GetGivenReferenceTarget(ctx, scopeName, expr, hasLocalDecl);
        }
        if (pLRUCache->HasCache(pkgForNoSrc)) {
            if (!pLRUCache->Get(pkgForNoSrc)) {
                return {};
            }
            return pLRUCache->Get(pkgForNoSrc)->GetGivenReferenceTarget(ctx, scopeName, expr, hasLocalDecl);
        }
        return {};
    };

    void CompilerOneFile(
        const std::string &file, const std::string &contents, Position pos = {0, 0, 0},
        bool onlyParse = false, const std::string &name = "");

    void IncrementForFileDelete(const std::string &fileName);

    // after workspace init, can use it. pair::second = ModulePath
    std::pair<CodiraFileKind, std::string> GetCodiraFileKind(const std::string &filePath, bool isPkg = false) const;

    int GetFileID(const std::string &fileName);

    int GetFileIDForCompete(const std::string &fileName);

    bool FileHasSemaCache(const std::string &fileName);

    bool PkgHasSemaCache(const std::string &pkgName);

    std::string GetPathBySource(const std::string &fileName, unsigned int id);

    std::string GetPathBySource(const Node &node, unsigned int id);

    void ClearParseCache();

    Position getPackageNameErrPos(const File &file) const;

    std::vector<std::string> GetMacroLibs() const;

    std::string GetCodec() const;

    // Set condition compile to cangjie compiler
    std::unordered_map<std::string, std::string> GetConditionCompile(const std::string& packageName,
        const std::string &moduleName) const;

    std::unordered_map<std::string, std::string> GetConditionCompile() const;

    std::vector<std::string> GetConditionCompilePaths() const;

    void GetDiagCurEditFile(const std::string &file);

    lsp::SymbolIndex *GetIndex() const
    {
        if (useDB) {
            return backgroundIndexDb.get();
        } else {
            return memIndex.get();
        }
    }

    lsp::MemIndex *GetMemIndex() const
    {
        return memIndex.get();
    }

    CodeoManager *GetCodeoManager() const
    {
        return codeoManager.get();
    }

    DependencyGraph *GetDependencyGraph() const
    {
        return graph.get();
    }

    double CalculateScore(const CodeCompletion &item, const std::string &prefix, uint8_t cursorDepth) const
    {
        return model->CalculateScore(item, prefix, cursorDepth);
    }

    void UpdateUsageFrequency(const std::string &item)
    {
        return model->UpdateUsageFrequency(item);
    }

    lsp::BackgroundIndexDB *GetBgIndexDB() const
    {
        return backgroundIndexDb.get();
    };

    bool CheckNeedCompiler(const std::string &fileName);

    void SubmitTasksToPool(const std::unordered_set<std::string> &tasks);

    void IncrementOnePkgCompile(const std::string &filePath, const std::string &contents);

    void IncrementTempPkgCompile(const std::string &basicString);

    void IncrementTempPkgCompileNotInSrc(const std::string &fullPkgName);

    void EraseOtherCache(const std::string &fullPkgName);

    void UpdateOnDisk(const std::string &path);

    std::string Denoising(std::string candidate);

    Modifier GetPackageSpecMod(Node *node);

    bool IsVisibleForPackage(const std::string &curPkgName, const std::string &importPkgName);

    bool IsCurModuleCodeoDep(const std::string &curModule, const std::string &fullPkgName);

    std::unordered_set<std::string> GetOneModuleDeps(const std::string &curModule);

    std::unordered_set<std::string> GetOneModuleDirectDeps(const std::string &curModule);

    bool GetModuleCombined(const std::string &curModule);

    bool IsCombinedSym(const std::string &curModule, const std::string &curPkg, const std::string &symPkg);

    std::string GetWorkSpace()
    {
        return workspace;
    }

    std::string GetModulesHome()
    {
        return modulesHome;
    }

    std::unordered_map<std::string, std::string> GetFullPkgNameToPathMap()
    {
        return fullPkgNameToPath;
    }

    std::string GetContentByFile(const std::string& filePath)
    {
        auto fullPkgName = GetFullPkgName(filePath);
        if (auto found = pkgInfoMap.find(fullPkgName); found != pkgInfoMap.end()) {
            if (auto buffer = found->second->bufferCache.find(filePath); buffer != found->second->bufferCache.end()) {
                return buffer->second;
            }
        }
        return {};
    }

    /**
     * @brief Determine if the modifiers of the current file's package declaration and its parent package
     * comply with the access level restrictions
     *
     * @param needCheckedFile The current file node
     * @param fullPackageName The full package name of the current file
     * @return true Complies with modifier restrictions, should not throw an error
     * @return false Does not comply with modifier restrictions, should throw an error
     */
    bool CheckPackageModifier(const File &needCheckedFile, const std::string &fullPackageName);

    std::unique_ptr<LRUCache> pLRUCache;

    Callbacks* GetCallback();

    std::unique_ptr<LSPCompilerInstance> GetCIForDotComplete(const std::string &filePath, Position pos,
                                                             std::string &contents);

    std::unique_ptr<LSPCompilerInstance> GetCIForFileRefactor(const std::string &filePath);

    void StoreAllPackagesCache();
    
    void EmitDiagsOfFile(const std::string &filePath);

private:
    CompilerCodiraProject(Callbacks *cb, lsp::IndexDatabase *arkIndexDB);

    bool InitCache(const std::unique_ptr<LSPCompilerInstance> &lspCI, const std::string &pkgForPath,
                   bool isInModule = true);

    void InitParseCache(const std::unique_ptr<LSPCompilerInstance> &lspCI, const std::string &pkgForPath);

    void IncrementCompile(const std::string &filePath, const std::string &contents = "", bool isDelete = false);

    void IncrementCompileForComplete(const std::string &name, const std::string &filePath,
        Position pos, const std::string &contents = "");

    void IncrementCompileForCompleteNotInSrc(const std::string &name,
        const std::string &filePath, const std::string &contents = "");

    void IncrementCompileForFileNotInSrc(const std::string &filePath, const std::string &contents = "",
                                         bool isDelete = false);

    void UpdateDownstreamPackages();

    void ClearCacheForDelete(const std::string &fullPkgName, const std::string &dirPath, bool isInModule);

    bool UpdateDependencies(std::string &fullPkgName, const std::unique_ptr<LSPCompilerInstance> &ci);

    bool ParseAndUpdateNotInSrcDep(const std::string &dirPath, const std::unique_ptr<LSPCompilerInstance> &newCI);

    void FullCompilation();

    void BuildIndexFromCodeo();

    void BuildIndexFromCache(const std::string &package);

    void BuildIndex(const std::unique_ptr<LSPCompilerInstance> &ci, bool isFullCompilation = false);

    bool LoadASTCache(const std::string &package);

    void StorePackageCache(const std::string& pkgName);

    void ReleaseMemoryAsync();

    std::string modulesHome;
    std::string stdLibPath;
    std::string cangjiePath;
    std::string codecPath;
    std::string workspace;
    static CompilerCodiraProject *instance;
    Callbacks *callback = nullptr;

    std::vector<std::string> macroLibs;
    // global condition compile
    std::unordered_map<std::string, std::string> passedWhenKeyValue;
    // module condition compile
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> moduleCondition;
    // single package condition compile
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> singlePackageCondition;
    // CODIRA condition path
    std::vector<std::string> passedWhenCfgPaths;

    std::unordered_map<std::string, std::unique_ptr<ArkAST>> fileCache;
    std::unordered_map<std::string, std::unique_ptr<ArkAST>> fileCacheForParse;
    std::unordered_map<std::string, std::unique_ptr<PackageInstance>> packageInstanceCache;    // key: packagePath
    std::unique_ptr<PackageInstance> packageInstanceCacheForParse;

    std::unique_ptr<ModuleManager> moduleManager;
    std::unique_ptr<ThrdPool> thrdPool;
    std::unique_ptr<lsp::CacheManager> cacheManager;
    std::unique_ptr<CodeoManager> codeoManager = std::make_unique<CodeoManager>();
    std::unique_ptr<DependencyGraph> graph = std::make_unique<DependencyGraph>();
    std::unique_ptr<lsp::MemIndex> memIndex = std::make_unique<lsp::MemIndex>();
    std::unique_ptr<SortModel> model = std::make_unique<SortModel>();

    std::unique_ptr<lsp::BackgroundIndexDB> backgroundIndexDb;
    std::unordered_map<std::string, std::string> fullPkgNameToPath;  // key: fullPkgName
    std::unordered_map<std::string, std::string> pathToFullPkgName;  // key: package path
    std::mutex cimapMtx;
    std::unordered_map<std::string, std::unique_ptr<LSPCompilerInstance>> CIMap;
    std::unordered_map<std::string, std::unique_ptr<LSPCompilerInstance>> CIMapNotInSrc;
    std::unique_ptr<LSPCompilerInstance> CIForParse;
    std::unordered_map<std::string, std::unique_ptr<PkgInfo>> pkgInfoMap;         // key: fullPackageName
    std::unordered_map<std::string, std::unique_ptr<PkgInfo>> pkgInfoMapNotInSrc; // key: dirPath for Codira file
    // key: fullPackageName, value: PackageSpec's modifier
    std::unordered_map<std::string, Modifier> pkgToModMap;
    static bool useDB;
};
} // namespace ark

#endif
