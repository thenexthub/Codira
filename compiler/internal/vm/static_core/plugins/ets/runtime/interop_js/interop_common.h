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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_INTEROP_COMMON_H_
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_INTEROP_COMMON_H_

#include "runtime/include/thread_scopes.h"
#include "runtime/mem/refstorage/global_object_storage.h"
#include "plugins/ets/runtime/interop_js/logger.h"
#include "plugins/ets/runtime/types/ets_bigint.h"
#include "libarkbase/utils/small_vector.h"

#include <node_api.h>

#include <functional>
#include <string_view>

#if defined(PANDA_JS_ETS_HYBRID_MODE)
#include "interfaces/inner_api/napi/native_node_hybrid_api.h"
#else
// NOLINTBEGIN(readability-identifier-naming, modernize-use-using)
typedef napi_value (*proxy_object_attach_cb)(napi_env env, void *data);
napi_status __attribute__((weak))  // CC-OFF(G.FMT.07) project code style
napi_wrap_with_xref(napi_env env, napi_value js_object, void *native_object, napi_finalize finalize_cb,
                    proxy_object_attach_cb proxy_cb, napi_ref *result);
napi_status __attribute__((weak))  // CC-OFF(G.FMT.07) project code style
napi_mark_attach_with_xref(napi_env env, napi_value js_object, void *attach_data, proxy_object_attach_cb attach_cb);
napi_status __attribute__((weak))  // CC-OFF(G.FMT.07) project code style
napi_xref_unwrap(napi_env env, napi_value js_object, void **result);
napi_status __attribute__((weak))  // CC-OFF(G.FMT.07) project code style
napi_create_xref(napi_env env, napi_value value, uint32_t initial_refcount, napi_ref *result);
napi_status __attribute__((weak))  // CC-OFF(G.FMT.07) project code style
napi_register_appstate_callback(napi_env env, void (*f)(int a1, int64_t a2));
// NOLINTEND(readability-identifier-naming, modernize-use-using)
#endif

