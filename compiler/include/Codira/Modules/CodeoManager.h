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
 * This file declares the codeo file related classes.
 */

#ifndef CODIRA_MODULES_CODEO_MANAGER_H
#define CODIRA_MODULES_CODEO_MANAGER_H

#include "Codira/AST/Node.h"
#include "Codira/Basic/DiagnosticEngine.h"
#include "Codira/Sema/TypeManager.h"

namespace Codira {
class ASTLoader;

#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
const uint8_t CODEO_MAJOR_VERSION = 0;
const uint8_t CODEO_MINOR_VERSION = 1;
const uint8_t CODEO_PATCH_VERSION = 0;
#endif

class CodeoManager {
public:
    struct Config {
        DiagnosticEngine& diag;
        TypeManager& typeManager;
        const GlobalOptions& globalOptions;
        bool& importSrcCode;
    };
    explicit CodeoManager(const Config& config);
    ~CodeoManager();

    void AddSourcePackage(AST::Package& pkg) const;
    void AddImportedPackageFromASTNode(OwnedPtr<AST::Package>&& pkg) const;
    bool LoadPackageHeader(const std::string& fullPackageName, const std::string& codeoPath) const;
    void LoadAllDeclsAndRefs() const;
    bool NeedCollectDependency(std::string curName, bool isCurMacro, std::string depName) const;
    /**
     * Loads the declaration of each package in packages on demand.
     * If @p fromLsp is false, only the dependent packages of each package in @p packages are loaded.
     * Otherwise, the packages in @p packages are also loaded.
     */
    void LoadPackageDeclsOnDemand(const std::vector<Ptr<AST::Package>>& packages, bool fromLsp = false) const;
    /**
     * Collect visible package of current 'fullPackageName'
     * @param importedPackage the package which imports 'fullPackageName'. Empty for source package.
     */
    void AddPackageDeclMap(const std::string& fullPackageName, const std::string& importedPackage = "");
    /** For loading cached types during incremental compilation. */
    std::unordered_set<std::string> LoadCachedPackage(const AST::Package& pkg, const std::string& codeoPath,
        const std::map<std::string, Ptr<AST::Decl>>& mangledName2DeclMap) const;
    /** For --scan-dependency of codeo. */
    std::string GetPackageDepInfo(const std::string& codeoPath) const;

    Ptr<AST::PackageDecl> GetPackageDecl(const std::string& fullPackageName) const;
    /* Load files from common part to current package.
     * This is required to correctly handle imports from common part.
     */
    void LoadFilesOfCommonPart(Ptr<AST::Package> pkg);
    std::optional<std::vector<std::string>> PreReadCommonPartCodeoFiles();
    Ptr<ASTLoader> GetCommonPartCodeo(std::string expectedName) const;
    Ptr<AST::Package> GetPackage(const std::string& fullPackageName) const;
    std::vector<Ptr<AST::PackageDecl>> GetAllPackageDecls(bool includeMacroPkg = false) const;

    void RemovePackage(const std::string& fullPkgName, const Ptr<AST::Package> package) const;

    const std::map<std::string, AST::OrderedDeclSet>& GetPackageMembers(const std::string& fullPackageName) const;
    const AST::OrderedDeclSet& GetPackageMembersByName(
        const std::string& fullPackageName, const std::string& name) const;
    Ptr<AST::Decl> GetImplicitPackageMembersByName(const std::string& fullPackageName, const std::string& name) const;

    std::string GetPackageCodeoPath(const std::string& fullPackageName) const;
    /** return {fullPackageName, codeoPath} */
    std::pair<std::string, std::string> GetPackageCodeo(const AST::ImportSpec& importSpec) const;
    std::vector<std::string> GetFullPackageNames(const AST::ImportSpec& import) const;
    // for single import "import a.b.c", the possible imported codeo's are a.b and a.b.c
    // for other import specs, the possible name is unique.
    std::vector<std::string> GetPossibleCodeoNames(const AST::ImportSpec& import) const;
    std::string GetPackageNameByImport(const AST::ImportSpec& importSpec) const;

    bool IsImportPackage(const AST::ImportSpec& importSpec) const;

    bool IsOnlyUsedByMacro(const std::string& fullPackageName) const;
    void SetOnlyUsedByMacro(const std::string& fullPackageName, bool onlyUsedByMacro) const;
    bool IsMacroRelatedPackageName(const std::string& fullPackageName) const;

    void UpdateSearchPath(const std::string& codiraModules) const;

    const std::vector<std::string>& GetSearchPath() const;

    bool GetCanInline() const;

    /**
     * For LSP, set cached codeo data @p codeoData and optional corresponding sha256 digest @p encrypt
     * for @param fullPackageName .
     */
    void SetPackageCodeoCache(const std::string& fullPackageName, const std::vector<uint8_t>& codeoData) const;

    void ClearCodeoCache() const;
    void DeleteASTLoaders() const noexcept;

    void ClearVisitedPkgs() const;
    DiagnosticEngine& GetDiag() const;
    Ptr<std::unordered_map<std::string, Ptr<AST::Decl>>> GetExportIdDeclMap(const std::string& fullPackageName) const;

private:
    class CodeoManagerImpl* impl;
};
} // namespace Codira
#endif
