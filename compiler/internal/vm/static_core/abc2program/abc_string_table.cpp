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

#include "abc_string_table.h"
#include "abc_file_utils.h"

namespace ark::abc2program {

std::string AbcStringTable::GetStringById(uint32_t stringId) const
{
    panda_file::File::EntityId entityId(stringId);
    return GetStringById(entityId);
}

std::string AbcStringTable::GetStringById(panda_file::File::EntityId entityId) const
{
    std::string str = StringDataToString(file_.GetStringData(entityId));
    return str;
}

void AbcStringTable::AddStringId(uint32_t stringId)
{
    auto it = stingIdSet_.find(stringId);
    if (it != stingIdSet_.end()) {
        return;
    }
    stingIdSet_.insert(stringId);
}

void AbcStringTable::AddStringId(panda_file::File::EntityId entityId)
{
    AddStringId(entityId.GetOffset());
}

std::set<std::string> AbcStringTable::GetStringSet() const
{
    std::set<std::string> stringSet;
    for (uint32_t stringId : stingIdSet_) {
        stringSet.insert(GetStringById(stringId));
    }
    return stringSet;
}

void AbcStringTable::Dump(std::ostream &os) const
{
    for (uint32_t stringId : stingIdSet_) {
        DumpStringById(os, stringId);
    }
}

void AbcStringTable::DumpStringById(std::ostream &os, uint32_t stringId) const
{
    os << "[offset:0x" << std::hex << stringId << ", name_value:" << GetStringById(stringId) << "]" << std::endl;
}

std::string AbcStringTable::StringDataToString(panda_file::File::StringData sd) const
{
    std::string str = std::string(utf::Mutf8AsCString(sd.data));

    size_t symPos = 0;
    while (symPos = str.find_first_of("\a\b\f\n\r\t\v\'\\\"", symPos), symPos != std::string::npos) {
        std::string sym;
        switch (str[symPos]) {
            case '\a':
                sym = R"(\a)";
                break;
            case '\b':
                sym = R"(\b)";
                break;
            case '\f':
                sym = R"(\f)";
                break;
            case '\n':
                sym = R"(\n)";
                break;
            case '\r':
                sym = R"(\r)";
                break;
            case '\t':
                sym = R"(\t)";
                break;
            case '\v':
                sym = R"(\v)";
                break;
            case '\"':
                sym = R"(\")";
                break;
            case '\'':
                sym = R"(\')";
                break;
            case '\\':
                sym = R"(\\)";
                break;
            default:
                UNREACHABLE();
        }
        str = str.replace(symPos, 1, sym);
        ASSERT(sym.size() == 2U);
        symPos += 2U;
    }
    return str;
}

}  // namespace ark::abc2program
