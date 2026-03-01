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

#ifndef COMMON_INTERFACES_OBJECTS_STRING_TREE_STRING_INL_H
#define COMMON_INTERFACES_OBJECTS_STRING_TREE_STRING_INL_H

#include "securec.h"
#include "common_interfaces/objects/string/base_string.h"
#include "common_interfaces/objects/string/tree_string.h"

namespace common {
template <typename Allocator, typename WriteBarrier, objects_traits::enable_if_is_allocate<Allocator, BaseObject *>,
          objects_traits::enable_if_is_write_barrier<WriteBarrier>>
TreeString *TreeString::Create(Allocator &&allocator, WriteBarrier &&writeBarrier, ReadOnlyHandle<BaseString> left,
                               ReadOnlyHandle<BaseString> right, uint32_t length, bool compressed)
{
    auto string =
        TreeString::Cast(std::invoke(std::forward<Allocator>(allocator), TreeString::SIZE, ObjectType::TREE_STRING));
    string->InitLengthAndFlags(length, compressed);
    string->SetMixHashcode(0);
    string->SetLeftSubString(std::forward<WriteBarrier>(writeBarrier), left.GetBaseObject());
    string->SetRightSubString(std::forward<WriteBarrier>(writeBarrier), right.GetBaseObject());
    return string;
}

template <typename ReadBarrier>
bool TreeString::IsFlat(ReadBarrier &&readBarrier) const
{
    auto strRight = BaseString::Cast(GetRightSubString<BaseObject *>(std::forward<ReadBarrier>(readBarrier)));
    return strRight->GetLength() == 0;
}

template <bool VERIFY, typename ReadBarrier>
uint16_t TreeString::Get(ReadBarrier &&readBarrier, int32_t index) const
{
    auto length = static_cast<int32_t>(GetLength());
    if constexpr (VERIFY) {
        if ((index < 0) || (index >= length)) {
            return 0;
        }
    }

    if (IsFlat(std::forward<ReadBarrier>(readBarrier))) {
        BaseString *left = BaseString::Cast(GetLeftSubString<BaseObject *>(std::forward<ReadBarrier>(readBarrier)));
        return left->At<VERIFY>(std::forward<ReadBarrier>(readBarrier), index);
    }
    const BaseString *string = this;
    while (true) {
        if (string->IsTreeString()) {
            BaseString *left = BaseString::Cast(
                TreeString::ConstCast(string)->GetLeftSubString<BaseObject *>(std::forward<ReadBarrier>(readBarrier)));
            if (static_cast<int32_t>(left->GetLength()) > index) {
                string = left;
            } else {
                index -= static_cast<int32_t>(left->GetLength());
                string = BaseString::Cast(TreeString::ConstCast(string)->GetRightSubString<BaseObject *>(
                    std::forward<ReadBarrier>(readBarrier)));
            }
        } else {
            return string->At<VERIFY>(std::forward<ReadBarrier>(readBarrier), index);
        }
    }
    UNREACHABLE_CC();
}
}  // namespace common
#endif  // COMMON_INTERFACES_OBJECTS_STRING_TREE_STRING_INL_H