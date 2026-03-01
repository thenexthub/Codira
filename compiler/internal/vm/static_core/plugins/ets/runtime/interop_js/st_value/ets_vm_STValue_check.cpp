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
#include "include/mem/panda_containers.h"
#include "plugins/ets/runtime/ets_utils.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "plugins/ets/runtime/ets_vm_api.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/ets_proxy.h"
#include "plugins/ets/runtime/interop_js/call/call.h"
#include "plugins/ets/runtime/interop_js/interop_common.h"
#include "plugins/ets/runtime/interop_js/code_scopes.h"
#include "plugins/ets/runtime/interop_js/interop_context_api.h"
#include "plugins/ets/runtime/interop_js/st_value/ets_vm_STValue.h"
#include "compiler_options.h"
#include "compiler/compiler_logger.h"
#include "interop_js/napi_impl/napi_impl.h"
#include "runtime/include/runtime.h"

namespace ark::ets::interop::js {

napi_value STValueTemplateCheck(napi_env env, napi_callback_info info,
                                const std::function<ani_boolean(ani_ref ref)> &check)
{
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;

    size_t jsArgc = 0;
    napi_value jsThis {};
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));
    if (jsArgc != 0) {
        ThrowJSBadArgCountError(env, jsArgc, 0);
        return nullptr;
    }
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, &jsThis, nullptr));

    // 1. extract this pointer
    STValueData *data = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsThis));
    // 2. check result
    napi_value result {};
    ani_boolean checkResult;

    if (!data->IsAniRef()) {
        checkResult = ANI_FALSE;
    } else {
        checkResult = check(data->GetAniRef());
    }

    NAPI_CHECK_FATAL(napi_get_boolean(env, checkResult, &result));
    return result;
}

napi_value STValueIsStringImpl(napi_env env, napi_callback_info info)
{
    auto checkIsString = [env](ani_ref ref) -> ani_boolean {
        auto aniEnv = GetAniEnv();
        ani_boolean isString = ANI_FALSE;
        ani_boolean isNull = ANI_FALSE;
        ani_boolean isUndefined = ANI_FALSE;
        AniCheckAndThrowToDynamic(env, aniEnv->Reference_IsNull(ref, &isNull));
        if (isNull) {
            return isString;
        }

        AniCheckAndThrowToDynamic(env, aniEnv->Reference_IsUndefined(ref, &isUndefined));
        if (isUndefined) {
            return isString;
        }

        ani_class stringCls = nullptr;
        ANI_CHECK_ERROR_RETURN(env, aniEnv->FindClass("std.core.String", &stringCls));
        auto stringClsType = reinterpret_cast<ani_type>(stringCls);
        ANI_CHECK_ERROR_RETURN(env,
                               aniEnv->Object_InstanceOf(reinterpret_cast<ani_object>(ref), stringClsType, &isString));
        return isString;
    };

    return STValueTemplateCheck(env, info, checkIsString);
}

napi_value STValueIsBigIntImpl(napi_env env, napi_callback_info info)
{
    auto checkIsBigInt = [env](ani_ref ref) -> ani_boolean {
        auto aniEnv = GetAniEnv();
        ani_boolean isBigint = ANI_FALSE;
        ani_boolean isNull = ANI_FALSE;
        ani_boolean isUndefined = ANI_FALSE;
        ANI_CHECK_ERROR_RETURN(env, aniEnv->Reference_IsNull(ref, &isNull));
        if (isNull) {
            return isBigint;
        }

        ANI_CHECK_ERROR_RETURN(env, aniEnv->Reference_IsUndefined(ref, &isUndefined));
        if (isUndefined) {
            return isBigint;
        }

        ani_class bigintCls = nullptr;

        AniCheckAndThrowToDynamic(env, aniEnv->FindClass("std.core.BigInt", &bigintCls));
        auto bigintClsType = reinterpret_cast<ani_type>(bigintCls);
        ANI_CHECK_ERROR_RETURN(env,
                               aniEnv->Object_InstanceOf(reinterpret_cast<ani_object>(ref), bigintClsType, &isBigint));
        return isBigint;
    };

    return STValueTemplateCheck(env, info, checkIsBigInt);
}

// isNull(): boolean
napi_value IsNullImpl(napi_env env, napi_callback_info info)
{
    auto checkIsNull = [env](ani_ref ref) -> ani_boolean {
        auto aniEnv = GetAniEnv();
        ani_boolean isNull = ANI_FALSE;
        ANI_CHECK_ERROR_RETURN(env, aniEnv->Reference_IsNull(ref, &isNull));
        return isNull;
    };

    return STValueTemplateCheck(env, info, checkIsNull);
}

// isUndefined(): boolean
napi_value IsUndefinedImpl(napi_env env, napi_callback_info info)
{
    auto checkIsUndefined = [env](ani_ref ref) -> ani_boolean {
        auto aniEnv = GetAniEnv();
        ani_boolean isUndefined = ANI_FALSE;
        ANI_CHECK_ERROR_RETURN(env, aniEnv->Reference_IsUndefined(ref, &isUndefined));
        return isUndefined;
    };

    return STValueTemplateCheck(env, info, checkIsUndefined);
}

