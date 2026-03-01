/**
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "utils/json_builder.h"

#include "utils/string_helpers.h"

using panda::helpers::string::Format;

namespace panda {
void JsonEscape(std::ostream &os, std::string_view string)
{
    os << '"';

    while (!string.empty()) {
        auto iter =
            std::find_if(string.begin(), string.end(), [](char ch) { return ch == '"' || ch == '\\' || ch < ' '; });
        auto pos = iter - string.begin();

        os << string.substr(0, pos);

        if (iter == string.end()) {
            break;
        }

        os << '\\';

        switch (*iter) {
            case '"':
            case '\\':
                os << *iter;
                break;
            case '\b':
                os << 'b';
                break;
            case '\f':
                os << 'f';
                break;
            case '\n':
                os << 'n';
                break;
            case '\r':
                os << 'r';
                break;
            case '\t':
                os << 't';
                break;
            default:
                os << Format("u%04X", *iter);  // NOLINT(cppcoreguidelines-pro-type-vararg)
        }

        string.remove_prefix(pos + 1);
    }

    os << '"';
}
}  // namespace panda
