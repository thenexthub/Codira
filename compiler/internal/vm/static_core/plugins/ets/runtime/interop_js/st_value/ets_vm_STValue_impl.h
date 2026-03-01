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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_VM_STVALUE_IMPL_H
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_VM_STVALUE_IMPL_H

#include <ani.h>
#include <js_native_api.h>
#include <js_native_api_types.h>
#include <node_api.h>
#include <variant>
#include "ets_coroutine.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"

namespace ark::ets::interop::js {

napi_value STValueNamespaceInvokeFunctionImpl(napi_env env, napi_callback_info info);
napi_value STValueFunctionalObjectInvokeImpl(napi_env env, napi_callback_info info);
napi_value STValueObjectInvokeMethodImpl(napi_env env, napi_callback_info info);
napi_value STValueClassInvokeStaticMethodImpl(napi_env env, napi_callback_info info);

napi_value WrapStringImpl(napi_env env, napi_callback_info info);
napi_value WrapByteImpl(napi_env env, napi_callback_info info);
napi_value WrapCharImpl(napi_env env, napi_callback_info info);
napi_value WrapShortImpl(napi_env env, napi_callback_info info);
napi_value WrapIntImpl(napi_env env, napi_callback_info info);
napi_value WrapLongImpl(napi_env env, napi_callback_info info);
napi_value WrapFloatImpl(napi_env env, napi_callback_info info);
napi_value WrapNumberImpl(napi_env env, napi_callback_info info);
napi_value WrapBooleanImpl(napi_env env, napi_callback_info info);
napi_value WrapBigIntImpl(napi_env env, napi_callback_info info);
napi_value GetNullImpl(napi_env env, napi_callback_info info);
napi_value GetUndefinedImpl(napi_env env, napi_callback_info info);

napi_value STValueUnwrapToNumberImpl(napi_env env, napi_callback_info info);
napi_value STValueUnwrapToStringImpl(napi_env env, napi_callback_info info);
napi_value STValueUnwrapToBigIntImpl(napi_env env, napi_callback_info info);
napi_value STValueUnwrapToBooleanImpl(napi_env env, napi_callback_info info);

napi_value STValueIsBooleanImpl(napi_env env, napi_callback_info info);
napi_value STValueIsByteImpl(napi_env env, napi_callback_info info);
napi_value STValueIsCharImpl(napi_env env, napi_callback_info info);
napi_value STValueIsShortImpl(napi_env env, napi_callback_info info);
napi_value STValueIsIntImpl(napi_env env, napi_callback_info info);
napi_value STValueIsLongImpl(napi_env env, napi_callback_info info);
napi_value STValueIsFloatImpl(napi_env env, napi_callback_info info);
napi_value STValueIsNumberImpl(napi_env env, napi_callback_info info);
napi_value STValueIsStringImpl(napi_env env, napi_callback_info info);
napi_value STValueIsBigIntImpl(napi_env env, napi_callback_info info);
napi_value IsNullImpl(napi_env env, napi_callback_info info);
napi_value IsUndefinedImpl(napi_env env, napi_callback_info info);
napi_value IsEqualToImpl(napi_env env, napi_callback_info info);
napi_value IsStrictlyEqualToImpl(napi_env env, napi_callback_info info);
napi_value STValueTypeIsAssignableFromImpl(napi_env env, napi_callback_info info);
napi_value STValueObjectInstanceOfImpl(napi_env env, napi_callback_info info);

napi_value ClassGetSuperClassImpl(napi_env env, napi_callback_info info);
napi_value FixedArrayGetLengthImpl(napi_env env, napi_callback_info info);
napi_value ArrayGetLengthImpl(napi_env env, napi_callback_info info);
napi_value EnumGetIndexByNameImpl(napi_env env, napi_callback_info info);
napi_value EnumGetValueByNameImpl(napi_env env, napi_callback_info info);
napi_value ClassGetStaticFieldImpl(napi_env env, napi_callback_info info);
napi_value ClassSetStaticFieldImpl(napi_env env, napi_callback_info info);
napi_value ObjectGetPropertyImpl(napi_env env, napi_callback_info info);
napi_value ObjectSetPropertyImpl(napi_env env, napi_callback_info info);
napi_value STValueObjectGetTypeImpl(napi_env env, napi_callback_info info);
napi_value FixedArrayGetImpl(napi_env env, napi_callback_info info);
napi_value FixedArraySetImpl(napi_env env, napi_callback_info info);
napi_value ArrayGetImpl(napi_env env, napi_callback_info info);
napi_value ArraySetImpl(napi_env env, napi_callback_info info);
napi_value ArrayPushImpl(napi_env env, napi_callback_info info);
napi_value ArrayPopImpl(napi_env env, napi_callback_info info);
napi_value STValueNamespaceGetVariableImpl(napi_env env, napi_callback_info info);
napi_value STValueNamespcaeSetVariableImpl(napi_env env, napi_callback_info info);
napi_value STValueFindClassImpl(napi_env env, napi_callback_info info);
napi_value STValueFindNamespaceImpl(napi_env env, napi_callback_info info);
napi_value STValueFindEnumImpl(napi_env env, napi_callback_info info);

napi_value STValueClassInstantiateImpl(napi_env env, napi_callback_info info);
napi_value STValueNewFixedArrayPrimitiveImpl(napi_env env, napi_callback_info info);
napi_value STValueNewFixedArrayReferenceImpl(napi_env env, napi_callback_info info);
napi_value STValueNewArrayImpl(napi_env env, napi_callback_info info);

}  // namespace ark::ets::interop::js

#endif  // PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_VM_STVALUE_IMPL_H
