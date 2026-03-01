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

#ifndef JSVM_PLATFORM_H
#define JSVM_PLATFORM_H
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <securec.h>
#include <string>

#ifdef TARGET_OHOS
#include "platform/platform_ohos.h"
#define OHOS_CALL(api_call) api_call
#define OHOS_SELECT(expr_ohos, expr_non_ohos) expr_ohos
#else
#define OHOS_CALL(api_call)
#define OHOS_SELECT(expr_ohos, expr_non_ohos) expr_non_ohos
#endif

namespace platform {
class OS {
public:
    [[noreturn]] static void Abort();
    static uint64_t GetUid();
    static uint64_t GetPid();
    static uint64_t GetTid();

    enum class LogLevel : uint64_t { LOG_DEBUG = 0, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL };

    static void Print(LogLevel level, const char* format, ...) __attribute__((format(printf, 2, 3)));
    static void PrintString(LogLevel level, const char* string);
};

class RunJsTrace {
public:
    explicit RunJsTrace(bool runJs);
    explicit RunJsTrace(const char* name);

    ~RunJsTrace();

private:
    bool runJs;
};
} // namespace platform

#endif
