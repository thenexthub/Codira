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
 * This file declares class CodeoManagerImpl.
 */

#ifndef CODIRA_MODULES_CODEO_MANAGERIMPL_H
#define CODIRA_MODULES_CODEO_MANAGERIMPL_H

#include "Codira/Modules/CodeoManager.h"

namespace Codira {
class CodeoManagerImpl {
public:
    explicit CodeoManagerImpl(const CodeoManager::Config& config);
    struct PackageInfo {
        Ptr<ASTLoader> loader;
        Ptr<AST::Package> pkg;
        OwnedPtr<AST::PackageDecl> pkgDecl;
        std::unordered_map<std::string, Ptr<AST::Decl>> exportIDDeclMap;
        std::map<std::string, AST::OrderedDeclSet> declMap;
        std::map<std::string, Ptr<AST::Decl>> implicitDeclMap;
        std::string codeoPath;
        bool onlyUsedByMacro{false};
    };
    std::unordered_map<std::string, OwnedPtr<CodeoManagerImpl::PackageInfo>>& GetPackageNameMap()
    {
        return packageNameMap;
    }
    Ptr<CodeoManagerImpl::PackageInfo> GetPackageInfo(const std::string& fullPackageName) const;
    DiagnosticEngine& GetDiag()
    {
        return diag;
    }
    Ptr<std::unordered_map<std::string, Ptr<AST::Decl>>> GetExportIdDeclMap(const std::string& fullPackageName) const;
    bool GetCanInline() const
    {
        return canInline;
    }
    OwnedPtr<ASTLoader> ReadCodeo(const std::string& fullPackageName, const std::string& codeoPath,
        const CodeoManager& codeoManager, bool printErr = true) const;
    void AddImportedPackages(OwnedPtr<AST::Package>& pkg)
    {
        importedPackages.emplace_back(std::move(pkg));
    }
    void RemoveImportedPackages(const Ptr<AST::Package> pkg)
    {
        for (auto it = importedPackages.cbegin(); it != importedPackages.cend(); ++it) {
            if (it->get() == pkg) {
                it = importedPackages.erase(it);
                return;
            }
        }
    }
    auto AddLoadedPackages(std::string pkgName)
    {
        return loadedPackages.emplace(pkgName);
    }
    bool AlreadyLoaded(std::string pkgName)
    {
        return std::find(loadedPackages.begin(), loadedPackages.end(), pkgName) != loadedPackages.end();
    }
    bool IsReExportBy(const std::string& srcPackage, const std::string& reExportPackage) const;
    void AddImportedPackageName(Ptr<const AST::ImportSpec> importSpec, std::pair<std::string, bool> pkgNamePair)
    {
        importedPackageNameMap.emplace(importSpec, pkgNamePair);
    }
    std::string GetPackageNameByImport(const AST::ImportSpec& importSpec) const
    {
        auto found = importedPackageNameMap.find(&importSpec);
        return found == importedPackageNameMap.end() ? "" : found->second.first;
    }
    bool IsImportPackage(const AST::ImportSpec& importSpec) const
    {
        auto found = importedPackageNameMap.find(&importSpec);
        return found == importedPackageNameMap.end() ? false : found->second.second;
    }
    void UpdateSearchPath(const std::string& codiraModules)
    {
        searchPath.clear();
        searchPath.insert(searchPath.end(), globalOptions.importPaths.cbegin(), globalOptions.importPaths.cend());
        searchPath.emplace_back(".");
        searchPath.insert(searchPath.end(), globalOptions.environment.codiraPaths.cbegin(),
            globalOptions.environment.codiraPaths.cend());
        searchPath.emplace_back(codiraModules);
    }
    const std::vector<std::string>& GetSearchPath() const
    {
        return searchPath;
    }
    void SetPackageCodeoCache(const std::string& fullPackageName, const std::vector<uint8_t>& codeoData)
    {
        if (fullPackageName.empty() || codeoData.empty()) {
            return;
        }
        codeoFileCacheMap[fullPackageName] = codeoData;
    }
    std::unordered_map<std::string, std::vector<uint8_t>>& GetCodeoFileCacheMap()
    {
        return codeoFileCacheMap;
    }
    bool IsVisitedPackage(const std::string& fullPackageName)
    {
        return visitedPkgs.count(fullPackageName) != 0;
    }
    void AddVisitedPackage(const std::string& fullPackageName)
    {
        visitedPkgs.emplace(fullPackageName);
    }
    void AddImportsToMap(const AST::ImportSpec& import, const std::string& importedPackage,
        std::map<std::string, AST::OrderedDeclSet>& declMap) const;
    void ClearCodeoCache()
    {
        codeoFileCacheMap.clear();
    }
    void ClearVisitedPkgs()
    {
        visitedPkgs.clear();
    }
    std::optional<std::vector<std::string>> PreReadCommonPartCodeoFiles(CodeoManager& codeoManager);
    Ptr<ASTLoader> GetCommonPartCodeo(std::string expectedName);

    const GlobalOptions& GetGlobalOptions()
    {
        return globalOptions;
    }
    /**
     * @brief Get the Codeo Path From Cache
     *
     * @param codeoName
     * @param codeoPath
     * @return true if found
     * @return false if not found
     */
    bool GetCodeoPathFromFindCache(const std::string& codeoName, std::string& codeoPath) const
    {
        auto found = codeoPathFindCache.find(codeoName);
        if (found == codeoPathFindCache.end()) {
            return false;
        }
        codeoPath = found->second;
        return true;
    }
    void CacheCodeoPathForFind(const std::string& codeoName, const std::string& codeoPath)
    {
        codeoPathFindCache[codeoName] = codeoPath;
    }
private:
    DiagnosticEngine& diag;
    TypeManager& typeManager;
    const GlobalOptions& globalOptions;
    bool& importSrcCode;
    std::vector<std::string> searchPath;
    /** Only used to hold ownership of imported packages. */
    std::vector<OwnedPtr<AST::Package>> importedPackages;
    std::unordered_map<std::string, OwnedPtr<CodeoManagerImpl::PackageInfo>> packageNameMap;
    std::unordered_map<std::string, std::vector<uint8_t>> codeoFileCacheMap;
    std::unordered_map<Ptr<const AST::ImportSpec>, std::pair<std::string, bool>> importedPackageNameMap;
    // Searching cache.
    std::unordered_set<std::string> visitedPkgs;

    // Indirectly imported packages which have been used is recorded in loader. Load their decls on demand.
    std::unordered_set<std::string> loadedPackages;
    // common part loader also stored in `packageNameMap`.
    OwnedPtr<ASTLoader> commonPartLoader;
    bool canInline{false};

    // cache codeo file path result for skip FindSerializationFile call, key is possible codeo name without extension, value
    // is codeo path (empty string means not found).
    std::unordered_map<std::string, std::string> codeoPathFindCache;
};
} // namespace Codira
#endif
