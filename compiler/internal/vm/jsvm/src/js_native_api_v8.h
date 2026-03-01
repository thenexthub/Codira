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

#ifndef SRC_JS_NATIVE_API_V8_H_
#define SRC_JS_NATIVE_API_V8_H_

#include "jsvm_env.h"
#include "jsvm_util.h"
#include "type_conversion.h"

inline JSVM_Status SetLastError(JSVM_Env env,
                                JSVM_Status errorCode,
                                uint32_t engineErrorCode = 0,
                                void* engineReserved = nullptr)
{
    env->lastError.errorCode = errorCode;
    env->lastError.engineErrorCode = engineErrorCode;
    env->lastError.engineReserved = engineReserved;
    return errorCode;
}

#define RETURN_STATUS_IF_FALSE(env, condition, status) \
    do {                                               \
        if (!(condition)) {                            \
            return SetLastError((env), (status));      \
        }                                              \
    } while (0)

#define RETURN_STATUS_IF_FALSE_WITHOUT_ENV(condition, status) \
    do {                                                      \
        if (!(condition)) {                                   \
            return status;                                    \
        }                                                     \
    } while (0)

#define RETURN_STATUS_IF_FALSE_WITH_PREAMBLE(env, condition, status)                              \
    do {                                                                                          \
        if (!(condition)) {                                                                       \
            return SetLastError((env), tryCatch.HasCaught() ? JSVM_PENDING_EXCEPTION : (status)); \
        }                                                                                         \
    } while (0)

#define RETURN_IF_EXCEPTION_HAS_CAUGHT(env)                     \
    do {                                                        \
        if (tryCatch.HasCaught()) {                             \
            return SetLastError((env), JSVM_PENDING_EXCEPTION); \
        }                                                       \
    } while (0)

#define CHECK_ENV(env)               \
    do {                             \
        if ((env) == nullptr) {      \
            return JSVM_INVALID_ARG; \
        }                            \
    } while (0)

#define CHECK_ENV_NOT_IN_GC(env) \
    do {                         \
        CHECK_ENV((env));        \
        (env)->CheckGCAccess();  \
    } while (0)

#define CHECK_ARG(env, arg) RETURN_STATUS_IF_FALSE((env), ((arg) != nullptr), JSVM_INVALID_ARG)

#define CHECK_ARG_WITHOUT_ENV(arg) RETURN_STATUS_IF_FALSE_WITHOUT_ENV(((arg) != nullptr), JSVM_INVALID_ARG)

#define CHECK_ARG_NOT_ZERO(env, arg) RETURN_STATUS_IF_FALSE((env), ((arg) != 0), JSVM_INVALID_ARG)

#define CHECK_ARG_WITH_PREAMBLE(env, arg) \
    RETURN_STATUS_IF_FALSE_WITH_PREAMBLE((env), ((arg) != nullptr), JSVM_INVALID_ARG)

#define CHECK_MAYBE_EMPTY(env, maybe, status) RETURN_STATUS_IF_FALSE((env), !((maybe).IsEmpty()), (status))

#define CHECK_MAYBE_EMPTY_WITH_PREAMBLE(env, maybe, status) \
    RETURN_STATUS_IF_FALSE_WITH_PREAMBLE((env), !((maybe).IsEmpty()), (status))

#define CHECK_MAYBE_NOTHING(env, maybe, status) RETURN_STATUS_IF_FALSE((env), !((maybe).IsNothing()), (status))

#define CHECK_MAYBE_NOTHING_WITH_PREAMBLE(env, maybe, status) \
    RETURN_STATUS_IF_FALSE_WITH_PREAMBLE((env), !((maybe).IsNothing()), (status))

