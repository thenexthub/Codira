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

#ifndef DPROF_LIBSTORAGE_DPROF_STORAGE_H
#define DPROF_LIBSTORAGE_DPROF_STORAGE_H

#include "libarkbase/macros.h"

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace ark::dprof {
class AppData {
public:
    using FeaturesMap = std::unordered_map<std::string, std::vector<uint8_t>>;
    ~AppData() = default;

    static std::unique_ptr<AppData> CreateByParams(const std::string &name, uint64_t hash, uint32_t pid,
                                                   FeaturesMap &&featuresMap);
    static std::unique_ptr<AppData> CreateByBuffer(const std::vector<uint8_t> &buffer);

    bool ToBuffer(std::vector<uint8_t> &buffer) const;

    std::string GetName() const
    {
        return commonInfo_.name;
    }

    uint64_t GetHash() const
    {
        return commonInfo_.hash;
    }

    uint32_t GetPid() const
    {
        return commonInfo_.pid;
    }

    const FeaturesMap &GetFeaturesMap() const
    {
        return featuresMap_;
    }

private:
    AppData() = default;

    NO_COPY_SEMANTIC(AppData);
    NO_MOVE_SEMANTIC(AppData);

    struct CommonInfo {
        std::string name {};
        uint64_t hash {};
        uint32_t pid {};
    };

    CommonInfo commonInfo_;
    FeaturesMap featuresMap_;
};

class AppDataStorage {
public:
    ~AppDataStorage() = default;

    static const size_t MAX_BUFFER_SIZE = 16 * 1024 * 1024;  // 16MB

    static std::unique_ptr<AppDataStorage> Create(const std::string &storageDir, bool createDir = false);

    bool SaveAppData(const AppData &appData);
    void ForEachApps(const std::function<bool(std::unique_ptr<AppData> &&)> &callback) const;

private:
    explicit AppDataStorage(std::string storageDir) : storageDir_(std::move(storageDir)) {}

    NO_COPY_SEMANTIC(AppDataStorage);
    NO_MOVE_SEMANTIC(AppDataStorage);

    std::string MakeAppPath(const std::string &name, uint64_t hash, uint32_t pid) const;

    std::string storageDir_;
};
}  // namespace ark::dprof

#endif  // DPROF_LIBSTORAGE_DPROF_STORAGE_H
