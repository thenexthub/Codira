/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef DPROF_LIBDPROF_DPROF_PROFILING_DATA_H
#define DPROF_LIBDPROF_DPROF_PROFILING_DATA_H

#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>

namespace ark::dprof {
class ProfilingData {
public:
    ProfilingData(std::string appName, uint64_t hash, uint32_t pid)
        : appName_(std::move(appName)), hash_(hash), pid_(pid)
    {
    }

    bool SetFeatureDate(const std::string &featureName, std::vector<uint8_t> &&data);
    bool DumpAndResetFeatures();

private:
    std::string appName_;
    uint64_t hash_;
    uint32_t pid_;

    using FeaturesDataMap = std::unordered_map<std::string, std::vector<uint8_t>>;
    FeaturesDataMap featuresDataMap_;
};
}  // namespace ark::dprof

#endif  // DPROF_LIBDPROF_DPROF_PROFILING_DATA_H
