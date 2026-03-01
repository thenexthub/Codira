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

#include <ani.h>
#include <js_native_api.h>
#include <js_native_api_types.h>
#include <node_api.h>
#include <array>
#include <cstdint>
#include <cstdio>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include "ets_coroutine.h"
#include "ets_vm_STValue.h"
#include "include/mem/panda_containers.h"
#include "interop_js/interop_context.h"
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

napi_value STValueUnwrapToNumberImpl(napi_env env, napi_callback_info info)
{
    INTEROP_TRACE();
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;

    size_t jsArgc = 0;
    napi_value jsThis;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));

    if (jsArgc != 0) {
        ThrowJSBadArgCountError(env, jsArgc, 0);
        return nullptr;
    }

    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, &jsThis, nullptr));

    STValueData *data = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsThis));
    napi_value js_number {};
    ani_double value = 0.0;

    if (data->IsAniBoolean()) {
        value = static_cast<ani_double>(data->GetAniBoolean());
    } else if (data->IsAniChar()) {
        value = static_cast<ani_double>(data->GetAniChar());
    } else if (data->IsAniByte()) {
        value = static_cast<ani_double>(data->GetAniByte());
    } else if (data->IsAniShort()) {
        value = static_cast<ani_double>(data->GetAniShort());
    } else if (data->IsAniInt()) {
        value = static_cast<ani_double>(data->GetAniInt());
    } else if (data->IsAniLong()) {
        value = static_cast<ani_double>(data->GetAniLong());
    } else if (data->IsAniFloat()) {
        value = static_cast<ani_double>(data->GetAniFloat());
    } else if (data->IsAniDouble()) {
        value = data->GetAniDouble();
    } else {
        ThrowTypeCheckError(env, "\'this\'", "primitive");
        return nullptr;
    }

    NAPI_CHECK_FATAL(napi_create_double(env, value, &js_number));
    return js_number;
}

napi_value STValueUnwrapToStringImpl(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    INTEROP_TRACE();
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;
    size_t jsArgc = 0;
    napi_value jsThis;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));

    if (jsArgc != 0) {
        ThrowJSBadArgCountError(env, jsArgc, 0);
        return nullptr;
    }

    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, &jsThis, nullptr));

    auto aniEnv = GetAniEnv();

    STValueData *data = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsThis));
    if (!data->IsAniRef()) {
        ThrowJSThisNonObjectError(env);
        return nullptr;
    }

    if (data->IsAniNullOrUndefined(env)) {
        STValueThrowJSError(env, "\'this\' STValue instance does not wrap a value of type std.core.String");
        return nullptr;
    }

    ani_object aniObject = reinterpret_cast<ani_object>(data->GetAniRef());

    ani_class stringClass;
    ANI_CHECK_ERROR_RETURN(env, aniEnv->FindClass("std.core.String", &stringClass));

    ani_boolean isString = ANI_FALSE;
    ANI_CHECK_ERROR_RETURN(env, aniEnv->Object_InstanceOf(aniObject, stringClass, &isString));

    if (isString == ANI_FALSE) {
        STValueThrowJSError(env, "\'this\' STValue instance does not wrap a value of type std.core.String");
        return nullptr;
    }

    ani_string aniString = static_cast<ani_string>(data->GetAniRef());

    ani_size size {};
    aniEnv->String_GetUTF8Size(aniString, &size);

    std::string stdString(size + 1, 0);
    aniEnv->String_GetUTF8(aniString, stdString.data(), stdString.size(), &size);
    stdString.resize(size);

    napi_value js_string {};
    NAPI_CHECK_FATAL(napi_create_string_utf8(env, stdString.data(), stdString.size(), &js_string));
    return js_string;
}

