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

#include <ani.h>
#include <js_native_api.h>
#include <js_native_api_types.h>
#include <node_api.h>
#include <array>
#include <cfloat>
#include <cstdint>
#include <cstdio>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include "ets_coroutine.h"
#include "include/mem/panda_containers.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "plugins/ets/runtime/ets_vm_api.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/ets_proxy.h"
#include "plugins/ets/runtime/interop_js/call/call.h"
#include "plugins/ets/runtime/interop_js/interop_common.h"
#include "plugins/ets/runtime/interop_js/code_scopes.h"

#include "compiler_options.h"
#include "compiler/compiler_logger.h"
#include "interop_js/napi_impl/napi_impl.h"
#include "plugins/ets/runtime/ets_utils.h"
#include "runtime/include/runtime.h"

#include "plugins/ets/runtime/interop_js/st_value/ets_vm_STValue.h"
#include "plugins/ets/runtime/interop_js/interop_context_api.h"

// NOLINTBEGIN(readability-identifier-naming, readability-redundant-declaration)
// CC-OFFNXT(G.FMT.10-CPP) project code style
napi_status __attribute__((weak)) napi_define_properties(napi_env env, napi_value object, size_t property_count,
                                                         const napi_property_descriptor *properties);
// NOLINTEND(readability-identifier-naming, readability-redundant-declaration)

namespace ark::ets::interop::js {

template <typename T>
napi_value WrapNumberImplIner(napi_env env, napi_callback_info info)
{
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;

    size_t jsArgc = 0;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));

    if (jsArgc != 1) {
        ThrowJSBadArgCountError(env, jsArgc, 1);
        return nullptr;
    }

    napi_value jsArgv[1];
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, jsArgv, nullptr, nullptr));

    if constexpr (std::is_same_v<T, ani_byte>) {
        double input;
        NAPI_CHECK_FATAL(napi_get_value_double(env, jsArgv[0], &input));
        if (input < INT8_MIN || input > INT8_MAX) {
            STValueThrowJSError(env, "Value is out of range for byte type.");
            return nullptr;
        }
        return CreateSTValueInstance(env, static_cast<ani_byte>(input));
    } else if constexpr (std::is_same_v<T, ani_boolean>) {
        bool nativeBool;
        NAPI_CHECK_FATAL(napi_get_value_bool(env, jsArgv[0], &nativeBool));
        return CreateSTValueInstance(env, static_cast<ani_boolean>(nativeBool));
    } else if constexpr (std::is_same_v<T, ani_char>) {
        std::string variableChar = GetString(env, jsArgv[0]);
        if (variableChar.length() != 1) {
            STValueThrowJSError(env, "input string length must be 1.");
            return nullptr;
        }
        return CreateSTValueInstance(env, static_cast<ani_char>(variableChar[0]));
    } else if constexpr (std::is_same_v<T, ani_short>) {
        double input;
        NAPI_CHECK_FATAL(napi_get_value_double(env, jsArgv[0], &input));
        if (input < INT16_MIN || input > INT16_MAX) {
            STValueThrowJSError(env, "Value is out of range for short type.");
            return nullptr;
        }
        return CreateSTValueInstance(env, static_cast<ani_short>(input));
    } else if constexpr (std::is_same_v<T, ani_int>) {
        double input;
        NAPI_CHECK_FATAL(napi_get_value_double(env, jsArgv[0], &input));
        if (input < INT32_MIN || input > INT32_MAX) {
            STValueThrowJSError(env, "Value is out of range for int type.");
            return nullptr;
        }
        return CreateSTValueInstance(env, static_cast<ani_int>(input));
    } else if constexpr (std::is_same_v<T, ani_long>) {
        double input;
        NAPI_CHECK_FATAL(napi_get_value_double(env, jsArgv[0], &input));
        static constexpr int64_t BASE = 2;
        static constexpr int64_t EXPONENT = 52;
        static constexpr double MAX_SAFE_INTEGER = static_cast<double>(BASE << EXPONENT) - 1;
        static constexpr double MIN_SAFE_INTEGER = -MAX_SAFE_INTEGER;
        if (input < MIN_SAFE_INTEGER || input > MAX_SAFE_INTEGER) {
            STValueThrowJSError(env, "Value is out of range for safe long type.");
            return nullptr;
        }
        return CreateSTValueInstance(env, static_cast<ani_long>(input));
    } else if constexpr (std::is_same_v<T, ani_float>) {
        double input;
        NAPI_CHECK_FATAL(napi_get_value_double(env, jsArgv[0], &input));
        // Regarding wrapFloat, directly static_cast a double to float (NaN remains NaN after casting,
        // casting a value beyond the float bounds will result in INF).
        return CreateSTValueInstance(env, static_cast<ani_float>(input));
    } else if constexpr (std::is_same_v<T, ani_double>) {
        double nativeDouble;
        NAPI_CHECK_FATAL(napi_get_value_double(env, jsArgv[0], &nativeDouble));
        return CreateSTValueInstance(env, static_cast<ani_double>(nativeDouble));
    } else {
        STValueThrowJSError(env, "Unsupported input type");
        return nullptr;
    }
}

