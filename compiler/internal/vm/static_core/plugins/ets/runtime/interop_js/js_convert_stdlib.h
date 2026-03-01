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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_JS_CONVERT_STDLIB_H
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_JS_CONVERT_STDLIB_H

#include "js_convert_base.h"

namespace ark::ets::interop::js {

JSCONVERT_DEFINE_TYPE(StdlibBoolean, EtsObject *);
JSCONVERT_WRAP(StdlibBoolean)
{
    auto *val = reinterpret_cast<EtsBoxPrimitive<EtsBoolean> *>(etsVal);
    napi_value jsVal;
    auto valValue = val->GetValue();
    ScopedNativeCodeThread nativeScope(EtsCoroutine::GetCurrent());
    NAPI_CHECK_FATAL(napi_get_boolean(env, valValue, &jsVal));
    return jsVal;
}
JSCONVERT_UNWRAP(StdlibBoolean)
{
    bool val;
    auto coro = EtsCoroutine::GetCurrent();
    {
        ScopedNativeCodeThread nativeScope(coro);
        napi_value result = jsVal;
        napi_valuetype valueType = GetValueType(env, jsVal);
        if (valueType == napi_object && !GetValueByValueOf(env, jsVal, CONSTRUCTOR_NAME_BOOLEAN, &result)) {
            TypeCheckFailed();
            return {};
        }
        if (UNLIKELY(GetValueType(env, result) != napi_boolean)) {
            TypeCheckFailed();
            return {};
        }
        NAPI_CHECK_FATAL(napi_get_value_bool(env, result, &val));
    }
    return EtsBoxPrimitive<EtsBoolean>::Create(coro, static_cast<EtsBoolean>(val));
}

JSCONVERT_DEFINE_TYPE(StdlibByte, EtsObject *);
JSCONVERT_WRAP(StdlibByte)
{
    auto *val = reinterpret_cast<EtsBoxPrimitive<EtsByte> *>(etsVal);
    napi_value jsVal;
    auto valValue = val->GetValue();
    ScopedNativeCodeThread nativeScope(EtsCoroutine::GetCurrent());
    NAPI_CHECK_FATAL(napi_create_int32(env, valValue, &jsVal));
    return jsVal;
}
JSCONVERT_UNWRAP(StdlibByte)
{
    int32_t val;
    auto coro = EtsCoroutine::GetCurrent();
    {
        ScopedNativeCodeThread nativeScope(coro);
        napi_value result = jsVal;
        napi_valuetype valueType = GetValueType(env, jsVal);
        if (valueType == napi_object && !GetValueByValueOf(env, jsVal, CONSTRUCTOR_NAME_NUMBER, &result)) {
            TypeCheckFailed();
            return {};
        }
        if (UNLIKELY(GetValueType(env, result) != napi_number)) {
            TypeCheckFailed();
            return {};
        }
        NAPI_CHECK_FATAL(napi_get_value_int32(env, result, &val));
    }
    return EtsBoxPrimitive<EtsByte>::Create(coro, val);
}

static bool GetValueFromNumber(napi_env env, napi_value &jsVal, EtsChar &val)
{
    ScopedNativeCodeThread nativeScope(EtsCoroutine::GetCurrent());
    napi_valuetype valueType = GetValueType(env, jsVal);
    napi_value result = jsVal;
    if (valueType == napi_object && !GetValueByValueOf(env, jsVal, CONSTRUCTOR_NAME_NUMBER, &result)) {
        return false;
    }
    napi_valuetype resultType = GetValueType(env, result);
    if (resultType != napi_number) {
        return false;
    }
    int32_t ival;
    NAPI_CHECK_FATAL(napi_get_value_int32(env, result, &ival));
    if (ival < 0 || ival > std::numeric_limits<EtsChar>::max()) {
        return false;
    }
    val = static_cast<uint16_t>(ival);
    return true;
}

static bool GetValueFromString(napi_env env, napi_value &jsVal, EtsChar &val)
{
    ScopedNativeCodeThread nativeScope(EtsCoroutine::GetCurrent());
    napi_valuetype valueType = GetValueType(env, jsVal);
    napi_value result = jsVal;
    if (valueType == napi_object && !GetValueByValueOf(env, jsVal, CONSTRUCTOR_NAME_STRING, &result)) {
        return false;
    }
    napi_valuetype resultType = GetValueType(env, result);
    if (resultType != napi_string) {
        return false;
    }
    size_t len = 0;
    NAPI_CHECK_FATAL(napi_get_value_string_utf16(env, result, nullptr, 0, &len));
    if (len != 1) {
        return false;
    }
    const size_t charArrayLength = 2U;
    std::array<char16_t, charArrayLength> cval {};
    NAPI_CHECK_FATAL(napi_get_value_string_utf16(env, result, cval.data(), charArrayLength, &len));
    val = static_cast<EtsChar>(cval[0]);
    return true;
}

JSCONVERT_DEFINE_TYPE(StdlibChar, EtsObject *);
JSCONVERT_WRAP(StdlibChar)
{
    auto *val = reinterpret_cast<EtsBoxPrimitive<EtsChar> *>(etsVal);
    napi_value jsVal;
    std::array<char16_t, 2U> str = {static_cast<char16_t>(val->GetValue()), 0};
    ScopedNativeCodeThread nativeScope(EtsCoroutine::GetCurrent());
    NAPI_CHECK_FATAL(napi_create_string_utf16(env, str.data(), 1, &jsVal));
    return jsVal;
}
JSCONVERT_UNWRAP(StdlibChar)
{
    EtsChar val = 0;
    if (GetValueFromNumber(env, jsVal, val) || GetValueFromString(env, jsVal, val)) {
        return EtsBoxPrimitive<EtsChar>::Create(EtsCoroutine::GetCurrent(), static_cast<EtsChar>(val));
    }
    TypeCheckFailed();
    return {};
}

JSCONVERT_DEFINE_TYPE(StdlibShort, EtsObject *);
JSCONVERT_WRAP(StdlibShort)
{
    auto *val = reinterpret_cast<EtsBoxPrimitive<EtsShort> *>(etsVal);
    napi_value jsVal;
    auto valValue = val->GetValue();
    ScopedNativeCodeThread nativeScope(EtsCoroutine::GetCurrent());
    NAPI_CHECK_FATAL(napi_create_int32(env, valValue, &jsVal));
    return jsVal;
}
JSCONVERT_UNWRAP(StdlibShort)
{
    int32_t val;
    auto coro = EtsCoroutine::GetCurrent();
    {
        ScopedNativeCodeThread nativeScope(coro);
        napi_value result = jsVal;
        napi_valuetype valueType = GetValueType(env, jsVal);
        if (valueType == napi_object && !GetValueByValueOf(env, jsVal, CONSTRUCTOR_NAME_NUMBER, &result)) {
            TypeCheckFailed();
            return {};
        }
        if (UNLIKELY(GetValueType(env, result) != napi_number)) {
            TypeCheckFailed();
            return {};
        }
        NAPI_CHECK_FATAL(napi_get_value_int32(env, result, &val));
    }
    return EtsBoxPrimitive<EtsShort>::Create(coro, static_cast<EtsShort>(val));
}

JSCONVERT_DEFINE_TYPE(StdlibInt, EtsObject *);
JSCONVERT_WRAP(StdlibInt)
{
    auto *val = reinterpret_cast<EtsBoxPrimitive<EtsInt> *>(etsVal);
    napi_value jsVal;
    auto valValue = val->GetValue();
    ScopedNativeCodeThread nativeScope(EtsCoroutine::GetCurrent());
    NAPI_CHECK_FATAL(napi_create_int32(env, valValue, &jsVal));
    return jsVal;
}
JSCONVERT_UNWRAP(StdlibInt)
{
    EtsLong val;
    auto coro = EtsCoroutine::GetCurrent();
    {
        ScopedNativeCodeThread nativeScope(coro);
        napi_value result = jsVal;
        napi_valuetype valueType = GetValueType(env, jsVal);
        if (valueType == napi_object && !GetValueByValueOf(env, jsVal, CONSTRUCTOR_NAME_NUMBER, &result)) {
            TypeCheckFailed();
            return {};
        }
        if (UNLIKELY(GetValueType(env, result) != napi_number)) {
            TypeCheckFailed();
            return {};
        }
        NAPI_CHECK_FATAL(napi_get_value_int64(env, result, &val));
    }
    return EtsBoxPrimitive<EtsInt>::Create(coro, static_cast<EtsInt>(val));
}

JSCONVERT_DEFINE_TYPE(StdlibLong, EtsObject *);
JSCONVERT_WRAP(StdlibLong)
{
    auto *val = reinterpret_cast<EtsBoxPrimitive<EtsLong> *>(etsVal);
    napi_value jsVal;
    auto valValue = val->GetValue();
    ScopedNativeCodeThread nativeScope(EtsCoroutine::GetCurrent());
    NAPI_CHECK_FATAL(napi_create_int64(env, valValue, &jsVal));
    return jsVal;
}
JSCONVERT_UNWRAP(StdlibLong)
{
    EtsLong val;
    auto coro = EtsCoroutine::GetCurrent();
    {
        ScopedNativeCodeThread nativeScope(coro);
        napi_value result = jsVal;
        napi_valuetype valueType = GetValueType(env, jsVal);
        if (valueType == napi_object && !GetValueByValueOf(env, jsVal, CONSTRUCTOR_NAME_NUMBER, &result)) {
            TypeCheckFailed();
            return {};
        }
        if (UNLIKELY(GetValueType(env, result) != napi_number)) {
            TypeCheckFailed();
            return {};
        }
        NAPI_CHECK_FATAL(napi_get_value_int64(env, result, &val));
    }
    return EtsBoxPrimitive<EtsLong>::Create(coro, val);
}

JSCONVERT_DEFINE_TYPE(StdlibFloat, EtsObject *);
JSCONVERT_WRAP(StdlibFloat)
{
    auto *val = reinterpret_cast<EtsBoxPrimitive<EtsFloat> *>(etsVal);
    napi_value jsVal;
    auto valValue = val->GetValue();
    ScopedNativeCodeThread nativeScope(EtsCoroutine::GetCurrent());
    NAPI_CHECK_FATAL(napi_create_double(env, static_cast<double>(valValue), &jsVal));
    return jsVal;
}
JSCONVERT_UNWRAP(StdlibFloat)
{
    double val;
    auto coro = EtsCoroutine::GetCurrent();
    {
        ScopedNativeCodeThread nativeScope(coro);
        napi_value result = jsVal;
        napi_valuetype valueType = GetValueType(env, jsVal);
        if (valueType == napi_object && !GetValueByValueOf(env, jsVal, CONSTRUCTOR_NAME_NUMBER, &result)) {
            TypeCheckFailed();
            return {};
        }
        if (UNLIKELY(GetValueType(env, result) != napi_number)) {
            TypeCheckFailed();
            return {};
        }
        NAPI_CHECK_FATAL(napi_get_value_double(env, result, &val));
    }
    auto fval = static_cast<EtsFloat>(val);
    return EtsBoxPrimitive<EtsFloat>::Create(coro, fval);
}

JSCONVERT_DEFINE_TYPE(StdlibDouble, EtsObject *);
JSCONVERT_WRAP(StdlibDouble)
{
    auto *val = reinterpret_cast<EtsBoxPrimitive<EtsDouble> *>(etsVal);
    napi_value jsVal;
    auto valValue = val->GetValue();
    ScopedNativeCodeThread nativeScope(EtsCoroutine::GetCurrent());
    NAPI_CHECK_FATAL(napi_create_double(env, valValue, &jsVal));
    return jsVal;
}
JSCONVERT_UNWRAP(StdlibDouble)
{
    EtsDouble val;
    auto coro = EtsCoroutine::GetCurrent();
    {
        ScopedNativeCodeThread nativeScope(coro);
        napi_value result = jsVal;
        napi_valuetype valueType = GetValueType(env, jsVal);
        if (valueType == napi_object && !GetValueByValueOf(env, jsVal, CONSTRUCTOR_NAME_NUMBER, &result)) {
            TypeCheckFailed();
            return {};
        }
        if (UNLIKELY(GetValueType(env, result) != napi_number)) {
            TypeCheckFailed();
            return {};
        }
        NAPI_CHECK_FATAL(napi_get_value_double(env, result, &val));
    }
    return EtsBoxPrimitive<EtsDouble>::Create(coro, val);
}

}  // namespace ark::ets::interop::js

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_JS_CONVERT_STDLIB_H