// isEqualTo(other: STValue): boolean  ref.isEqualTo(ref)
napi_value IsEqualToImpl(napi_env env, napi_callback_info info)
{
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;

    size_t jsArgc = 0;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));

    if (jsArgc != 1) {
        ThrowJSBadArgCountError(env, jsArgc, 1);
        return nullptr;
    }

    napi_value jsThis;
    napi_value jsArgv[1];
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, jsArgv, &jsThis, nullptr));

    auto *aniEnv = GetAniEnv();
    STValueData *data = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsThis));
    if (!data->IsAniRef()) {
        ThrowJSThisNonObjectError(env);
        return nullptr;
    }
    STValueData *otherData = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsArgv[0]));
    if (!otherData->IsAniRef()) {
        ThrowJSOtherNonObjectError(env);
        return nullptr;
    }

    ani_boolean isEqual = ANI_FALSE;
    ANI_CHECK_ERROR_RETURN(env, aniEnv->Reference_Equals(data->GetAniRef(), otherData->GetAniRef(), &isEqual));

    napi_value jsBool {};
    NAPI_CHECK_FATAL(napi_get_boolean(env, (isEqual == ANI_TRUE), &jsBool));

    return jsBool;
}

napi_value IsStrictlyEqualToImpl(napi_env env, napi_callback_info info)
{
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;

    size_t jsArgc = 0;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));

    if (jsArgc != 1) {
        ThrowJSBadArgCountError(env, jsArgc, 1);
        return nullptr;
    }

    napi_value jsThis;
    napi_value jsArgv[1];
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, jsArgv, &jsThis, nullptr));

    auto aniEnv = GetAniEnv();
    STValueData *data = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsThis));
    if (!data->IsAniRef()) {
        ThrowJSThisNonObjectError(env);
        return nullptr;
    }
    STValueData *otherData = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsArgv[0]));
    if (!otherData->IsAniRef()) {
        ThrowJSOtherNonObjectError(env);
        return nullptr;
    }

    ani_boolean isEqual = ANI_FALSE;
    ANI_CHECK_ERROR_RETURN(env, aniEnv->Reference_StrictEquals(data->GetAniRef(), otherData->GetAniRef(), &isEqual));

    napi_value js_bool {};
    NAPI_CHECK_FATAL(napi_get_boolean(env, (isEqual == ANI_TRUE), &js_bool));

    return js_bool;
}

napi_value STValueIsBooleanImpl(napi_env env, napi_callback_info info)
{
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;

    size_t jsArgc = 0;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));

    if (jsArgc != 0) {
        ThrowJSBadArgCountError(env, jsArgc, 0);
        return nullptr;
    }

    napi_value jsThis;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, &jsThis, nullptr));

    STValueData *data = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsThis));

    napi_value jsBool {};
    NAPI_CHECK_FATAL(napi_get_boolean(env, data->IsAniBoolean(), &jsBool));
    return jsBool;
}

napi_value STValueIsByteImpl(napi_env env, napi_callback_info info)
{
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;

    size_t jsArgc = 0;

    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));

    if (jsArgc != 0) {
        ThrowJSBadArgCountError(env, jsArgc, 0);
        return nullptr;
    }

    napi_value jsThis;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, &jsThis, nullptr));

    STValueData *data = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsThis));

    napi_value jsBool {};
    NAPI_CHECK_FATAL(napi_get_boolean(env, data->IsAniByte(), &jsBool));
    return jsBool;
}

napi_value STValueIsCharImpl(napi_env env, napi_callback_info info)
{
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;

    size_t jsArgc = 0;

    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));

    if (jsArgc != 0) {
        ThrowJSBadArgCountError(env, jsArgc, 0);
        return nullptr;
    }

    napi_value jsThis;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, &jsThis, nullptr));

    STValueData *data = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsThis));

    napi_value jsBool {};
    NAPI_CHECK_FATAL(napi_get_boolean(env, data->IsAniChar(), &jsBool));
    return jsBool;
}

napi_value STValueIsShortImpl(napi_env env, napi_callback_info info)
{
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;

    size_t jsArgc = 0;

    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));

    if (jsArgc != 0) {
        ThrowJSBadArgCountError(env, jsArgc, 0);
        return nullptr;
    }

    napi_value jsThis;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, &jsThis, nullptr));

    STValueData *data = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsThis));

    napi_value jsBool {};
    NAPI_CHECK_FATAL(napi_get_boolean(env, data->IsAniShort(), &jsBool));
    return jsBool;
}

napi_value STValueIsIntImpl(napi_env env, napi_callback_info info)
{
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;

    size_t jsArgc = 0;

    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));

    if (jsArgc != 0) {
        ThrowJSBadArgCountError(env, jsArgc, 0);
        return nullptr;
    }

    napi_value jsThis;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, &jsThis, nullptr));

    STValueData *data = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsThis));

    napi_value jsBool;
    NAPI_CHECK_FATAL(napi_get_boolean(env, data->IsAniInt(), &jsBool));
    return jsBool;
}

