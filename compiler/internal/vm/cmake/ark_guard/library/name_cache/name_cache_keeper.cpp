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

#include "name_cache_keeper.h"

#include "logger.h"
#include "obfuscator/obfuscator_data_manager.h"

namespace {
const std::string FULL_NAME_SEP = ".";
}

void ark::guard::NameCacheKeeper::Process(const ObjectNameCacheTable &moduleCaches)
{
    for (const auto &[moduleName, moduleNameCache] : moduleCaches) {
        auto module = fileView_.GetModule(moduleName);
        if (!module.has_value()) {
            continue;
        }

        ObfuscatorDataManager manager(module.value());
        auto obfuscateData = manager.GetData();
        if (obfuscateData == nullptr) {
            LOG_E << "Get module obfuscate data failed, module:" << moduleName;
            continue;
        }
        obfuscateData->SetObfName(moduleNameCache->obfName);

        ProcessObjectNameCaches(moduleName, moduleNameCache->objects);
        ProcessMemberNameCaches(moduleName, moduleNameCache->methods);
        ProcessMemberNameCaches(moduleName, moduleNameCache->fields);
    }
}

void ark::guard::NameCacheKeeper::ProcessObjectNameCaches(const std::string &packageName,
                                                          const ObjectNameCacheTable &objectCaches)
{
    for (const auto &[objectName, objectNameCache] : objectCaches) {
        std::string fullName;
        fullName.append(packageName).append(FULL_NAME_SEP).append(objectName);

        auto object = fileView_.GetObject(fullName);
        if (!object.has_value()) {
            continue;
        }

        auto obfObjectName = objectNameCache->obfName;
        ObfuscatorDataManager manager(object.value());
        auto obfuscateData = manager.GetData();
        if (obfuscateData == nullptr) {
            LOG_E << "Get object obfuscate data failed, object:" << fullName;
            continue;
        }
        obfuscateData->SetObfName(obfObjectName);

        ProcessMemberNameCaches(fullName, objectNameCache->methods);
        ProcessMemberNameCaches(fullName, objectNameCache->fields);
    }
}

void ark::guard::NameCacheKeeper::ProcessMemberNameCaches(
    const std::string &packageName, const std::unordered_map<std::string, std::string> &memberCaches)
{
    for (const auto &[oriName, obfName] : memberCaches) {
        std::string fullName;
        fullName.append(packageName).append(FULL_NAME_SEP).append(oriName);

        auto member = fileView_.GetObject(fullName);
        if (!member.has_value()) {
            continue;
        }

        ObfuscatorDataManager manager(member.value());
        auto obfuscateData = manager.GetData();
        if (obfuscateData == nullptr) {
            LOG_E << "Get member obfuscate data failed, member:" << fullName;
            continue;
        }
        obfuscateData->SetObfName(obfName);
    }
}
