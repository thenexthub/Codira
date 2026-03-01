/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_STDLIB_NATIVE_ESCOMPAT_REGEXP_REGEXP_8_H
#define PANDA_PLUGINS_ETS_STDLIB_NATIVE_ESCOMPAT_REGEXP_REGEXP_8_H

#include "plugins/ets/stdlib/native/core/regexp/regexp_exec_result.h"

#include <cstdint>

namespace ark::ets::stdlib {

using Pcre2Obj = void *;

class RegExp8 {
public:
    static Pcre2Obj CreatePcre2Object(const uint8_t *pattern, uint32_t flags, uint32_t extraFlags, const int len);
    static RegExpExecResult Execute(Pcre2Obj re, uint32_t matchFlags, const uint8_t *str, const int len,
                                    const int startOffset);
    static void ExtractGroups(Pcre2Obj expression, int count, RegExpExecResult &result, void *data);
    static void FreePcre2Object(Pcre2Obj re);
    static void EraseExtraGroups(const uint8_t *pattern, const size_t len, RegExpExecResult &result);
    static bool IsUncountable(const uint8_t *pattern, const size_t len, size_t index);
    static void SanitizeGroupCaptureResults(const std::vector<bool> &countableGroups,
                                            const std::map<size_t, size_t> &parentGroups, RegExpExecResult &result);
};

}  // namespace ark::ets::stdlib

#endif  // PANDA_PLUGINS_ETS_STDLIB_NATIVE_ESCOMPAT_REGEXP_REGEXP_8_H
