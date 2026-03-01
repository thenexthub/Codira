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


#include "CString.h"

namespace MapleRuntime {
FixedCString::FixedCString() { length = 0; }

FixedCString::FixedCString(const char* initStr)
{
    if (initStr != nullptr) {
        size_t initLen = strlen(initStr);
        PRINT_FATAL_IF(initLen > C_STRING_MAX_SIZE, "FixedCString::Init failed");
        if (*initStr != '\0') {
            PRINT_FATAL_IF(memcpy_s(mem, C_STRING_MAX_SIZE, initStr, initLen) != EOK,
                           "FixedCString::FixedCString memcpy_s failed");
        }
        length = initLen;
        mem[length] = '\0';
    }
}

FixedCString::FixedCString(const FixedCString& other)
{
    size_t initLen = other.Length();
    PRINT_FATAL_IF(initLen > C_STRING_MAX_SIZE, "FixedCString::Init failed");
    if (!other.IsEmpty()) {
        PRINT_FATAL_IF(memcpy_s(mem, C_STRING_MAX_SIZE, other.Str(), initLen) != EOK,
                       "FixedCString::FixedCString memcpy_s failed");
    }
    length = initLen;
    mem[length] = '\0';
}

FixedCString& FixedCString::operator=(const FixedCString& other)
{
    if (this == &other) {
        return *this;
    }
    size_t initLen = other.Length();
    PRINT_FATAL_IF(initLen > C_STRING_MAX_SIZE, "FixedCString::Init failed");
    if (!other.IsEmpty()) {
        PRINT_FATAL_IF(memcpy_s(mem, C_STRING_MAX_SIZE, other.Str(), initLen) != EOK,
                       "FixedCString::operator= memcpy_s failed");
    }
    length = initLen;
    mem[length] = '\0';
    return *this;
}

FixedCString::~FixedCString() {}

size_t FixedCString::Length() const { return length; }

const char* FixedCString::Str() const noexcept { return mem; }
} // namespace MapleRuntime
