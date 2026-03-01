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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_JS_REFCONVERT_RECORD_H_
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_JS_REFCONVERT_RECORD_H_

#include "libarkbase/macros.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/interop_common.h"
#include "plugins/ets/runtime/interop_js/js_refconvert.h"
#include "plugins/ets/runtime/interop_js/js_convert.h"
#include "plugins/ets/runtime/interop_js/js_refconvert_function.h"
#include "plugins/ets/runtime/interop_js/code_scopes.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_type.h"
#include "runtime/mem/local_object_handle.h"

namespace ark::ets::interop::js {

class JSRefConvertRecord : public JSRefConvert {
public:
    explicit JSRefConvertRecord(InteropCtx *ctx) : JSRefConvert(this)
    {
        auto env = ctx->GetJSEnv();
        napi_value handlerObj;
        NAPI_CHECK_FATAL(napi_create_object(env, &handlerObj));

        napi_value getHandleFunc;
        napi_value setHandleFunc;
        NAPI_CHECK_FATAL(napi_create_function(env, nullptr, NAPI_AUTO_LENGTH, JSRefConvertRecord::RecordGetHandler,
                                              nullptr, &getHandleFunc));
        NAPI_CHECK_FATAL(napi_create_function(env, nullptr, NAPI_AUTO_LENGTH, JSRefConvertRecord::RecordSetHandler,
                                              nullptr, &setHandleFunc));
        NAPI_CHECK_FATAL(napi_set_named_property(env, handlerObj, "get", getHandleFunc));
        NAPI_CHECK_FATAL(napi_set_named_property(env, handlerObj, "set", setHandleFunc));

        NAPI_CHECK_FATAL(napi_create_reference(env, handlerObj, 1, &handleObjCache_));
    }

    napi_value WrapImpl(InteropCtx *ctx, EtsObject *obj);

    EtsObject *UnwrapImpl(InteropCtx *ctx, napi_value jsValue);

private:
    napi_ref handleObjCache_ {};

    static napi_value RecordGetHandler(napi_env env, napi_callback_info cbinfo);

    static napi_value RecordSetHandler(napi_env env, napi_callback_info cbinfo);
};

}  // namespace ark::ets::interop::js

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_JS_REFCONVERT_RECORD_H_
