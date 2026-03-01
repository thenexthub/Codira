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

#include "plugins/ets/runtime/interop_js/ets_proxy/ets_class_wrapper.h"
#include <js_native_api.h>
#include <js_native_api_types.h>
#include <iostream>
#include <ostream>
#include <vector>

#include "plugins/ets/runtime/ets_coroutine.h"
#include "include/mem/panda_containers.h"
#include "interop_js/interop_common.h"
#include "interop_js/js_proxy/js_proxy.h"
#include "interop_js/js_refconvert.h"
#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/shared_reference.h"
#include "plugins/ets/runtime/interop_js/call/call.h"
#include "plugins/ets/runtime/interop_js/code_scopes.h"
#include "runtime/mem/local_object_handle.h"
#include "plugins/ets/runtime/ets_platform_types.h"

// NOLINTBEGIN(readability-identifier-naming, readability-redundant-declaration)
// CC-OFFNXT(G.FMT.10-CPP) project code style
napi_status __attribute__((weak)) napi_get_ets_implements(napi_env env, napi_value jsValue, napi_value *result);
// NOLINTEND(readability-identifier-naming, readability-redundant-declaration)

namespace ark::ets::interop::js::ets_proxy {

class JSRefConvertEtsProxy : public JSRefConvert {
public:
    explicit JSRefConvertEtsProxy(EtsClassWrapper *etsClassWrapper)
        : JSRefConvert(this), etsClassWrapper_(etsClassWrapper)
    {
    }

    napi_value WrapImpl(InteropCtx *ctx, EtsObject *etsObject)
    {
        return etsClassWrapper_->Wrap(ctx, etsObject);
    }
    EtsObject *UnwrapImpl(InteropCtx *ctx, napi_value jsValue)
    {
        return etsClassWrapper_->Unwrap(ctx, jsValue);
    }

private:
    EtsClassWrapper *etsClassWrapper_ {};
};

napi_value EtsClassWrapper::Wrap(InteropCtx *ctx, EtsObject *etsObject)
{
    CheckClassInitialized(etsClass_->GetRuntimeClass());

    napi_env env = ctx->GetJSEnv();

    /**
     * False-positive static-analyzer report:
     * CheckClassInitialized is marked as GC trigger function
     * but with <false> template (which is default) GC won't be triggered
     */
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    ASSERT(etsObject != nullptr);
    // See (CheckClassInitialized) reason
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    ASSERT(etsClass_ == etsObject->GetClass());

    SharedReferenceStorage *storage = ctx->GetSharedRefStorage();
    // See (CheckClassInitialized) reason
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    if (LIKELY(storage->HasReference(etsObject, env))) {
        // See (CheckClassInitialized) reason
        // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
        return storage->GetJsObject(etsObject, env);
    }

    napi_value jsValue;
    // etsObject will be wrapped in jsValue in responce to jsCtor call
    auto *coro = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(coro);
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    // See (CheckClassInitialized) reason
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    EtsHandle<EtsObject> handle(coro, etsObject);
    ctx->SetPendingNewInstance(handle);
    {
        ScopedNativeCodeThread nativeScope(coro);
        NAPI_CHECK_FATAL(napi_new_instance(env, GetJsCtor(env), 0, nullptr, &jsValue));
    }

    // NOTE(MockMockBlack, #IC59ZS): put proxy to SharedReferenceStorage more prettily
    if (this->needProxy_) {
        ASSERT(storage->HasReference(handle.GetPtr(), env));
        return storage->GetJsObject(handle.GetPtr(), env);
    }
    return jsValue;
}

// Use UnwrapEtsProxy if you expect exactly a EtsProxy
EtsObject *EtsClassWrapper::Unwrap(InteropCtx *ctx, napi_value jsValue)
{
    CheckClassInitialized(etsClass_->GetRuntimeClass());

    napi_env env = ctx->GetJSEnv();
    if (IsUndefined<true>(env, jsValue)) {
        InteropCtx::ThrowJSTypeError(env, "Value is undefined");
    }

    // Check if object has SharedReference
    SharedReference *sharedRef = ctx->GetSharedRefStorage()->GetReference(env, jsValue);
    if (LIKELY(sharedRef != nullptr)) {
        EtsObject *etsObject = sharedRef->GetEtsObject();
        if (UNLIKELY(!etsClass_->IsAssignableFrom(etsObject->GetClass()))) {
            ThrowJSErrorNotAssignable(env, etsObject->GetClass(), etsClass_);
            return nullptr;
        }
        return etsObject;
    }

    // Check if object is subtype of js builtin class
    if (LIKELY(HasBuiltin())) {
        ASSERT(jsBuiltinMatcher_ != nullptr);
        auto res = jsBuiltinMatcher_(ctx, jsValue, false);
        if (res == nullptr && !ctx->SanityJSExceptionPending() && !ctx->SanityETSExceptionPending()) {
            InteropCtx::ThrowJSTypeError(env, std::string("Value is not assignable to ") + etsClass_->GetDescriptor());
        }
        return res;
    }

    InteropCtx::ThrowJSTypeError(env, std::string("Value is not assignable to ") + etsClass_->GetDescriptor());
    return nullptr;
}

// Special method to ensure unwrapped object is not a JSProxy
EtsObject *EtsClassWrapper::UnwrapEtsProxy(InteropCtx *ctx, napi_value jsValue)
{
    CheckClassInitialized(etsClass_->GetRuntimeClass());

    napi_env env = ctx->GetJSEnv();

    ASSERT(!IsNullOrUndefined<true>(env, jsValue));

    // Check if object has SharedReference
    SharedReference *sharedRef = ctx->GetSharedRefStorage()->GetReference(env, jsValue);
    if (LIKELY(sharedRef != nullptr)) {
        EtsObject *etsObject = sharedRef->GetEtsObject();
        if (UNLIKELY(!etsClass_->IsAssignableFrom(etsObject->GetClass()))) {
            ThrowJSErrorNotAssignable(env, etsObject->GetClass(), etsClass_);
            return nullptr;
        }
        if (UNLIKELY(!sharedRef->HasETSFlag())) {
            InteropCtx::ThrowJSTypeError(env, std::string("JS object in context of EtsProxy of class ") +
                                                  etsClass_->GetDescriptor());
            return nullptr;
        }
        return etsObject;
    }
    return nullptr;
}

EtsObject *EtsClassWrapper::CreateJSBuiltinProxy(InteropCtx *ctx, napi_value jsValue)
{
    ASSERT(jsproxyWrapper_ != nullptr);
    auto *storage = ctx->GetSharedRefStorage();
    ASSERT(storage->GetReference(ctx->GetJSEnv(), jsValue) == nullptr);

    EtsObject *etsObject = EtsObject::Create(jsproxyWrapper_->GetProxyClass());
    if (UNLIKELY(etsObject == nullptr)) {
        ctx->ForwardEtsException(EtsCoroutine::GetCurrent());
        return nullptr;
    }

    auto coro = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope s(coro);
    EtsHandle<EtsObject> objHandle(coro, etsObject);
    SharedReference *sharedRef = storage->CreateJSObjectRefwithWrap(ctx, objHandle, jsValue);
    if (UNLIKELY(sharedRef == nullptr)) {
        ASSERT(InteropCtx::SanityJSExceptionPending());
        return nullptr;
    }
    return sharedRef->GetEtsObject();  // fetch again after gc
}

/*static*/
std::unique_ptr<JSRefConvert> EtsClassWrapper::CreateJSRefConvertEtsProxy(InteropCtx *ctx, Class *klass)
{
    ASSERT(!klass->IsInterface());
    EtsClass *etsClass = EtsClass::FromRuntimeClass(klass);
    EtsClassWrapper *wrapper = EtsClassWrapper::Get(ctx, etsClass);
    if (UNLIKELY(wrapper == nullptr)) {
        return nullptr;
    }
    ASSERT(wrapper->etsClass_ == etsClass);
    return std::make_unique<JSRefConvertEtsProxy>(wrapper);
}

/*static*/
std::unique_ptr<JSRefConvert> EtsClassWrapper::CreateJSRefConvertJSProxy([[maybe_unused]] InteropCtx *ctx,
                                                                         [[maybe_unused]] Class *klass)
{
    ASSERT(js_proxy::JSProxy::IsProxyClass(klass));
    Class *base = klass->GetBase();
    auto *etsClass = EtsClass::FromRuntimeClass(base);
    auto *wrapper = ctx->GetEtsClassWrappersCache()->Lookup(etsClass);
    if (wrapper == nullptr) {
        return nullptr;
    }
    return std::make_unique<JSRefConvertJSProxy>(wrapper);
}

JSRefConvertJSProxy::JSRefConvertJSProxy(EtsClassWrapper *wrapper)
    : JSRefConvert(this), overloadNameMapping_(&wrapper->GetOverloadNameMapping())
{
}

napi_value JSRefConvertJSProxy::WrapImpl(InteropCtx *ctx, EtsObject *etsObject)
{
    SharedReferenceStorage *storage = ctx->GetSharedRefStorage();
    INTEROP_FATAL_IF(!storage->HasReference(etsObject, ctx->GetJSEnv()));
    return storage->GetJsObject(etsObject, ctx->GetJSEnv());
}

EtsObject *JSRefConvertJSProxy::UnwrapImpl(InteropCtx *ctx, [[maybe_unused]] napi_value jsValue)
{
    ctx->Fatal("Unwrap called on JSProxy class");
    return nullptr;
}

std::string JSRefConvertJSProxy::GetJSMethodName(Method *method)
{
    ASSERT(overloadNameMapping_ != nullptr);
    std::string implName = utf::Mutf8AsCString(method->GetName().data);
    auto iter = overloadNameMapping_->find(implName);
    if (iter != overloadNameMapping_->end()) {
        return iter->second;
    }
    return implName;
}

class JSRefConvertInterface : public JSRefConvert {
public:
    explicit JSRefConvertInterface(Class *klass) : JSRefConvert(this), klass_(klass)
    {
        ASSERT(klass->IsInterface());
    }