namespace ark::ets::interop::js {

constexpr size_t BIGINT_BITS_NUM = 32;
constexpr size_t BIT_64 = 64;
constexpr std::string_view IS_STATIC_PROXY = "_isStaticProxy";

// NOLINTBEGIN(cppcoreguidelines-macro-usage)

[[noreturn]] PANDA_PUBLIC_API void InteropFatal(const char *message);
[[noreturn]] PANDA_PUBLIC_API void InteropFatal(const std::string &message);
[[noreturn]] PANDA_PUBLIC_API void InteropFatal(const char *message, napi_status status);

std::pair<SmallVector<uint64_t, 4U>, int> GetBigInt(napi_env env, napi_value jsVal);
SmallVector<uint64_t, 4U> ConvertBigIntArrayFromEtsToJs(const std::vector<uint32_t> &etsArray);
std::vector<EtsInt> ConvertBigIntArrayFromJsToEts(SmallVector<uint64_t, 4U> &jsArray);

PANDA_PUBLIC_API void ThrowNoInteropContextException();

void ThrowJSErrorNotAssignable(napi_env env, const EtsClass *fromKlass, EtsClass *toKlass);

bool NapiGetProperty(napi_env env, napi_value object, napi_value key, napi_value *result);
bool NapiGetNamedProperty(napi_env env, napi_value object, const char *utf8name, napi_value *result);

// Alternative for ASSERT(!expr) with interop stacktraces, enabled in NDEBUG
#define INTEROP_FATAL_IF(expr)                     \
    do {                                           \
        bool _expr = (expr);                       \
        if (UNLIKELY(_expr)) {                     \
            InteropFatal("INTEROP_FATAL: " #expr); \
            UNREACHABLE();                         \
        }                                          \
    } while (0)

#define INTEROP_RETURN_IF(expr) \
    do {                        \
        bool _expr = (expr);    \
        if (UNLIKELY(_expr)) {  \
            return false;       \
        }                       \
    } while (0)

#if defined(OHOS_PANDA_TRACE_ENABLE)
constexpr static const char *const INTEROP_TRACE_ENABLE = "ark.interop.trace.enable";
ALWAYS_INLINE void InteropTrace(const char *func, const char *file, int line);
#define INTEROP_TRACE()                                                    \
    do {                                                                   \
        ark::ets::interop::js::InteropTrace(__func__, __FILE__, __LINE__); \
    } while (0)
#else
#define INTEROP_TRACE() \
    do {                \
    } while (0)
#endif

#if !defined(NDEBUG)
#define NAPI_ASSERT_OK(expr) INTEROP_FATAL_IF(expr)
#else
#define NAPI_ASSERT_OK(expr) \
    do {                     \
        (expr);              \
    } while (0)
#endif

// Assertion for napi_* calls success, enabled in NDEBUG
#define NAPI_CHECK_FATAL(status)                                 \
    do {                                                         \
        napi_status _status = (status);                          \
        if (UNLIKELY(_status != napi_ok)) {                      \
            InteropFatal("NAPI_CHECK_FATAL: " #status, _status); \
            UNREACHABLE();                                       \
        }                                                        \
    } while (0)

#define NAPI_CHECK_RETURN(status)           \
    do {                                    \
        napi_status _status = (status);     \
        if (UNLIKELY(_status != napi_ok)) { \
            return false;                   \
        }                                   \
    } while (0)

#define CHECK_NAPI_STATUS(jsStatus, ctx, coro, result)   \
    do {                                                 \
        napi_status local_status = (jsStatus);           \
        if (local_status != napi_ok) {                   \
            if ((ctx) != nullptr && (coro) != nullptr) { \
                (ctx)->ForwardJSException(coro);         \
            }                                            \
            (result) = false;                            \
        }                                                \
    } while (0)

// NOLINTEND(cppcoreguidelines-macro-usage)

class NapiScope {
public:
    explicit NapiScope(napi_env env) : env_(env)
    {
        ASSERT_MANAGED_CODE();
        ScopedNativeCodeThread nativeScope(EtsCoroutine::GetCurrent());
        [[maybe_unused]] auto status = napi_open_handle_scope(env_, &scope_);
        ASSERT(status == napi_ok);
    }

    ~NapiScope()
    {
        ScopedNativeCodeThread nativeScope(EtsCoroutine::GetCurrent());
        [[maybe_unused]] auto status = napi_close_handle_scope(env_, scope_);
        ASSERT(status == napi_ok);
    }

    NO_COPY_SEMANTIC(NapiScope);
    NO_MOVE_SEMANTIC(NapiScope);

private:
    napi_env env_ {};
    napi_handle_scope scope_ {};
};

class NapiEscapableScope {
public:
    explicit NapiEscapableScope(napi_env env) : env_(env)
    {
        ASSERT_MANAGED_CODE();
        [[maybe_unused]] auto status = napi_open_escapable_handle_scope(env_, &scope_);
        ASSERT(status == napi_ok);
    }

    void Escape(napi_value &val)
    {
        [[maybe_unused]] auto status = napi_escape_handle(env_, scope_, val, &val);
        ASSERT(status == napi_ok);
    }

    ~NapiEscapableScope()
    {
        [[maybe_unused]] auto status = napi_close_escapable_handle_scope(env_, scope_);
        ASSERT(status == napi_ok);
    }

    NO_COPY_SEMANTIC(NapiEscapableScope);
    NO_MOVE_SEMANTIC(NapiEscapableScope);

private:
    napi_env env_ {};
    napi_escapable_handle_scope scope_ {};
};

template <bool IS_ADD_NATIVE_SCOPE = false>
inline napi_valuetype GetValueType(napi_env env, napi_value val)
{
    napi_valuetype vtype;
    if constexpr (IS_ADD_NATIVE_SCOPE) {
        ScopedNativeCodeThread nativeScope(EtsCoroutine::GetCurrent());
        NAPI_CHECK_FATAL(napi_typeof(env, val, &vtype));
    } else {
        ASSERT_NATIVE_CODE();
        NAPI_CHECK_FATAL(napi_typeof(env, val, &vtype));
    }
    return vtype;
}

inline napi_valuetype GetValueTypeNoScope(napi_env env, napi_value val)
{
    napi_valuetype vtype;
    NAPI_CHECK_FATAL(napi_typeof(env, val, &vtype));
    return vtype;
}

inline napi_value GetReferenceValue(napi_env env, napi_ref ref)
{
    napi_value val;
    NAPI_CHECK_FATAL(napi_get_reference_value(env, ref, &val));
    return val;
}

inline napi_value GetUndefined(napi_env env)
{
    napi_value jsValueUndefined {};
    NAPI_CHECK_FATAL(napi_get_undefined(env, &jsValueUndefined));
    return jsValueUndefined;
}

inline napi_value GetBoolean(napi_env env, bool value)
{
    napi_value result;
    NAPI_CHECK_FATAL(napi_get_boolean(env, value, &result));
    return result;
}

inline napi_value GetNull(napi_env env)
{
    napi_value jsValueNull {};
    NAPI_CHECK_FATAL(napi_get_null(env, &jsValueNull));
    return jsValueNull;
}

inline napi_value GetGlobal(napi_env env)
{
    napi_value jsValueGlobal {};
    NAPI_CHECK_FATAL(napi_get_global(env, &jsValueGlobal));
    return jsValueGlobal;
}

inline napi_value GetBooleanValue(napi_env env, bool val)
{
    napi_value jsValueBoolean {};
    NAPI_CHECK_FATAL(napi_get_boolean(env, val, &jsValueBoolean));
    return jsValueBoolean;
}

template <bool IS_ADD_NATIVE_SCOPE = false>
inline bool IsNull(napi_env env, napi_value val)
{
    return GetValueType<IS_ADD_NATIVE_SCOPE>(env, val) == napi_null;
}

template <bool IS_ADD_NATIVE_SCOPE = false>
inline bool IsUndefined(napi_env env, napi_value val)
{
    return GetValueType<IS_ADD_NATIVE_SCOPE>(env, val) == napi_undefined;
}

template <bool IS_ADD_NATIVE_SCOPE = false>
inline bool IsNullOrUndefined(napi_env env, napi_value val)
{
    napi_valuetype vtype = GetValueType<IS_ADD_NATIVE_SCOPE>(env, val);
    return vtype == napi_undefined || vtype == napi_null;
}

inline std::string GetString(napi_env env, napi_value jsVal)
{
    size_t length;
    NAPI_CHECK_FATAL(napi_get_value_string_utf8(env, jsVal, nullptr, 0, &length));
    std::string value;
    value.resize(length);
    // +1 for NULL terminated string!!!
    NAPI_CHECK_FATAL(napi_get_value_string_utf8(env, jsVal, value.data(), value.size() + 1, &length));
    return value;
}

inline uint32_t GetUint32(napi_env env, napi_value jsVal)
{
    uint32_t data;
    NAPI_CHECK_FATAL(napi_get_value_uint32(env, jsVal, &data));
    return data;
}

inline bool NapiIsExceptionPending(napi_env env)
{
    bool pending;
    NAPI_CHECK_FATAL(napi_is_exception_pending(env, &pending));
    return pending;
}

inline bool NapiThrownGeneric(napi_status rc)
{
    INTEROP_FATAL_IF(rc != napi_ok && rc != napi_generic_failure);
    return rc == napi_generic_failure;
}

inline napi_status NapiCallFunction(napi_env env, napi_value recv, napi_value func, size_t argc, const napi_value *argv,
                                    napi_value *result)
{
#if defined(PANDA_TARGET_OHOS) || defined(PANDA_JS_ETS_HYBRID_MODE)
    napi_value dummy;
    result = result != nullptr ? result : &dummy;
#endif  // PANDA_TARGET_OHOS
    return napi_call_function(env, recv, func, argc, argv, result);
}

}  // namespace ark::ets::interop::js

#endif  // PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_INTEROP_COMMON_H_
