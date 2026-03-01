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

#ifndef PANDA_PLUGINS_ETS_STDLIB_NATIVE_CORE_STDLIB_ANI_HELPERS_H
#define PANDA_PLUGINS_ETS_STDLIB_NATIVE_CORE_STDLIB_ANI_HELPERS_H

#include <ani.h>
#include "libarkbase/macros.h"

// This functions must be moved from stdlib. #23681
namespace ark::ets::stdlib {

constexpr const char *ERR_CLS_RUNTIME_EXCEPTION = "std.core.RuntimeError";
constexpr const char *CTOR_SIGNATURE_STR = "C{std.core.String}:";

void StdlibLogFatal(const char *msg);
void StdlibLogFatal(const char *msg, ani_status status);

ANI_EXPORT std::string StatusToString(ani_status status);

ANI_EXPORT void ThrowNewError(ani_env *env, ani_class errorClass, std::string_view msg,
                              const char *ctorSignature = nullptr);
ANI_EXPORT void ThrowNewError(ani_env *env, std::string_view classDescriptor, std::string_view msg,
                              const char *ctorSignature = nullptr);

ANI_EXPORT std::string ConvertFromAniString(ani_env *env, ani_string aniStr);

ANI_EXPORT ani_string CreateUtf8String(ani_env *env, const char *data, ani_size size);
ANI_EXPORT ani_string CreateUtf16String(ani_env *env, const uint16_t *data, ani_size size);

ANI_EXPORT std::string GetFieldStrUndefined(ani_env *env, ani_object obj, const char *name);
ANI_EXPORT std::string GetFieldStr(ani_env *env, ani_object obj, const char *name);

// NOLINTBEGIN(cppcoreguidelines-macro-usage)

// CC-OFFNXT(G.PRE.02) should be with define
#define ANI_FATAL_IF_ERROR(status)                                                     \
    do {                                                                               \
        ani_status _status = (status);                                                 \
        if (UNLIKELY(_status != ANI_OK)) {                                             \
            ark::ets::stdlib::StdlibLogFatal("ANI_FATAL_IF_ERROR: " #status, _status); \
            UNREACHABLE();                                                             \
        }                                                                              \
    } while (0)

// CC-OFFNXT(G.PRE.02) should be with define
#define ANI_FATAL_IF(cond)                                         \
    do {                                                           \
        bool _cond = (cond);                                       \
        if (UNLIKELY(_cond)) {                                     \
            ark::ets::stdlib::StdlibLogFatal("ANI FATAL: " #cond); \
            UNREACHABLE();                                         \
        }                                                          \
    } while (0)

// NOLINTEND(cppcoreguidelines-macro-usage)

}  // namespace ark::ets::stdlib

#endif  // PANDA_PLUGINS_ETS_STDLIB_NATIVE_CORE_STDLIB_ANI_HELPERS_H
