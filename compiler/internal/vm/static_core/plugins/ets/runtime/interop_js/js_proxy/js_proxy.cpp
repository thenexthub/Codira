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

#include "include/class.h"
#include "include/mem/panda_containers.h"
#include "include/method.h"
#include "plugins/ets/runtime/interop_js/js_proxy/js_proxy.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"

#include "plugins/ets/runtime/types/ets_object.h"

namespace ark::ets::interop::js::js_proxy {

extern "C" void CallJSProxyBridge(Method *method, ...);
extern "C" void CallJSFunctionBridge(Method *method, ...);

using MethodMap = std::unordered_map<uint8_t const *, Method *, utf::Mutf8Hash, utf::Mutf8Equal>;

// Create JSProxy class descriptor that will respond to IsProxyClass
// NOLINTNEXTLINE(modernize-avoid-c-arrays)
static std::unique_ptr<uint8_t[]> MakeProxyDescriptor(const uint8_t *descriptorP)
{
    Span<const uint8_t> descriptor(descriptorP, utf::Mutf8Size(descriptorP));

    ASSERT(descriptor.size() > 2U);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    ASSERT(descriptor[0] == 'L');
    ASSERT(descriptor[descriptor.size() - 1] == ';');

    size_t proxyDescriptorSize = descriptor.size() + 3U;  // + $$\0
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    auto proxyDescriptorData = std::make_unique<uint8_t[]>(proxyDescriptorSize);
    Span<uint8_t> proxyDescriptor(proxyDescriptorData.get(), proxyDescriptorSize);

    proxyDescriptor[0] = 'L';
    proxyDescriptor[1] = '$';
    std::copy_n(&descriptor[1], descriptor.size() - 2U, &proxyDescriptor[2U]);
    proxyDescriptor[proxyDescriptor.size() - 3U] = '$';
    proxyDescriptor[proxyDescriptor.size() - 2U] = ';';
    proxyDescriptor[proxyDescriptor.size() - 1U] = '\0';

    return proxyDescriptorData;
}

static void GetAllInterfaceMethod(Class *interfaceCls, PandaVector<Method *> &methodPtrs, PandaSet<Class *> &methodPSet)
{
    if (methodPSet.count(interfaceCls) != 0) {
        return;
    }
    methodPSet.insert(interfaceCls);
    auto methods = interfaceCls->GetMethods();
    for (auto &method : methods) {
        methodPtrs.push_back(&method);
    }
    for (auto *itf : interfaceCls->GetInterfaces()) {
        GetAllInterfaceMethod(itf, methodPtrs, methodPSet);
    }
}

static void GetInterfaceMethodDistinct(Class *interfaceCls, MethodMap &methodMap, PandaSet<Class *> &interfaceSet)
{
    if (interfaceSet.count(interfaceCls) != 0) {
        return;
    }
    interfaceSet.insert(interfaceCls);
    auto methods = interfaceCls->GetMethods();
    for (auto &method : methods) {
        methodMap.insert({method.GetName().data, &method});
    }
    for (auto *itf : interfaceCls->GetInterfaces()) {
        GetInterfaceMethodDistinct(itf, methodMap, interfaceSet);
    }
}

static void InitProxyMethod(Class *cls, Method *src, Method *proxy, void *entryPoint)
{
    new (proxy) Method(src);

    proxy->SetAccessFlags((src->GetAccessFlags() & ~(ACC_ABSTRACT | ACC_DEFAULT_INTERFACE_METHOD)) | ACC_FINAL);
    proxy->SetClass(cls);
    proxy->SetCompiledEntryPoint(entryPoint);
}

/*
 * BuildProxyMethods
 */
static Span<Method> BuildProxyMethods(Class *cls, Span<Method *> targetMethods, void *entryPoint)
{
    ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();

    PandaVector<size_t> targetMethodsIdx;

    for (size_t i = 0; i < targetMethods.size(); ++i) {
        auto m = targetMethods[i];
        if (!m->IsFinal()) {  // NOTE(vpukhov): consider internal methods, final methods in builtins
            targetMethodsIdx.push_back(i);
        }
    }

    size_t const numTargets = targetMethodsIdx.size();
    Span<Method> proxyMethods {classLinker->GetAllocator()->AllocArray<Method>(numTargets), numTargets};

    for (size_t i = 0; i < numTargets; ++i) {
        InitProxyMethod(cls, targetMethods[targetMethodsIdx[i]], &proxyMethods[i],
                        reinterpret_cast<void *>(entryPoint));
    }

    return proxyMethods;
}

/*static*/
JSProxy *JSProxy::CreateInterfaceProxy(const PandaSet<Class *> &interfaces, std::string &interfaceName)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    auto descriptor = MakeProxyDescriptor(utf::CStringAsMutf8(interfaceName.data()));
    Class *objectClass = ctx->GetObjectClass();
    ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();
    ClassLinkerContext *context = objectClass->GetLoadContext();

