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

#ifndef PANDA_PLUGINS_ETS_STDLIB_NATIVE_ESCOMPAT_REGEXP_H
#define PANDA_PLUGINS_ETS_STDLIB_NATIVE_ESCOMPAT_REGEXP_H

#include "plugins/ets/stdlib/native/core/regexp/regexp_exec_result.h"

#include <ani.h>

namespace ark::ets::stdlib {

class pcre2_code;

class EtsRegExp {
public:
    EtsRegExp() = delete;
    explicit EtsRegExp(ani_env *env)
    {
        env_ = env;
    }
    void SetFlags(const std::string &flagsStr);
    bool Compile(const std::vector<uint8_t> &pattern, const bool isUtf16, const int len);
    RegExpExecResult Execute(const std::vector<uint8_t> &pattern, const std::vector<uint8_t> &str, const int len,
                             const int startOffset);
    void Destroy();

    bool IsUtf16() const
    {
        return utf16_;
    }

private:
    void SetFlag(const char &chr);
    void SetUnicodeFlag(const char &chr);
    void SetIfNotSet(bool &flag);
    static void ThrowBadFlagsException(ani_env *env);

    void *re_ = nullptr;
    bool flagGlobal_ = false;           // g
    bool flagMultiline_ = false;        // m
    bool flagCaseInsentitive_ = false;  // i
    bool flagSticky_ = false;           // y
    bool flagUnicode_ = false;          // u
    bool flagVnicode_ = false;          // v
    bool flagDotAll_ = false;           // s
    bool flagIndices_ = false;          // d
    bool utf16_ = false;

    ani_env *env_ = nullptr;
};

}  // namespace ark::ets::stdlib
#endif  // PANDA_PLUGINS_ETS_STDLIB_NATIVE_ESCOMPAT_REGEXP_H