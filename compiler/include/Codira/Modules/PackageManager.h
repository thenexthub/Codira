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
 * This file declares PackageManager related classes, which resolves dependencies between packages.
 */

#ifndef CODIRA_FRONTENDTOOL_PACKAGEMANAGER_H
#define CODIRA_FRONTENDTOOL_PACKAGEMANAGER_H

#include <stack>
#include <string>
#include <utility>
#include <vector>

#include "Codira/Modules/ImportManager.h"

namespace Codira {
class PackageManager;

/**
 * PackageInfo stores information about the current package, including to packageName, package dir path, source files
 * and dependencies.
 */
struct PackageInfo {
    std::string fullPackageName;
    /**
     * The dependencies of current package.
     */
    std::unordered_set<PackageInfo*> deps;

    explicit PackageInfo(const std::string& fullPackageName) : fullPackageName(fullPackageName)
    {
    }
};

/**
 * PackageManager is used to do module compilation by resolving package dependencies and generating build commands.
 */
class PackageManager {
public:
    explicit PackageManager(ImportManager& importManager) : importManager(importManager)
    {
    }

    /**
     * Resolve package dependencies using topological sort method. It should be noted that circular dependencies are
     * allowed.
     * @param pkgs packages whose dependencies need to be resolved.
     * @param withCodeGen it is true when call this function in CodeGen staged.
     */
    bool ResolveDependence(std::vector<Ptr<AST::Package>>& pkgs);

    /**
     * Return buildOrders for read private member.
     */
    const std::vector<std::vector<PackageInfo*>>& GetBuildOrders() const
    {
        return orderedPackageInfos;
    }

    /**
     * If a package relies on packages which are from other SCC (Strongly Connected Component), it should be recorded.
     */
    std::unordered_map<std::string, std::set<std::string>> packageToOtherSccMap;
    std::unordered_map<std::string, std::unique_ptr<PackageInfo>> packageInfoMap;

private:
    ImportManager& importManager;
    std::vector<std::vector<PackageInfo*>> orderedPackageInfos;
    struct TarjanContext {
        std::unordered_map<PackageInfo*, size_t> indices;  /**< The discovered order of vertices in a DFS. */
        std::unordered_map<PackageInfo*, size_t> lowlinks; /**< The smallest index reachable from the vertex. */
        std::unordered_map<PackageInfo*, bool> onStack;    /**< Indicate whether the vertex is on stack. */
    };

    void TarjanForSCC(TarjanContext& ctx, std::stack<PackageInfo*>& st, size_t& index, PackageInfo* u);

    /**
     * Check and collect dependencies for each package by parsing source files.
     */
    void CollectDeps(std::vector<Ptr<AST::Package>>& pkgs);
    void CollectDepsInFile(AST::File& file, const std::unique_ptr<PackageInfo>& pkgInfo, const AST::Package& pkg);
};
} // namespace Codira
#endif // CODIRA_FRONTENDTOOL_PACKAGEMANAGER_H