napi_value WrapByteImpl(napi_env env, napi_callback_info info)
{
    INTEROP_TRACE();
    return WrapNumberImplIner<ani_byte>(env, info);
}

napi_value WrapCharImpl(napi_env env, napi_callback_info info)
{
    INTEROP_TRACE();
    return WrapNumberImplIner<ani_char>(env, info);
}

napi_value WrapShortImpl(napi_env env, napi_callback_info info)
{
    INTEROP_TRACE();
    return WrapNumberImplIner<ani_short>(env, info);
}

napi_value WrapIntImpl(napi_env env, napi_callback_info info)
{
    INTEROP_TRACE();
    return WrapNumberImplIner<ani_int>(env, info);
}

std::string BigIntToStringJS(napi_env env, napi_value bigint)
{
    napi_value global;
    napi_get_global(env, &global);
    napi_value bigIntCtor;
    napi_get_named_property(env, global, "BigInt", &bigIntCtor);
    napi_value bigIntCtorProto;
    napi_get_named_property(env, bigIntCtor, "prototype", &bigIntCtorProto);
    napi_value bigIntToStringFunc;
    napi_get_named_property(env, bigIntCtorProto, "toString", &bigIntToStringFunc);
    napi_value result;
    napi_call_function(env, bigint, bigIntToStringFunc, 0, nullptr, &result);

    size_t length;
    napi_get_value_string_utf8(env, result, nullptr, 0, &length);
    std::string str(length + 1, '\0');
    napi_get_value_string_utf8(env, result, &str[0], length + 1, &length);
    str.resize(length);
    return str;
}

