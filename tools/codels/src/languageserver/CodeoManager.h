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

#ifndef LSPSERVER_CODEO_MANAGER_H
#define LSPSERVER_CODEO_MANAGER_H

#include <cstdint>
#include <iostream>
#include <set>
#include <shared_mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "DependencyGraph.h"
#include "logger/Logger.h"

namespace ark {
enum class DataStatus: uint8_t {
    FRESH,
    // Indicates that the package status may be stale, whether compilation is required is determined at runtime
    WEAKSTALE,
    STALE,
};

using SerializedT = std::vector<uint8_t>;

struct CodeoData {
    std::optional<SerializedT> data;
    DataStatus status;
    bool isDocChange;
};

class CodeoManager {
public:
    // copy data
    void SetData(const std::string &fullPkgName, CodeoData data)
    {
        std::unique_lock lock(mutex);
        auto ret = codeoMap.insert_or_assign(fullPkgName, data);
        if (ret.second) {
            Trace::Log("Insert codeo cache of package ", fullPkgName);
        } else {
            Trace::Log("Update codeo cache of package ", fullPkgName);
        }
    }

    // return a shared_ptr
    std::shared_ptr<SerializedT> GetData(const std::string &fullPkgName)
    {
        std::shared_lock lock(mutex);
        auto it = codeoMap.find(fullPkgName);
        if (it != codeoMap.end()) {
            if (it->second.data.has_value()) {
                return std::make_shared<SerializedT>(it->second.data.value());
            }
        }
        return nullptr;
    }

    /**
     * @brief Update the cache status of the package. Usually, after the astdata of this package is exported,
     * the codeo cache of the downstream package is updated to Stale.
     *
     * @param packages  Packages that change state
     * @param status  the state needs to be changed
     * @param isDocChange  the state of pkg files has changed
     */
    void UpdateStatus(const std::unordered_set<std::string> &packages, DataStatus status, bool isDocChange = false)
    {
        std::unique_lock lock(mutex);
        for (auto &package : packages) {
            if (codeoMap.find(package) != codeoMap.end()) {
                if (isDocChange) {
                    codeoMap[package].isDocChange = isDocChange;
                }
                if (codeoMap[package].status == DataStatus::STALE && status == DataStatus::WEAKSTALE) {
                    Trace::Log("package status is stale, cann't change to weak stale", package);
                    continue;
                }
                codeoMap[package].status = status;
                if (status == DataStatus::STALE) {
                    Trace::Log(package + "'s codeo cache is stale");
                } else if (status == DataStatus::WEAKSTALE) {
                    Trace::Log(package + "'s codeo cache is weak stale");
                } else {
                    codeoMap[package].isDocChange = false;
                    Trace::Log(package + "'s codeo cache is fresh");
                }
            }
        }
    }

    /**
     * @brief Check whether the codeo cache corresponding to the package is fresh.
     *
     * @param packages The set of packages to be checked is usually
     * the upstream packages of the packages to be compiled.
     * @return std::unordered_set<std::string> The codeo cache is a collection of
     * stale packages that need to be recompiled.
     */
    std::unordered_set<std::string> CheckStatus(const std::unordered_set<std::string> &packages)
    {
        std::shared_lock lock(mutex);
        std::unordered_set<std::string> result;
        for (auto &package : packages) {
            if (codeoMap.find(package) != codeoMap.end() && codeoMap[package].status != DataStatus::FRESH) {
                    result.emplace(package);
            }
        }
        return result;
    }

    bool IsDocChanged(const std::string& package)
    {
        std::shared_lock lock(mutex);
        if (codeoMap.find(package) != codeoMap.end()) {
            return codeoMap[package].isDocChange;
        }
        return false;
    }

    DataStatus GetStatus(const std::string& package)
    {
        std::shared_lock lock(mutex);
        if (codeoMap.find(package) != codeoMap.end()) {
            return codeoMap[package].status;
        }
        return DataStatus::STALE;
    }

    bool CheckChanged(const std::string& fullPkgName, const SerializedT& data)
    {
        std::shared_lock lock(mutex);
        if (codeoMap.find(fullPkgName) == codeoMap.end()) {
            return true;
        }
        auto oldData = GetData(fullPkgName);
        return !oldData || data != *oldData;
    }

    void UpdateDownstreamPackages(const std::string& package, const std::unique_ptr<DependencyGraph>& graph)
    {
        auto downPackages = graph->FindAllDependents(package);
        auto directDownPackages = graph->FindMayDependents(package);
        UpdateStatus(directDownPackages, DataStatus::STALE);
        UpdateStatus(downPackages, DataStatus::WEAKSTALE);
    }

private:
    mutable std::shared_mutex mutex;
    std::unordered_map<std::string, CodeoData> codeoMap;
};
} // namespace ark
#endif // LSPSERVER_CODEO_MANAGER_H
