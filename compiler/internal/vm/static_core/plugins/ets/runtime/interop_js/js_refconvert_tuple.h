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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_JS_REFCONVERT_TUPLE_H
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_JS_REFCONVERT_TUPLE_H

#include "plugins/ets/runtime/interop_js/call/call.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/js_convert.h"
#include "plugins/ets/runtime/interop_js/js_proxy/js_proxy.h"

namespace ark::ets::interop::js {

template <bool IS_TUPLEN>
class JSRefConvertTuple : public JSRefConvert {
public:
    explicit JSRefConvertTuple(InteropCtx *ctx, Class *klass)
        : JSRefConvert(this), klass_ {EtsClass::FromRuntimeClass(klass)}, jsProxyHandlerRef_ {CreateJsProxyHandler(ctx)}
    {
    }

    JSRefConvertTuple(const JSRefConvertTuple &) = delete;
    JSRefConvertTuple(JSRefConvertTuple &&) = delete;

    JSRefConvertTuple &operator=(const JSRefConvertTuple &) = delete;
    JSRefConvertTuple &operator=(JSRefConvertTuple &&) = delete;

    ~JSRefConvertTuple() override
    {
        napi_env env = InteropCtx::Current(EtsCoroutine::GetCurrent())->GetJSEnv();
        NAPI_CHECK_FATAL(napi_delete_reference(env, jsProxyHandlerRef_));
    }

    napi_value WrapImpl(InteropCtx *ctx, EtsObject *etsObject);
    EtsObject *UnwrapImpl(InteropCtx *ctx, napi_value jsObject);

private:
    EtsClass *klass_;
    napi_ref jsProxyHandlerRef_;

    napi_ref CreateJsProxyHandler(InteropCtx *ctx);
};

}  // namespace ark::ets::interop::js

#endif
