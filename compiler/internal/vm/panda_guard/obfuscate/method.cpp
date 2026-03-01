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

#include "method.h"

#include "utils/logger.h"

#include "configs/guard_context.h"
#include "program.h"
#include "util/assert_util.h"

namespace {
constexpr std::string_view TAG = "[Method]";
}

void panda::guard::Method::InitNameCacheScope()
{
    this->nameCacheScope_ = this->name_;
}

void panda::guard::Method::RefreshNeedUpdate()
{
    this->contentNeedUpdate_ = PropertyOptionEntity::NeedUpdate(*this);
    this->nameNeedUpdate_ = this->contentNeedUpdate_ && !IsWhiteListOrAnonymousFunction(this->idx_);

    LOG(INFO, PANDAGUARD) << TAG << "Method contentNeedUpdate: " << (this->contentNeedUpdate_ ? "true" : "false");
    LOG(INFO, PANDAGUARD) << TAG << "Method nameNeedUpdate: " << (this->nameNeedUpdate_ ? "true" : "false");
}

void panda::guard::Method::UpdateDefine() const
{
    auto &literalArrayTable = program_->prog_->literalarray_table;
    auto it = literalArrayTable.find(this->literalArrayIdx_);
    PANDA_GUARD_ASSERT_PRINT(it == literalArrayTable.end(), TAG, ErrorCode::GENERIC_ERROR,
                             "get bad literalArrayIdx:" << literalArrayIdx_);

    it->second.literals_[this->idxIndex_].value_ = this->obfIdx_;
    it->second.literals_[this->nameIndex_].value_ = this->obfName_;
}

void panda::guard::Method::WriteFileCache(const std::string &filePath)
{
    GuardContext::GetInstance()->GetNameCache()->AddObfMemberMethodName(
        filePath, this->nameCacheScope_ + this->GetLines(), this->obfName_);
}

void panda::guard::Method::WritePropertyCache()
{
    PropertyOptionEntity::WritePropertyCache(*this);
}

void panda::guard::OuterMethod::RefreshNeedUpdate()
{
    this->contentNeedUpdate_ = PropertyOptionEntity::NeedUpdate(*this);
    this->nameNeedUpdate_ = this->contentNeedUpdate_ && !IsWhiteListOrAnonymousFunction(this->idx_);

    LOG(INFO, PANDAGUARD) << TAG
                          << "Refresh outerMethod contentNeedUpdate: " << (this->contentNeedUpdate_ ? "true" : "false");
    LOG(INFO, PANDAGUARD) << TAG
                          << "Refresh outerMethod nameNeedUpdate: " << (this->nameNeedUpdate_ ? "true" : "false");
}

void panda::guard::OuterMethod::WriteFileCache(const std::string &filePath)
{
    GuardContext::GetInstance()->GetNameCache()->AddObfMemberMethodName(filePath, this->nameDefine_ + this->GetLines(),
                                                                        this->obfNameDefine_);
}

void panda::guard::OuterMethod::WritePropertyCache()
{
    PropertyOptionEntity::WritePropertyCache(*this);
}

void panda::guard::OuterMethod::SetContentNeedUpdate(bool toUpdate)
{
    this->contentNeedUpdate_ = toUpdate && this->contentNeedUpdate_;
    this->nameNeedUpdate_ = toUpdate && this->nameNeedUpdate_;

    LOG(INFO, PANDAGUARD) << TAG
                          << "Set outerMethod contentNeedUpdate: " << (this->contentNeedUpdate_ ? "true" : "false");
    LOG(INFO, PANDAGUARD) << TAG << "Set outerMethod nameNeedUpdate: " << (this->nameNeedUpdate_ ? "true" : "false");
}

std::string panda::guard::OuterMethod::GetName() const
{
    return this->nameDefine_;
}

std::string panda::guard::OuterMethod::GetObfName() const
{
    return this->obfNameDefine_;
}

void panda::guard::OuterMethod::Build()
{
    Function::Build();
    if (this->nameInfo_.IsValid()) {
        this->nameDefine_ = this->nameInfo_.ins_->GetId(0);
    } else {
        this->nameDefine_ = this->name_;
    }
}

void panda::guard::OuterMethod::Update()
{
    Function::Update();
    if (this->contentNeedUpdate_) {
        this->UpdateNameDefine();
    }
}

void panda::guard::OuterMethod::UpdateNameDefine()
{
    if (this->nameInfo_.IsValid()) {
        this->obfNameDefine_ = GuardContext::GetInstance()->GetNameMapping()->GetName(this->nameDefine_);
        this->nameInfo_.ins_->GetId(0) = this->obfNameDefine_;
        this->program_->prog_->strings.emplace(this->obfNameDefine_);
    } else {
        this->obfNameDefine_ = this->obfName_;
    }
}

bool panda::guard::OuterMethod::IsNameObfuscated() const
{
    return Function::IsNameObfuscated() || this->nameInfo_.IsValid();
}

void panda::guard::PropertyMethod::RefreshNeedUpdate() {}

void panda::guard::PropertyMethod::SetContentNeedUpdate(bool toUpdate)
{
    this->contentNeedUpdate_ = toUpdate && this->contentNeedUpdate_;
    this->nameNeedUpdate_ = toUpdate && this->nameNeedUpdate_;
}
