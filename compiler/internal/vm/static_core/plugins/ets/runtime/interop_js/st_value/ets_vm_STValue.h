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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_VM_STVALUE_H
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_VM_STVALUE_H

#include <ani.h>
#include <js_native_api.h>
#include <js_native_api_types.h>
#include <node_api.h>
#include <variant>
#include "ets_coroutine.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"

namespace ark::ets::interop::js {

#define STVALUE_DATA_PTR_LOW "_STValueDataPtrLow"
#define STVALUE_DATA_PTR_HIGH "_STValueDataPtrHigh"

constexpr uint32_t UINT32_BIT_SHIFT = 32;

class STValueData {
public:
    STValueData([[maybe_unused]] napi_env env, ani_ref refData);
    STValueData([[maybe_unused]] napi_env env, ani_boolean booleanData);
    STValueData([[maybe_unused]] napi_env env, ani_char charData);
    STValueData([[maybe_unused]] napi_env env, ani_byte byteData);
    STValueData([[maybe_unused]] napi_env env, ani_short shortData);
    STValueData([[maybe_unused]] napi_env env, ani_int intData);
    STValueData([[maybe_unused]] napi_env env, ani_long longData);
    STValueData([[maybe_unused]] napi_env env, ani_float floatData);
    STValueData([[maybe_unused]] napi_env env, ani_double doubleData);
    ~STValueData();

    ani_ref GetAniRef() const
    {
        ASSERT(IsAniRef());
        return std::get<ani_ref>(dataRef);
    }

    ani_boolean GetAniBoolean() const
    {
        ASSERT(IsAniBoolean());
        return std::get<ani_boolean>(dataRef);
    }

    ani_char GetAniChar() const
    {
        ASSERT(IsAniChar());
        return std::get<ani_char>(dataRef);
    }

    ani_byte GetAniByte() const
    {
        ASSERT(IsAniByte());
        return std::get<ani_byte>(dataRef);
    }

    ani_short GetAniShort() const
    {
        ASSERT(IsAniShort());
        return std::get<ani_short>(dataRef);
    }

    ani_int GetAniInt() const
    {
        ASSERT(IsAniInt());
        return std::get<ani_int>(dataRef);
    }

    ani_long GetAniLong() const
    {
        ASSERT(IsAniLong());
        return std::get<ani_long>(dataRef);
    }

    ani_float GetAniFloat() const
    {
        ASSERT(IsAniFloat());
        return std::get<ani_float>(dataRef);
    }

    ani_double GetAniDouble() const
    {
        ASSERT(IsAniDouble());
        return std::get<ani_double>(dataRef);
    }

    bool IsAniRef() const
    {
        return std::holds_alternative<ani_ref>(dataRef);
    }
    bool IsAniBoolean() const
    {
        return std::holds_alternative<ani_boolean>(dataRef);
    }
    bool IsAniChar() const
    {
        return std::holds_alternative<ani_char>(dataRef);
    }
    bool IsAniByte() const
    {
        return std::holds_alternative<ani_byte>(dataRef);
    }
    bool IsAniShort() const
    {
        return std::holds_alternative<ani_short>(dataRef);
    }
    bool IsAniInt() const
    {
        return std::holds_alternative<ani_int>(dataRef);
    }
    bool IsAniLong() const
    {
        return std::holds_alternative<ani_long>(dataRef);
    }
    bool IsAniFloat() const
    {
        return std::holds_alternative<ani_float>(dataRef);
    }
    bool IsAniDouble() const
    {
        return std::holds_alternative<ani_double>(dataRef);
    }

    bool IsAniNullOrUndefined(napi_env env) const;