// JSVM_PREAMBLE is not wrapped in do..while: tryCatch must have function scope
#define JSVM_PREAMBLE(env)                                                                               \
    CHECK_ENV((env));                                                                                    \
    RETURN_STATUS_IF_FALSE((env), (env)->lastException.IsEmpty(), JSVM_PENDING_EXCEPTION);               \
    RETURN_STATUS_IF_FALSE((env), (env)->CanCallIntoJS(),                                                \
                           ((env)->GetVersion() == JSVM_VERSION_EXPERIMENTAL ? JSVM_CANNOT_RUN_JS        \
                                                                             : JSVM_PENDING_EXCEPTION)); \
    ClearLastError((env));                                                                               \
    v8impl::TryCatch tryCatch((env))

#define CHECK_TO_TYPE(env, type, context, result, src, status)                    \
    do {                                                                          \
        CHECK_ARG((env), (src));                                                  \
        auto maybe = v8impl::V8LocalValueFromJsValue((src))->To##type((context)); \
        CHECK_MAYBE_EMPTY((env), maybe, (status));                                \
        (result) = maybe.ToLocalChecked();                                        \
    } while (0)

#define CHECK_TO_TYPE_WITH_PREAMBLE(env, type, context, result, src, status)      \
    do {                                                                          \
        CHECK_ARG_WITH_PREAMBLE((env), (src));                                    \
        auto maybe = v8impl::V8LocalValueFromJsValue((src))->To##type((context)); \
        CHECK_MAYBE_EMPTY_WITH_PREAMBLE((env), maybe, (status));                  \
        (result) = maybe.ToLocalChecked();                                        \
    } while (0)

#define CHECK_TO_FUNCTION(env, result, src)                                     \
    do {                                                                        \
        CHECK_ARG((env), (src));                                                \
        v8::Local<v8::Value> v8value = v8impl::V8LocalValueFromJsValue((src));  \
        RETURN_STATUS_IF_FALSE((env), v8value->IsFunction(), JSVM_INVALID_ARG); \
        (result) = v8value.As<v8::Function>();                                  \
    } while (0)

#define CHECK_TO_OBJECT(env, context, result, src) \
    CHECK_TO_TYPE((env), Object, (context), (result), (src), JSVM_OBJECT_EXPECTED)

#define CHECK_TO_BIGINT(env, context, result, src) \
    CHECK_TO_TYPE((env), BigInt, (context), (result), (src), JSVM_BIGINT_EXPECTED)

#define CHECK_TO_OBJECT_WITH_PREAMBLE(env, context, result, src) \
    CHECK_TO_TYPE_WITH_PREAMBLE((env), Object, (context), (result), (src), JSVM_OBJECT_EXPECTED)

#define CHECK_TO_STRING(env, context, result, src) \
    CHECK_TO_TYPE((env), String, (context), (result), (src), JSVM_STRING_EXPECTED)

#define CHECK_TO_NUMBER(env, context, result, src) \
    CHECK_TO_TYPE((env), Number, (context), (result), (src), JSVM_NUMBER_EXPECTED)

#define GET_RETURN_STATUS(env) (!tryCatch.HasCaught() ? JSVM_OK : SetLastError((env), JSVM_PENDING_EXCEPTION))

#define THROW_RANGE_ERROR_IF_FALSE(env, condition, error, message) \
    do {                                                           \
        if (!(condition)) {                                        \
            OH_JSVM_ThrowRangeError((env), (error), (message));    \
            return SetLastError((env), JSVM_GENERIC_FAILURE);      \
        }                                                          \
    } while (0)

// jsvm-api defines JSVM_AUTO_LENGTH as the indicator that a string
// is null terminated. For V8 the equivalent is -1. The assert
// validates that our cast of JSVM_AUTO_LENGTH results in -1 as
// needed by V8.
#define CHECK_NEW_FROM_UTF8_LEN(env, result, str, len)                                                                \
    do {                                                                                                              \
        static_assert(static_cast<int>(JSVM_AUTO_LENGTH) == -1, "Casting JSVM_AUTO_LENGTH to int must result in -1"); \
        RETURN_STATUS_IF_FALSE((env), ((len) == JSVM_AUTO_LENGTH) || (len) <= INT_MAX, JSVM_INVALID_ARG);             \
        RETURN_STATUS_IF_FALSE((env), (str) != nullptr, JSVM_INVALID_ARG);                                            \
        auto str_maybe =                                                                                              \
            v8::String::NewFromUtf8((env)->isolate, (str), v8::NewStringType::kInternalized, static_cast<int>(len));  \
        CHECK_MAYBE_EMPTY((env), str_maybe, JSVM_GENERIC_FAILURE);                                                    \
        (result) = str_maybe.ToLocalChecked();                                                                        \
    } while (0)

