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

#include "stdlib_ani_helpers.h"
#include "libarkbase/utils/logger.h"

namespace ark::ets::stdlib {

void StdlibLogFatal(const char *msg)
{
    LOG(FATAL, STDLIB) << msg;
    UNREACHABLE();
}

void StdlibLogFatal(const char *msg, ani_status status)
{
    LOG(FATAL, STDLIB) << (std::string(msg) + " status = " + StatusToString(status) + "(" + std::to_string(status) +
                           ")");
    UNREACHABLE();
}

ANI_EXPORT void ThrowNewError(ani_env *env, ani_class errorClass, std::string_view msg, const char *ctorSignature)
{
    ASSERT(env != nullptr);

    ani_method ctor;
    ANI_FATAL_IF_ERROR(env->Class_FindMethod(errorClass, "<ctor>", ctorSignature, &ctor));

    ani_string message = CreateUtf8String(env, msg.data(), msg.size());
    if (message == nullptr) {
        // Exception occured in CreateUtf8String.
        return;
    }

    ani_object errorObject;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ANI_FATAL_IF_ERROR(env->Object_New(errorClass, ctor, &errorObject, message));

    ANI_FATAL_IF_ERROR(env->ThrowError(reinterpret_cast<ani_error>(errorObject)));
}

ANI_EXPORT void ThrowNewError(ani_env *env, std::string_view classDescriptor, std::string_view msg,
                              const char *ctorSignature)
{
    ASSERT(env != nullptr);

    ani_class errCls;
    ANI_FATAL_IF_ERROR(env->FindClass(classDescriptor.data(), &errCls));
    ThrowNewError(env, errCls, msg, ctorSignature);
}

ANI_EXPORT std::string ConvertFromAniString(ani_env *env, ani_string aniStr)
{
    ani_size strSz;
    ANI_FATAL_IF_ERROR(env->String_GetUTF8Size(aniStr, &strSz));

    std::string buffer;
    buffer.resize(strSz + 1U);

    ani_size nmbCopiedBytes;
    ANI_FATAL_IF_ERROR(env->String_GetUTF8SubString(aniStr, 0U, strSz, buffer.data(), buffer.size(), &nmbCopiedBytes));
    ANI_FATAL_IF(nmbCopiedBytes != strSz);

    buffer.resize(strSz);
    return buffer;
}

ANI_EXPORT std::string StatusToString(ani_status status)
{
    constexpr size_t STATUS_NUM = static_cast<size_t>(ANI_AMBIGUOUS) + 1U;

    static std::array<std::string, STATUS_NUM> statusToStringTable {
        "ANI_OK",
        "ANI_ERROR",
        "ANI_INVALID_ARGS",
        "ANI_INVALID_TYPE",
        "ANI_INVALID_DESCRIPTOR",
        "ANI_INCORRECT_REF",
        "ANI_PENDING_ERROR",
        "ANI_NOT_FOUND",
        "ANI_ALREADY_BINDED",
        "ANI_OUT_OF_REF",
        "ANI_OUT_OF_MEMORY",
        "ANI_OUT_OF_RANGE",
        "ANI_BUFFER_TO_SMALL",
        "ANI_INVALID_VERSION",
        "ANI_AMBIGUOUS",
    };

    auto idx = static_cast<size_t>(status);
    if (idx >= STATUS_NUM) {
        StdlibLogFatal("Invalid status");
        UNREACHABLE();
    }
    return statusToStringTable[idx];
}

ANI_EXPORT ani_string CreateUtf8String(ani_env *env, const char *data, ani_size size)
{
    ani_string result {};

    auto status = env->String_NewUTF8(data, size, &result);
    if (status != ANI_OK) {
        ani_boolean unhandledExc;
        ANI_FATAL_IF_ERROR(env->ExistUnhandledError(&unhandledExc));
        if (unhandledExc == ANI_TRUE) {
            return nullptr;
        }
        UNREACHABLE();
    }
    return result;
}

ANI_EXPORT ani_string CreateUtf16String(ani_env *env, const uint16_t *data, ani_size size)
{
    ani_string result;

    auto status = env->String_NewUTF16(data, size, &result);
    if (status != ANI_OK) {
        ani_boolean unhandledExc;
        ANI_FATAL_IF_ERROR(env->ExistUnhandledError(&unhandledExc));
        if (unhandledExc == ANI_TRUE) {
            return nullptr;
        }
        UNREACHABLE();
    }
    return result;
}

ANI_EXPORT std::string GetFieldStrUndefined(ani_env *env, ani_object obj, const char *name)
{
    ani_ref ref = nullptr;
    ANI_FATAL_IF_ERROR(env->Object_GetFieldByName_Ref(obj, name, &ref));
    ani_boolean isUndefined = ANI_FALSE;
    ANI_FATAL_IF_ERROR(env->Reference_IsUndefined(ref, &isUndefined));
    if (isUndefined == ANI_TRUE) {
        return std::string();
    }
    auto aniStr = static_cast<ani_string>(ref);
    ASSERT(aniStr != nullptr);
    return ConvertFromAniString(env, aniStr);
}

ANI_EXPORT std::string GetFieldStr(ani_env *env, ani_object obj, const char *name)
{
    ani_ref ref = nullptr;
    ANI_FATAL_IF_ERROR(env->Object_GetFieldByName_Ref(obj, name, &ref));
    auto aniStr = static_cast<ani_string>(ref);
    ASSERT(aniStr != nullptr);
    return ConvertFromAniString(env, aniStr);
}

}  // namespace ark::ets::stdlib
