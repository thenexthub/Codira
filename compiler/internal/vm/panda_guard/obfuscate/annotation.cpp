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

#include "annotation.h"

#include "utils/logger.h"

#include "configs/guard_context.h"
#include "program.h"

namespace {
constexpr std::string_view TAG = "[Annotation]";
constexpr std::string_view ANNOTATION_RECORD_DELIMITER = ".";
constexpr std::string_view SCOPE_DELIMITER = "#";
const std::vector<std::string> SYS_ANNOTATIONS = {"_ESConcurrentModuleRequestsAnnotation",
                                                  "_ESExpectedPropertyCountAnnotation", "_ESSlotNumberAnnotation"};
}  // namespace

bool panda::guard::Annotation::IsWhiteListAnnotation(const std::string &name)
{
    return std::find(SYS_ANNOTATIONS.begin(), SYS_ANNOTATIONS.end(), name) != SYS_ANNOTATIONS.end();
}

void panda::guard::Annotation::WriteNameCache(const std::string &filePath)
{
    if (!this->needUpdate_) {
        return;
    }
    this->WriteFileCache(filePath);
    this->WritePropertyCache();
}

void panda::guard::Annotation::Update()
{
    LOG(INFO, PANDAGUARD) << TAG << "annotation update for:" << this->name_ << " start";
    this->obfName_ = GuardContext::GetInstance()->GetNameMapping()->GetName(this->name_);

    const auto node = this->program_->nodeTable_.at(this->nodeName_);
    auto obfRecordName = node->obfName_ + ANNOTATION_RECORD_DELIMITER.data() + this->obfName_;
    LOG(INFO, PANDAGUARD) << TAG << "annotation obfName:" << obfRecordName;

    auto entry = this->program_->prog_->record_table.extract(this->recordName_);
    entry.key() = obfRecordName;
    entry.mapped().name = obfRecordName;
    this->program_->prog_->record_table.insert(std::move(entry));

    recordName_ = obfRecordName;
    LOG(INFO, PANDAGUARD) << TAG << "annotation update for:" << this->name_ << " end";
}

void panda::guard::Annotation::RefreshNeedUpdate()
{
    // when skipping obfuscated, the needUpdate_ field will be set to false
    this->needUpdate_ = this->needUpdate_ && TopLevelOptionEntity::NeedUpdate(*this);
}

void panda::guard::Annotation::WriteFileCache(const std::string &filePath)
{
    GuardContext::GetInstance()->GetNameCache()->AddObfIdentifierName(filePath, SCOPE_DELIMITER.data() + this->name_,
                                                                      this->obfName_);
}

void panda::guard::Annotation::WritePropertyCache()
{
    TopLevelOptionEntity::WritePropertyCache(*this);
}