napi_value STValueIsLongImpl(napi_env env, napi_callback_info info)
{
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;

    size_t jsArgc = 0;

    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));

    if (jsArgc != 0) {
        ThrowJSBadArgCountError(env, jsArgc, 0);
        return nullptr;
    }

    napi_value jsThis;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, &jsThis, nullptr));

    STValueData *data = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsThis));

    napi_value jsBool {};
    NAPI_CHECK_FATAL(napi_get_boolean(env, data->IsAniLong(), &jsBool));
    return jsBool;
}

napi_value STValueIsFloatImpl(napi_env env, napi_callback_info info)
{
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;

    size_t jsArgc = 0;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));

    if (jsArgc != 0) {
        ThrowJSBadArgCountError(env, jsArgc, 0);
        return nullptr;
    }

    napi_value jsThis;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, &jsThis, nullptr));

    STValueData *data = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsThis));

    napi_value jsBool {};
    NAPI_CHECK_FATAL(napi_get_boolean(env, data->IsAniFloat(), &jsBool));
    return jsBool;
}

napi_value STValueIsNumberImpl(napi_env env, napi_callback_info info)
{
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;

    size_t jsArgc = 0;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));

    if (jsArgc != 0) {
        ThrowJSBadArgCountError(env, jsArgc, 0);
        return nullptr;
    }

    napi_value jsThis;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, &jsThis, nullptr));

    STValueData *data = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsThis));

    napi_value jsBool {};
    NAPI_CHECK_FATAL(napi_get_boolean(env, data->IsAniDouble(), &jsBool));
    return jsBool;
}

// typeIsAssignableFrom(from_type: STValue, to_type: STValue): boolean
napi_value STValueTypeIsAssignableFromImpl(napi_env env, napi_callback_info info)
{
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;
    auto *aniEnv = GetAniEnv();

    size_t jsArgc = 0;
    napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr);
    if (jsArgc != 2U) {
        ThrowJSBadArgCountError(env, jsArgc, 2U);
        return nullptr;
    }
    napi_value jsArgv[2];
    napi_get_cb_info(env, info, &jsArgc, jsArgv, nullptr, nullptr);

    STValueData *fromData = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsArgv[0]));
    if (fromData == nullptr || !fromData->IsAniRef() || fromData->IsAniNullOrUndefined(env)) {
        ThrowJSNonObjectError(env, "fromType");
        return nullptr;
    }
    STValueData *toData = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsArgv[1]));
    if (toData == nullptr || !toData->IsAniRef() || toData->IsAniNullOrUndefined(env)) {
        ThrowJSNonObjectError(env, "toType");
        return nullptr;
    }
    ani_type fromType = static_cast<ani_type>(fromData->GetAniRef());
    ani_type toType = static_cast<ani_type>(toData->GetAniRef());

    ani_boolean resBoolean {};
    ANI_CHECK_ERROR_RETURN(env, aniEnv->Type_IsAssignableFrom(fromType, toType, &resBoolean));

    napi_value jsBoolean {};
    NAPI_CHECK_FATAL(napi_get_boolean(env, resBoolean == ANI_TRUE, &jsBoolean));
    return jsBoolean;
}

napi_value STValueObjectInstanceOfImpl(napi_env env, napi_callback_info info)
{
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;
    auto *aniEnv = GetAniEnv();

    size_t jsArgc = 0;
    napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr);
    if (jsArgc != 1) {
        ThrowJSBadArgCountError(env, jsArgc, 1);
        return nullptr;
    }
    napi_value jsThis {};
    napi_value jsArgv[1];
    napi_get_cb_info(env, info, &jsArgc, jsArgv, &jsThis, nullptr);

    STValueData *objectData = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsThis));
    if (objectData == nullptr || !objectData->IsAniRef() || objectData->IsAniNullOrUndefined(env)) {
        ThrowJSThisNonObjectError(env);
        return nullptr;
    }
    STValueData *typeData = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsArgv[0]));
    if (typeData == nullptr || !typeData->IsAniRef() || typeData->IsAniNullOrUndefined(env)) {
        ThrowJSNonObjectError(env, "type");
        return nullptr;
    }
    ani_object objObject = static_cast<ani_object>(objectData->GetAniRef());
    ani_type typeObject = static_cast<ani_type>(typeData->GetAniRef());
    ani_boolean resBoolean {};
    ANI_CHECK_ERROR_RETURN(env, aniEnv->Object_InstanceOf(objObject, typeObject, &resBoolean));

    napi_value jsBoolean {};
    NAPI_CHECK_FATAL(napi_get_boolean(env, resBoolean == ANI_TRUE, &jsBoolean));
    return jsBoolean;
}
}  // namespace ark::ets::interop::js
