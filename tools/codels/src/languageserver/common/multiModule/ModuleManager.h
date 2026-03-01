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

#ifndef LSPSERVER_MODULEMANAGER_H
#define LSPSERVER_MODULEMANAGER_H

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include "Codira/AST/Node.h"
#include "nlohmann/json.hpp"
#include "MultiModuleCommon.h"

namespace ark {
class ModuleManager {
public:
    ModuleManager(std::string mainModulePath, nlohmann::json multiModuleOption)
        : projectRootPath(std::move(mainModulePath)), multiModuleOption(std::move(multiModuleOption))
    {
    }

    ~ModuleManager() = default;

    void WorkspaceModeParser(const std::string &workspace = "");

    std::unordered_set<std::string> GetAllRequiresOneModule(
        const std::string &require, std::unordered_map<std::string, bool> &isVisited);

    void SetPackageRequires(const nlohmann::json &jsonData, const std::string &modulePath);

    void SetRequireAllPackages();

    std::string GetExpectedPkgName(const Codira::AST::File &file);

    std::string projectRootPath;
    nlohmann::json multiModuleOption;
    // key: modulePath, value: ModuleInfo
    std::unordered_map<std::string, ModuleInfo> moduleInfoMap;
    // key: moduleName, value: requires
    std::unordered_map<std::string, std::unordered_set<std::string>> requirePackages;
    // key: moduleName, value: allRequires
    std::unordered_map<std::string, std::unordered_set<std::string>> requireAllPackages;
    // key: moduleName, value: modulePaths with same moduleName
    std::unordered_map<std::string, std::vector<std::string>> duplicateModules;
    // key: moduleName, value: cur module is combined
    std::unordered_map<std::string, bool> combinedMap;
};
} // namespace ark

#endif // LSPSERVER_MODULEMANAGER_H
