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

#ifndef COMMON_FUZZER_H
#define COMMON_FUZZER_H

#include "ecmascript/napi/include/jsnapi.h"

#define MAXBYTELEN sizeof(int32_t)

namespace panda::ecmascript {
class common_fuzzer {
public:
    common_fuzzer() = default;
    ~common_fuzzer() = default;

    EcmaVM* GetEcvm();
    void DestroyEcvm(EcmaVM* vm);
    std::string GetString(const uint8_t* data, size_t size);
};

EcmaVM* common_fuzzer::GetEcvm()
{
    RuntimeOption option;
    option.SetLogLevel(RuntimeOption::LOG_LEVEL::ERROR);
    auto vm = JSNApi::CreateJSVM(option);
    return vm;
}

void common_fuzzer::DestroyEcvm(EcmaVM* vm)
{
    JSNApi::DestroyJSVM(vm);
}

std::string common_fuzzer::GetString(const uint8_t* data, size_t size)
{
    int index = 0;
    std::string str(data, data + size);
    if (size <= 0) {
        return "";
    }
    if (size > MAXBYTELEN) {
        size = MAXBYTELEN;
    }
    if (memcpy_s(&index, MAXBYTELEN, data, size) != 0) {
        std::cout << "memcpy_s failed!";
        UNREACHABLE();
    }
    return str;
}
}

#endif