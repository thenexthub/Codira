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

#include <cctype>
#include <cstddef>
#include <cstdint>
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/intrinsics/helpers/reflection_helpers.h"
#include "runtime/include/mem/panda_string.h"
#include "runtime/include/object_header.h"
#include "intrinsics.h"
#include "runtime/mem/vm_handle.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_box_primitive.h"
#include "plugins/ets/runtime/types/ets_box_primitive-inl.h"
#include "plugins/ets/runtime/types/ets_class.h"
#include "plugins/ets/runtime/types/ets_escompat_array.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/ets_utils.h"
#include "plugins/ets/runtime/types/ets_typeapi.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_primitives.h"
#include "plugins/ets/runtime/types/ets_reflect_method.h"
#include "common_interfaces/objects/string/line_string.h"

namespace ark::ets::intrinsics {

// Utility functions
extern "C" EtsClass *ReflectMethodGetReturnTypeImpl(EtsLong etsMethodPtr)
{
    auto *retCls = reinterpret_cast<EtsMethod *>(etsMethodPtr)->ResolveReturnType();
    if (UNLIKELY(retCls == nullptr)) {
        ASSERT(EtsCoroutine::GetCurrent()->HasPendingException());
        return nullptr;
    }
    return retCls->ResolvePublicClass();
}

extern "C" EtsClass *ReflectMethodGetParameterTypeByIdxImpl(EtsLong etsFunctionPtr, EtsInt i)
{
    auto *function = reinterpret_cast<EtsMethod *>(etsFunctionPtr);
    ASSERT(function != nullptr);
    ASSERT(i >= 0 && static_cast<uint32_t>(i) < function->GetNumArgs());
    // 0 is recevier type
    i = function->IsStatic() ? i : i + 1;
    auto *argType = function->ResolveArgType(i);
    if (UNLIKELY(argType == nullptr)) {
        ASSERT(EtsCoroutine::GetCurrent()->HasPendingException());
        return nullptr;
    }

    return argType->ResolvePublicClass();
}

extern "C" EtsInt ReflectMethodGetParametersNumImpl(EtsLong etsFunctionPtr)
{
    auto *function = reinterpret_cast<EtsMethod *>(etsFunctionPtr);
    ASSERT(function != nullptr);
    return function->GetParametersNum();
}

extern "C" ObjectHeader *ReflectMethodGetParametersTypesImpl(EtsLong etsFunctionPtr)
{
    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);

    auto *function = reinterpret_cast<EtsMethod *>(etsFunctionPtr);
    ASSERT(function != nullptr);
    auto numParams = function->GetParametersNum();
    [[maybe_unused]] EtsHandleScope scope(coro);
    EtsHandle<EtsObjectArray> arrayH(coro, EtsObjectArray::Create(PlatformTypes(coro)->coreClass, numParams));
    if (UNLIKELY(arrayH.GetPtr() == nullptr)) {
        ASSERT(coro->HasPendingException());
        return nullptr;
    }
    ASSERT(arrayH.GetPtr() != nullptr);

    for (uint32_t idx = 0; idx < numParams; ++idx) {
        // 0 is recevier type
        auto i = function->IsStatic() ? idx : idx + 1;
        auto *argType = function->ResolveArgType(i);
        if (UNLIKELY(argType == nullptr)) {
            ASSERT(EtsCoroutine::GetCurrent()->HasPendingException());
            return nullptr;
        }
        auto *resolvedParameterType = argType->ResolvePublicClass();

        arrayH->Set(idx, resolvedParameterType->AsObject());
    }

    return arrayH->AsObject()->GetCoreType();
}

extern "C" EtsString *ReflectMethodGetNameInternal(EtsLong etsMethodPtr)
{
    auto *method = reinterpret_cast<EtsMethod *>(etsMethodPtr);
    EtsString *name;
    if (method->IsInstanceConstructor()) {
        name = EtsString::CreateFromMUtf8(CONSTRUCTOR_NAME);
    } else {
        name = method->GetNameString();
    }
    if (UNLIKELY(name == nullptr)) {
        [[maybe_unused]] auto *coro = EtsCoroutine::GetCurrent();
        ASSERT(coro != nullptr);
        ASSERT(coro->HasPendingException());
        return nullptr;
    }
    ASSERT(name != nullptr);
    return name;
}

