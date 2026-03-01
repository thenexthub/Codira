/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_CLASS_WRAPPER_H_
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_CLASS_WRAPPER_H_

#include "libarkbase/macros.h"
#include "plugins/ets/runtime/ets_utils.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/ets_field_wrapper.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/ets_method_wrapper.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/ets_object_reference.h"
#include "plugins/ets/runtime/interop_js/js_proxy/js_proxy.h"
#include "plugins/ets/runtime/interop_js/js_refconvert.h"
#include "plugins/ets/runtime/interop_js/interop_common.h"

#include <node_api.h>
#include <vector>

namespace ark::ets {
class EtsClass;
}  // namespace ark::ets

namespace ark::ets::interop::js::ets_proxy {

using EtsClassWrappersCache = WrappersCache<EtsClass *, EtsClassWrapper>;

class EtsClassWrapper {
public:
    // clang-format off
    static constexpr auto FIELD_ATTR         = static_cast<napi_property_attributes>(napi_writable | napi_enumerable);
    static constexpr auto METHOD_ATTR        = napi_default;
    // NOLINTBEGIN(hicpp-signed-bitwise)
    static constexpr auto STATIC_FIELD_ATTR  = static_cast<napi_property_attributes>(napi_writable | napi_enumerable |\
                                                                       napi_static);
    // NOLINTEND(hicpp-signed-bitwise)
    static constexpr auto STATIC_METHOD_ATTR = napi_static;
    // clang-format on

    struct OverloadEntry {
        const char *signature;
        uint32_t argCount;
        const char *implMethodName;
    };

    using OverloadsMap = std::unordered_multimap<uint8_t const *, OverloadEntry, utf::Mutf8Hash, utf::Mutf8Equal>;

    static std::unique_ptr<EtsClassWrapper> Create(InteropCtx *ctx, EtsClass *etsClass,
                                                   const char *jsBuiltinName = nullptr,
                                                   const OverloadsMap *overloads = nullptr);

    static EtsClassWrapper *Get(InteropCtx *ctx, EtsClass *etsClass);

    static std::unique_ptr<JSRefConvert> CreateJSRefConvertEtsProxy(InteropCtx *ctx, Class *klass);
    static std::unique_ptr<JSRefConvert> CreateJSRefConvertJSProxy(InteropCtx *ctx, Class *klass);
    static std::unique_ptr<JSRefConvert> CreateJSRefConvertEtsInterface(InteropCtx *ctx, Class *klass);

    PandaUnorderedMap<std::string, std::string> &GetOverloadNameMapping()
    {
        return overloadNameMapping_;
    }

    bool IsEtsGlobalClass() const
    {
        return IsEtsGlobalClassName(etsClass_->GetDescriptor());
    }

    EtsClass *GetEtsClass()
    {
        return etsClass_;
    }

    EtsMethodSet *GetMethod(const std::string &name) const;

    napi_value GetJsCtor(napi_env env) const
    {
        return GetReferenceValue(env, jsCtorRef_);
    }

    bool HasBuiltin() const
    {
        return jsBuiltinCtorRef_ != nullptr;
    }

    napi_value GetBuiltin(napi_env env) const
    {
        ASSERT(HasBuiltin());
        return GetReferenceValue(env, jsBuiltinCtorRef_);
    }

    void SetJSBuiltinMatcher(std::function<EtsObject *(InteropCtx *, napi_value, bool)> &&matcher)
    {
        jsBuiltinMatcher_ = std::move(matcher);
    }

    napi_value Wrap(InteropCtx *ctx, EtsObject *etsObject);
    EtsObject *Unwrap(InteropCtx *ctx, napi_value jsValue);

    EtsObject *UnwrapEtsProxy(InteropCtx *ctx, napi_value jsValue);
    EtsObject *CreateJSBuiltinProxy(InteropCtx *ctx, napi_value jsValue);

    ~EtsClassWrapper() = default;

private:
    explicit EtsClassWrapper(EtsClass *etsCls) : etsClass_(etsCls) {}
    NO_COPY_SEMANTIC(EtsClassWrapper);
    NO_MOVE_SEMANTIC(EtsClassWrapper);

    bool SetupHierarchy(InteropCtx *ctx, const char *jsBuiltinName);

