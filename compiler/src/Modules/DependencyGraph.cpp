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
 * This file implements the sub-class DependencyGraph of ImportManager.
 */

#include "Codira/Modules/ImportManager.h"

#include <string>
#include <vector>

#include "Codira/AST/Match.h"
#include "Codira/AST/Utils.h"
#include "Codira/Frontend/CompilerInstance.h"
#include "Codira/Modules/ASTSerialization.h"
#include "Codira/Modules/ModulesUtils.h"

using namespace Codira;
using namespace AST;

const std::vector<Ptr<AST::PackageDecl>>& ImportManager::DependencyGraph::GetDirectDependencyPackageDecls(
    const std::string& fullPackageName)
{
    auto cacheIter = cacheDirectDependencyPackageDecls.find(fullPackageName);
    if (cacheIter != cacheDirectDependencyPackageDecls.cend()) {
        return cacheIter->second;
    }
    std::vector<Ptr<AST::PackageDecl>> packageDecls;
    std::unordered_set<Ptr<AST::PackageDecl>> collected;
    std::queue<Ptr<AST::PackageDecl>> tmpQueue;
    for (auto& edge : GetEdges(fullPackageName)) {
        auto pkgDecl = codeoManager.GetPackageDecl(edge.first);
        if (pkgDecl == nullptr) {
            continue;
        }
        tmpQueue.push(pkgDecl);
    }
    while (!tmpQueue.empty()) {
        auto pkgDecl = tmpQueue.front();
        tmpQueue.pop();
        if (auto [_, succ] = collected.emplace(pkgDecl); succ) {
            packageDecls.emplace_back(pkgDecl);
        }
        if (!pkgDecl->srcPackage->isMacroPackage) {
            continue;
        }
        for (auto& reExportPackageName : pkgReExportMap[pkgDecl->srcPackage->fullPackageName]) {
            auto reExportPkgDecl = codeoManager.GetPackageDecl(reExportPackageName);
            CODEC_NULLPTR_CHECK(reExportPkgDecl);
            if (!reExportPkgDecl->srcPackage->isMacroPackage && collected.count(reExportPkgDecl) == 0) {
                packageDecls.emplace_back(reExportPkgDecl);
                collected.emplace(reExportPkgDecl);
            } else if (reExportPkgDecl->srcPackage->isMacroPackage) {
                tmpQueue.push(reExportPkgDecl);
                collected.emplace(reExportPkgDecl);
            }
        }
    }
    auto [cacheIter1, _] = cacheDirectDependencyPackageDecls.emplace(fullPackageName, std::move(packageDecls));
    return cacheIter1->second;
}

const std::vector<Ptr<AST::PackageDecl>>& ImportManager::DependencyGraph::GetAllDependencyPackageDecls(
    const std::string& fullPackageName, bool includeMacroPkg)
{
    auto keyPair = std::make_pair(fullPackageName, includeMacroPkg);
    auto cacheIter = cacheDependencyPackageDecls.find(keyPair);
    if (cacheIter != cacheDependencyPackageDecls.cend()) {
        return cacheIter->second;
    }
    auto& packageNames = GetAllDependencyPackageNames(fullPackageName, includeMacroPkg);
    std::vector<Ptr<AST::PackageDecl>> packageDecls;
    for (auto& pkgName : packageNames) {
        auto pkgDecl = codeoManager.GetPackageDecl(pkgName);
        if (pkgDecl != nullptr) {
            packageDecls.emplace_back(pkgDecl);
        }
    }
    auto [cacheIter1, _] = cacheDependencyPackageDecls.emplace(keyPair, std::move(packageDecls));
    return cacheIter1->second;
}

const std::set<std::string>& ImportManager::DependencyGraph::GetAllDependencyPackageNames(
    const std::string& fullPackageName, bool includeMacroPkg)
{
    auto cacheIter = cacheDependencyPackageNames.find(fullPackageName);
    if (cacheIter != cacheDependencyPackageNames.cend()) {
        return cacheIter->second;
    }
    std::set<std::string> packageNames;
    // Collect all the dependencies by DFS.
    std::vector<std::string> stack;
    for (auto& edge : GetEdges(fullPackageName)) {
        stack.emplace_back(edge.first);
    }
    while (!stack.empty()) {
        auto pkgName = std::move(stack.back());
        stack.pop_back();
        if (packageNames.count(pkgName) > 0) {
            continue;
        }
        for (auto& edge : GetEdges(pkgName)) {
            stack.emplace_back(edge.first);
        }
        packageNames.emplace(std::move(pkgName));
    }
    Utils::EraseIf(packageNames, [this, includeMacroPkg](auto it) {
        return !includeMacroPkg && codeoManager.IsMacroRelatedPackageName(it);
    });
    auto [cacheIter1, _] = cacheDependencyPackageNames.emplace(fullPackageName, std::move(packageNames));
    return cacheIter1->second;
}

const std::map<std::string, std::set<Ptr<const ImportSpec>, CmpNodeByPos>>& ImportManager::DependencyGraph::GetEdges(
    const std::string& fullPackageName) const
{
    const static std::map<std::string, std::set<Ptr<const AST::ImportSpec>, AST::CmpNodeByPos>> EMPTY_EDGES;
    auto iter = dependencyMap.find(fullPackageName);
    if (iter != dependencyMap.cend()) {
        return iter->second;
    }
    return EMPTY_EDGES;
}

void ImportManager::DependencyGraph::AddDependenciesForPackage(Package& pkg)
{
    if (dependencyMap.count(pkg.fullPackageName) != 0) {
        // Ignore recursive dependency.
        return;
    }
    for (auto& file : pkg.files) {
        CODEC_NULLPTR_CHECK(file);
        for (auto& import : file->imports) {
            CODEC_NULLPTR_CHECK(import);
            AddDependenciesForImport(pkg, *import);
        }
    }
}

void ImportManager::DependencyGraph::AddDependenciesForImport(Package& pkg, const ImportSpec& import)
{
    auto fullPackageName = codeoManager.GetPackageNameByImport(import);
    auto package = codeoManager.GetPackage(fullPackageName);
    if (package == nullptr || package->fullPackageName == pkg.fullPackageName) {
        return; // The import may be failed. Also ignore self import.
    }
    AddDependency(pkg.fullPackageName, package->fullPackageName, import);
    AddDependenciesForPackage(*package);
    auto relation = Modules::GetPackageRelation(pkg.fullPackageName, fullPackageName);
    if (import.IsReExport() && Modules::IsVisible(import, relation)) {
        pkgReExportMap[pkg.fullPackageName].emplace(fullPackageName);
    }
}

// There is an `import v` in package `u`.
void ImportManager::DependencyGraph::AddDependency(
    const std::string& u, const std::string& v, const AST::ImportSpec& import)
{
    auto uIter = dependencyMap.find(u);
    if (uIter == dependencyMap.cend()) {
        std::set<Ptr<const AST::ImportSpec>, AST::CmpNodeByPos> imports = {&import};
        std::map<std::string, std::set<Ptr<const AST::ImportSpec>, AST::CmpNodeByPos>> edges;
        edges.emplace(v, std::move(imports));
        dependencyMap.emplace(u, std::move(edges));
    } else {
        std::map<std::string, std::set<Ptr<const AST::ImportSpec>, AST::CmpNodeByPos>>& edges = uIter->second;
        auto vIter = edges.find(v);
        if (vIter == edges.cend()) {
            std::set<Ptr<const AST::ImportSpec>, AST::CmpNodeByPos> imports = {&import};
            edges.emplace(v, std::move(imports));
        } else {
            vIter->second.emplace(&import);
        }
    }
}
