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

#include "libarkbase/macros.h"
#include "interop_js/napi_impl/napi_impl.h"
#include "interop_js/napi_impl/detail/enumerate_napi.h"
#include "interop_js/interop_common.h"
#include "libarkbase/utils/logger.h"
#include "interop_js/logger.h"

#include <iostream>

#if defined(PANDA_JS_ETS_HYBRID_MODE)
// NOLINTBEGIN(readability-identifier-naming)
napi_status __attribute__((weak))  // CC-OFF(G.FMT.10) project code style
napi_wrap_with_xref([[maybe_unused]] napi_env env, [[maybe_unused]] napi_value js_object,
                    [[maybe_unused]] void *native_object, [[maybe_unused]] napi_finalize finalize_cb,
                    [[maybe_unused]] proxy_object_attach_cb proxy_cb, [[maybe_unused]] napi_ref *result)
{
    INTEROP_LOG(FATAL) << "ETS_INTEROP_GTEST_PLUGIN: " << __func__
                       << " is implemented in later versions of OHOS, please update." << std::endl;
    return napi_ok;
}

napi_status __attribute__((weak))  // CC-OFF(G.FMT.10) project code style
napi_mark_attach_with_xref([[maybe_unused]] napi_env env, [[maybe_unused]] napi_value js_object,
                           [[maybe_unused]] void *attach_data, [[maybe_unused]] proxy_object_attach_cb attach_cb)
{
    INTEROP_LOG(FATAL) << "ETS_INTEROP_GTEST_PLUGIN: " << __func__
                       << " is implemented in later versions of OHOS, please update." << std::endl;
    return napi_ok;
}

napi_status __attribute__((weak))  // CC-OFF(G.FMT.10) project code style
napi_xref_unwrap([[maybe_unused]] napi_env env, [[maybe_unused]] napi_value js_object, [[maybe_unused]] void **result)
{
    INTEROP_LOG(FATAL) << "ETS_INTEROP_GTEST_PLUGIN: " << __func__
                       << " is implemented in later versions of OHOS, please update." << std::endl;
    return napi_ok;
}

napi_status __attribute__((weak))  // CC-OFF(G.FMT.10) project code style
napi_create_xref([[maybe_unused]] napi_env env, [[maybe_unused]] napi_value value,
                 [[maybe_unused]] uint32_t initial_refcount, [[maybe_unused]] napi_ref *result)
{
    INTEROP_LOG(FATAL) << "ETS_INTEROP_GTEST_PLUGIN: " << __func__
                       << " is implemented in later versions of OHOS, please update." << std::endl;
    return napi_ok;
}

napi_status __attribute__((weak))  // CC-OFF(G.FMT.10) project code style
napi_mark_from_object([[maybe_unused]] napi_env env, [[maybe_unused]] napi_ref ref)
{
    INTEROP_LOG(FATAL) << "ETS_INTEROP_GTEST_PLUGIN: " << __func__
                       << " is implemented in later versions of OHOS, please update." << std::endl;
    return napi_ok;
}

napi_status __attribute__((weak))  // CC-OFF(G.FMT.10) project code style
napi_is_alive_object([[maybe_unused]] napi_env env, [[maybe_unused]] napi_ref ref, [[maybe_unused]] bool *result)
{
    INTEROP_LOG(FATAL) << "ETS_INTEROP_GTEST_PLUGIN: " << __func__
                       << " is implemented in later versions of OHOS, please update." << std::endl;
    return napi_ok;
}

napi_status __attribute__((weak))  // CC-OFF(G.FMT.10) project code style
napi_is_contain_object([[maybe_unused]] napi_env env, [[maybe_unused]] napi_ref ref, [[maybe_unused]] bool *result)
{
    INTEROP_LOG(FATAL) << "ETS_INTEROP_GTEST_PLUGIN: " << __func__
                       << " is implemented in later versions of OHOS, please update." << std::endl;
    return napi_ok;
}

napi_status __attribute__((weak))  // CC-OFF(G.FMT.10) project code style
napi_is_xref_type([[maybe_unused]] napi_env env, [[maybe_unused]] napi_value js_object, [[maybe_unused]] bool *result)
{
    INTEROP_LOG(FATAL) << "ETS_INTEROP_GTEST_PLUGIN: " << __func__
                       << " is implemented in later versions of OHOS, please update." << std::endl;
    return napi_ok;
}
// NOLINTEND(readability-identifier-naming)
#endif  // PANDA_JS_ETS_HYBRID_MODE

#ifdef PANDA_TARGET_OHOS
// NOLINTBEGIN(readability-identifier-naming)
napi_status __attribute__((weak))  // CC-OFF(G.FMT.10) project code style
napi_register_appstate_callback([[maybe_unused]] napi_env env, [[maybe_unused]] void (*f)(int a1, int64_t a2))
{
    return napi_ok;
}
// NOLINTEND(readability-identifier-naming)
#endif  // PANDA_TARGET_OHOS

namespace ark::ets::interop::js {

static NapiImpl gNapiImpl;

void NapiImpl::Init(NapiImpl impl)
{
    gNapiImpl = impl;
}

#ifdef PANDA_TARGET_OHOS

#include <node_api.h>
#include <node_api_types.h>

// NOTE: napi_fatal_exception() is not public in libace_napi.z.so.
extern "C" napi_status __attribute__((weak))  // CC-OFF(G.FMT.10) project code style
napi_fatal_exception([[maybe_unused]] napi_env env, [[maybe_unused]] napi_value err)
{
    INTEROP_LOG(FATAL) << "ETS_INTEROP_GTEST_PLUGIN: " << __func__ << " will be implemented in OHOS 5.0" << std::endl;
    return napi_ok;
}

extern "C" napi_status __attribute__((weak))  // CC-OFF(G.FMT.10) project code style
napi_add_env_cleanup_hook([[maybe_unused]] napi_env env, [[maybe_unused]] void (*fun)(void *arg),
                          [[maybe_unused]] void *arg)
{
    // NOTE: Empty stub. In CI currently used OHOS 4.0.8, but `napi_add_env_cleanup_hook`
    // is public since 4.1.0. Remove this method after CI upgrade.
    INTEROP_LOG(ERROR) << "napi_add_env_cleanup_hook is implemented in OHOS since 4.1.0, please update" << std::endl;
    return napi_ok;
}

extern "C" napi_status __attribute__((weak))  // CC-OFF(G.FMT.10) project code style
napi_get_value_string_utf16([[maybe_unused]] napi_env env, [[maybe_unused]] napi_value value,
                            [[maybe_unused]] char16_t *buf, [[maybe_unused]] size_t bufsize,
                            [[maybe_unused]] size_t *result)
{
    INTEROP_LOG(FATAL) << "ETS_INTEROP_GTEST_PLUGIN: " << __func__
                       << " is implemented in later versions of OHOS, please update." << std::endl;
    return napi_ok;
}

#else

/**
 * Since libarkruntime don't link with napi directly on host we should provide
 * implementation for all used napi symbols.
 */

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define WEAK_SYMBOL(name, ...)                              \
    extern "C" napi_status name(PARAMS_PAIR(__VA_ARGS__))   \
    {                                                       \
        /* CC-OFFNXT(G.PRE.05, G.PRE.09) code generation */ \
        return gNapiImpl.name(EVERY_SECOND(__VA_ARGS__));   \
    }

ENUMERATE_NAPI(WEAK_SYMBOL)
// NOLINTEND(cppcoreguidelines-macro-usage)

#endif
}  // namespace ark::ets::interop::js