#define CHECK_NEW_FROM_UTF8(env, result, str) CHECK_NEW_FROM_UTF8_LEN((env), (result), (str), JSVM_AUTO_LENGTH)

#define CHECK_NEW_STRING_ARGS(env, str, length, result)                                                         \
    do {                                                                                                        \
        CHECK_ENV_NOT_IN_GC((env));                                                                             \
        if ((length) > 0)                                                                                       \
            CHECK_ARG((env), (str));                                                                            \
        CHECK_ARG((env), (result));                                                                             \
        RETURN_STATUS_IF_FALSE((env), ((length) == JSVM_AUTO_LENGTH) || (length) <= INT_MAX, JSVM_INVALID_ARG); \
    } while (0)

#define CREATE_TYPED_ARRAY(env, type, sizeOfElement, buffer, byteOffset, length, out)                          \
    do {                                                                                                       \
        if ((sizeOfElement) > 1) {                                                                             \
            THROW_RANGE_ERROR_IF_FALSE((env), (byteOffset) % (sizeOfElement) == 0,                             \
                                       "ERR_JSVM_INVALID_TYPEDARRAY_ALIGNMENT",                                \
                                       "start offset of " #type " should be a multiple of " #sizeOfElement);   \
        }                                                                                                      \
        THROW_RANGE_ERROR_IF_FALSE((env), (length) * (sizeOfElement) + (byteOffset) <= (buffer)->ByteLength(), \
                                   "ERR_JSVM_INVALID_TYPEDARRAY_LENGTH", "Invalid typed array length");        \
        (out) = v8::type::New((buffer), (byteOffset), (length));                                               \
    } while (0)

#define JSVM_PRIVATE_KEY(isolate, suffix) (v8impl::GetIsolateData(isolate)->suffix##Key.Get(isolate))

FORCE_NOINLINE void AddValueToScopeCheck(JSVM_Env env, JSVM_Value val);
FORCE_NOINLINE void AddValueToEscapeScopeCheck(JSVM_Env env, JSVM_Value val);
FORCE_NOINLINE void CheckScope(JSVM_Env env, JSVM_Value val, const char *callerFunctionName);

#define ADD_VAL_TO_SCOPE_CHECK(env, val)              \
    do {                                              \
        if (UNLIKELY((env)->debugFlags)) {            \
            AddValueToScopeCheck(env, val, __func__); \
        }                                             \
    } while (0)

#define ADD_VAL_TO_ESCAPE_SCOPE_CHECK(env, val)             \
    do {                                                    \
        if (UNLIKELY((env)->debugFlags)) {                  \
            AddValueToScopeCheck(env, val, __func__, true); \
        }                                                   \
    } while (0)

#define CHECK_SCOPE(env, val)               \
    do {                                    \
        if (UNLIKELY((env)->debugFlags)) {  \
            CheckScope(env, val, __func__); \
        }                                   \
    } while (0)

#define CHECK_SCOPE_ISTYPE(env, val, func)  \
    do {                                    \
        if (UNLIKELY((env)->debugFlags)) {  \
            CheckScope(env, val, __func__); \
            return func;                    \
        }                                   \
    } while (0)

#define STATUS_CALL(call)            \
    do {                             \
        JSVM_Status status = (call); \
        if (status != JSVM_OK)       \
            return status;           \
    } while (0)

namespace v8impl {

class TryCatch : public v8::TryCatch {
public:
    explicit TryCatch(JSVM_Env env) : v8::TryCatch(env->isolate), env(env) {}

    ~TryCatch()
    {
        if (HasCaught()) {
            env->lastException.Reset(env->isolate, Exception());
        }
    }

private:
    JSVM_Env env;
};

typedef JSVM_Value (*GetterCallback)(JSVM_Env, JSVM_Value, JSVM_Value, JSVM_Value);
typedef JSVM_Value (*SetterCallback)(JSVM_Env, JSVM_Value, JSVM_Value, JSVM_Value, JSVM_Value);
typedef JSVM_Value (*DeleterCallback)(JSVM_Env, JSVM_Value, JSVM_Value, JSVM_Value);
typedef JSVM_Value (*EnumeratorCallback)(JSVM_Env, JSVM_Value, JSVM_Value);

struct JSVM_PropertyHandlerCfgStruct {
    GetterCallback namedGetterCallback;
    SetterCallback namedSetterCallback;
    DeleterCallback nameDeleterCallback;
    EnumeratorCallback namedEnumeratorCallback;
    GetterCallback indexedGetterCallback;
    SetterCallback indexedSetterCallback;
    DeleterCallback indexedDeleterCallback;
    EnumeratorCallback indexedEnumeratorCallback;
    JSVM_Ref namedPropertyData;
    JSVM_Ref indexedPropertyData;
};

inline JSVM_PropertyHandlerCfgStruct* CreatePropertyCfg(JSVM_Env env, JSVM_PropertyHandlerCfg propertyCfg)
{
    JSVM_PropertyHandlerCfgStruct* newPropertyCfg = new JSVM_PropertyHandlerCfgStruct;
    if (newPropertyCfg != nullptr && propertyCfg != nullptr) {
        newPropertyCfg->namedGetterCallback = propertyCfg->genericNamedPropertyGetterCallback;
        newPropertyCfg->namedSetterCallback = propertyCfg->genericNamedPropertySetterCallback;
        newPropertyCfg->nameDeleterCallback = propertyCfg->genericNamedPropertyDeleterCallback;
        newPropertyCfg->namedEnumeratorCallback = propertyCfg->genericNamedPropertyEnumeratorCallback;
        newPropertyCfg->indexedGetterCallback = propertyCfg->genericIndexedPropertyGetterCallback;
        newPropertyCfg->indexedSetterCallback = propertyCfg->genericIndexedPropertySetterCallback;
        newPropertyCfg->indexedDeleterCallback = propertyCfg->genericIndexedPropertyDeleterCallback;
        newPropertyCfg->indexedEnumeratorCallback = propertyCfg->genericIndexedPropertyEnumeratorCallback;
        newPropertyCfg->namedPropertyData = nullptr;
        newPropertyCfg->indexedPropertyData = nullptr;
        if (propertyCfg->namedPropertyData != nullptr) {
            v8::Local<v8::Value> v8_value = v8impl::V8LocalValueFromJsValue(propertyCfg->namedPropertyData);
            v8impl::UserReference* reference = v8impl::UserReference::New(env, v8_value, 1);
            newPropertyCfg->namedPropertyData = reinterpret_cast<JSVM_Ref>(reference);
        }

        if (propertyCfg->indexedPropertyData != nullptr) {
            v8::Local<v8::Value> v8_value = v8impl::V8LocalValueFromJsValue(propertyCfg->indexedPropertyData);
            v8impl::UserReference* reference = v8impl::UserReference::New(env, v8_value, 1);
            newPropertyCfg->indexedPropertyData = reinterpret_cast<JSVM_Ref>(reference);
        }
    }

    return newPropertyCfg;
}

inline void CfgFinalizedCallback(JSVM_Env env, void* finalizeData, void* finalizeHint)
{
    auto cfg = reinterpret_cast<JSVM_PropertyHandlerCfgStruct*>(finalizeData);
    if (cfg->namedPropertyData != nullptr) {
        delete reinterpret_cast<v8impl::UserReference*>(cfg->namedPropertyData);
    }
    if (cfg->indexedPropertyData != nullptr) {
        delete reinterpret_cast<v8impl::UserReference*>(cfg->indexedPropertyData);
    }
    delete cfg;
}

} // end of namespace v8impl

#endif // SRC_JS_NATIVE_API_V8_H_