    napi_value WrapImpl(InteropCtx *ctx, EtsObject *etsObject)
    {
        auto realConverter = JSRefConvertResolve(ctx, etsObject->GetClass()->GetRuntimeClass());
        if (realConverter == nullptr) {
            InteropFatal("Cannot get ref converter for object");
        }
        return realConverter->Wrap(ctx, etsObject);
    }

    EtsObject *UnwrapImpl(InteropCtx *ctx, napi_value jsValue)
    {
        auto *coro = EtsCoroutine::GetCurrent();
        napi_env env = ctx->GetJSEnv();
        SharedReference *sharedRef = ctx->GetSharedRefStorage()->GetReference(env, jsValue);
        if (LIKELY(sharedRef != nullptr)) {
            EtsObject *etsObject = sharedRef->GetEtsObject();
            return etsObject;
        }
        if (IsStdClass(klass_)) {
            auto *objectConverter =
                ctx->GetEtsClassWrappersCache()->Lookup(EtsClass::FromRuntimeClass(ctx->GetObjectClass()));
            auto *ret = objectConverter->Unwrap(ctx, jsValue);
            ASSERT(ret != nullptr);
            if (!ret->IsInstanceOf(EtsClass::FromRuntimeClass(klass_))) {
                ctx->ThrowJSTypeError(ctx->GetJSEnv(), "object of type " +
                                                           ret->GetClass()->GetRuntimeClass()->GetName() +
                                                           " is not assignable to " + klass_->GetName());
                return nullptr;
            }
            return ret;
        }

        napi_value result;
        NAPI_CHECK_FATAL(napi_get_ets_implements(env, jsValue, &result));
        if (GetValueType<true>(env, result) != napi_string) {
            ctx->ThrowJSTypeError(ctx->GetJSEnv(), std::string("object is not a type of Interface: ") +
                                                       utf::Mutf8AsCString(klass_->GetDescriptor()));
            return nullptr;
        }
        auto interfaceName = GetString(env, result);
        auto proxy = ctx->GetInterfaceProxyInstance(interfaceName);
        if (proxy == nullptr) {
            auto interfaces = GetInterfaceClass(ctx, interfaceName);
            proxy = js_proxy::JSProxy::CreateInterfaceProxy(interfaces, interfaceName);
            ctx->SetInterfaceProxyInstance(interfaceName, proxy);
        }
        EtsHandle<EtsObject> etsObject(coro, EtsObject::Create(proxy->GetProxyClass()));
        if (UNLIKELY(etsObject.GetPtr() == nullptr)) {
            ctx->ThrowJSTypeError(ctx->GetJSEnv(),
                                  "Interface Proxy EtsObject create failed, interfaceList: " + interfaceName);
            return nullptr;
        }
        sharedRef = ctx->GetSharedRefStorage()->CreateHybridObjectRef(ctx, etsObject, jsValue);
        if (UNLIKELY(sharedRef == nullptr)) {
            ASSERT(InteropCtx::SanityJSExceptionPending());
            return nullptr;
        }
        return etsObject.GetPtr();
    }

protected:
    template <typename D>
    explicit JSRefConvertInterface(Class *klass, D *instance) : JSRefConvert(instance), klass_(klass)
    {
        ASSERT(klass->IsInterface());
    }

