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
 * This file implements the CodeoManager related classes.
 */

#include "Codira/Modules/CodeoManager.h"

#include <queue>

#include "CodeoManagerImpl.h"
#include "Codira/AST/ASTCasting.h"
#include "Codira/AST/Utils.h"
#include "Codira/AST/Walker.h"
#include "Codira/Modules/ASTSerialization.h"
#include "Codira/Modules/ModulesUtils.h"

using namespace Codira;
using namespace AST;

namespace Codira {
namespace {
void AddDeclToMap(Decl& decl, std::map<std::string, OrderedDeclSet>& declMap)
{
    if (decl.astKind == ASTKind::EXTEND_DECL) {
        return; // ExtendDecl cannot be referenced by name.
    }
    if (auto vpd = DynamicCast<VarWithPatternDecl*>(&decl); vpd) {
        // A VarWithPatternDecl is viewed as a collection of VarDecls.
        Walker walker(vpd->irrefutablePattern.get(), [&declMap](Ptr<Node> node) {
            if (auto vd = DynamicCast<VarDecl>(node)) {
                declMap[vd->identifier].emplace(vd);
            }
            return VisitAction::WALK_CHILDREN;
        });
        walker.Walk();
    } else if (auto funcDecl = DynamicCast<FuncDecl*>(&decl);
        funcDecl == nullptr || (!funcDecl->TestAttr(Attribute::MAIN_ENTRY) && funcDecl->identifier != MAIN_INVOKE)) {
        // Main function won't be imported.
        declMap[decl.identifier].emplace(&decl);
    }
}

bool CanInline(const GlobalOptions& opts)
{
    return opts.chirLLVM && opts.optimizationLevel > GlobalOptions::OptimizationLevel::O1 && !opts.enableCompileTest &&
        !opts.enableHotReload;
}
} // namespace

CodeoManager::CodeoManager(const CodeoManager::Config& config) : impl{new CodeoManagerImpl{config}}
{
}

CodeoManager::~CodeoManager()
{
    DeleteASTLoaders();
    delete impl;
}

void CodeoManager::DeleteASTLoaders() const noexcept
{
    for (auto& p : impl->GetPackageNameMap()) {
        delete p.second->loader.get();
        p.second->loader = nullptr;
    }
}

CodeoManagerImpl::CodeoManagerImpl(const CodeoManager::Config& config)
    : diag(config.diag),
      typeManager(config.typeManager),
      globalOptions(config.globalOptions),
      importSrcCode(config.importSrcCode),
      canInline(CanInline(config.globalOptions))
{
}

Ptr<CodeoManagerImpl::PackageInfo> CodeoManagerImpl::GetPackageInfo(const std::string& fullPackageName) const
{
    auto iter = packageNameMap.find(fullPackageName);
    if (iter != packageNameMap.cend()) {
        CODEC_NULLPTR_CHECK(iter->second);
        return iter->second.get();
    }
    return nullptr;
}

bool CodeoManager::IsOnlyUsedByMacro(const std::string& fullPackageName) const
{
    auto info = impl->GetPackageInfo(fullPackageName);
    return info ? info->onlyUsedByMacro : false;
}

void CodeoManager::SetOnlyUsedByMacro(const std::string& fullPackageName, bool onlyUsedByMacro) const
{
    auto info = impl->GetPackageInfo(fullPackageName);
    if (info != nullptr) {
        info->onlyUsedByMacro = onlyUsedByMacro;
    }
}

bool CodeoManager::IsMacroRelatedPackageName(const std::string& fullPackageName) const
{
    auto info = impl->GetPackageInfo(fullPackageName);
    return info != nullptr ? info->onlyUsedByMacro : false;
}

void CodeoManager::UpdateSearchPath(const std::string& codiraModules) const
{
    impl->UpdateSearchPath(codiraModules);
}

const std::vector<std::string>& CodeoManager::GetSearchPath() const
{
    return impl->GetSearchPath();
}

bool CodeoManager::GetCanInline() const
{
    return impl->GetCanInline();
}

void CodeoManager::SetPackageCodeoCache(const std::string& fullPackageName, const std::vector<uint8_t>& codeoData) const
{
    impl->SetPackageCodeoCache(fullPackageName, codeoData);
}

void CodeoManager::ClearCodeoCache() const
{
    impl->ClearCodeoCache();
}

void CodeoManager::ClearVisitedPkgs() const
{
    impl->ClearVisitedPkgs();
}

DiagnosticEngine& CodeoManager::GetDiag() const
{
    return impl->GetDiag();
}

Ptr<std::unordered_map<std::string, Ptr<AST::Decl>>> CodeoManager::GetExportIdDeclMap(
    const std::string& fullPackageName) const
{
    return impl->GetExportIdDeclMap(fullPackageName);
}

std::optional<std::vector<std::string>> CodeoManager::PreReadCommonPartCodeoFiles()
{
    return impl->PreReadCommonPartCodeoFiles(*this);
}

Ptr<ASTLoader> CodeoManager::GetCommonPartCodeo(std::string expectedName) const
{
    return impl->GetCommonPartCodeo(expectedName);
}

Ptr<PackageDecl> CodeoManager::GetPackageDecl(const std::string& fullPackageName) const
{
    auto info = impl->GetPackageInfo(fullPackageName);
    if (info == nullptr) {
        return nullptr;
    }
    return info->pkgDecl.get();
}

Ptr<Package> CodeoManager::GetPackage(const std::string& fullPackageName) const
{
    auto info = impl->GetPackageInfo(fullPackageName);
    if (info == nullptr) {
        return nullptr;
    }
    return info->pkg.get();
}

std::string CodeoManager::GetPackageDepInfo(const std::string& codeoPath) const
{
    auto loader = impl->ReadCodeo("", codeoPath, *this);
    if (loader == nullptr) {
        return "";
    }
    return loader->LoadPackageDepInfo();
}

Ptr<std::unordered_map<std::string, Ptr<AST::Decl>>> CodeoManagerImpl::GetExportIdDeclMap(
    const std::string& fullPackageName) const
{
    auto iter = packageNameMap.find(fullPackageName);
    if (iter != packageNameMap.cend()) {
        CODEC_NULLPTR_CHECK(iter->second);
        return &iter->second->exportIDDeclMap;
    }
    return nullptr;
}

const std::map<std::string, OrderedDeclSet>& CodeoManager::GetPackageMembers(const std::string& fullPackageName) const
{
    const static std::map<std::string, OrderedDeclSet> EMPTY_MAP;
    auto iter = impl->GetPackageNameMap().find(fullPackageName);
    if (iter != impl->GetPackageNameMap().cend()) {
        CODEC_NULLPTR_CHECK(iter->second);
        return iter->second->declMap;
    }
    return EMPTY_MAP;
}

const OrderedDeclSet& CodeoManager::GetPackageMembersByName(
    const std::string& fullPackageName, const std::string& name) const
{
    const static OrderedDeclSet EMPTY_DECLS;
    auto& declMap = GetPackageMembers(fullPackageName);
    auto iter = declMap.find(name);
    if (iter != declMap.cend()) {
        return iter->second;
    }
    return EMPTY_DECLS;
}

Ptr<Decl> CodeoManager::GetImplicitPackageMembersByName(const std::string& fullPackageName, const std::string& name) const
{
    auto info = impl->GetPackageInfo(fullPackageName);
    if (!info) {
        return nullptr;
    }
    auto found = info->implicitDeclMap.find(name);
    return found == info->implicitDeclMap.end() ? nullptr : found->second;
}

bool CodeoManager::LoadPackageHeader(const std::string& fullPackageName, const std::string& codeoPath) const
{
    auto iter = impl->GetPackageNameMap().find(fullPackageName);
    if (iter != impl->GetPackageNameMap().cend()) {
        return true;
    }
    auto loader = impl->ReadCodeo(fullPackageName, codeoPath, *this);
    if (loader == nullptr) {
        return false;
    }
    auto pkg = loader->LoadPackageDependencies();
    if (pkg == nullptr) {
        return false;
    }
    auto pkgInfo = MakeOwned<CodeoManagerImpl::PackageInfo>(CodeoManagerImpl::PackageInfo(
        {.loader = loader.release(), .pkg = pkg.get(), .pkgDecl = MakeOwned<PackageDecl>(*pkg)}));
    impl->AddImportedPackages(pkg);
    impl->GetPackageNameMap().emplace(fullPackageName, std::move(pkgInfo));
    return true;
}

bool CodeoManagerImpl::IsReExportBy(const std::string& srcPackage, const std::string& reExportPackage) const
{
    auto info = GetPackageInfo(srcPackage);
    CODEC_NULLPTR_CHECK(info);
    for (auto& file : info->pkg->files) {
        for (auto& importSpec : file->imports) {
            if (!importSpec->IsReExport()) {
                continue;
            }
            auto found = importedPackageNameMap.find(importSpec.get());
            if (found != importedPackageNameMap.end() && found->second.first == reExportPackage) {
                return true;
            }
        }
    }
    return false;
}

bool CodeoManager::NeedCollectDependency(std::string curName, bool isCurMacro, std::string depName) const
{
    if (depName == curName || impl->AlreadyLoaded(depName)) {
        return false;
    }

    // If current is macro package, only load decls for dependent macro package,
    // otherwise, load decls for all dependent package (macro package was filtered before).
    // NOTE: non-macro package's will never be used through macro package.
    if (auto depPd = GetPackageDecl(depName); depPd) {
        bool isDepMacro = depPd->srcPackage->isMacroPackage;
        if (!isCurMacro || isDepMacro || depName == AST_PACKAGE_NAME || impl->IsReExportBy(curName, depName)) {
            return true;
        }
    }

    return false;
}

void CodeoManager::LoadFilesOfCommonPart(Ptr<Package> pkg)
{
    if (!impl->GetGlobalOptions().IsCompilingCODEMP()) {
        return;
    }
    auto commonLoader = GetCommonPartCodeo(pkg->fullPackageName);
    if (!commonLoader) {
        return;
    }
    commonLoader->PreloadCommonPartOfPackage(*pkg);
}

void CodeoManager::LoadPackageDeclsOnDemand(const std::vector<Ptr<Package>>& packages, bool fromLsp) const
{
    // Add all directly imported package's loader.
    std::queue<Ptr<CodeoManagerImpl::PackageInfo>> q;
    for (auto pkg : packages) {
        if (fromLsp) {
            q.push(impl->GetPackageInfo(pkg->fullPackageName));
        }
        for (auto& file : pkg->files) {
            for (auto& import : file->imports) {
                auto pkgName = GetPackageNameByImport(*import);
                if (!pkgName.empty()) {
                    q.push(impl->GetPackageInfo(pkgName));
                }
            }
        }
    }

    std::vector<Ptr<ASTLoader>> loaders;
    // Load common part codeo
    for (auto pkg : packages) {
        if (impl->GetGlobalOptions().IsCompilingCODEMP()) {
            std::string expectedPackageName = pkg->fullPackageName;
            auto commonLoader = GetCommonPartCodeo(expectedPackageName);
            if (!commonLoader) {
                continue;
            }
            commonLoader->LoadPackageDecls();
            loaders.emplace_back(commonLoader);
        }
    }

    while (!q.empty()) {
        auto cur = q.front();
        q.pop();
        if (cur == nullptr || cur->loader == nullptr) {
            continue; // If any error happens during loading 'codeo', the loader will be null.
        }
        auto pkgName = cur->loader->GetImportedPackageName();
        if (auto [_, success] = impl->AddLoadedPackages(pkgName); !success) {
            continue;
        }
        loaders.emplace_back(cur->loader);
        cur->loader->LoadPackageDecls();
        bool isCurMacro = cur->pkg->isMacroPackage;
        auto deps = cur->loader->GetDependentPackageNames();
        for (auto pkg : deps) {
            if (NeedCollectDependency(pkgName, isCurMacro, pkg)) {
                q.push(impl->GetPackageInfo(pkg));
            }
        }
    }

    for (auto loader : loaders) {
        loader->LoadRefs();
    }
}

void CodeoManager::LoadAllDeclsAndRefs() const
{
    // 'packageNameMap' also contains source package, we only need to load for imported package.
    for (auto& p : impl->GetPackageNameMap()) {
        auto& pkgInfo = p.second;
        CODEC_NULLPTR_CHECK(pkgInfo);
        if (pkgInfo->loader) {
            pkgInfo->loader->LoadPackageDecls();
        }
    }
    for (auto& p : impl->GetPackageNameMap()) {
        if (p.second->loader) {
            p.second->loader->LoadRefs();
        }
    }
}

// Reading common part .codeo is required before parsing to keep fileID stable.
// This method only reads file content and does not build ast nodes.
std::optional<std::vector<std::string>> CodeoManagerImpl::PreReadCommonPartCodeoFiles(CodeoManager& codeoManager)
{
    // use `codeoFileCacheMap`
    std::vector<uint8_t> buffer;
    std::string failedReason;

    if (!globalOptions.commonPartCodeo) {
        diag.DiagnoseRefactor(DiagKindRefactor::module_common_part_path_is_required, DEFAULT_POSITION);
        return std::nullopt;
    }

    CODEC_ASSERT(globalOptions.commonPartCodeo);
    std::string commonPartCodeoPath = *globalOptions.commonPartCodeo;
    FileUtil::ReadBinaryFileToBuffer(commonPartCodeoPath, buffer, failedReason);
    if (!failedReason.empty()) {
        diag.DiagnoseRefactor(
            DiagKindRefactor::module_read_file_to_buffer_failed, DEFAULT_POSITION, commonPartCodeoPath, failedReason);
        return {};
    }

    // name of package is unknown before parsing and reading .codeo, so fake is used.
    std::string fakeName = "";
    commonPartLoader = MakeOwned<ASTLoader>(std::move(buffer), fakeName, typeManager, codeoManager, globalOptions);
    commonPartLoader->SetImportSourceCode(importSrcCode);
    commonPartLoader->PreReadAndSetPackageName();

    return commonPartLoader->ReadFileNames();
}

Ptr<ASTLoader> CodeoManagerImpl::GetCommonPartCodeo(std::string expectedName)
{
    CODEC_ASSERT(commonPartLoader);
    CODEC_ASSERT(globalOptions.commonPartCodeo);

    std::string realName = commonPartLoader->PreReadAndSetPackageName();
    if (realName != expectedName) {
        diag.DiagnoseRefactor(
            DiagKindRefactor::module_common_codeo_wrong_package, DEFAULT_POSITION, realName, expectedName);
        return nullptr;
    }

    return commonPartLoader.get();
}

OwnedPtr<ASTLoader> CodeoManagerImpl::ReadCodeo(
    const std::string& fullPackageName, const std::string& codeoPath, const CodeoManager& codeoManager, bool printErr) const
{
    std::vector<uint8_t> buffer;
    if (auto found = codeoFileCacheMap.find(fullPackageName); found != codeoFileCacheMap.end()) {
        buffer = found->second;
    } else {
        std::string failedReason;
        FileUtil::ReadBinaryFileToBuffer(codeoPath, buffer, failedReason);
        if (printErr && !failedReason.empty()) {
            diag.DiagnoseRefactor(
                DiagKindRefactor::module_read_file_to_buffer_failed, DEFAULT_POSITION, codeoPath, failedReason);
            return nullptr;
        }
    }
    auto loader = MakeOwned<ASTLoader>(std::move(buffer), fullPackageName, typeManager, codeoManager, globalOptions);
    loader->SetImportSourceCode(importSrcCode);
    return loader;
}

std::unordered_set<std::string> CodeoManager::LoadCachedPackage(const AST::Package& pkg, const std::string& codeoPath,
    const std::map<std::string, Ptr<AST::Decl>>& mangledName2DeclMap) const
{
    auto loader = impl->ReadCodeo(pkg.fullPackageName, codeoPath, *this, false);
    if (loader == nullptr) {
        return {};
    }
    return loader->LoadCachedTypeForPackage(pkg, mangledName2DeclMap);
}

void CodeoManager::AddSourcePackage(AST::Package& pkg) const
{
    auto pkgInfo = MakeOwned<CodeoManagerImpl::PackageInfo>(
        CodeoManagerImpl::PackageInfo({.pkg = &pkg, .pkgDecl = MakeOwned<PackageDecl>(pkg)}));
    impl->GetPackageNameMap().emplace(pkg.fullPackageName, std::move(pkgInfo));
}

void CodeoManager::AddImportedPackageFromASTNode(OwnedPtr<AST::Package>&& pkg) const
{
    auto pkgInfo = MakeOwned<CodeoManagerImpl::PackageInfo>(
        CodeoManagerImpl::PackageInfo({.pkg = pkg.get(), .pkgDecl = MakeOwned<PackageDecl>(*pkg)}));
    impl->GetPackageNameMap().emplace(pkg->fullPackageName, std::move(pkgInfo));
    pkg->EnableAttr(Attribute::TOOL_ADD);
    impl->AddImportedPackages(pkg);
}

void CodeoManagerImpl::AddImportsToMap(
    const ImportSpec& import, const std::string& importedPackage, std::map<std::string, OrderedDeclSet>& declMap) const
{
    auto pkgInfo = GetPackageInfo(importedPackage);
    if (!pkgInfo) {
        return; // Failed to load current package.
    }
    auto importLevel = GetAccessLevel(import);
    if (import.content.kind == ImportKind::IMPORT_ALL) {
        for (auto& [name, decls] : pkgInfo->declMap) {
            auto& targetMap = declMap[name];
            Modules::AddImportedDeclToMap(decls, targetMap, importLevel);
        }
        return;
    }
    auto& decls = pkgInfo->declMap[import.content.identifier];
    if (import.content.kind == ImportKind::IMPORT_SINGLE) {
        auto& targetMap = declMap[import.content.identifier];
        Modules::AddImportedDeclToMap(decls, targetMap, importLevel);
    } else if (import.content.kind == ImportKind::IMPORT_ALIAS) {
        auto& targetMap = declMap[import.content.aliasName];
        Modules::AddImportedDeclToMap(decls, targetMap, importLevel);
    }
}

void CodeoManager::AddPackageDeclMap(const std::string& fullPackageName, const std::string& importedPackage)
{
    auto pkgInfo = impl->GetPackageInfo(fullPackageName);
    // Failed to load current package or already collect the decls.
    if (!pkgInfo || impl->IsVisitedPackage(fullPackageName)) {
        return;
    }
    impl->AddVisitedPackage(fullPackageName);
    auto relation = importedPackage.empty() ? Modules::PackageRelation::CHILD
                                            : Modules::GetPackageRelation(importedPackage, fullPackageName);
    for (auto& file : pkgInfo->pkg->files) {
        // For imported package, 'file->decls' will only contains public decls.
        for (auto& decl : file->decls) {
            if (decl->astKind == ASTKind::MACRO_EXPAND_DECL) {
                continue; // Macro expand decl cannot be reference.
            }
            AddDeclToMap(*decl, pkgInfo->declMap);
        }
        for (auto& decl : file->exportedInternalDecls) {
            if (decl->TestAttr(Attribute::IMPLICIT_USED)) {
                pkgInfo->implicitDeclMap.emplace(decl->identifier, decl.get());
            }
        }
    }
    for (auto& file : pkgInfo->pkg->files) {
        for (auto& import : file->imports) {
            if (import->IsImportMulti()) {
                continue;
            }
            auto pkgName = GetPackageNameByImport(*import);
            if (pkgName.empty()) {
                continue;
            }
            AddPackageDeclMap(pkgName, fullPackageName);
            if (import->IsReExport() && Modules::IsVisible(*import, relation)) {
                impl->AddImportsToMap(*import, pkgName, pkgInfo->declMap);
            }
        }
    }
}

std::string CodeoManager::GetPackageCodeoPath(const std::string& fullPackageName) const
{
    if (auto found = impl->GetCodeoFileCacheMap().find(fullPackageName); found != impl->GetCodeoFileCacheMap().end()) {
        return fullPackageName; // Set dummy path for cached codeo data.
    }
    std::string codeoPath = "";
    if (impl->GetCodeoPathFromFindCache(fullPackageName, codeoPath)) {
        return codeoPath;
    }
    codeoPath = FileUtil::FindSerializationFile(fullPackageName, SERIALIZED_FILE_EXTENSION, GetSearchPath());
    impl->CacheCodeoPathForFind(fullPackageName, codeoPath);
    return codeoPath;
}

std::pair<std::string, std::string> CodeoManager::GetPackageCodeo(const AST::ImportSpec& importSpec) const
{
    std::string codeoPath;
    std::string codeoName;
    for (auto it : GetPossibleCodeoNames(importSpec)) {
        codeoName = it;
        if (auto found = impl->GetCodeoFileCacheMap().find(FileUtil::ToPackageName(codeoName));
            found != impl->GetCodeoFileCacheMap().end()) {
            codeoPath = codeoName; // Set dummy path for cached codeo data.
        } else {
            if (!impl->GetCodeoPathFromFindCache(codeoName, codeoPath)) {
                codeoPath = FileUtil::FindSerializationFile(
                    FileUtil::ToPackageName(codeoName), SERIALIZED_FILE_EXTENSION, GetSearchPath());
                impl->CacheCodeoPathForFind(codeoName, codeoPath);
            }
        }
        if (!codeoPath.empty()) {
            break;
        }
    }
    CODEC_ASSERT(!codeoName.empty());
    auto codeoPackageName = FileUtil::ToPackageName(codeoName);
    // Store importSpec with packageName.
    std::string possibleName = importSpec.content.GetImportedPackageName();
    impl->AddImportedPackageName(&importSpec,
        std::make_pair(
            codeoPackageName, codeoPackageName == possibleName && importSpec.content.kind != ImportKind::IMPORT_ALL));
    return {codeoPackageName, codeoPath};
}

std::vector<std::string> CodeoManager::GetPossibleCodeoNames(const ImportSpec& import) const
{
    // Multi-imports are desugared after parser which should not be used for get package name.
    CODEC_ASSERT(import.content.kind != ImportKind::IMPORT_MULTI);
    if (import.content.prefixPaths.empty()) {
        return {import.content.identifier};
    }
    std::string name;
    std::string_view dot = TOKENS[static_cast<int>(TokenKind::DOT)];
    bool needDc{import.content.hasDoubleColon};
    for (size_t i{needDc ? 1UL : 0UL}; i < import.content.prefixPaths.size(); ++i) {
        name += import.content.prefixPaths[i];
        if (i != import.content.prefixPaths.size() - 1) {
            name += dot;
        }
        needDc = false;
    }
    auto appendOrg = [&import](const std::string& name) {
        if (import.content.hasDoubleColon) {
            return name + std::string{ORG_NAME_SEPARATOR} + import.content.prefixPaths[0];
        }
        return name;
    };
    if (import.content.kind == ImportKind::IMPORT_ALL) {
        return {appendOrg(name)};
    }
    if (auto it = GetPackageNameByImport(import); !it.empty()) {
        return {FileUtil::ToCodeoFileName(it)};
    }
    // if needDc, this import must be of from a::b
    // in this case, the only possible pacakge name is a::b
    if (needDc) {
        return {appendOrg(import.content.identifier)};
    }
    auto maybePackageName = name + std::string{dot} + import.content.identifier.Val();
    return {appendOrg(maybePackageName), appendOrg(name)};
}

std::string CodeoManager::GetPackageNameByImport(const AST::ImportSpec& importSpec) const
{
    return impl->GetPackageNameByImport(importSpec);
}

bool CodeoManager::IsImportPackage(const AST::ImportSpec& importSpec) const
{
    return impl->IsImportPackage(importSpec);
}

std::vector<Ptr<AST::PackageDecl>> CodeoManager::GetAllPackageDecls(bool includeMacroPkg) const
{
    std::vector<Ptr<AST::PackageDecl>> ret;
    for (auto& p : impl->GetPackageNameMap()) {
        auto& pkgInfo = p.second;
        CODEC_ASSERT(pkgInfo && pkgInfo->pkgDecl);
        // Temporarily contains source package to keep same implementation as before.
        if (!pkgInfo->pkg->TestAttr(Attribute::IMPORTED) || includeMacroPkg ||
            (!pkgInfo->onlyUsedByMacro && !pkgInfo->pkg->isMacroPackage)) {
            ret.emplace_back(pkgInfo->pkgDecl.get());
        }
    }
    return ret;
}

void CodeoManager::RemovePackage(const std::string& fullPkgName, const Ptr<Package> package) const
{
    impl->RemoveImportedPackages(package);
    if (auto found = impl->GetPackageNameMap().find(fullPkgName); found != impl->GetPackageNameMap().end()) {
        delete found->second->loader.get();
        found->second->loader = nullptr;
        impl->GetPackageNameMap().erase(fullPkgName);
    }
}
} // namespace Codira