    using PropsMethod = std::unordered_map<uint8_t const *, EtsMethodSet *, utf::Mutf8Hash, utf::Mutf8Equal>;
    using PropsField = std::unordered_map<uint8_t const *, Field *, utf::Mutf8Hash, utf::Mutf8Equal>;
    using GetterSetterPropsMap = std::unordered_map<std::string, napi_property_descriptor>;
    using FieldsVec = std::vector<Field *>;
    using MethodsVec = std::vector<EtsMethodSet *>;
    // Mapping between 1.2 proxy method names and their corresponding 1.1 real method names.
    PandaUnorderedMap<std::string, std::string> overloadNameMapping_;

    std::pair<FieldsVec, MethodsVec> CalculateProperties(const OverloadsMap *overloads);
    void SetBaseWrapperMethods(napi_env env, const EtsClassWrapper::MethodsVec &methods);
    void UpdatePropsWithBaseClasses(EtsClassWrapper::PropsMethod *propsMethod, EtsClassWrapper::PropsField *propsField);
    void CollectClassMethods(EtsClassWrapper::PropsMethod *props, const OverloadsMap *overloads);
    bool HasOverloadsMethod(const OverloadsMap *overloads, Method *m);
    std::pair<FieldsVec, MethodsVec> CalculateFieldsAndMethods(const EtsClassWrapper::PropsMethod &propsMethod,
                                                               const EtsClassWrapper::PropsField &propsField);
    std::vector<napi_property_descriptor> BuildJSProperties(napi_env &env, Span<Field *> fields,
                                                            Span<EtsMethodSet *> methods);

    static napi_value GetGlobalSymbolIterator(napi_env &env);

    EtsClassWrapper *LookupBaseWrapper(EtsClass *klass);
    void BuildGetterSetterFieldProperties(GetterSetterPropsMap &propMap, EtsMethodSet *method);
    void SetUpMimicHandler(napi_env env);
    static napi_value CreateProxy(napi_env env, napi_value jsCtor, EtsClassWrapper *thisWrapper);
    static napi_value MimicGetHandler(napi_env env, napi_callback_info info);
    static napi_value MimicSetHandler(napi_env env, napi_callback_info info);

    static napi_value JSCtorCallback(napi_env env, napi_callback_info cinfo);
    bool CreateAndWrap(napi_env env, napi_value jsNewtarget, napi_value jsThis, Span<napi_value> jsArgs);

    Span<EtsFieldWrapper> GetFields()
    {
        return Span<EtsFieldWrapper>(etsFieldWrappers_.get(), numFields_);
    }

    Span<LazyEtsMethodWrapperLink> GetMethods()
    {
        return Span<LazyEtsMethodWrapperLink>(etsMethodWrappers_.get(), numMethods_);
    }

    EtsClass *const etsClass_ {};
    EtsClassWrapper *baseWrapper_ {};

    LazyEtsMethodWrapperLink etsCtorLink_ {};
    napi_ref jsCtorRef_ {};

    // For built-in classes
    napi_ref jsBuiltinCtorRef_ {};
    std::function<EtsObject *(InteropCtx *, napi_value, bool)> jsBuiltinMatcher_;

    js_proxy::JSProxy *jsproxyWrapper_ {};

    // NOTE(vpukhov): allocate inplace to reduce memory consumption
    std::unique_ptr<LazyEtsMethodWrapperLink[]> etsMethodWrappers_;  // NOLINT(modernize-avoid-c-arrays)
    std::unique_ptr<EtsFieldWrapper[]> etsFieldWrappers_;            // NOLINT(modernize-avoid-c-arrays)
    std::vector<std::unique_ptr<EtsMethodSet>> etsMethods_;
    std::vector<std::unique_ptr<EtsFieldWrapper>> getterSetterFieldWrappers_;
    uint32_t numMethods_ {};
    uint32_t numFields_ {};

    bool needProxy_ = false;
    napi_ref jsProxyCtorRef_ {};
    napi_ref jsProxyHandlerRef_ {};

    static constexpr const char *INTERFACE_ITERABLE_NAME = "std.core.IterableIterator";
};

class JSRefConvertJSProxy : public JSRefConvert {
public:
    explicit JSRefConvertJSProxy(EtsClassWrapper *wrapper);
    napi_value WrapImpl(InteropCtx *ctx, EtsObject *etsObject);
    EtsObject *UnwrapImpl(InteropCtx *ctx, napi_value jsValue);

    std::string GetJSMethodName(Method *method);

private:
    const PandaUnorderedMap<std::string, std::string> *overloadNameMapping_ {nullptr};
};

void DoSetPrototype(napi_env env, napi_value obj, napi_value proto);

}  // namespace ark::ets::interop::js::ets_proxy

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_CLASS_WRAPPER_H_
