/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "intrinsics.h"

#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_exceptions.h"

namespace ark::ets::intrinsics {

static std::optional<Span<Class *>> CheckAndTrasformInterfaces(EtsObjectArray *interfaces,
                                                               InternalAllocatorPtr allocator)
{
    Span<Class *> proxyInterfaces;

    uint32_t intfNum = interfaces->GetLength();
    if (intfNum > 0) {
        proxyInterfaces = Span(allocator->AllocArray<Class *>(intfNum), intfNum);
        for (uint32_t idx = 0; idx < intfNum; ++idx) {
            EtsObject *iface = interfaces->Get(idx);
            ASSERT(iface != nullptr);

            proxyInterfaces[idx] = EtsClass::FromEtsClassObject(iface)->GetRuntimeClass();
            if (UNLIKELY(!proxyInterfaces[idx]->IsInterface())) {
                PandaOStringStream msg;
                msg << "All given to Proxy classes must be an interface. But given class '"
                    << proxyInterfaces[idx]->GetName() << " is not an interface";
                ark::ets::ThrowEtsException(EtsCoroutine::GetCurrent(), panda_file_items::class_descriptors::TYPE_ERROR,
                                            msg.str().c_str());

                allocator->Free(proxyInterfaces.Data());
                return std::nullopt;
            }
        }
    }

    return proxyInterfaces;
}

extern "C" EtsClass *ReflectProxyGenerateProxy(EtsRuntimeLinker *linker, EtsString *name, ObjectHeader *interfaces)
{
    ASSERT(linker != nullptr);
    ASSERT(name != nullptr);
    ASSERT(interfaces != nullptr);

    auto *coroutine = EtsCoroutine::GetCurrent();
    ASSERT(coroutine != nullptr);

    auto *linkerCtx = linker->GetClassLinkerContext();
    ASSERT(linkerCtx != nullptr);

    auto *classLinkerExt = coroutine->GetPandaVM()->GetClassLinker()->GetEtsClassLinkerExtension();
    ASSERT(classLinkerExt != nullptr);

    Class *baseProxyClass = classLinkerExt->GetPlatformTypes()->coreReflectProxy->GetRuntimeClass();
    ASSERT(baseProxyClass != nullptr);

    [[maybe_unused]] EtsHandleScope scope(coroutine);
    EtsHandle<EtsString> nameH(coroutine, name);
    EtsHandle<EtsObjectArray> interfacesH(coroutine, EtsObjectArray::FromCoreType(interfaces));

    PandaString descriptor;
    ClassHelper::GetDescriptor(utf::CStringAsMutf8(nameH->GetMutf8().data()), &descriptor);
    const uint8_t *descriptorMutf8 = utf::CStringAsMutf8(descriptor.c_str());

    auto internalAllocator = coroutine->GetPandaVM()->GetHeapManager()->GetInternalAllocator();
    auto proxyInterfaces = CheckAndTrasformInterfaces(interfacesH.GetPtr(), internalAllocator);
    if (UNLIKELY(!proxyInterfaces)) {
        return nullptr;
    }

    constexpr uint32_t PROXY_CLASS_ACCESS_FLAGS = ACC_PROXY | ACC_FINAL;

    Class *proxyClass = Runtime::GetCurrent()->GetClassLinker()->BuildProxyClass(
        descriptorMutf8, true, PROXY_CLASS_ACCESS_FLAGS, Span<Field>(), baseProxyClass, *proxyInterfaces, linkerCtx,
        classLinkerExt->GetErrorHandler());
    if (UNLIKELY(proxyClass == nullptr)) {
        ASSERT(coroutine->HasPendingException());
        LOG(INFO, ETS) << "Can't build proxy class " << nameH->GetMutf8();
        internalAllocator->Free(proxyInterfaces->data());
        return nullptr;
    }

    proxyClass->SetFieldIndex(baseProxyClass->GetFieldIndex());
    proxyClass->SetMethodIndex(baseProxyClass->GetMethodIndex());
    proxyClass->SetClassIndex(baseProxyClass->GetClassIndex());

    proxyClass->SetState(Class::State::INITIALIZING);
    proxyClass->SetState(Class::State::INITIALIZED);

    return EtsClass::FromRuntimeClass(proxyClass);
}

extern "C" EtsBoolean ReflectProxyHasProxyFlag(EtsClass *klass)
{
    ASSERT(klass != nullptr);
    return ToEtsBoolean(klass->GetRuntimeClass()->IsProxy());
}

}  // namespace ark::ets::intrinsics
