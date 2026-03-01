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

#include "ModuleManager.h"

#include "Codira/Utils/FileUtil.h"
#include "../Constants.h"
#include "../FileStore.h"
#include "../../CompilerCodiraProject.h"

using namespace Codira;
using namespace CONSTANTS;
using namespace Codira::FileUtil;

namespace {
std::string GetParentPath(const std::string &filePath)
{
    size_t lastSlashPos = filePath.find_last_of("/\\");
    if (lastSlashPos == std::string::npos) {
        return "";
    }
    return filePath.substr(0, lastSlashPos);
}
}

namespace ark {
void ModuleManager::WorkspaceModeParser(const std::string &workspace)
{
    if (multiModuleOption.is_null()) {
        std::string moduleName = CONSTANTS::DEFAULT_ROOT_PACKAGE;
        std::string normalizeModulePath = FileStore::NormalizePath(URI::Resolve(workspace));
        duplicateModules[moduleName].push_back(normalizeModulePath);
        moduleInfoMap[normalizeModulePath] = {moduleName, normalizeModulePath};
        requirePackages[moduleName].insert(moduleName);
        return;
    }
    for (const auto &moduleOptItem : multiModuleOption.items()) {
        auto &key = moduleOptItem.key();
        const nlohmann::json &value = multiModuleOption[key];
        std::string path = FileStore::NormalizePath(URI::Resolve(key));
        std::string name = CONSTANTS::DEFAULT_ROOT_PACKAGE;
        if (value != nullptr && value.contains(MODULE_JSON_NAME)) {
            name = value.value(MODULE_JSON_NAME, "");
        }
        duplicateModules[name].push_back(path);
        moduleInfoMap[path] = {name, path};
        if (value.contains(SRC_PATH)) {
            auto srcPath = value.value(SRC_PATH, "");
            moduleInfoMap[path].srcPath = FileStore::NormalizePath(URI::Resolve(srcPath));
        }
        if (value.contains(COMBINED)) {
            combinedMap[name] = value.value(COMBINED, false);
        }
        (void)requirePackages[name].insert(name);

        SetPackageRequires(value, path);

        if (value.contains(REQUIRES)) {
            for (const auto &item : value[REQUIRES].items()) {
                auto &reqKey = item.key();
                auto itemPath = value[REQUIRES][reqKey].value(MODULE_JSON_PATH, "");
                std::string requirePath = FileStore::NormalizePath(URI::Resolve(itemPath));
                if (!FileExist(requirePath)) {
                    continue;
                }
                (void)requirePackages[name].insert(reqKey);
            }
        }
    }
}

void ModuleManager::SetPackageRequires(const nlohmann::json &jsonData, const std::string &modulePath)
{
    std::string path;
    std::string normalizePath;
    std::string codeoModuleName;
    if (jsonData.contains(PACKAGES_REQUIRES)) {
        if (jsonData[PACKAGES_REQUIRES].contains(PACKAGE_OPTION)) {
            auto items = jsonData[PACKAGES_REQUIRES][PACKAGE_OPTION].items();
            for (const auto &item : items) {
                auto &key = item.key();
                path = jsonData[PACKAGES_REQUIRES][PACKAGE_OPTION].value(key, "");
                normalizePath = FileStore::NormalizePath(URI::Resolve(path));
                if (!FileExist(normalizePath)) {
                    continue;
                }
                codeoModuleName = GetDirName(GetDirPath(path));
                (void)moduleInfoMap[modulePath].codeoRequiresMap.emplace(codeoModuleName, normalizePath);
            }
        }
        if (jsonData[PACKAGES_REQUIRES].contains(PATH_OPTION) &&
            jsonData[PACKAGES_REQUIRES][PATH_OPTION].is_array()) {
            for (auto &member : jsonData[PACKAGES_REQUIRES][PATH_OPTION]) {
                std::string codeoDir = FileStore::NormalizePath(URI::Resolve(member.get<std::string>()));
                if (!FileExist(codeoDir)) {
                    continue;
                }
                for (const auto &codeoFileName : GetAllFilesUnderCurrentPath(codeoDir, "codeo")) {
                    path = NormalizePath(JoinPath(codeoDir, codeoFileName));
                    auto codeoPackageName = GetFileNameWithoutExtension(codeoFileName);
                    (void)moduleInfoMap[modulePath].codeoRequiresMap.emplace(codeoPackageName, path);
                }
            }
        }
    }
}

std::unordered_set<std::string> ModuleManager::GetAllRequiresOneModule(
    const std::string &require,
    std::unordered_map<std::string, bool> &isVisited)
{
    std::unordered_set<std::string> res;
    if (isVisited[require]) {
        return res;
    }
    isVisited[require] = true;
    auto deps = requirePackages[require];

    if (deps.empty()) {
        return res;
    }
    for (const auto &dependent : requirePackages[require]) {
        auto temp = GetAllRequiresOneModule(dependent, isVisited);
        res.insert(temp.begin(), temp.end());
    }
    res.insert(deps.begin(), deps.end());
    return res;
}

void ModuleManager::SetRequireAllPackages()
{
    for (const auto &require : requirePackages) {
        std::unordered_map<std::string, bool> isVisited;
        auto item = GetAllRequiresOneModule(require.first, isVisited);
        (void)requireAllPackages.emplace(require.first, item);
    }
}

std::string ModuleManager::GetExpectedPkgName(const Codira::AST::File &file)
{
    for (const auto &iter : moduleInfoMap) {
        auto curModulePath = CompilerCodiraProject::GetInstance()->GetModuleSrcPath(iter.second.modulePath);
        if (!IsUnderPath(curModulePath, file.filePath)) {
            continue;
        }
        auto parentDirPath = ::GetParentPath(file.filePath);
        if (curModulePath == parentDirPath) {
            return iter.second.moduleName;
        }
    }
    std::string path = Normalize(file.filePath);
    return CompilerCodiraProject::GetInstance()->GetFullPkgName(path);
}
} // namespace ark