napi_value WrapLongImpl(napi_env env, napi_callback_info info)
{
    INTEROP_TRACE();
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;

    size_t jsArgc = 0;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));

    if (jsArgc != 1) {
        ThrowJSBadArgCountError(env, jsArgc, 1);
        return nullptr;
    }

    napi_value jsArgv[1];
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, jsArgv, nullptr, nullptr));

    napi_value input = jsArgv[0];
    napi_valuetype value_type;
    napi_typeof(env, input, &value_type);
    ani_long res = 0;
    if (value_type == napi_number) {
        double result;
        NAPI_CHECK_FATAL(napi_get_value_double(env, input, &result));
        static constexpr int64_t BASE = 2;
        static constexpr int64_t EXPONENT = 52;
        static constexpr double MAX_SAFE_INTEGER = static_cast<double>(BASE << EXPONENT) - 1;
        static constexpr double MIN_SAFE_INTEGER = -MAX_SAFE_INTEGER;
        if (result < MIN_SAFE_INTEGER || result > MAX_SAFE_INTEGER) {
            STValueThrowJSError(env, "Value is out of range for safe long type.");
            return nullptr;
        }
        res = static_cast<ani_long>(result);
    } else if (value_type == napi_bigint) {
        std::string bigIntString = BigIntToStringJS(env, input);
        static constexpr const char *className = "std.core.BigInt";
        ani_class bigIntClass;
        auto aniEnv = GetAniEnv();

        ANI_CHECK_ERROR_RETURN(env, aniEnv->FindClass(className, &bigIntClass));
        ani_method bigIntCtor {};
        ANI_CHECK_ERROR_RETURN(env,
                               aniEnv->Class_FindMethod(bigIntClass, "<ctor>", "C{std.core.String}:", &bigIntCtor));
        ani_object staticBigIntObject;

        ani_string aniBigIntString;
        ANI_CHECK_ERROR_RETURN(env,
                               aniEnv->String_NewUTF8(bigIntString.c_str(), bigIntString.length(), &aniBigIntString));
        ANI_CHECK_ERROR_RETURN(env, aniEnv->Object_New(bigIntClass, bigIntCtor, &staticBigIntObject, aniBigIntString));

        std::string INT64_MAX_STR = "9223372036854775807";
        ani_string INT64_MAX_STR_ANI {};
        ani_object INT64_MAX_ANI_OBJ {};
        ANI_CHECK_ERROR_RETURN(
            env, aniEnv->String_NewUTF8(INT64_MAX_STR.c_str(), INT64_MAX_STR.length(), &INT64_MAX_STR_ANI));
        ANI_CHECK_ERROR_RETURN(env, aniEnv->Object_New(bigIntClass, bigIntCtor, &INT64_MAX_ANI_OBJ, INT64_MAX_STR_ANI));
        ani_boolean isGreaterThanInt64Max = ANI_FALSE;
        auto status =
            aniEnv->Object_CallMethodByName_Boolean(staticBigIntObject, "operatorGreaterThan", "C{std.core.BigInt}:z",
                                                    &isGreaterThanInt64Max, INT64_MAX_ANI_OBJ);
        ANI_CHECK_ERROR_RETURN(env, status);
        if (isGreaterThanInt64Max == ANI_TRUE) {
            STValueThrowJSError(env, "Value is out of range for long type.");
            return nullptr;
        }

        std::string INT64_MIN_STR = "-9223372036854775808";
        ani_string INT64_MIN_STR_ANI {};
        ani_object INT64_MIN_ANI_OBJ {};
        ANI_CHECK_ERROR_RETURN(
            env, aniEnv->String_NewUTF8(INT64_MIN_STR.c_str(), INT64_MIN_STR.length(), &INT64_MIN_STR_ANI));
        ANI_CHECK_ERROR_RETURN(env, aniEnv->Object_New(bigIntClass, bigIntCtor, &INT64_MIN_ANI_OBJ, INT64_MIN_STR_ANI));
        ani_boolean isLessThanInt64Min = ANI_FALSE;
        AniCheckAndThrowToDynamic(env, aniEnv->Object_CallMethodByName_Boolean(staticBigIntObject, "operatorLessThan",
                                                                               "C{std.core.BigInt}:z",
                                                                               &isLessThanInt64Min, INT64_MIN_ANI_OBJ));
        if (isLessThanInt64Min == ANI_TRUE) {
            STValueThrowJSError(env, "Value is out of range for long type.");
            return nullptr;
        }

        ANI_CHECK_ERROR_RETURN(env, aniEnv->Object_CallMethodByName_Long(staticBigIntObject, "getLong", ":l", &res));
    } else {
        STValueThrowJSError(env, "input is neither a number or a bigint.");
        return nullptr;
    }

    return CreateSTValueInstance(env, res);
}

