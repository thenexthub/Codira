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
#include "IntlCommon.h"
#include "libarkbase/macros.h"
#include "stdlib_ani_helpers.h"
#include <cassert>

namespace ark::ets::stdlib::intl {

icu::UnicodeString StdStrToUnicode(const std::string &str)
{
    return icu::UnicodeString::fromUTF8(icu::StringPiece(str));
}

ani_string StdStrToAni(ani_env *env, const std::string &str)
{
    return CreateUtf8String(env, str.data(), str.size());
}

ani_string UnicodeToAniStr(ani_env *env, icu::UnicodeString &ustr)
{
    return CreateUtf16String(env, reinterpret_cast<const uint16_t *>(ustr.getBuffer()), ustr.length());
}

icu::UnicodeString AniToUnicodeStr(ani_env *env, ani_string aniStr)
{
    ani_size aniStrSize = 0;
    ANI_FATAL_IF_ERROR(env->String_GetUTF16Size(aniStr, &aniStrSize));

    ani_size copiedCharsCount = 0;
    std::vector<uint16_t> buf(aniStrSize + 1);
    ANI_FATAL_IF_ERROR(env->String_GetUTF16(aniStr, buf.data(), buf.size(), &copiedCharsCount));
    ANI_FATAL_IF(copiedCharsCount != aniStrSize);

    return icu::UnicodeString(buf.data(), aniStrSize);
}

}  // namespace ark::ets::stdlib::intl
