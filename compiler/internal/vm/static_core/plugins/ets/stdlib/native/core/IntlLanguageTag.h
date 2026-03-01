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

#ifndef PANDA_PLUGINS_ETS_STDLIB_NATIVE_CORE_INTLLOCALETAG_H
#define PANDA_PLUGINS_ETS_STDLIB_NATIVE_CORE_INTLLOCALETAG_H

#include "unicode/locid.h"

#include <string>
#include <vector>

namespace ark::ets::stdlib::intl {
constexpr size_t INTL_INDEX_TWO = 2;
constexpr size_t INTL_INDEX_THREE = 3;
constexpr size_t INTL_INDEX_FOUR = 4;
constexpr size_t INTL_INDEX_FIVE = 5;
constexpr size_t INTL_INDEX_EIGHT = 8;
constexpr uint32_t FLAG = 0x20;

std::string ToStdStringLanguageTag(const icu::Locale &locale);

bool IsStructurallyValidLanguageTag(const std::string &tag);

bool IsPrivateSubTag(const std::string &result, size_t &len);

class LanguageTagListIterator : public icu::Locale::Iterator {
public:
    explicit LanguageTagListIterator(const std::vector<std::string> &tags) : tags_(tags), it_(tags.cbegin()) {}

    UBool hasNext() const override
    {
        return static_cast<UBool>(it_ != tags_.cend());
    }

    const icu::Locale &next() override
    {
        auto status = UErrorCode::U_ZERO_ERROR;
        locale_ = icu::Locale::forLanguageTag((it_++)->c_str(), status);
        return locale_;
    }

private:
    const std::vector<std::string> &tags_;
    std::vector<std::string>::const_iterator it_;
    icu::Locale locale_;
};

inline constexpr int AsciiAlphaToLower(uint32_t c)
{
    return static_cast<int>(c | FLAG);
}

}  // namespace ark::ets::stdlib::intl

#endif  //  PANDA_PLUGINS_ETS_STDLIB_NATIVE_CORE_INTLLOCALETAG_H
