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

#ifndef LIBPANDAFILE_FILE_INL_H_
#define LIBPANDAFILE_FILE_INL_H_

#include "libarkfile/helpers.h"
#include "libarkfile/file.h"
#include "libarkbase/utils/leb128.h"

namespace ark::panda_file {

inline File::StringData File::GetStringData(EntityId id) const
{
    StringData strData {};
    auto sp = GetSpanFromId(id);

    auto tagUtf16Length = panda_file::helpers::ReadULeb128(&sp);
    strData.utf16Length = tagUtf16Length >> 1U;
    strData.isAscii = static_cast<bool>(tagUtf16Length & 1U);
    strData.data = sp.data();

    return strData;
}

// CC-OFFNXT(G.FUD.06) switch-case
inline std::string StringDataToString(File::StringData sd)
{
    std::string str = std::string(utf::Mutf8AsCString(sd.data));
    size_t symPos = 0;
    while (symPos = str.find_first_of("\a\b\f\n\r\t\v\'\?\\", symPos), symPos != std::string::npos) {
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
            case '\'':
                sym = R"(\')";
                break;
            case '\?':
                sym = R"(\?)";
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

}  // namespace ark::panda_file

#endif
