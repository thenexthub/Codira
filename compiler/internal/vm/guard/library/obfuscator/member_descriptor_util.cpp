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

#include "member_descriptor_util.h"

namespace {
constexpr std::string_view CLOSE_PAREN = ")";

std::string GetDescriptor(const std::string &name)
{
    std::string_view nameView(name);
    const auto closeParenPos = nameView.find_first_of(CLOSE_PAREN);
    if (closeParenPos != std::string_view::npos) {
        return std::string(nameView.substr(0, closeParenPos + 1));
    }
    return std::string();
}
}

std::unordered_map<std::string, std::string> ark::guard::MemberDescriptorUtil::memberDescriptorCache_;

std::string ark::guard::MemberDescriptorUtil::GetOrCreateNewMemberDescriptor(const std::string &descriptor)
{
    const auto cache = memberDescriptorCache_.find(descriptor);
    if (cache != memberDescriptorCache_.end()) {
        return cache->second;
    }

    const auto newDescriptor = GetDescriptor(descriptor);
    return memberDescriptorCache_.emplace(descriptor, newDescriptor).first->second;
}
