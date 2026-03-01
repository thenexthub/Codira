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

#include "name_mapping_manager.h"

#include "abckit_wrapper/member.h"
#include "abckit_wrapper/object.h"

#include "logger.h"
#include "util/assert_util.h"
#include "util/string_util.h"

namespace {
bool IsInWhiteList(const std::string &name)
{
    static const std::unordered_set<std::string> whiteList = {"_ctor_", "_cctor_", "_$init$_"};
    return whiteList.find(name) != whiteList.end();
}
}  // namespace

ark::guard::ErrorCode ark::guard::NameMapping::Create(NameGeneratorType type)
{
    if (type == NameGeneratorType::ORDER) {
        nameGenerator_ = std::make_unique<OrderNameGenerator>();
        return ErrorCode::ERR_SUCCESS;
    }

    LOG_E << "unknown name generator type :" << static_cast<uint32_t>(type);
    return ErrorCode::UNKNOWN_NAME_GENERATOR_TYPE;
}

std::string ark::guard::NameMapping::GetName(const std::string &originName)
{
    if (IsInWhiteList(originName)) {
        LOG_I << "NameMapping WhiteList:" << originName << " --> " << originName;
        return originName;
    }

    auto it = nameMapping_.find(originName);
    if (it != nameMapping_.end()) {
        LOG_I << "NameMapping Cached:" << originName << " --> " << it->second;
        return it->second;
    }

    if (!nameGenerator_) {
        LOG_W << "Name generator not initialized, fallback to original name";
        LOG_W << "NameMapping fallback:" << originName << " --> " << originName;
        return originName;
    }

    std::string obfName;
    do {
        obfName = nameGenerator_->GenerateName();
    } while (obfName == originName);
    nameMapping_.emplace(originName, obfName);
    LOG_I << "NameMapping generated:" << originName << " --> " << obfName;
    return obfName;
}

void ark::guard::NameMapping::AddUsedNameList(const std::string &usedName) const
{
    if (!nameGenerator_) {
        LOG_W << "Failed to add used name list, name generator not initialized";
        return;
    }
    nameGenerator_->AddUsedNameList(usedName);
}

void ark::guard::NameMapping::AddUsedNameList(const std::unordered_set<std::string> &usedNameList) const
{
    if (!nameGenerator_) {
        LOG_W << "Failed to add used name list, name generator not initialized";
        return;
    }
    nameGenerator_->AddUsedNameList(usedNameList);
}

std::shared_ptr<ark::guard::NameMapping> ark::guard::NameMappingManager::GetNameMapping(const std::string &descriptor,
                                                                                        NameGeneratorType type)
{
    auto it = descriptorNameMapping_.find(descriptor);
    if (it != descriptorNameMapping_.end()) {
        return it->second;
    }

    auto nameMapping = std::make_shared<NameMapping>();
    if (nameMapping->Create(type) != ErrorCode::ERR_SUCCESS) {
        return nullptr;
    }

    descriptorNameMapping_.emplace(descriptor, nameMapping);
    return nameMapping;
}