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

#ifndef PANDA_ASSEMBLER_IDE_HELPERS_H
#define PANDA_ASSEMBLER_IDE_HELPERS_H

#include <cstdint>
#include <sstream>

namespace ark::pandasm {

// NOLINTBEGIN(misc-non-private-member-variables-in-classes)
struct SourcePosition {
    std::uint32_t line = 0U;
    std::uint32_t column = 0U;

    std::string JsonSerialize() const
    {
        std::stringstream ss;
        ss << "{ "
           << "\"line\": " << line << ", "
           << "\"column\": " << column << " }";
        return ss.str();
    }
};

struct SourceLocation {
    SourcePosition begin;
    SourcePosition end;

    std::string JsonSerialize() const
    {
        std::stringstream ss;
        ss << "{ "
           << "\"begin\": " << begin.JsonSerialize() << ", "
           << "\"end\": " << end.JsonSerialize();
        ss << " }";
        return ss.str();
    }
};
// NOLINTEND(misc-non-private-member-variables-in-classes)

template <typename T>
std::string JsonSerializeItemBody(const T &item)
{
    std::stringstream ss;
    std::string quotedName = "\"" + item.name + "\"";
    ss << "{ "
       << "\"name\": " << quotedName;
    if (item.fileLocation.isDefined) {
        ss << ", "
           << "\"bodyLocation\": " << item.bodyLocation.JsonSerialize() << " }";
    } else {
        ss << " }";
    }
    return ss.str();
}

template <typename T>
std::string JsonSerializeProgramItems(const T &itemTable)
{
    std::stringstream ss;
    ss << "[ ";
    auto it = itemTable.begin();
    if (it != itemTable.end()) {
        ss << JsonSerializeItemBody(it->second);
        ++it;
    }
    while (it != itemTable.end()) {
        ss << ", " << JsonSerializeItemBody(it->second);
        ++it;
    }
    ss << " ]";
    return ss.str();
}
}  // namespace ark::pandasm

#endif  // PANDA_ASSEMBLER_IDE_HELPERS_H