extern "C" EtsObject *ReflectMethodCreateInstanceInternal(EtsReflectMethod *thisConstructor, ObjectHeader *argsArr)
{
    ASSERT(thisConstructor != nullptr);

    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);

    [[maybe_unused]] EtsHandleScope scope(coro);

    EtsHandle argsArrH(coro, EtsObjectArray::FromCoreType(argsArr));
    EtsHandle thisConstructorH(coro, thisConstructor);

    auto *constructor = thisConstructor->GetEtsMethod();
    ASSERT(constructor->IsInstanceConstructor());

    EtsHandle objToCreateH(coro, EtsObject::Create(constructor->GetClass()));
    if (objToCreateH.GetPtr() == nullptr) {
        ASSERT(coro->HasPendingException());
        return nullptr;
    }

    // Discard return value, because result is in passed EtsObject as this
    ReflectMethodInvokeInternal(thisConstructorH.GetPtr(), objToCreateH.GetPtr(),
                                EtsArray::GetCoreType(argsArrH.GetPtr()));

    return objToCreateH.GetPtr();
}

// CC-OFFNXT(G.FUN.01-CPP) solid logic
extern "C" EtsObject *ReflectMethodInvokeInternal(ark::ObjectHeader *thisMethod, EtsObject *thisObj,
                                                  ObjectHeader *argsArr)
{
    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);

    [[maybe_unused]] EtsHandleScope scope(coro);

    EtsHandle thisObjH(coro, thisObj);
    EtsHandle args(coro, EtsObjectArray::FromCoreType(argsArr));

    auto *reflectMethod = EtsReflectMethod::FromEtsObject(EtsObject::FromCoreType(thisMethod));
    ASSERT(reflectMethod != nullptr);
    auto *method = reflectMethod->GetEtsMethod();
    ASSERT(method != nullptr);

    const uint8_t instanceFlag = !method->IsStatic();
    // Step 1: Handle thisObj validation for instance methods
    if (instanceFlag != 0) {
        method = helpers::ValidateAndResolveInstanceMethod(coro, thisObjH.GetPtr(), method);
        if (method == nullptr) {
            ASSERT(coro->HasPendingException());
            return nullptr;
        }
    }

    // Step 2: Calculate argument sizes and validate
    size_t argsSize = (args.GetPtr() == nullptr) ? 0 : args->GetLength();
    const size_t parametersNum = method->GetParametersNum();
    if (argsSize != parametersNum) {
        PandaStringStream pss;
        pss << "Expected " << parametersNum << " arguments, " << argsSize << " given.";
        ThrowEtsException(coro, panda_file_items::class_descriptors::TYPE_ERROR, pss.str());
        return nullptr;
    }

    // Step 3: Build arguments vector and get parameters type
    PandaVector<EtsClass *> paramTypes;
    paramTypes.reserve(method->GetParametersNum());
    for (size_t i = 0; i < argsSize; ++i) {
        auto resolvedMethod = method->ResolveArgType(i + instanceFlag);
        if (UNLIKELY(resolvedMethod == nullptr)) {
            ASSERT(coro->HasPendingException());
            return nullptr;
        }
        paramTypes.emplace_back(resolvedMethod);
    }

    PandaVector<Value> passedArgs;
    passedArgs.reserve(argsSize + instanceFlag);
    if (instanceFlag != 0) {
        ASSERT(thisObjH.GetPtr() != nullptr);
        passedArgs.emplace_back(Value(thisObjH->GetCoreType()));  // Add 'this' argument for instance methods
    }

    // Step 4: Process regular arguments with type checking
    for (size_t i = 0; i < argsSize; ++i) {
        Value argValue;
        // Check and convert type
        if (!helpers::CheckReceiverType(coro, args->Get(i), paramTypes[i], &argValue)) {
            ASSERT(coro->HasPendingException());
            return nullptr;  // Exception already thrown
        }
        passedArgs.emplace_back(argValue);
    }

    // Step 5: Invoke method and handle result
    return helpers::InvokeAndResolveReturnValue(method, coro, passedArgs.data());
}

}  // namespace ark::ets::intrinsics
