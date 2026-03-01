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

#include "storage.h"
#include "libarkbase/serializer/serializer.h"
#include "libarkbase/utils/logger.h"

#include <dirent.h>
#include <fstream>
#include <memory>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace ark::dprof {
/* static */
std::unique_ptr<AppData> AppData::CreateByParams(const std::string &name, uint64_t hash, uint32_t pid,
                                                 FeaturesMap &&featuresMap)
{
    // CC-OFFNXT(G.RES.09) private constructor
    std::unique_ptr<AppData> appData(new AppData);

    appData->commonInfo_.name = name;
    appData->commonInfo_.hash = hash;
    appData->commonInfo_.pid = pid;
    appData->featuresMap_ = std::move(featuresMap);

    return appData;
}

/* static */
std::unique_ptr<AppData> AppData::CreateByBuffer(const std::vector<uint8_t> &buffer)
{
    // CC-OFFNXT(G.RES.09) private constructor
    std::unique_ptr<AppData> appData(new AppData);

    const uint8_t *data = buffer.data();
    size_t size = buffer.size();
    auto r = serializer::RawBufferToStruct<3>(data, size, appData->commonInfo_);  // 3
    if (!r) {
        LOG(ERROR, DPROF) << "Cannot deserialize buffer to common_info. Error: " << r.Error();
        return nullptr;
    }
    ASSERT(r.Value() <= size);
    data = serializer::ToUint8tPtr(serializer::ToUintPtr(data) + r.Value());
    size -= r.Value();

    r = serializer::BufferToType(data, size, appData->featuresMap_);
    if (!r) {
        LOG(ERROR, DPROF) << "Cannot deserialize features_map. Error: " << r.Error();
        return nullptr;
    }
    ASSERT(r.Value() <= size);
    size -= r.Value();
    if (size != 0) {
        LOG(ERROR, DPROF) << "Cannot deserialize all buffer, unused buffer size: " << size;
        return nullptr;
    }

    return appData;
}

bool AppData::ToBuffer(std::vector<uint8_t> &buffer) const
{
    // 3
    if (!serializer::StructToBuffer<3>(commonInfo_, buffer)) {
        LOG(ERROR, DPROF) << "Cannot serialize common_info";
        return false;
    }
    auto ret = serializer::TypeToBuffer(featuresMap_, buffer);
    if (!ret) {
        LOG(ERROR, DPROF) << "Cannot serialize features_map. Error: " << ret.Error();
        return false;
    }
    return true;
}

/* static */
std::unique_ptr<AppDataStorage> AppDataStorage::Create(const std::string &storageDir, bool createDir)
{
    if (storageDir.empty()) {
        LOG(ERROR, DPROF) << "Storage directory is not set";
        return nullptr;
    }

    struct stat statBuffer {};
    if (::stat(storageDir.c_str(), &statBuffer) == 0) {
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        if (S_ISDIR(statBuffer.st_mode)) {
            // CC-OFFNXT(G.RES.09) private constructor
            return std::unique_ptr<AppDataStorage>(new AppDataStorage(storageDir));
        }

        LOG(ERROR, DPROF) << storageDir << " is already exists and it is neither directory";
        return nullptr;
    }

    if (createDir) {
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        if (::mkdir(storageDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) {
            PLOG(ERROR, DPROF) << "mkdir() failed";
            return nullptr;
        }
        // CC-OFFNXT(G.RES.09) private constructor
        return std::unique_ptr<AppDataStorage>(new AppDataStorage(storageDir));
    }

    return nullptr;
}

bool AppDataStorage::SaveAppData(const AppData &appData)
{
    std::vector<uint8_t> buffer;
    if (!appData.ToBuffer(buffer)) {
        LOG(ERROR, DPROF) << "Cannot serialize AppData to buffer";
        return false;
    }

    // Save buffer to file
    std::string fileName = MakeAppPath(appData.GetName(), appData.GetHash(), appData.GetPid());
    std::ofstream file(fileName, std::ios::binary);
    if (!file.is_open()) {
        LOG(ERROR, DPROF) << "Cannot open file: " << fileName;
        return false;
    }
    file.write(reinterpret_cast<const char *>(buffer.data()), buffer.size());
    if (file.bad()) {
        LOG(ERROR, DPROF) << "Cannot write AppData to file: " << fileName;
        return false;
    }

    LOG(DEBUG, DPROF) << "Save AppData to file: " << fileName;
    return true;
}

struct dirent *DoReadDir(DIR *dirp)
{
    errno = 0;
    return ::readdir(dirp);
}

void AppDataStorage::ForEachApps(const std::function<bool(std::unique_ptr<AppData> &&)> &callback) const
{
    using UniqueDir = std::unique_ptr<DIR, void (*)(DIR *)>;
    UniqueDir dir(::opendir(storageDir_.c_str()), [](DIR *directory) {
        if (::closedir(directory) == -1) {
            PLOG(FATAL, DPROF) << "closedir() failed";
        }
    });
    if (dir.get() == nullptr) {
        PLOG(FATAL, DPROF) << "opendir() failed, dir=" << storageDir_;
        return;
    }

    struct dirent *ent;
    while ((ent = DoReadDir(dir.get())) != nullptr) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
            // Skip a valid name
            continue;
        }

        if (ent->d_type != DT_REG) {
            LOG(ERROR, DPROF) << "Not a regular file: " << ent->d_name;
            continue;
        }

        std::string path = storageDir_ + "/" + ent->d_name;
        struct stat statbuf {};
        if (stat(path.c_str(), &statbuf) == -1) {
            PLOG(ERROR, DPROF) << "stat() failed, path=" << path;
            continue;
        }
        size_t fileSize = statbuf.st_size;

        if (fileSize > MAX_BUFFER_SIZE) {
            LOG(ERROR, DPROF) << "File is to large: " << path;
            continue;
        }

        // Read file
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open()) {
            LOG(ERROR, DPROF) << "Cannot open file: " << path;
            continue;
        }
        std::vector<uint8_t> buffer;
        buffer.reserve(fileSize);
        buffer.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

        auto appData = AppData::CreateByBuffer(buffer);
        if (!appData) {
            LOG(ERROR, DPROF) << "Cannot deserialize file: " << path;
            continue;
        }

        if (!callback(std::move(appData))) {
            break;
        }
    }
}

std::string AppDataStorage::MakeAppPath(const std::string &name, uint64_t hash, uint32_t pid) const
{
    ASSERT(!storageDir_.empty());
    ASSERT(!name.empty());

    std::stringstream str;
    str << storageDir_ << "/" << name << "@" << pid << "@" << hash;
    return str.str();
}
}  // namespace ark::dprof
