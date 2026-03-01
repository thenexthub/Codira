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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_JS_CONVERT_BASE_H
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_JS_CONVERT_BASE_H

#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "plugins/ets/runtime/interop_js/interop_common.h"
#include "plugins/ets/runtime/interop_js/js_value.h"
#include "runtime/handle_scope-inl.h"
#include "runtime/include/coretypes/class.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_arraybuffer.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/types/ets_bigint.h"
#include "plugins/ets/runtime/types/ets_promise.h"
#include "plugins/ets/runtime/types/ets_promise_ref.h"
#include "plugins/ets/runtime/types/ets_box_primitive-inl.h"
#include "plugins/ets/runtime/types/ets_method.h"

namespace ark::ets::interop::js {

static constexpr const char *CONSTRUCTOR_NAME_BOOLEAN = "Boolean";
static constexpr const char *CONSTRUCTOR_NAME_NUMBER = "Number";
static constexpr const char *CONSTRUCTOR_NAME_STRING = "String";

template <typename T>
inline EtsObject *AsEtsObject(T *obj)
{
    static_assert(std::is_base_of_v<ObjectHeader, T>);
    return reinterpret_cast<EtsObject *>(obj);
}

template <typename T>
inline T *FromEtsObject(EtsObject *obj)
{
    static_assert(std::is_base_of_v<ObjectHeader, T>);
    return reinterpret_cast<T *>(obj);
}

void JSConvertTypeCheckFailed(const char *typeName);
inline void JSConvertTypeCheckFailed(const std::string &s)
{
    JSConvertTypeCheckFailed(s.c_str());
}

static bool DoIsConstructor(napi_env &env, napi_value &jsValue, const char *constructorName)
{
    napi_value constructor;
    bool isInstanceof;
    NAPI_CHECK_FATAL(napi_get_named_property(env, GetGlobal(env), constructorName, &constructor));
    NAPI_CHECK_FATAL(napi_instanceof(env, jsValue, constructor, &isInstanceof));
    return isInstanceof;
}

template <bool IS_ADD_NATIVE_SCOPE = false>
static bool IsConstructor(napi_env &env, napi_value &jsValue, const char *constructorName)
{
    if constexpr (IS_ADD_NATIVE_SCOPE) {
        ScopedNativeCodeThread nativeScope(EtsCoroutine::GetCurrent());
        return DoIsConstructor(env, jsValue, constructorName);
    } else {
        ASSERT_NATIVE_CODE();
        return DoIsConstructor(env, jsValue, constructorName);
    }
}

static bool DoGetValueByValueOf(napi_env env, napi_value &jsValue, const char *constructorName, napi_value *result)
{
    if (!IsConstructor(env, jsValue, constructorName)) {
        return false;
    }
    napi_value method;
    NAPI_CHECK_FATAL(napi_get_named_property(env, jsValue, "valueOf", &method));
    NAPI_CHECK_FATAL(napi_call_function(env, jsValue, method, 0, nullptr, result));
    return true;
}

template <bool IS_ADD_NATIVE_SCOPE = false>
static bool GetValueByValueOf(napi_env env, napi_value &jsValue, const char *constructorName, napi_value *result)
{
    if constexpr (IS_ADD_NATIVE_SCOPE) {
        ScopedNativeCodeThread nativeScope(EtsCoroutine::GetCurrent());
        return DoGetValueByValueOf(env, jsValue, constructorName, result);
    } else {
        ASSERT_NATIVE_CODE();
        return DoGetValueByValueOf(env, jsValue, constructorName, result);
    }
}

// Base mixin class of JSConvert interface
// Represents primitive types and some built-in classes, has no state
template <typename Impl, typename ImplCpptype>
struct JSConvertBase {
    JSConvertBase() = delete;
    using cpptype = ImplCpptype;
    static constexpr bool IS_REFTYPE = std::is_pointer_v<cpptype>;
    static constexpr bool IS_JSVALUE = std::is_same_v<JSValue *, cpptype>;
    static constexpr size_t TYPE_SIZE = IS_REFTYPE ? ClassHelper::OBJECT_POINTER_SIZE : sizeof(cpptype);

    static void TypeCheckFailed()
    {
        JSConvertTypeCheckFailed(Impl::TYPE_NAME);
    }

    // Convert ets->js, returns nullptr if failed, throws JS exceptions
    static napi_value Wrap(napi_env env, cpptype etsVal)
    {
        if constexpr (IS_REFTYPE) {
            ASSERT(etsVal != nullptr);
        }
        auto res = Impl::WrapImpl(env, etsVal);
        ASSERT(res != nullptr || InteropCtx::SanityJSExceptionPending());
        return res;
    }

    // Convert js->ets, returns nullopt if failed, throws ETS/JS exceptions
    static std::optional<cpptype> Unwrap(InteropCtx *ctx, napi_env env, napi_value jsVal)
    {
        if constexpr (IS_REFTYPE) {
            ASSERT(!IsUndefined<true>(env, jsVal));
        }
        auto res = Impl::UnwrapImpl(ctx, env, jsVal);
        ASSERT(res.has_value() || InteropCtx::SanityJSExceptionPending() || InteropCtx::SanityETSExceptionPending());
        return res;
    }

    static napi_value WrapWithNullCheck(napi_env env, cpptype etsVal)
    {
        INTEROP_TRACE();
        if constexpr (IS_REFTYPE) {
            if (UNLIKELY(etsVal == nullptr)) {
                return GetUndefined(env);
            }
        }
        auto res = Impl::WrapImpl(env, etsVal);
        ASSERT(res != nullptr || InteropCtx::SanityJSExceptionPending());
        return res;
    }

    static std::optional<cpptype> UnwrapWithNullCheck(InteropCtx *ctx, napi_env env, napi_value jsVal)
    {
        INTEROP_TRACE();
        if constexpr (IS_REFTYPE) {
            // NOTE(kprokopenko) can't assign null to EtsString *, hence fallback into UnwrapImpl
            if (UNLIKELY(IsUndefined<true>(env, jsVal))) {
                return nullptr;
            }
        }
        auto res = Impl::UnwrapImpl(ctx, env, jsVal);
        ASSERT(res.has_value() || InteropCtx::SanityJSExceptionPending() || InteropCtx::SanityETSExceptionPending());
        return res;
    }
};

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define JSCONVERT_DEFINE_TYPE(type, cpptype_)                                                                \
    struct JSConvert##type : public JSConvertBase<JSConvert##type, cpptype_> {                               \
        static constexpr const char *TYPE_NAME = #type;                                                      \
        /* Must not fail */                                                                                  \
        [[maybe_unused]] static inline napi_value WrapImpl([[maybe_unused]] napi_env env,                    \
                                                           [[maybe_unused]] cpptype etsVal);                 \
        /* May fail */                                                                                       \
        [[maybe_unused]] static inline std::optional<cpptype> UnwrapImpl([[maybe_unused]] InteropCtx *ctx,   \
                                                                         [[maybe_unused]] napi_env env,      \
                                                                         [[maybe_unused]] napi_value jsVal); \
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define JSCONVERT_WRAP(type) \
    inline napi_value JSConvert##type::WrapImpl([[maybe_unused]] napi_env env, [[maybe_unused]] cpptype etsVal)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define JSCONVERT_UNWRAP(type)                                                  \
    inline std::optional<JSConvert##type::cpptype> JSConvert##type::UnwrapImpl( \
        [[maybe_unused]] InteropCtx *ctx, [[maybe_unused]] napi_env env, [[maybe_unused]] napi_value jsVal)
}  // namespace ark::ets::interop::js

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_JS_CONVERT_BASE_H