    PandaSet<Class *> GetInterfaceClass(InteropCtx *ctx, std::string &interfaces)
    {
        PandaSet<Class *> interfaceList;
        std::istringstream iss {interfaces};
        std::string descriptor;
        auto *coro = EtsCoroutine::GetCurrent();
        ASSERT(coro != nullptr);
        while (std::getline(iss, descriptor, ',')) {
            auto interfaceCls =
                coro->GetPandaVM()->GetClassLinker()->GetClass(descriptor.data(), true, ctx->LinkerCtx());
            ASSERT(interfaceCls != nullptr);
            interfaceList.insert(interfaceCls->GetRuntimeClass());
        }
        return interfaceList;
    }

    Class *GetKlass()
    {
        return klass_;
    }

private:
    Class *klass_;
};

class JSRefConvertInterfaceIterator : public JSRefConvertInterface {
public:
    explicit JSRefConvertInterfaceIterator(Class *klass) : JSRefConvertInterface(klass, this)
    {
        ASSERT(klass->IsInterface());
    }

    EtsObject *UnwrapImpl(InteropCtx *ctx, napi_value jsValue)
    {
        auto objectConverter =
            ctx->GetEtsClassWrappersCache()->Lookup(EtsClass::FromRuntimeClass(ctx->GetObjectClass()));
        ASSERT(objectConverter != nullptr);
        auto ret = objectConverter->Unwrap(ctx, jsValue);

        std::array args = {Value {ret->GetCoreType()}};
        auto *method = EtsClass::FromRuntimeClass(ctx->GetJSRuntimeClass())->GetStaticMethod("CreateIterator", nullptr);
        auto *coro = EtsCoroutine::GetCurrent();
        auto resObject = method->GetPandaMethod()->Invoke(coro, args.data());
        ret = EtsObject::FromCoreType(resObject.GetAs<ObjectHeader *>());
        if (!ret->IsInstanceOf(EtsClass::FromRuntimeClass(GetKlass()))) {
            ctx->ThrowJSTypeError(ctx->GetJSEnv(), "object of type " + ret->GetClass()->GetRuntimeClass()->GetName() +
                                                       " is not a assignable to " + GetKlass()->GetName());
            return nullptr;
        }
        return ret;
    }
};

/*static*/
std::unique_ptr<JSRefConvert> EtsClassWrapper::CreateJSRefConvertEtsInterface([[maybe_unused]] InteropCtx *ctx,
                                                                              Class *klass)
{
    if (klass->GetName() == INTERFACE_ITERABLE_NAME) {
        return std::make_unique<JSRefConvertInterfaceIterator>(klass);
    }
    return std::make_unique<JSRefConvertInterface>(klass);
}

EtsMethodSet *EtsClassWrapper::GetMethod(const std::string &name) const
{
    for (const auto &item : etsMethods_) {
        if (name == item->GetName()) {
            return item.get();
        }
    }
    return nullptr;
}

/*static*/
EtsClassWrapper *EtsClassWrapper::Get(InteropCtx *ctx, EtsClass *etsClass)
{
    EtsClassWrappersCache *cache = ctx->GetEtsClassWrappersCache();

    ASSERT(etsClass != nullptr);
    EtsClassWrapper *etsClassWrapper = cache->Lookup(etsClass);
    if (LIKELY(etsClassWrapper != nullptr)) {
        return etsClassWrapper;
    }

    ASSERT(!etsClass->IsPrimitive() && etsClass->GetComponentType() == nullptr);
    ASSERT(ctx->GetRefConvertCache()->Lookup(etsClass->GetRuntimeClass()) == nullptr);

    if (IsStdClass(etsClass) && !etsClass->IsInterface() && !etsClass->IsEtsEnum() &&
        !IsSubClassOfError(etsClass)) {  // NOTE(gogabr): temporary ugly workaround for Function... interfaces
        return nullptr;
    }
    ASSERT(!js_proxy::JSProxy::IsProxyClass((etsClass->GetRuntimeClass())));

    std::unique_ptr<EtsClassWrapper> wrapper = EtsClassWrapper::Create(ctx, etsClass);
    if (UNLIKELY(wrapper == nullptr)) {
        return nullptr;
    }
    return cache->Insert(etsClass, std::move(wrapper));
}

bool EtsClassWrapper::SetupHierarchy(InteropCtx *ctx, const char *jsBuiltinName)
{
    ASSERT(etsClass_->GetBase() != etsClass_);
    if (etsClass_->GetBase() != nullptr) {
        baseWrapper_ = EtsClassWrapper::Get(ctx, etsClass_->GetBase());
        if (baseWrapper_ == nullptr) {
            return false;
        }
    }

    if (jsBuiltinName != nullptr && std::string(jsBuiltinName) != "Object") {
        auto env = ctx->GetJSEnv();
        napi_value jsBuiltinCtor;
        NAPI_CHECK_FATAL(napi_get_named_property(env, GetGlobal(env), jsBuiltinName, &jsBuiltinCtor));
        NAPI_CHECK_FATAL(napi_create_reference(env, jsBuiltinCtor, 1, &jsBuiltinCtorRef_));
    }
    return true;
}

void EtsClassWrapper::SetBaseWrapperMethods(napi_env env, const EtsClassWrapper::MethodsVec &methods)
{
    for (auto method : methods) {
        const std::string methodName(method->GetName());
        if (methodName == GET_INDEX_METHOD || methodName == SET_INDEX_METHOD) {
            SetUpMimicHandler(env);
        }
        auto baseClassWrapper = baseWrapper_;
        while (nullptr != baseClassWrapper) {
            EtsMethodSet *baseMethodSet = baseClassWrapper->GetMethod(methodName);
            if (nullptr != baseMethodSet) {
                method->SetBaseMethodSet(baseMethodSet);
                break;
            }
            baseClassWrapper = baseClassWrapper->baseWrapper_;
        }
    }
}

std::pair<EtsClassWrapper::FieldsVec, EtsClassWrapper::MethodsVec> EtsClassWrapper::CalculateProperties(
    const OverloadsMap *overloads)
{
    auto fatalNoMethod = [](Class *klass, const char *methodName, const char *signature) {
        InteropCtx::Fatal(std::string("No method ") + methodName + " " + signature + " in " + klass->GetName());
    };

    auto klass = etsClass_->GetRuntimeClass();
    PropsMethod propsMethod;
    PropsField propsField;

    // Collect fields
    for (auto &f : klass->GetFields()) {
        if (f.IsPublic()) {
            propsField.insert({f.GetName().data, &f});
        }
    }
    // Select preferred overloads
    if (overloads != nullptr) {
        for (const auto &[jsMethodName, entry] : *overloads) {
            const char *implName = entry.implMethodName;
            const char *signature = entry.signature;

            Method *method = etsClass_->GetDirectMethod(utf::CStringAsMutf8(implName), signature)->GetPandaMethod();
            if (UNLIKELY(method == nullptr)) {
                fatalNoMethod(klass, implName, signature);
            }
            auto jsNameStr = utf::Mutf8AsCString(jsMethodName);
            auto etsMethodSet =
                std::make_unique<EtsMethodSet>(EtsMethodSet::Create(EtsMethod::FromRuntimeMethod(method), jsNameStr));
            auto it = propsMethod.insert({method->GetName().data, etsMethodSet.get()});
            if (!it.second) {
                // Possible correct method overloading: merge to existing entry
                auto addedMethods = it.first->second;
                addedMethods->MergeWith(*etsMethodSet.get());
            } else {
                etsMethods_.push_back(std::move(etsMethodSet));
            }
        }
    }

    CollectClassMethods(&propsMethod, overloads);
    if (etsClass_ != PlatformTypes()->coreObject) {
        UpdatePropsWithBaseClasses(&propsMethod, &propsField);
    }

    return CalculateFieldsAndMethods(propsMethod, propsField);
}

void EtsClassWrapper::CollectClassMethods(EtsClassWrapper::PropsMethod *props, const OverloadsMap *overloads)
{
    auto klass = etsClass_->GetRuntimeClass();
    for (auto &m : klass->GetMethods()) {
        if (m.IsPrivate()) {
            continue;
        }
        if (overloads != nullptr) {
            if (HasOverloadsMethod(overloads, &m)) {
                continue;
            }
        }
        auto methodSet = std::make_unique<EtsMethodSet>(EtsMethodSet::Create(&m));
        auto it = props->insert({m.GetName().data, methodSet.get()});
        if (!it.second) {
            // Possible correct method overloading: merge to existing entry
            auto addedMethods = it.first->second;
            addedMethods->MergeWith(*methodSet.get());
        } else {
            etsMethods_.push_back(std::move(methodSet));
        }
    }
}

bool EtsClassWrapper::HasOverloadsMethod(const OverloadsMap *overloads, Method *m)
{
    EtsMethod *etsMethod = EtsMethod::FromRuntimeMethod(m);
    const char *methodName = utf::Mutf8AsCString(m->GetName().data);
    uint32_t argCount = etsMethod->GetNumArgs() - static_cast<unsigned int>(etsMethod->GetPandaMethod()->HasVarArgs());

    for (const auto &[jsMethodName, entry] : *overloads) {
        if (strcmp(entry.implMethodName, methodName) == 0 && entry.argCount == argCount) {
            return true;
        }
    }
    return false;
}

void EtsClassWrapper::UpdatePropsWithBaseClasses(EtsClassWrapper::PropsMethod *propsMethod,
                                                 EtsClassWrapper::PropsField *propsField)
{
    auto hasSquashedProtoOhosImpl = [](EtsClassWrapper *wclass) {
        ASSERT(wclass->HasBuiltin() || wclass->baseWrapper_ != nullptr);
        return wclass->HasBuiltin() || wclass->baseWrapper_->HasBuiltin();
    };

    auto hasSquashedProtoOtherImpl = [](EtsClassWrapper *wclass) {
        // NOTE(vpukhov): some napi implementations add explicit receiver checks in call handler,
        //                thus method inheritance via prototype chain wont work
        (void)wclass;
        return true;
    };

#if defined(PANDA_TARGET_OHOS) || defined(PANDA_JS_ETS_HYBRID_MODE)
    auto hasSquashedProto = hasSquashedProtoOhosImpl;
    (void)hasSquashedProtoOtherImpl;
#else
    auto hasSquashedProto = hasSquashedProtoOtherImpl;
    (void)hasSquashedProtoOhosImpl;
#endif

    if (hasSquashedProto(this)) {
        // Copy properties of base classes if we have to split prototype chain
        for (auto wclass = baseWrapper_; wclass != nullptr && (wclass->etsClass_ != PlatformTypes()->coreObject);
             wclass = wclass->baseWrapper_) {
            for (auto &wfield : wclass->GetFields()) {
                Field *field = wfield.GetField();
                propsField->insert({field->GetName().data, field});
            }
            for (auto &link : wclass->GetMethods()) {
                EtsMethodSet *methodSet = link.IsResolved() ? link.GetResolved()->GetMethodSet() : link.GetUnresolved();
                propsMethod->insert({utf::CStringAsMutf8(methodSet->GetName()), methodSet});
            }

            if (hasSquashedProto(wclass)) {
                break;
            }
        }
    }
}

std::pair<EtsClassWrapper::FieldsVec, EtsClassWrapper::MethodsVec> EtsClassWrapper::CalculateFieldsAndMethods(
    const EtsClassWrapper::PropsMethod &propsMethod, const EtsClassWrapper::PropsField &propsField)
{
    std::vector<EtsMethodSet *> methods;
    std::vector<Field *> fields;

    for (auto &[n, p] : propsMethod) {
        if (p->IsConstructor() && !p->IsStatic()) {
            etsCtorLink_ = LazyEtsMethodWrapperLink(p);
        }
        if (!p->IsConstructor()) {
            methods.push_back(p);
        }
    }

    for (auto &[n, p] : propsField) {
        fields.push_back(p);
    }

    return {fields, methods};
}

std::vector<napi_property_descriptor> EtsClassWrapper::BuildJSProperties(napi_env &env, Span<Field *> fields,
                                                                         Span<EtsMethodSet *> methods)
{
    std::vector<napi_property_descriptor> jsProps;
    jsProps.reserve(fields.size() + methods.size() + 1);

    // Process fields
    numFields_ = fields.size();
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    etsFieldWrappers_ = std::make_unique<EtsFieldWrapper[]>(numFields_);
    Span<EtsFieldWrapper> etsFieldWrappers(etsFieldWrappers_.get(), numFields_);
    size_t fieldIdx = 0;

    for (Field *field : fields) {
        auto wfield = &etsFieldWrappers[fieldIdx++];
        if (field->IsStatic()) {
            EtsClassWrapper *fieldWclass = LookupBaseWrapper(EtsClass::FromRuntimeClass(field->GetClass()));
            ASSERT(fieldWclass != nullptr);
            jsProps.emplace_back(wfield->MakeStaticProperty(fieldWclass, field));
        } else {
            jsProps.emplace_back(wfield->MakeInstanceProperty(this, field));
        }
    }

    // Process methods
    numMethods_ = methods.size();
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    etsMethodWrappers_ = std::make_unique<LazyEtsMethodWrapperLink[]>(numMethods_);
    Span<LazyEtsMethodWrapperLink> etsMethodWrappers(etsMethodWrappers_.get(), numMethods_);
    size_t methodIdx = 0;
    GetterSetterPropsMap propMap;
    for (auto &method : methods) {
        ASSERT(!method->IsConstructor());
        auto lazyLink = &etsMethodWrappers[methodIdx++];
        lazyLink->Set(method);
        jsProps.emplace_back(EtsMethodWrapper::MakeNapiProperty(method, lazyLink));
        if (strcmp(method->GetName(), ITERATOR_METHOD) == 0) {
            auto iterator = EtsClassWrapper::GetGlobalSymbolIterator(env);
            jsProps.emplace_back(EtsMethodWrapper::MakeNapiIteratorProperty(iterator, method, lazyLink));
        }
        if (method->IsGetter() || method->IsSetter()) {
            BuildGetterSetterFieldProperties(propMap, method);
        }
    }
    jsProps.reserve(jsProps.size() + propMap.size());
    for (auto &item : propMap) {
        jsProps.emplace_back(item.second);
    }

    return jsProps;
}

void EtsClassWrapper::BuildGetterSetterFieldProperties(GetterSetterPropsMap &propMap, EtsMethodSet *method)
{
    auto ptr = reinterpret_cast<uintptr_t>(method->GetName());
    const char *fieldName = reinterpret_cast<char *>(ptr + strlen(SETTER_BEGIN));
    const std::string key(fieldName);
    auto result = propMap.find(key);
    if (result != propMap.end()) {
        EtsMethodWrapper::AttachGetterSetterToProperty(method, result->second);
    } else {
        napi_property_descriptor prop {};
        auto fieldWrapper = std::make_unique<EtsFieldWrapper>(this);
        prop.utf8name = fieldName;
        prop.attributes = method->IsStatic() ? EtsClassWrapper::STATIC_FIELD_ATTR : EtsClassWrapper::FIELD_ATTR;
        prop.data = fieldWrapper.get();
        EtsMethodWrapper::AttachGetterSetterToProperty(method, prop);
        propMap.insert({key, prop});
        getterSetterFieldWrappers_.push_back(std::move(fieldWrapper));
    }
}

/* static */
napi_value EtsClassWrapper::GetGlobalSymbolIterator(napi_env &env)
{
    napi_value global;
    napi_value symbol;
    napi_value iterator;
    NAPI_CHECK_FATAL(napi_get_global(env, &global));
    NAPI_CHECK_FATAL(napi_get_named_property(env, global, "Symbol", &symbol));
    NAPI_CHECK_FATAL(napi_get_named_property(env, symbol, "iterator", &iterator));

    return iterator;
}

EtsClassWrapper *EtsClassWrapper::LookupBaseWrapper(EtsClass *klass)
{
    for (auto wclass = this; wclass != nullptr; wclass = wclass->baseWrapper_) {
        if (wclass->etsClass_ == klass) {
            return wclass;
        }
    }
    return nullptr;
}

void DoSetPrototype(napi_env env, napi_value obj, napi_value proto)
{
    napi_value builtinObject;
    napi_value setprotoFn;
    NAPI_CHECK_FATAL(napi_get_named_property(env, GetGlobal(env), "Object", &builtinObject));
    NAPI_CHECK_FATAL(napi_get_named_property(env, builtinObject, "setPrototypeOf", &setprotoFn));

    std::array args = {obj, proto};
    NAPI_CHECK_FATAL(NapiCallFunction(env, builtinObject, setprotoFn, args.size(), args.data(), nullptr));
}

static void SetNullPrototype(napi_env env, napi_value jsCtor)
{
    napi_value prot;
    NAPI_CHECK_FATAL(napi_get_named_property(env, jsCtor, "prototype", &prot));

    napi_value trueValue = GetBooleanValue(env, true);
    NAPI_CHECK_FATAL(napi_set_named_property(env, jsCtor, IS_STATIC_PROXY.data(), trueValue));
    NAPI_CHECK_FATAL(napi_set_named_property(env, prot, IS_STATIC_PROXY.data(), trueValue));

    auto nullProto = GetNull(env);
    DoSetPrototype(env, jsCtor, nullProto);
    DoSetPrototype(env, prot, nullProto);
}

static void SimulateJSInheritance(napi_env env, napi_value jsCtor, napi_value jsBaseCtor)
{
    napi_value cprototype;
    napi_value baseCprototype;
    NAPI_CHECK_FATAL(napi_get_named_property(env, jsCtor, "prototype", &cprototype));
    NAPI_CHECK_FATAL(napi_get_named_property(env, jsBaseCtor, "prototype", &baseCprototype));

    DoSetPrototype(env, jsCtor, jsBaseCtor);
    DoSetPrototype(env, cprototype, baseCprototype);

    napi_value trueValue = GetBooleanValue(env, true);
    NAPI_CHECK_FATAL(napi_set_named_property(env, jsCtor, IS_STATIC_PROXY.data(), trueValue));
    NAPI_CHECK_FATAL(napi_set_named_property(env, cprototype, IS_STATIC_PROXY.data(), trueValue));
}

static napi_value AttachCBForClass([[maybe_unused]] napi_env env, void *data)
{
    auto ctx = InteropCtx::Current();
    auto *etsClass = reinterpret_cast<EtsClass *>(data);
    EtsClassWrapper *wrapper = EtsClassWrapper::Get(ctx, etsClass);
    return wrapper->GetJsCtor(ctx->GetJSEnv());
}

static napi_value AttachCBForFunc([[maybe_unused]] napi_env env, void *data)
{
    auto ctx = InteropCtx::Current();
    auto curEnv = ctx->GetJSEnv();
    auto *method = reinterpret_cast<EtsMethodSet *>(data);
    EtsClassWrapper *wrapper = EtsClassWrapper::Get(ctx, method->GetEnclosingClass());
    napi_value methodFunc;
    NAPI_CHECK_FATAL(napi_get_named_property(curEnv, wrapper->GetJsCtor(curEnv), method->GetName(), &methodFunc));
    return methodFunc;
}

static void SetAttachCallbackForClass(napi_env env, napi_value jsCtor, std::vector<EtsMethodSet *> &methods,
                                      EtsClass *etsClass)
{
    for (auto method : methods) {
        if (method->IsStatic()) {
            napi_value func;
            NAPI_CHECK_FATAL(napi_get_named_property(env, jsCtor, method->GetName(), &func));
            NAPI_CHECK_FATAL(napi_mark_attach_with_xref(env, func, static_cast<void *>(method), AttachCBForFunc));
        }
    }
    NAPI_CHECK_FATAL(napi_mark_attach_with_xref(env, jsCtor, static_cast<void *>(etsClass), AttachCBForClass));
}

static void SortMethodByName(PandaVector<Method *> &methods)
{
    std::sort(methods.begin(), methods.end(),
              [](const Method *lhs, const Method *rhs) { return lhs->GetName() < rhs->GetName(); });
}

/*static*/
std::unique_ptr<EtsClassWrapper> EtsClassWrapper::Create(InteropCtx *ctx, EtsClass *etsClass, const char *jsBuiltinName,
                                                         const OverloadsMap *overloads)
{
    auto env = ctx->GetJSEnv();

    // CC-OFFNXT(G.RES.09) private constructor
    // NOLINTNEXTLINE(readability-identifier-naming)
    auto _this = std::unique_ptr<EtsClassWrapper>(new EtsClassWrapper(etsClass));
    if (!_this->SetupHierarchy(ctx, jsBuiltinName)) {
        return nullptr;
    }

    auto [fields, methods] = _this->CalculateProperties(overloads);

    _this->SetBaseWrapperMethods(env, methods);

    auto jsProps = _this->BuildJSProperties(env, {fields.data(), fields.size()}, {methods.data(), methods.size()});

    // NOTE(vpukhov): fatal no-public-fields check when escompat adopt accessors
    if (_this->HasBuiltin() && !fields.empty()) {
        INTEROP_LOG(ERROR) << "built-in class " << etsClass->GetDescriptor() << " has field properties";
    }
    if (_this->HasBuiltin() && etsClass->IsFinal()) {
        INTEROP_LOG(FATAL) << "built-in class " << etsClass->GetDescriptor() << " is final";
    }
    // NOTE(vpukhov): forbid "true" ets-field overriding in js-derived class, as it cannot be proxied back
    if (!etsClass->IsFinal()) {
        auto allEtsMethod = etsClass->GetMethods();
        auto ungroupedMethods = CollectAllPandaMethods(allEtsMethod.begin(), allEtsMethod.end());
        SortMethodByName(ungroupedMethods);

        _this->jsproxyWrapper_ = ctx->GetJsProxyInstance(etsClass);
        if (_this->jsproxyWrapper_ == nullptr) {
            // NOTE(konstanting): we assume that the method list stays the same for every proxied class
            auto *proxyWrapper =
                js_proxy::JSProxy::CreateBuiltinProxy(etsClass, {ungroupedMethods.data(), ungroupedMethods.size()});
            if (proxyWrapper == nullptr) {
                return nullptr;
            }
            _this->jsproxyWrapper_ = proxyWrapper;
            ctx->SetJsProxyInstance(etsClass, _this->jsproxyWrapper_);
        }
    }
    napi_value jsCtor {};
    NAPI_CHECK_FATAL(napi_define_class(env, etsClass->GetDescriptor(), NAPI_AUTO_LENGTH,
                                       EtsClassWrapper::JSCtorCallback, _this.get(), jsProps.size(), jsProps.data(),
                                       &jsCtor));

    if (etsClass == PlatformTypes()->coreObject) {
        SetNullPrototype(env, jsCtor);

        NAPI_CHECK_FATAL(napi_create_reference(env, jsCtor, 1, &_this->jsCtorRef_));
        NAPI_CHECK_FATAL(napi_create_reference(env, jsCtor, 1, &_this->jsBuiltinCtorRef_));
        NAPI_CHECK_FATAL(napi_object_seal(env, jsCtor));

        return _this;
    }

    SetAttachCallbackForClass(env, jsCtor, methods, etsClass);

    auto base = _this->baseWrapper_;
    napi_value fakeSuper = _this->HasBuiltin() ? _this->GetBuiltin(env)
                                               : (base->HasBuiltin() ? base->GetBuiltin(env) : base->GetJsCtor(env));

    SimulateJSInheritance(env, jsCtor, fakeSuper);
    NAPI_CHECK_FATAL(napi_object_seal(env, jsCtor));
    NAPI_CHECK_FATAL(napi_create_reference(env, jsCtor, 1, &_this->jsCtorRef_));

    return _this;
}

void EtsClassWrapper::SetUpMimicHandler(napi_env env)
{
    this->needProxy_ = true;

    napi_value global;
    NAPI_CHECK_FATAL(napi_get_global(env, &global));

    napi_value jsProxyCtorRef;
    NAPI_CHECK_FATAL(napi_get_named_property(env, global, "Proxy", &jsProxyCtorRef));
    NAPI_CHECK_FATAL(napi_create_reference(env, jsProxyCtorRef, 1, &this->jsProxyCtorRef_));

    napi_value jsProxyHandlerRef;
    NAPI_CHECK_FATAL(napi_create_object(env, &jsProxyHandlerRef));
    NAPI_CHECK_FATAL(napi_create_reference(env, jsProxyHandlerRef, 1, &this->jsProxyHandlerRef_));

    napi_value getHandlerFunc;
    NAPI_CHECK_FATAL(napi_create_function(env, nullptr, NAPI_AUTO_LENGTH, EtsClassWrapper::MimicGetHandler, nullptr,
                                          &getHandlerFunc));
    napi_value setHandlerFunc;
    NAPI_CHECK_FATAL(napi_create_function(env, nullptr, NAPI_AUTO_LENGTH, EtsClassWrapper::MimicSetHandler, nullptr,
                                          &setHandlerFunc));

    NAPI_CHECK_FATAL(napi_set_named_property(env, jsProxyHandlerRef, "get", getHandlerFunc));
    NAPI_CHECK_FATAL(napi_set_named_property(env, jsProxyHandlerRef, "set", setHandlerFunc));
}

/*static*/
napi_value EtsClassWrapper::CreateProxy(napi_env env, napi_value jsCtor, EtsClassWrapper *thisWrapper)
{
    napi_value jsProxyHandlerRef;
    NAPI_CHECK_FATAL(napi_get_reference_value(env, thisWrapper->jsProxyHandlerRef_, &jsProxyHandlerRef));
    napi_value jsProxyCtorRef;
    NAPI_CHECK_FATAL(napi_get_reference_value(env, thisWrapper->jsProxyCtorRef_, &jsProxyCtorRef));

    napi_value proxyObj;
    std::vector<napi_value> args = {jsCtor, jsProxyHandlerRef};

    NAPI_CHECK_FATAL(napi_new_instance(env, jsProxyCtorRef, args.size(), args.data(), &proxyObj));
    return proxyObj;
}

/*static*/
napi_value EtsClassWrapper::MimicGetHandler(napi_env env, napi_callback_info info)
{
    INTEROP_TRACE();
    ASSERT_SCOPED_NATIVE_CODE();
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_JS_TO_ETS(coro);

    size_t argc;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &argc, nullptr, nullptr, nullptr));
    auto jsArgs = ctx->GetTempArgs<napi_value>(argc);
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &argc, jsArgs->data(), nullptr, nullptr));

    napi_value target = jsArgs[0];
    napi_value property = jsArgs[1];

    if (GetValueType(env, property) == napi_number) {
        ScopedManagedCodeThread managedCode(coro);
        ets_proxy::SharedReferenceStorage *storage = ctx->GetSharedRefStorage();
        ASSERT(storage != nullptr);
        ets_proxy::SharedReference *sharedRef = storage->GetReference(env, jsArgs[0]);
        if (sharedRef == nullptr) {
            InteropFatal("MimicGetHandler sharedRef is empty");
        }

        auto *etsThis = sharedRef->GetEtsObject();
        ASSERT(etsThis != nullptr);
        EtsMethod *method = etsThis->GetClass()->GetInstanceMethod(GET_INDEX_METHOD, nullptr);
        ASSERT(method != nullptr);

        Span sp(jsArgs->begin(), jsArgs->end());
        const size_t startIndex = 1;
        const size_t argSize = 1;
        return CallETSInstance(coro, ctx, method->GetPandaMethod(), sp.SubSpan(startIndex, argSize), etsThis);
    }

    napi_value result;
    NAPI_CHECK_FATAL(napi_get_property(env, target, property, &result));
    return result;
}

