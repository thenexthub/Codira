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

#ifndef LSPSERVER_DEPENDENCY_GRAPH_H
#define LSPSERVER_DEPENDENCY_GRAPH_H

#include <algorithm>
#include <iostream>
#include <mutex>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "logger/Logger.h"

namespace ark {

enum EdgeType: uint8_t {
    PRIVATE,
    INTERNEL,
    PROTECTED,
    PUBLIC
};

using MarkedEdgesMap = std::unordered_map<std::string, std::unordered_map<std::string, EdgeType>>;

class DependencyGraph {
public:
    // Get all packages that package 'a' directly depends on
    std::unordered_set<std::string> GetDependencies(const std::string &a) const
    {
        std::lock_guard<std::mutex> lock(graphMutex);
        auto it = dependencies.find(a);
        return it != dependencies.end() ? it->second : std::unordered_set<std::string>();
    }

    // Get all packages that depend on package 'a'
    std::unordered_set<std::string> GetDependents(const std::string &a) const
    {
        std::lock_guard<std::mutex> lock(graphMutex);
        auto it = reverseDependencies.find(a);
        return it != reverseDependencies.end() ? it->second : std::unordered_set<std::string>();
    }

    // Update dependencies for a package
    void UpdateDependencies(const std::string &package, const std::set<std::string> &newDependencies,
                            const std::unordered_map<std::string, EdgeType>& edges)
    {
        std::lock_guard<std::mutex> lock(graphMutex);

        // First, remove all existing dependencies
        if (dependencies.find(package) != dependencies.end()) {
            for (const auto &dep : dependencies[package]) {
                reverseDependencies[dep].erase(package);
                reverseDependencyEdges[dep].erase(package);
            }
            dependencies[package].clear();
        }

        // Add new dependencies
        if (newDependencies.empty()) {
            dependencies[package].insert({});
        }

        for (const auto &newDep : newDependencies) {
            auto ret = dependencies[package].insert(newDep);
            if (!ret.second) {
                Trace::Elog("dependencies insert failed");
            }
            reverseDependencies[newDep].insert(package);
            reverseDependencyEdges[newDep][package] = edges.at(newDep);
        }
    }

    std::unordered_set<std::string> FindMayDependents(const std::string& package) const
    {
        std::unordered_set<std::string> visited;
        std::unordered_set<std::string> mayDeps;

        std::function<void(const std::string&, const std::string&)> dfs =
            [&](const std::string& up, const std::string& down)-> void {
                if (visited.count(down)) {
                    return;
                }
                visited.insert(down);
                if (reverseDependencyEdges.find(up) == reverseDependencyEdges.end() ||
                    reverseDependencyEdges.at(up).find(down) == reverseDependencyEdges.at(up).end()) {
                    return;
                }
                mayDeps.insert(down);
                if (reverseDependencyEdges.at(up).at(down) == EdgeType::PRIVATE) {
                    return;
                }
                for (const std::string& dep: this->GetDependents(down)) {
                    dfs(down, dep);
                }
            };

        for (const auto& downPkg: GetDependents(package)) {
            dfs(package, downPkg);
        }

        return mayDeps;
    }

    // Find all dependencies (direct and transitive) of a given package using DFS
    std::unordered_set<std::string> FindAllDependencies(const std::string &a) const
    {
        std::unordered_set<std::string> allDeps;
        std::unordered_set<std::string> visited;

        std::lock_guard<std::mutex> lock(graphMutex);
        DfsDependencies(a, visited, allDeps);

        return allDeps;
    }

    // Find all dependents (direct and transitive) of a given package using DFS
    std::unordered_set<std::string> FindAllDependents(const std::string &a) const
    {
        std::unordered_set<std::string> allDeps;
        std::unordered_set<std::string> visited;

        std::lock_guard<std::mutex> lock(graphMutex);
        DfsDependent(a, visited, allDeps);

        return allDeps;
    }

    // Topological Sort
    std::vector<std::string> TopologicalSort(bool reverse = false) const
    {
        std::unordered_set<std::string> visited;
        std::unordered_set<std::string> recStack;
        std::vector<std::string> result;

        std::lock_guard<std::mutex> lock(graphMutex);
        for (const auto &pair : dependencies) {
            if (visited.find(pair.first) == visited.end()) {
                if (!TopoSortUtil(pair.first, visited, recStack, result)) {
                    Trace::Elog("Return empty for cyclic graph");
                    return {};
                }
            }
        }

        if (reverse) {
            std::reverse(result.begin(), result.end());
        }
        return result;
    }

    std::vector<std::string> PartialTopologicalSort(std::unordered_set<std::string>& selected,
                                                    bool reverse = false) const
    {
        auto fullTopoResult = TopologicalSort(reverse);
        std::vector<std::string> result;
        for (auto& node: fullTopoResult) {
            if (selected.count(node)) {
                result.emplace_back(node);
            }
        }
        return result;
    }