napi_value STValueUnwrapToBooleanImpl(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    INTEROP_TRACE();
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;

    size_t jsArgc = 0;
    napi_value jsThis;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));
    if (jsArgc != 0) {
        ThrowJSBadArgCountError(env, jsArgc, 0);
        return nullptr;
    }

    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, &jsThis, nullptr));

    STValueData *data = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsThis));

    ani_boolean value_t = ANI_FALSE;

    if (data->IsAniBoolean()) {
        value_t = data->GetAniBoolean();
    } else if (data->IsAniChar()) {
        value_t = static_cast<ani_boolean>(data->GetAniChar());
    } else if (data->IsAniByte()) {
        value_t = static_cast<ani_boolean>(data->GetAniByte());
    } else if (data->IsAniShort()) {
        value_t = static_cast<ani_boolean>(data->GetAniShort());
    } else if (data->IsAniInt()) {
        value_t = static_cast<ani_boolean>(data->GetAniInt());
    } else if (data->IsAniLong()) {
        value_t = static_cast<ani_boolean>(data->GetAniLong());
    } else if (data->IsAniFloat()) {
        value_t = static_cast<ani_boolean>(data->GetAniFloat());
    } else if (data->IsAniDouble()) {
        value_t = static_cast<ani_boolean>(data->GetAniDouble());
    } else {
        ThrowTypeCheckError(env, "\'this\'", "primitive");
        return nullptr;
    }
    napi_value js_boolean {};
    NAPI_CHECK_FATAL(napi_get_boolean(env, value_t, &js_boolean));
    return js_boolean;
}

napi_value STValueUnwrapToBigIntImpl(napi_env env, napi_callback_info info)
{
    INTEROP_TRACE();
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;

    size_t jsArgc = 0;
    napi_value jsThis;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));
    if (jsArgc != 0) {
        ThrowJSBadArgCountError(env, jsArgc, 0);
        return nullptr;
    }
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, &jsThis, nullptr));

    auto aniEnv = GetAniEnv();
    STValueData *data = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsThis));
    if (!data->IsAniRef()) {
        ThrowJSThisNonObjectError(env);
        return nullptr;
    }
    if (data->IsAniNullOrUndefined(env)) {
        STValueThrowJSError(env, "Expected BigInt object, but got different type.");
        return nullptr;
    }
    ani_object aniObject = reinterpret_cast<ani_object>(data->GetAniRef());

    ani_class bigIntClass;
    AniCheckAndThrowToDynamic(env, aniEnv->FindClass("std.core.BigInt", &bigIntClass));
    ani_boolean isBigInt = ANI_FALSE;
    ANI_CHECK_ERROR_RETURN(env, aniEnv->Object_InstanceOf(aniObject, bigIntClass, &isBigInt));
    if (isBigInt == ANI_FALSE) {
        STValueThrowJSError(env, "Expected BigInt object, but got different type.");
        return nullptr;
    }

    ani_ref stringRef;
    if (aniEnv->Object_CallMethodByName_Ref(aniObject, "toString", ":C{std.core.String}", &stringRef) != ANI_OK) {
        STValueThrowJSError(env, "Failed to call toString on BigInt object.");
        return nullptr;
    }

    ani_string aniString = static_cast<ani_string>(stringRef);
    ani_size strSize;
    aniEnv->String_GetUTF8Size(aniString, &strSize);

    std::string cppStr(strSize + 1, '\0');
    aniEnv->String_GetUTF8(aniString, cppStr.data(), cppStr.size(), &strSize);
    cppStr.resize(strSize);

    napi_value global;
    NAPI_CHECK_FATAL(napi_get_global(env, &global));

    napi_value bigIntConstructor;
    NAPI_CHECK_FATAL(napi_get_named_property(env, global, "BigInt", &bigIntConstructor));

    napi_value stringArg;
    NAPI_CHECK_FATAL(napi_create_string_utf8(env, cppStr.c_str(), cppStr.size(), &stringArg));

    napi_value args[1] = {stringArg};
    napi_value jsBigInt;
    NAPI_CHECK_FATAL(napi_call_function(env, global, bigIntConstructor, 1, args, &jsBigInt));

    return jsBigInt;
}
}  // namespace ark::ets::interop::js