/*static*/
napi_value EtsClassWrapper::MimicSetHandler(napi_env env, napi_callback_info info)
{
    INTEROP_TRACE();
    ASSERT_SCOPED_NATIVE_CODE();
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_JS_TO_ETS(coro);

    size_t argc;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &argc, nullptr, nullptr, nullptr));
    auto jsArgs = ctx->GetTempArgs<napi_value>(argc);
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &argc, jsArgs->data(), nullptr, nullptr));

    ScopedManagedCodeThread managedCode(coro);
    ets_proxy::SharedReferenceStorage *storage = ctx->GetSharedRefStorage();
    ASSERT(storage != nullptr);
    ets_proxy::SharedReference *sharedRef = storage->GetReference(env, jsArgs[0]);
    if (sharedRef == nullptr) {
        InteropFatal("MimicSetHandler sharedRef is empty");
    }

    auto *etsThis = sharedRef->GetEtsObject();
    ASSERT(etsThis != nullptr);
    EtsMethod *method = etsThis->GetClass()->GetInstanceMethod(SET_INDEX_METHOD, nullptr);
    ASSERT(method != nullptr);

    Span sp(jsArgs->begin(), jsArgs->end());

    const size_t startIndex = 1;
    const size_t argSize = 2;
    CallETSInstance(coro, ctx, method->GetPandaMethod(), sp.SubSpan(startIndex, argSize), etsThis);

    napi_value trueValue;
    NAPI_CHECK_FATAL(napi_get_boolean(env, true, &trueValue));
    return trueValue;
}

