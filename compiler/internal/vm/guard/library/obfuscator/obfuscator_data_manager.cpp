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

#include "obfuscator_data_manager.h"

#include "logger.h"
#include "member_linker.h"
#include "util/assert_util.h"

ark::guard::ObfuscatorDataManager::ObfuscatorDataManager(abckit_wrapper::Object *object)
{
    this->object_ = object;
    if (this->object_ && !this->object_->data_.has_value()) {
        this->object_->data_ = std::make_shared<ObfuscatorData>();
    }
}

std::shared_ptr<ark::guard::ObfuscatorData> ark::guard::ObfuscatorDataManager::GetData()
{
    if (!this->object_ || !this->object_->data_.has_value()) {
        return nullptr;
    }
    return std::any_cast<std::shared_ptr<ObfuscatorData>>(this->object_->data_);
}

std::shared_ptr<ark::guard::ObfuscatorData> ark::guard::ObfuscatorDataManager::GetParentData()
{
    auto parentObj = this->object_ ? this->object_->parent_.value_or(nullptr) : nullptr;
    if (!parentObj || !parentObj->data_.has_value()) {
        return nullptr;
    }
    return std::any_cast<std::shared_ptr<ObfuscatorData>>(parentObj->data_);
}

void ark::guard::ObfuscatorDataManager::SetNameAndObfuscatedName(const std::string &name, const std::string &obfName)
{
    auto data = GetData();
    if (data == nullptr) {
        LOG_E << "Get obfuscate data failed, name:" << name;
        return;
    }
    data->SetName(name);
    if (data->IsKept()) {
        data->SetObfName(obfName);
    }
}

void ark::guard::ObfuscatorDataManager::SetKeptWithMemberLink(abckit_wrapper::Member *member)
{
    GUARD_ASSERT(member == nullptr, ErrorCode::GENERIC_ERROR, "SetKeptWithMemberLink input nullptr");
    auto lastMember = MemberLinker::LastMember(member);
    if (lastMember != member) {
        ObfuscatorDataManager memberManager(member);
        auto data = memberManager.GetData();
        GUARD_ASSERT(data == nullptr, ErrorCode::GENERIC_ERROR,
                         "Get obfuscate data failed, name:" + member->GetFullyQualifiedName());
        data->SetKept();
    }
    ObfuscatorDataManager lastMemberManager(lastMember);
    auto lastData = lastMemberManager.GetData();
    GUARD_ASSERT(lastData == nullptr, ErrorCode::GENERIC_ERROR,
                     "Get obfuscate data failed, name:" + lastMember->GetFullyQualifiedName());
    lastData->SetKept();
}

bool ark::guard::ObfuscatorDataManager::IsKeptWithMemberLink(abckit_wrapper::Member *member)
{
    GUARD_ASSERT(member == nullptr, ErrorCode::GENERIC_ERROR, "IsKeptWithMemberLink input nullptr");
    auto lastMember = MemberLinker::LastMember(member);
    ObfuscatorDataManager lastMemberManager(lastMember);
    auto lastData = lastMemberManager.GetData();
    GUARD_ASSERT(lastData == nullptr, ErrorCode::GENERIC_ERROR,
                     "Get obfuscate data failed, name:" + lastMember->GetFullyQualifiedName());
    return lastData->IsKept();
}