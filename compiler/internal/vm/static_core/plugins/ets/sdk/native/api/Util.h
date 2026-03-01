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

#ifndef PANDA_PLUGINS_ETS_SDK_NATIVE_CORE_UTIL_H
#define PANDA_PLUGINS_ETS_SDK_NATIVE_CORE_UTIL_H

#include <ani.h>
#include <string>
#include <string_view>

namespace ark::ets::sdk::util {

extern "C" {
ANI_EXPORT ani_string ETSApiUtilHelperGenerateRandomUUID(ani_env *env, [[maybe_unused]] ani_class klass,
                                                         ani_boolean entropyCache);

ANI_EXPORT ani_object ETSApiUtilHelperGenerateRandomBinaryUUID(ani_env *env, [[maybe_unused]] ani_class klass,
                                                               ani_boolean entropyCache);

ANI_EXPORT ani_string ETSApiUtilHelperGetErrorString(ani_env *env, [[maybe_unused]] ani_class klass, ani_int err);

ANI_EXPORT void ThrowNewError(ani_env *env, std::string_view classDescriptor, std::string_view msg,
                              const char *ctorSignature = nullptr);

ANI_EXPORT ani_string CreateUtf16String(ani_env *env, const uint16_t *data, ani_size size);
}

ani_string CreateUtf8String(ani_env *env, const char *data, ani_size size);
inline ani_string CreateStringUtf8(ani_env *env, const std::string &str)
{
    return CreateUtf8String(env, str.data(), str.size());
}

ani_ref GetAniNull(ani_env *env);
bool IsNullishValue(ani_env *env, ani_ref ref);

ani_error CreateBusinessError(ani_env *env, int code, const std::string &message);
void ThrowBusinessError(ani_env *env, int code, const std::string &message);

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BOX_VALUE_LIST(V)        \
    V(ani_boolean, Boolean, "z") \
    V(ani_byte, Byte, "b")       \
    V(ani_char, Char, "c")       \
    V(ani_double, Double, "d")   \
    V(ani_float, Float, "f")     \
    V(ani_int, Int, "i")         \
    V(ani_long, Long, "l")       \
    V(ani_short, Short, "s")

// CC-OFFNXT(G.PRE.09) function defination~, no effects.
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEF_TOTYPE_METHOD(aniType, typeName, _signature) aniType To##typeName(ani_env *env, ani_object value);
BOX_VALUE_LIST(DEF_TOTYPE_METHOD)
#undef DEF_TOTYPE_METHOD
}  // namespace ark::ets::sdk::util

#endif  //  PANDA_PLUGINS_ETS_SDK_NATIVE_CORE_UTIL_H
