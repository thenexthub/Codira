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

#include "runtime/include/mem/panda_string.h"

#include <cmath>
#include <cstdlib>

#include "libarkbase/macros.h"
#include "runtime/include/coretypes/line_string.h"
#include "runtime/include/coretypes/string.h"

namespace ark {

static constexpr int BASE = 10;

int64_t PandaStringToLL(const PandaString &str)
{
    [[maybe_unused]] char *endPtr = nullptr;
    int64_t result = std::strtoll(str.c_str(), &endPtr, BASE);
    ASSERT(!(result == 0 && str.c_str() == endPtr) && "PandaString argument is not long long int");
    return result;
}

uint64_t PandaStringToULL(const PandaString &str)
{
    [[maybe_unused]] char *endPtr = nullptr;
    uint64_t result = std::strtoull(str.c_str(), &endPtr, BASE);
    ASSERT(!(result == 0 && str.c_str() == endPtr) && "PandaString argument is not unsigned long long int");
    return result;
}

float PandaStringToF(const PandaString &str)
{
    [[maybe_unused]] char *endPtr = nullptr;
    float result = std::strtof(str.c_str(), &endPtr);
    ASSERT(result != HUGE_VALF && "PandaString argument is not float");
    ASSERT(!(result == 0 && str.c_str() == endPtr) && "PandaString argument is not float");
    return result;
}

double PandaStringToD(const PandaString &str)
{
    [[maybe_unused]] char *endPtr = nullptr;
    double result = std::strtod(str.c_str(), &endPtr);
    ASSERT(result != HUGE_VALF && "PandaString argument is not double");
    ASSERT(!(result == 0 && str.c_str() == endPtr) && "PandaString argument is not double");
    return result;
}

PandaString ConvertToString(Span<const uint8_t> sp)
{
    PandaString res;
    res.reserve(sp.size());

    for (auto c : sp) {
        res.push_back(c);
    }

    return res;
}

// NB! the following function need additional mem allocation, donnot use when unnecessary!
PandaString ConvertToString(const std::string &str)
{
    PandaString res;
    res.reserve(str.size());
    for (auto c : str) {
        res.push_back(c);
    }
    return res;
}

PandaString ConvertToString(coretypes::String *s)
{
    ASSERT(s != nullptr);
    size_t len = s->GetUtf8Length();
    PandaVector<uint8_t> buf(len);
    s->CopyDataRegionUtf8(buf.data(), 0, len, len);

    Span<const uint8_t> sp(buf.data(), len);
    return ConvertToString(sp);
}

}  // namespace ark
