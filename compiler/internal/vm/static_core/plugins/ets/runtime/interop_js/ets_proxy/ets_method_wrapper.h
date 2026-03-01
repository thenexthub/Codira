/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_METHOD_WRAPPER_H_
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_METHOD_WRAPPER_H_

#include "plugins/ets/runtime/interop_js/ets_proxy/typed_pointer.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/wrappers_cache.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/ets_field_wrapper.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/ets_method_set.h"
#include "plugins/ets/runtime/interop_js/interop_common.h"
#include "plugins/ets/runtime/types/ets_method.h"

#include <node_api.h>

namespace ark::ets::interop::js {
class InteropCtx;
}  // namespace ark::ets::interop::js

namespace ark::ets::interop::js::ets_proxy {

class EtsMethodWrapper;
class EtsClassWrapper;

using LazyEtsMethodWrapperLink = TypedPointer<EtsMethodSet, EtsMethodWrapper>;
using EtsMethodWrappersCache = WrappersCache<EtsMethodSet *, EtsMethodWrapper>;
using EtsMethodAndClassWrapper = std::tuple<EtsMethod *, const char *, EtsClassWrapper *>;
using FindMethodFunc = std::function<std::tuple<EtsMethod *, const char *, EtsClassWrapper *>(void *, size_t)>;

class EtsMethodWrapper {
public:
    static EtsMethodWrapper *GetMethod(InteropCtx *ctx, EtsMethodSet *etsMethodSet);

    napi_value GetJsValue(napi_env env) const
    {
        ASSERT(jsRef_);
        napi_value jsValue;
        NAPI_CHECK_FATAL(napi_get_reference_value(env, jsRef_, &jsValue));
        return jsValue;
    }

    EtsMethodSet *GetMethodSet() const
    {
        return etsMethodSet_;
    }

    EtsMethod *GetEtsMethod(uint32_t parametersNum) const
    {
        return etsMethodSet_->GetMethod(parametersNum);
    }

    Method *GetMethod(uint32_t parametersNum) const
    {
        EtsMethod *const etsMethod = etsMethodSet_->GetMethod(parametersNum);
        return etsMethod != nullptr ? etsMethod->GetPandaMethod() : nullptr;
    }

    /**
     * @param ctx - interop context
     * @param lazy_link in/out lazy  method wrapper link
     */
    static inline EtsMethodWrapper *ResolveLazyLink(InteropCtx *ctx, LazyEtsMethodWrapperLink &lazyLink)
    {
        if (LIKELY(lazyLink.IsResolved())) {
            return lazyLink.GetResolved();
        }
        EtsMethodWrapper *wrapper = EtsMethodWrapper::GetMethod(ctx, lazyLink.GetUnresolved());
        if (UNLIKELY(wrapper == nullptr)) {
            return nullptr;
        }
        ASSERT(wrapper->jsRef_ == nullptr);
        // Update lazyLink
        lazyLink = LazyEtsMethodWrapperLink(wrapper);
        return wrapper;
    }

    static napi_property_descriptor MakeNapiProperty(EtsMethodSet *method, LazyEtsMethodWrapperLink *lazyLinkSpace);
    static void AttachGetterSetterToProperty(EtsMethodSet *method, napi_property_descriptor &property);

    static napi_property_descriptor MakeNapiIteratorProperty(napi_value &iterator, EtsMethodSet *method,
                                                             LazyEtsMethodWrapperLink *lazyLinkSpace);

    template <bool IS_STATIC>
    static napi_value EtsMethodCallHandler(napi_env env, napi_callback_info cinfo);

    template <bool IS_STATIC>
    static napi_value GetterSetterCallHandler(napi_env env, napi_callback_info cinfo);

private:
    static std::unique_ptr<EtsMethodWrapper> CreateMethod(EtsMethodSet *etsMethodSet, EtsClassWrapper *owner);

    explicit EtsMethodWrapper(EtsMethodSet *etsMethodSet, EtsClassWrapper *owner)
        : etsMethodSet_(etsMethodSet), owner_(owner)
    {
    }

    template <bool IS_STATIC>
    static napi_value DoEtsMethodCall(napi_env env, napi_callback_info cinfo, FindMethodFunc &findMethodFunc);

    EtsMethodSet *const etsMethodSet_;
    EtsClassWrapper *const owner_ {};  // only for instance methods
    napi_ref jsRef_ {};                // only for functions (ETSGLOBAL::)
};

}  // namespace ark::ets::interop::js::ets_proxy

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_METHOD_WRAPPER_H_
