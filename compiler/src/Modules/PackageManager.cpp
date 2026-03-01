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
 * This file implements PackageManager related classes.
 */

#include "Codira/Modules/PackageManager.h"

using namespace Codira;
using namespace AST;

void PackageManager::CollectDepsInFile(File& file, const std::unique_ptr<PackageInfo>& pkgInfo, const Package& pkg)
{
    for (auto& it : file.imports) {
        std::string fullPkgName = importManager.codeoManager->GetPackageNameByImport(*it);
        auto pd = importManager.codeoManager->GetPackageDecl(fullPkgName);
        if (pd == nullptr) {
            continue;
        }
        CODEC_ASSERT(pd && pd->srcPackage);
        auto importPkg = pd->srcPackage;
        // Global init func should not be generated for Java package.
        if (importPkg->TestAttr(AST::Attribute::TOOL_ADD)) {
            continue;
        }
        fullPkgName = importPkg->fullPackageName;
        auto found = packageInfoMap.find(fullPkgName);
        if (found != packageInfoMap.end()) {
            pkgInfo->deps.insert(found->second.get());
        } else {
            (void)packageToOtherSccMap[pkg.fullPackageName].emplace(importPkg->fullPackageName);
        }
    }
}

void PackageManager::CollectDeps(std::vector<Ptr<Package>>& pkgs)
{
    for (auto& pkg : pkgs) {
        auto [_, success] =
            packageInfoMap.emplace(pkg->fullPackageName, std::make_unique<PackageInfo>(pkg->fullPackageName));
        CODEC_ASSERT(success);
    }
    for (auto& pkg : pkgs) {
        auto& pkgInfo = packageInfoMap[pkg->fullPackageName];
        for (auto& file : pkg->files) {
            CollectDepsInFile(*file, pkgInfo, *pkg);
        }
    }
}

// Using Tarjan Algorithm to resolve circular dependence between two or more packages
// and get their buildOrders.
// The function invoke TarjanForSCC and use "for loop" resolve the condition having isolated island.
bool PackageManager::ResolveDependence(std::vector<Ptr<Package>>& pkgs)
{
    packageInfoMap.clear();
    orderedPackageInfos.clear();
    packageToOtherSccMap.clear();
    CollectDeps(pkgs);
    TarjanContext ctx = {};
    std::stack<PackageInfo*> st;
    size_t index = 0;
    while (ctx.indices.size() < packageInfoMap.size()) {
        if (ctx.indices.empty()) {
            TarjanForSCC(ctx, st, index, packageInfoMap.begin()->second.get());
            continue;
        }
        for (auto& i : packageInfoMap) {
            if (ctx.indices[i.second.get()] == 0) {
                TarjanForSCC(ctx, st, index, i.second.get());
            }
        }
    }
    std::unordered_map<std::string, Ptr<Package>> sortPackageMap;
    for (auto& pkg : pkgs) {
        sortPackageMap[pkg->fullPackageName] = pkg;
    }
    bool ret = true;
    size_t pkgIndex = 0;
    for (auto& it : orderedPackageInfos) {
        // Currently compiler does not support to compile multiple circular dependent source packages.
        // CodeoGen will resolve circular dependency before this process.
        ret = ret && it.size() == 1;
        for (auto& pkg : it) {
            pkgs[pkgIndex++] = sortPackageMap[pkg->fullPackageName];
        }
    }
    return ret;
}

void PackageManager::TarjanForSCC(TarjanContext& ctx, std::stack<PackageInfo*>& st, size_t& index, PackageInfo* u)
{
    // Set the bookkeeping info for `u` to the smallest unused `index`.
    ++index;
    ctx.indices[u] = index;
    ctx.lowlinks[u] = index;
    st.emplace(u);
    ctx.onStack[u] = true;

    PackageInfo* v = nullptr;
    for (auto w : u->deps) {
        v = w;
        if (ctx.indices[v] == 0) {
            TarjanForSCC(ctx, st, index, v);
            ctx.lowlinks[u] = std::min(ctx.lowlinks[u], ctx.lowlinks[v]);
        } else if (ctx.onStack[v]) {
            ctx.lowlinks[u] = std::min(ctx.lowlinks[u], ctx.indices[v]);
        }
    }
    if (ctx.lowlinks[u] == ctx.indices[u]) {
        std::vector<PackageInfo*> infos;
        do {
            v = st.top();
            infos.emplace_back(v);
            st.pop();
            ctx.onStack[v] = false;
        } while (v != u);
        orderedPackageInfos.emplace_back(infos);
    }
}