    Class *proxyCls = classLinker->GetClass(descriptor.get(), true, context);
    if (proxyCls == nullptr) {
        PandaSet<Class *> interfaceSet;
        MethodMap methodMap;
        for (auto cls : interfaces) {
            GetInterfaceMethodDistinct(cls, methodMap, interfaceSet);
        }

        PandaVector<Method *> methodPtrs;
        for (auto &[n, p] : methodMap) {
            methodPtrs.push_back(p);
        }
        PandaVector<Class *> interfacesList = {interfaces.begin(), interfaces.end()};
        return CreateProxy(descriptor.get(), objectClass, {methodPtrs.data(), methodPtrs.size()}, interfacesList,
                           reinterpret_cast<void *>(CallJSProxyBridge));
    }

    ASSERT(IsProxyClass(proxyCls));

    auto jsProxy = Runtime::GetCurrent()->GetInternalAllocator()->New<JSProxy>(EtsClass::FromRuntimeClass(proxyCls));
    return jsProxy;
}

/*static*/
JSProxy *JSProxy::CreateBuiltinProxy(EtsClass *etsClass, Span<Method *> targetMethods)
{
    Class *cls = etsClass->GetRuntimeClass();
    ASSERT(!IsProxyClass(cls) && !etsClass->IsFinal());
    auto descriptor = MakeProxyDescriptor(cls->GetDescriptor());
    return CreateProxy(descriptor.get(), cls, targetMethods, {}, reinterpret_cast<void *>(CallJSProxyBridge));
}

/*static*/
JSProxy *JSProxy::CreateFunctionProxy(EtsClass *functionInterface)
{
    auto coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);
    auto ctx = InteropCtx::Current(coro);
    ASSERT(functionInterface != nullptr);
    ASSERT(ctx != nullptr);
    Class *interfaceCls = functionInterface->GetRuntimeClass();
    ASSERT(interfaceCls->IsInterface());

    // create JSFunctoinProxy class descriptor that will respond to IsProxyClass
    auto descriptor = MakeProxyDescriptor(interfaceCls->GetDescriptor());

    // use `Object` as the base class for the proxy function
    Class *objectClass = ctx->GetObjectClass();
    ASSERT(objectClass != nullptr);
    // get the proxy function class if it is already created
    // otherwise, create the class
    ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();
    ClassLinkerContext *context = objectClass->GetLoadContext();

    Class *proxyFunctionCls = classLinker->GetClass(descriptor.get(), true, context);

    if (proxyFunctionCls == nullptr) {
        PandaSet<Class *> methodPSet;
        PandaVector<Method *> methodPtrs;
        GetAllInterfaceMethod(interfaceCls, methodPtrs, methodPSet);

        return CreateProxy(descriptor.get(), objectClass, {methodPtrs.data(), methodPtrs.size()}, {interfaceCls},
                           reinterpret_cast<void *>(CallJSFunctionBridge));
    }

    ASSERT(IsProxyClass(proxyFunctionCls));

    auto functionProxy =
        Runtime::GetCurrent()->GetInternalAllocator()->New<JSProxy>(EtsClass::FromRuntimeClass(proxyFunctionCls));
    return functionProxy;
}

/*static*/
JSProxy *JSProxy::CreateProxy(const uint8_t *descriptor, Class *baseClass, Span<Method *> targetMethods,
                              const PandaVector<Class *> &interfaces, void *callBridge)
{
    ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();
    ClassLinkerContext *context = baseClass->GetLoadContext();
    auto proxyMethods = BuildProxyMethods(baseClass, targetMethods, callBridge);

    uint32_t accessFlags = baseClass->GetAccessFlags() | ACC_PROXY | ACC_FINAL;
    Span<Field> fields {};
    Span<Class *> interfacesSpan {classLinker->GetAllocator()->AllocArray<Class *>(interfaces.size()),
                                  interfaces.size()};
    for (size_t i = 0; i < interfaces.size(); i++) {
        interfacesSpan[i] = interfaces[i];
    }

    Class *proxyCls = classLinker->BuildClass(descriptor, true, accessFlags, proxyMethods, fields, baseClass,
                                              interfacesSpan, context, false);
    if (UNLIKELY(proxyCls == nullptr)) {
        ASSERT(EtsCoroutine::GetCurrent()->HasPendingException());
        classLinker->GetAllocator()->Delete<Method>(proxyMethods.Data());
        classLinker->GetAllocator()->Delete<Class *>(interfacesSpan.Data());
        return nullptr;
    }
    proxyCls->SetState(Class::State::INITIALIZING);
    proxyCls->SetState(Class::State::INITIALIZED);

    // make the proxy class a xref class
    proxyCls->SetXRefClass();

    ASSERT(IsProxyClass(proxyCls));

    auto jsProxy = Runtime::GetCurrent()->GetInternalAllocator()->New<JSProxy>(EtsClass::FromRuntimeClass(proxyCls));
    return jsProxy;
}

}  // namespace ark::ets::interop::js::js_proxy