    // Find a cycle that includes the given package
    std::pair<std::vector<std::vector<std::string>>, bool> FindCycles() const
    {
        std::lock_guard<std::mutex> lock(graphMutex);
        std::vector<std::vector<std::string>> allCycles;
        std::unordered_set<std::string> visited;
        std::unordered_set<std::string> inPath;

        for (const auto &pair : dependencies) {
            if (visited.find(pair.first) == visited.end()) {
                std::vector<std::string> path;
                CyclesDFS(pair.first, visited, inPath, path, allCycles);
            }
        }

        return {allCycles, !allCycles.empty()};
    }

    // For debug: Print all dependencies in the graph
    void PrintDependencies() const
    {
        std::lock_guard<std::mutex> lock(graphMutex);
        std::cerr << "Dependency Graph:\n";
        for (const auto &pair : dependencies) {
            const std::string &package = pair.first;
            const std::unordered_set<std::string> &deps = pair.second;

            std::cerr << package << " depends on: ";
            if (deps.empty()) {
                std::cerr << "No dependencies";
            } else {
                for (const auto &dep : deps) {
                    auto edge = reverseDependencyEdges.at(dep).at(package);
                    std::string import;
                    switch (edge) {
                        case ark::EdgeType::PRIVATE:
                            import = "private";break;
                        case ark::EdgeType::INTERNEL:
                            import = "internel"; break;
                        case ark::EdgeType::PROTECTED:
                            import = "protected"; break;
                        case ark::EdgeType::PUBLIC:
                            import = "public"; break;
                        default:
                            import = "unknown";
                    }
                    std::cerr << dep << "/" << import << " ";
                }
            }
            std::cerr << "\n";
        }
    }

private:
    std::unordered_map<std::string, std::unordered_set<std::string>> dependencies;        // {down pkg, up pkgs}
    std::unordered_map<std::string, std::unordered_set<std::string>> reverseDependencies; // {up pkg, down pkgs}
    MarkedEdgesMap reverseDependencyEdges; // {up pkg, {down pkg, import accessibility}}
    mutable std::mutex graphMutex;

    // Helper function for find all dependencies, will be called with pre-acquired lock
    void DfsDependencies(const std::string &package,
        std::unordered_set<std::string> &visited,
        std::unordered_set<std::string> &result) const
    {
        if (visited.find(package) != visited.end()) {
            return;
        }
        visited.insert(package);

        // Access dependencies within locked scope
        auto it = dependencies.find(package);
        if (it != dependencies.end()) {
            for (const auto &dep : it->second) {
                if (visited.find(dep) == visited.end()) {
                    result.insert(dep);
                    DfsDependencies(dep, visited, result);
                }
            }
        }
    }

    // Helper function for find all dependent, will be called with pre-acquired lock
    void DfsDependent(const std::string &package,
        std::unordered_set<std::string> &visited,
        std::unordered_set<std::string> &result) const
    {
        if (visited.find(package) != visited.end()) {
            return;
        }
        visited.insert(package);

        // Access dependencies within locked scope
        auto it = reverseDependencies.find(package);
        if (it != reverseDependencies.end()) {
            for (const auto &dep : it->second) {
                if (visited.find(dep) == visited.end()) {
                    result.insert(dep);
                    DfsDependent(dep, visited, result);
                }
            }
        }
    }

    void CyclesDFS(const std::string &package,
        std::unordered_set<std::string> &visited,
        std::unordered_set<std::string> &inPath,
        std::vector<std::string> &path,
        std::vector<std::vector<std::string>> &cycles) const
    {
        visited.insert(package);
        inPath.insert(package);
        path.push_back(package);

        auto it = dependencies.find(package);
        if (it != dependencies.end()) {
            for (const auto &dep : it->second) {
                if (inPath.find(dep) != inPath.end()) {
                    // Found a cycle
                    auto cycleStart = std::find(path.begin(), path.end(), dep);
                    cycles.push_back(std::vector<std::string>(cycleStart, path.end()));
                } else if (visited.find(dep) == visited.end()) {
                    CyclesDFS(dep, visited, inPath, path, cycles);
                }
            }
        }

        path.pop_back();
        inPath.erase(package);
    }

    // Helper function for Topological Sort
    bool TopoSortUtil(const std::string &node,
        std::unordered_set<std::string> &visited,
        std::unordered_set<std::string> &recStack,
        std::vector<std::string> &result,
        const bool isCycleDependencies = false) const
    {
        if (recStack.find(node) != recStack.end()) {
            if (isCycleDependencies) {
                return true;
            }
            return false; // detect cycle
        }

        if (visited.find(node) == visited.end()) {
            visited.insert(node);
            recStack.insert(node);

            // Access dependencies within locked scope
            auto it = dependencies.find(node);
            if (it != dependencies.end()) {
                for (const std::string &neighbor : it->second) {
                    if (!TopoSortUtil(neighbor, visited, recStack, result, neighbor == node)) {
                        return false;
                    }
                }
            }

            recStack.erase(node);
            result.push_back(node); // our current finish order
        }
        return true;
    }
};
} // namespace ark
#endif // LSPSERVER_DEPENDENCY_GRAPH_H
