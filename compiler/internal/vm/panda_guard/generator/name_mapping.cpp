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

#include "name_mapping.h"

#include "utils/logger.h"

#include "configs/guard_context.h"
#include "order_name_generator.h"
#include "util/assert_util.h"
#include "util/string_util.h"

namespace {
constexpr std::string_view TAG = "[Name_Mapping]";
constexpr std::string_view PATH_DELIMITER = "/";
}  // namespace

panda::guard::NameMapping::NameMapping(const std::shared_ptr<NameCache> &nameCache,
                                       std::shared_ptr<GuardOptions> options)
    : options_(std::move(options))
{
    fileNameGenerator_ = std::make_unique<OrderNameGenerator>(nameCache->GetHistoryUsedNames());
    nameGenerator_ = std::make_unique<OrderNameGenerator>(nameCache->GetHistoryUsedNames());
    fileNameMapping_ = nameCache->GetHistoryFileNameMapping();
    nameMapping_ = nameCache->GetHistoryNameMapping();
}

std::string panda::guard::NameMapping::GetName(const std::string &origin, bool generateNew)
{
    std::string obfName;
    if (origin.empty()) {
        LOG(WARNING, PANDAGUARD) << TAG << "origin name is empty";
        return obfName;
    }
    const auto &[prefix, suffix] = StringUtil::SplitAnonymousName(origin);
    do {
        auto item = nameMapping_.find(prefix);
        if (item != nameMapping_.end()) {
            obfName = item->second;
            break;
        }

        if (!generateNew) {
            obfName = prefix;
            break;
        }

        if (StringUtil::IsNumber(prefix)) {
            obfName = prefix;
            break;
        }

        if (StringUtil::IsAnonymousNameSpaceName(prefix)) {
            obfName = prefix;
            break;
        }

        if (options_ && (options_->IsReservedNames(prefix) || options_->IsReservedProperties(prefix) ||
                         options_->IsReservedToplevelNames(prefix))) {
            obfName = prefix;
            break;
        }

        obfName = nameGenerator_->GetName();
    } while (false);
    PANDA_GUARD_ASSERT_PRINT(obfName.empty(), TAG, ErrorCode::GENERIC_ERROR,
                             "GetName name generator failed, name is empty");
    nameMapping_.emplace(origin, obfName);
    obfName.append(suffix);
    LOG(INFO, PANDAGUARD) << TAG << "NameMapping:" << origin << " --> " << obfName;
    return obfName;
}

std::string panda::guard::NameMapping::GetFileName(const std::string &origin, bool generateNew)
{
    std::string obfName;
    if (origin.empty()) {
        LOG(WARNING, PANDAGUARD) << TAG << "origin file name is empty";
        return obfName;
    }
    do {
        auto item = fileNameMapping_.find(origin);
        if (item != fileNameMapping_.end()) {
            obfName = item->second;
            break;
        }

        if (!generateNew) {
            obfName = origin;
            break;
        }

        if (options_ && options_->IsReservedFileNames(origin)) {
            obfName = origin;
            break;
        }

        obfName = fileNameGenerator_->GetName();
        GuardContext::GetInstance()->GetNameCache()->AddNewNameCacheObfFileName(origin, obfName);
    } while (false);
    PANDA_GUARD_ASSERT_PRINT(obfName.empty(), TAG, ErrorCode::GENERIC_ERROR,
                             "GetFileName name generator failed, name is empty");
    fileNameMapping_.emplace(origin, obfName);
    LOG(INFO, PANDAGUARD) << TAG << "FileNameMapping:" << origin << " --> " << obfName;
    return obfName;
}

std::string panda::guard::NameMapping::GetFilePath(const std::string &origin, bool generateNew)
{
    auto parts = StringUtil::StrictSplit(origin, PATH_DELIMITER.data());
    PANDA_GUARD_ASSERT_PRINT(parts.empty() || parts[0].empty(), TAG, ErrorCode::GENERIC_ERROR, "invalid file path");

    std::string obfPath = GetFileName(parts[0], generateNew);
    for (size_t i = 1; i < parts.size(); i++) {
        obfPath += PATH_DELIMITER.data() + GetFileName(parts[i], generateNew);
    }
    return obfPath;
}

void panda::guard::NameMapping::AddReservedNames(const std::set<std::string> &nameList) const
{
    nameGenerator_->AddReservedNames(nameList);
    fileNameGenerator_->AddReservedNames(nameList);
}

void panda::guard::NameMapping::AddNameMapping(const std::string &name)
{
    if (name.empty()) {
        return;
    }
    LOG(INFO, PANDAGUARD) << TAG << "NameMapping[Add]:" << name << " --> " << name;
    nameMapping_.emplace(name, name);
}

void panda::guard::NameMapping::AddNameMapping(const std::string &origin, const std::string &name)
{
    LOG(INFO, PANDAGUARD) << TAG << "NameMapping[Add]:" << origin << " --> " << name;
    nameMapping_.emplace(origin, name);
}

void panda::guard::NameMapping::AddNameMapping(const std::set<std::string> &nameList)
{
    for (const auto &name : nameList) {
        AddNameMapping(name);
    }
}

void panda::guard::NameMapping::AddFileNameMapping(const std::string &name)
{
    if (name.empty()) {
        return;
    }
    LOG(INFO, PANDAGUARD) << TAG << "AddFileNameMapping:" << name << " --> " << name;
    fileNameMapping_.emplace(name, name);
}

void panda::guard::NameMapping::AddFileNameMapping(const std::set<std::string> &nameList)
{
    for (const auto &name : nameList) {
        AddFileNameMapping(name);
    }
}
