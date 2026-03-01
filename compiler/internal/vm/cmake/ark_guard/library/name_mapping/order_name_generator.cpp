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

#include "order_name_generator.h"

namespace {
constexpr uint32_t CHAR_COUNT = 26;
constexpr uint32_t CHAR_CODE_A = 97;

std::string FromCharCode(uint32_t code)
{
    return {static_cast<char>(code)};
}
}  // namespace

std::string ark::guard::OrderNameGenerator::GenerateName()
{
    std::string name = FromCharCode(CHAR_CODE_A + charIndex_);
    if (loopNumber_ > 0) {
        name += std::to_string(loopNumber_);
    }

    charIndex_ = (charIndex_ + 1) % CHAR_COUNT;
    if (charIndex_ == 0) {
        loopNumber_ += 1;
    }

    if (IsNameUsed(name)) {
        return GenerateName();
    }
    return name;
}