napi_value WrapFloatImpl(napi_env env, napi_callback_info info)
{
    INTEROP_TRACE();
    return WrapNumberImplIner<ani_float>(env, info);
}

napi_value WrapNumberImpl(napi_env env, napi_callback_info info)
{
    INTEROP_TRACE();
    return WrapNumberImplIner<double>(env, info);
}

napi_value WrapStringImpl(napi_env env, napi_callback_info info)
{
    INTEROP_TRACE();
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;

    size_t jsArgc = 0;
    napi_value jsThis;
    napi_value jsArgv[1];
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));

    if (jsArgc != 1) {
        ThrowJSBadArgCountError(env, jsArgc, 1);
        return nullptr;
    }

    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, jsArgv, &jsThis, nullptr));
    napi_value variableNameNapiValue = jsArgv[0];
    std::string variableName = GetString(env, variableNameNapiValue);

    auto aniEnv = GetAniEnv();
    ani_string aniString;
    ANI_CHECK_ERROR_RETURN(env, aniEnv->String_NewUTF8(variableName.c_str(), variableName.length(), &aniString));

    return CreateSTValueInstance(env, aniString);
}

napi_value WrapBooleanImpl(napi_env env, napi_callback_info info)
{
    INTEROP_TRACE();
    return WrapNumberImplIner<ani_boolean>(env, info);
}

napi_value WrapBigIntImpl(napi_env env, napi_callback_info info)
{
    INTEROP_TRACE();
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;

    size_t jsArgc = 0;
    napi_value jsThis;
    napi_value jsArgv[1];
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));

    if (jsArgc != 1) {
        ThrowJSBadArgCountError(env, jsArgc, 1);
        return nullptr;
    }

    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, jsArgv, &jsThis, nullptr));

    napi_value dynBigInt = jsArgv[0];
    std::string bigIntString = BigIntToStringJS(env, dynBigInt);
    static constexpr const char *className = "std.core.BigInt";
    ani_class bigIntClass;
    auto aniEnv = GetAniEnv();

    ANI_CHECK_ERROR_RETURN(env, aniEnv->FindClass(className, &bigIntClass));
    ani_method bigIntCtor {};
    ANI_CHECK_ERROR_RETURN(env, aniEnv->Class_FindMethod(bigIntClass, "<ctor>", "C{std.core.String}:", &bigIntCtor));
    ani_object staticBigIntObject;

    ani_string aniBigIntString;
    ANI_CHECK_ERROR_RETURN(env, aniEnv->String_NewUTF8(bigIntString.c_str(), bigIntString.length(), &aniBigIntString));
    ANI_CHECK_ERROR_RETURN(env, aniEnv->Object_New(bigIntClass, bigIntCtor, &staticBigIntObject, aniBigIntString));

    return CreateSTValueInstance(env, staticBigIntObject);
}

napi_value GetNullImpl(napi_env env, napi_callback_info info)
{
    INTEROP_TRACE();
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;

    size_t jsArgc = 0;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));

    if (jsArgc != 0) {
        ThrowJSBadArgCountError(env, jsArgc, 0);
        return nullptr;
    }
    auto *aniEnv = GetAniEnv();
    ani_ref result;
    ani_status status = aniEnv->GetNull(&result);
    ANI_CHECK_ERROR_RETURN(env, status);

    return CreateSTValueInstance(env, result);
}

napi_value GetUndefinedImpl(napi_env env, napi_callback_info info)
{
    INTEROP_TRACE();
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;

    size_t jsArgc = 0;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));

    if (jsArgc != 0) {
        ThrowJSBadArgCountError(env, jsArgc, 0);
        return nullptr;
    }
    auto *aniEnv = GetAniEnv();
    ani_ref result;
    ani_status status = aniEnv->GetUndefined(&result);
    ANI_CHECK_ERROR_RETURN(env, status);

    return CreateSTValueInstance(env, result);
}
}  // namespace ark::ets::interop::js