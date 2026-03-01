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

#include "file_path.h"

#include "configs/guard_context.h"
#include "program.h"
#include "util/assert_util.h"
#include "util/string_util.h"

namespace {
constexpr std::string_view TAG = "[FilePath]";
constexpr std::string_view COLON_DELIMITER = ":";
constexpr std::string_view BUNDLE_PREFIX = "@bundle:";
constexpr std::string_view NORMALIZED_LOCAL_PREFIX = "@normalized:N&&";
constexpr std::string_view NORMALIZED_OHM_DELIMITER = "&";
constexpr size_t FILEPATH_ITEM_MAX_PART_NUM = 2;

const std::vector<std::string_view> PATH_PREFIX_LIST = {BUNDLE_PREFIX, NORMALIZED_LOCAL_PREFIX};

bool FindPrefix(const std::string &name, std::string &prefix)
{
    return std::any_of(PATH_PREFIX_LIST.begin(), PATH_PREFIX_LIST.end(), [&](const std::string_view &elem) {
        bool ret = panda::guard::StringUtil::IsPrefixMatched(name, elem.data());
        if (ret) {
            prefix = elem;
        }
        return ret;
    });
}
}  // namespace

void panda::guard::ReferenceFilePath::Init()
{
    std::string prefix;
    if (FindPrefix(filePath_, prefix)) {
        this->prefix_ = prefix;
        this->rawName_ = filePath_.substr(prefix.size(), filePath_.size() - prefix.size());
        this->DeterminePathType();
    } else if (filePath_.find(COLON_DELIMITER) == std::string::npos) {
        pathType_ = FilePathType::LOCAL_FILE_NO_PREFIX;
        rawName_ = filePath_;
    } else {
        pathType_ = FilePathType::EXTERNAL_DEPENDENCE;
    }

    this->isRemoteFile_ = pathType_ == FilePathType::EXTERNAL_DEPENDENCE;
    LOG(INFO, PANDAGUARD) << TAG << "file path: " << (int)pathType_ << " " << rawName_;
}

void panda::guard::ReferenceFilePath::DeterminePathType()
{
    // here determines if this records need to be processed as external dependence, including the following three scene
    auto it = program_->nodeTable_.find(rawName_);
    std::string pkgName;
    if (it != program_->nodeTable_.end()) {
        pkgName = it->second->pkgName_;
    } else {
        /* scene 1: record that does not exist in abc, when HAP references the record in HSP, the corresponding file
         * does not exist in abc */
        const auto itRaw = program_->prog_->record_table.find(rawName_);
        if (itRaw == program_->prog_->record_table.end()) {
            LOG(INFO, PANDAGUARD) << TAG << "not found record: " << rawName_;
            pathType_ = FilePathType::EXTERNAL_DEPENDENCE;
            return;
        }

        // scene 2: records that are not code files
        if (!Node::FindPkgName(itRaw->second, pkgName) && !Node::IsJsonFile(itRaw->second)) {
            LOG(INFO, PANDAGUARD) << TAG << "invalid record: " << rawName_;
            pathType_ = FilePathType::EXTERNAL_DEPENDENCE;
            return;
        }
    }

    // scene 3: records whose package names are in the skip list or in remote har pkg list
    const auto &options = GuardContext::GetInstance()->GetGuardOptions();
    if (options->IsSkippedRemoteHar(pkgName) || options->IsReservedRemoteHarPkgNames(pkgName)) {
        pathType_ = FilePathType::EXTERNAL_DEPENDENCE;
        return;
    }

    this->pathType_ = FilePathType::LOCAL_FILE_WITH_PREFIX;
}

void panda::guard::ReferenceFilePath::UpdateObfFilePath()
{
    PANDA_GUARD_ASSERT_PRINT(!StringUtil::IsPrefixMatched(filePath_, prefix_), TAG, ErrorCode::GENERIC_ERROR,
                             "prefix mismatched");
    auto it = program_->nodeTable_.find(rawName_);
    if (it == program_->nodeTable_.end()) {
        LOG(INFO, PANDAGUARD) << TAG << "not found record: " << rawName_;
        obfFilePath_ = filePath_;
    } else {
        obfFilePath_ = prefix_ + it->second->obfName_;
    }
}

std::string panda::guard::ReferenceFilePath::GetRawPath() const
{
    if (prefix_.empty() || prefix_ != NORMALIZED_LOCAL_PREFIX.data()) {
        return rawName_;
    }

    auto parts = StringUtil::StrictSplit(rawName_, NORMALIZED_OHM_DELIMITER.data());
    PANDA_GUARD_ASSERT_PRINT(parts.size() < FILEPATH_ITEM_MAX_PART_NUM, TAG, ErrorCode::GENERIC_ERROR,
                             "unexpected FilePathItem");
    return parts[1];
}

void panda::guard::ReferenceFilePath::SetFilePath(const std::string &filePath)
{
    filePath_ = filePath;
    obfFilePath_ = filePath;
}

void panda::guard::ReferenceFilePath::Update()
{
    switch (pathType_) {
        case FilePathType::LOCAL_FILE_NO_PREFIX: {
            obfFilePath_ = GuardContext::GetInstance()->GetNameMapping()->GetFilePath(filePath_, false);
            break;
        }
        case FilePathType::LOCAL_FILE_WITH_PREFIX: {
            UpdateObfFilePath();
            break;
        }
        case FilePathType::EXTERNAL_DEPENDENCE: {
            return;
        }
    }
}