/*static*/
napi_value EtsClassWrapper::JSCtorCallback(napi_env env, napi_callback_info cinfo)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    InteropCtx *ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_JS_TO_ETS(coro);

    napi_value jsThis;
    size_t argc;
    void *data;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, cinfo, &argc, nullptr, &jsThis, &data));
    auto etsClassWrapper = reinterpret_cast<EtsClassWrapper *>(data);

    ScopedManagedCodeThread managedScope(coro);
    EtsObject *etsObject = ctx->AcquirePendingNewInstance();

    if (LIKELY(etsObject != nullptr)) {
        // proxy $get and $set
        napi_value jsObject = nullptr;
        if (etsClassWrapper->needProxy_) {
            jsObject = CreateProxy(env, jsThis, etsClassWrapper);
        } else {
            jsObject = jsThis;
        }

        [[maybe_unused]] EtsHandleScope s(coro);
        EtsHandle<EtsObject> objHandle(coro, etsObject);
        // Create shared reference for existing ets object
        SharedReferenceStorage *storage = ctx->GetSharedRefStorage();
        if (UNLIKELY(!storage->CreateETSObjectRef(ctx, objHandle, jsObject))) {
            ASSERT(InteropCtx::SanityJSExceptionPending());
            return nullptr;
        }
        NAPI_CHECK_FATAL(napi_object_seal(env, jsThis));
        return nullptr;
    }

    if (!etsClassWrapper->etsCtorLink_.IsResolved() && etsClassWrapper->etsCtorLink_.GetUnresolved() == nullptr) {
        InteropCtx::ThrowJSError(env,
                                 etsClassWrapper->GetEtsClass()->GetRuntimeClass()->GetName() + " has no constructor");
        return nullptr;
    }

    auto jsArgs = ctx->GetTempArgs<napi_value>(argc);
    NAPI_CHECK_FATAL(napi_get_cb_info(env, cinfo, &argc, jsArgs->data(), nullptr, nullptr));

    napi_value jsNewtarget;
    NAPI_CHECK_FATAL(napi_get_new_target(env, cinfo, &jsNewtarget));

    // create new object and wrap it
    if (UNLIKELY(!etsClassWrapper->CreateAndWrap(env, jsNewtarget, jsThis, *jsArgs))) {
        ASSERT(InteropCtx::SanityJSExceptionPending());
        return nullptr;
    }

    // NOTE(ivagin): JS constructor is not required to return 'this', but ArkUI NAPI requires it
    return jsThis;
}

