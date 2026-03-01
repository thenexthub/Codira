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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_XREF_OBJECT_OPERATOR_H_
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_XREF_OBJECT_OPERATOR_H_

#include <string>

#include "plugins/ets/runtime/interop_js/interop_common.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/types/ets_object.h"

namespace ark::ets::interop::js {

class XRefObjectOperator {
public:
    static XRefObjectOperator FromEtsObject(EtsHandle<EtsObject> &etsObject);

    EtsObject *GetProperty(EtsCoroutine *coro, const PandaString &name) const;

    EtsObject *GetProperty(EtsCoroutine *coro, EtsHandle<EtsObject> &keyObject) const;

    EtsObject *GetProperty(EtsCoroutine *coro, const uint32_t index) const;

    bool SetProperty(EtsCoroutine *coro, const std::string &name, EtsHandle<EtsObject> &valueObject) const;

    bool SetProperty(EtsCoroutine *coro, EtsHandle<EtsObject> &keyObject, EtsHandle<EtsObject> &valueObject) const;

    bool SetProperty(EtsCoroutine *coro, const uint32_t index, EtsHandle<EtsObject> &valueObject) const;

    bool IsInstanceOf(EtsCoroutine *coro, const XRefObjectOperator &rhsObject) const;

    EtsObject *Invoke(EtsCoroutine *coro, Span<VMHandle<ObjectHeader>> args) const;

    EtsObject *InvokeMethod(EtsCoroutine *coro, EtsHandle<EtsObject> &methodObject,
                            Span<VMHandle<ObjectHeader>> args) const;

    EtsObject *InvokeMethod(EtsCoroutine *coro, const std::string &name, Span<VMHandle<ObjectHeader>> args) const;

    bool HasProperty(EtsCoroutine *coro, const std::string &name, bool isOwnProperty = false) const;

    bool HasProperty(EtsCoroutine *coro, EtsHandle<EtsObject> &keyObject, bool isOwnProperty = false) const;

    bool HasProperty(EtsCoroutine *coro, const uint32_t index) const;

    EtsObject *Instantiate(EtsCoroutine *coro, Span<VMHandle<ObjectHeader>> args) const;

    static std::string TypeOf(EtsCoroutine *coro, EtsObject *obj);

    static bool IsTrue(EtsCoroutine *coro, EtsObject *obj);

    napi_value GetNapiValue(EtsCoroutine *coro) const;

    static bool StrictEquals(EtsCoroutine *coro, EtsObject *obj1, EtsObject *obj2);

private:
    static napi_value ConvertStaticObjectToDynamic(EtsCoroutine *coro, EtsHandle<EtsObject> &object);

    static napi_valuetype GetValueType(EtsCoroutine *coro, EtsObject *obj);

    explicit XRefObjectOperator(EtsHandle<EtsObject> &etsObject);

    EtsHandle<EtsObject> &etsObject_;
};

}  // namespace ark::ets::interop::js

#endif  // PANDA_PLUGINS_ETS_RUNTIME_INTEROP_XREF_OBJECT_OPERATOR_H_