    double ToDouble() const
    {
        ASSERT(!IsAniRef());

        if (IsAniBoolean()) {
            return GetAniBoolean() ? 1.0 : 0.0;
        } else if (IsAniChar()) {
            return static_cast<double>(GetAniChar());
        } else if (IsAniByte()) {
            return static_cast<double>(GetAniByte());
        } else if (IsAniShort()) {
            return static_cast<double>(GetAniShort());
        } else if (IsAniInt()) {
            return static_cast<double>(GetAniInt());
        } else if (IsAniLong()) {
            return static_cast<double>(GetAniLong());
        } else if (IsAniFloat()) {
            return static_cast<double>(GetAniFloat());
        } else if (IsAniDouble()) {
            return GetAniDouble();
        }

        UNREACHABLE();
        return 0;
    }

private:
    std::variant<ani_ref, ani_boolean, ani_char, ani_byte, ani_short, ani_int, ani_long, ani_float, ani_double> dataRef;
};

enum class SType : int32_t { BOOLEAN, BYTE, CHAR, SHORT, INT, LONG, FLOAT, DOUBLE, REFERENCE, VOID };

PANDA_PUBLIC_API napi_value GetSTValueClass(napi_env env);
PANDA_PUBLIC_API napi_value CreateSTypeObject(napi_env env);
uintptr_t GetSTValueDataPtr(napi_env env, napi_value jsSTValue);
bool AniCheckAndThrowToDynamic(napi_env env, ani_status status);
bool AniCheckAndThrowToDynamic(napi_env env, ani_status status, const std::string &errorMsg);
void AniExpectOK(ani_status status);
SType GetTypeFromType(napi_env env, napi_value stNapiType);
bool GetAniValueFromSTValue(napi_env env, napi_value element, ani_value &value);
bool IsSTValueInstance(napi_env env, napi_value value);
bool CheckNapiIsArray(napi_env env, napi_value jsObject);
void STValueThrowJSError(napi_env env, const std::string &msg);

void ThrowJSBadArgCountError(napi_env env, size_t jsArgc, size_t expectedArgc);
std::string GetSTypeName(SType stype);
void ThrowUnsupportedSTypeError(napi_env env, SType stype);
void ThrowTypeCheckError(napi_env env, const std::string &name, const std::string &type);
void ThrowJSNonBooleanError(napi_env env, const std::string &name);
void ThrowJSNonByteError(napi_env env, const std::string &name);
void ThrowJSNonCharError(napi_env env, const std::string &name);
void ThrowJSNonShortError(napi_env env, const std::string &name);
void ThrowJSNonIntError(napi_env env, const std::string &name);
void ThrowJSNonLongError(napi_env env, const std::string &name);
void ThrowJSNonFloatError(napi_env env, const std::string &name);
void ThrowJSNonDoubleError(napi_env env, const std::string &name);
void ThrowJSNonObjectError(napi_env env, const std::string &name);
void ThrowJSThisNonObjectError(napi_env env);
void ThrowJSOtherNonObjectError(napi_env env);

template <bool IS_NOT_DESTRUCT = true>
ani_env *GetAniEnv()
{
    ani_vm *vm {};
    ani_env *aniEnv {};
    ani_size size = 0;
    ani_size bufferSize = 1;
    AniExpectOK(ANI_GetCreatedVMs(&vm, bufferSize, &size));
    if constexpr (IS_NOT_DESTRUCT) {
        ASSERT(size == bufferSize);
    } else if (size == 0) {
        return nullptr;
    }
    AniExpectOK(vm->GetEnv(ANI_VERSION_1, &aniEnv));

    return aniEnv;
}

#define NAPI_TO_ANI_SCOPE                                       \
    do {                                                        \
        auto *t = Thread::GetCurrent();                         \
        if (t == nullptr) {                                     \
            STValueThrowJSError(env, "Thread is not attached"); \
            /* CC-OFFNXT(G.PRE.05) function gen */              \
            return nullptr;                                     \
        }                                                       \
        auto *c = EtsCoroutine::GetCurrent();                   \
        if (c == nullptr) {                                     \
            STValueThrowJSError(env, "Thread is not attached"); \
            /* CC-OFFNXT(G.PRE.05) function gen */              \
            return nullptr;                                     \
        }                                                       \
        auto *ctx = InteropCtx::Current(c);                     \
        if (ctx == nullptr) {                                   \
            STValueThrowJSError(env, "Thread is not attached"); \
            /* CC-OFFNXT(G.PRE.05) function gen */              \
            return nullptr;                                     \
        }                                                       \
    } while (0);                                                \
    INTEROP_CODE_SCOPE_JS_TO_ETS(EtsCoroutine::GetCurrent())

#define ANI_CHECK_ERROR_RETURN(env, status)                     \
    do {                                                        \
        auto hasError = AniCheckAndThrowToDynamic(env, status); \
        if (!hasError) {                                        \
            return {};                                          \
        }                                                       \
    } while (0)

template <typename T>
napi_value CreateSTValueInstance(napi_env env, T &&arg)
{
    INTEROP_TRACE();
    STValueData *data = new STValueData(env, std::forward<T>(arg));
    napi_value stvalueCtor = GetSTValueClass(env);
    uint64_t ptr = reinterpret_cast<uintptr_t>(data);
    napi_value ptrLow;
    napi_value ptrHigh;
    NAPI_CHECK_FATAL(napi_create_uint32(env, static_cast<uint32_t>(ptr & 0xFFFFFFFF), &ptrLow));
    NAPI_CHECK_FATAL(napi_create_uint32(env, static_cast<uint32_t>(ptr >> UINT32_BIT_SHIFT), &ptrHigh));

    std::array argv = {ptrLow, ptrHigh};
    napi_value jsSTValue;
    NAPI_CHECK_FATAL(napi_new_instance(env, stvalueCtor, argv.size(), argv.data(), &jsSTValue));
    return jsSTValue;
}

}  // namespace ark::ets::interop::js

#endif  // PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_VM_STVALUE_H
