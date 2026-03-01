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

#include "array.h"

#include "configs/guard_context.h"
#include "program.h"

void panda::guard::Array::RefreshNeedUpdate()
{
    this->needUpdate_ = GuardContext::GetInstance()->GetGuardOptions()->IsFileNameObfEnabled();
}

void panda::guard::Array::Update()
{
    if (!nameInfo_.IsValid()) {
        return;
    }
    if (!node_.has_value()) {
        return;
    }
    const auto &node = node_.value();
    if (node->name_ == node->obfName_) {
        return;
    }

    std::string literalArrayIdx = this->nameInfo_.ins_->GetId(INDEX_0);
    std::string updatedLiteralArrayIdx = literalArrayIdx;
    updatedLiteralArrayIdx.replace(updatedLiteralArrayIdx.find(node->name_), node->name_.size(), node->obfName_);

    UpdateLiteralArrayTableIdx(literalArrayIdx, updatedLiteralArrayIdx);

    this->nameInfo_.ins_->GetId(INDEX_0) = updatedLiteralArrayIdx;
}