bool EtsClassWrapper::CreateAndWrap(napi_env env, napi_value jsNewtarget, napi_value jsThis, Span<napi_value> jsArgs)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    InteropCtx *ctx = InteropCtx::Current(coro);

    if (UNLIKELY(!CheckClassInitialized<true>(etsClass_->GetRuntimeClass()))) {
        ctx->ForwardEtsException(coro);
        return false;
    }

    bool notExtensible;
    NAPI_CHECK_FATAL(napi_strict_equals(env, jsNewtarget, GetJsCtor(env), &notExtensible));

    EtsClass *instanceClass {};

    if (LIKELY(notExtensible)) {
#if !defined(PANDA_TARGET_OHOS) && !defined(PANDA_JS_ETS_HYBRID_MODE)
        // In case of OHOS sealed object can't be wrapped, therefore seal it after wrapping
        NAPI_CHECK_FATAL(napi_object_seal(env, jsThis));
#endif  // PANDA_TARGET_OHOS
        instanceClass = etsClass_;
    } else {
        if (UNLIKELY(jsproxyWrapper_ == nullptr)) {
            ctx->ThrowJSTypeError(env, std::string("Proxy for ") + etsClass_->GetDescriptor() + " is not extensible");
            return false;
        }
        instanceClass = jsproxyWrapper_->GetProxyClass();
    }

    [[maybe_unused]] EtsHandleScope s(coro);
    EtsHandle<EtsObject> etsObject(coro, EtsObject::Create(instanceClass));
    if (UNLIKELY(etsObject.GetPtr() == nullptr)) {
        return false;
    }

    // NOTE(MockMockBlack, #IC59ZS): put proxy to SharedReferenceStorage more prettily
    SharedReference *sharedRef;
    if (LIKELY(notExtensible)) {
        sharedRef = ctx->GetSharedRefStorage()->CreateETSObjectRef(ctx, etsObject, jsThis);
#if defined(PANDA_TARGET_OHOS) || defined(PANDA_JS_ETS_HYBRID_MODE)
        // In case of OHOS sealed object can't be wrapped, therefore seal it after wrapping
        NAPI_CHECK_FATAL(napi_object_seal(env, jsThis));
#endif  // PANDA_TARGET_OHOS
    } else {
        sharedRef = ctx->GetSharedRefStorage()->CreateHybridObjectRef(ctx, etsObject, jsThis);
    }
    if (UNLIKELY(sharedRef == nullptr)) {
        ASSERT(InteropCtx::SanityJSExceptionPending());
        return false;
    }

    EtsMethodWrapper *ctorWrapper = EtsMethodWrapper::ResolveLazyLink(ctx, etsCtorLink_);
    ASSERT(ctorWrapper != nullptr);
    EtsMethod *ctorMethod = ctorWrapper->GetEtsMethod(jsArgs.Size());
    ASSERT(ctorMethod->IsInstanceConstructor());

    napi_value callRes = CallETSInstance(coro, ctx, ctorMethod->GetPandaMethod(), jsArgs, etsObject.GetPtr());
    return callRes != nullptr;
}

}  // namespace ark::ets::interop::js::ets_proxy
