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

#include "plugins/ets/runtime/intrinsics/helpers/reflection_helpers.h"

#include "plugins/ets/runtime/types/ets_primitives.h"

namespace ark::ets::intrinsics::helpers {

bool CheckPrimitiveReciever(EtsCoroutine *coro, EtsObject *arg, EtsClass *argClass, EtsClass *paramClass,
                            Value *argValue)
{
    if (!argClass->IsBoxed()) {
        ThrowEtsInvalidType(coro, argClass->GetDescriptor());
        return false;
    }

    bool checked = false;
    auto *linkExt = coro->GetPandaVM()->GetEtsClassLinkerExtension();

    switch (argClass->GetBoxedType()) {
        case EtsClass::BoxedType::BOOLEAN:
            checked = CheckAndUnpackBoxedType<EtsBoolean>(linkExt, arg, paramClass, argValue, ClassRoot::U1);
            break;
        case EtsClass::BoxedType::BYTE:
            checked = CheckAndUnpackBoxedType<EtsByte>(linkExt, arg, paramClass, argValue, ClassRoot::I8);
            break;
        case EtsClass::BoxedType::CHAR:
            checked = CheckAndUnpackBoxedType<EtsChar>(linkExt, arg, paramClass, argValue, ClassRoot::U16);
            break;
        case EtsClass::BoxedType::DOUBLE:
            checked = CheckAndUnpackBoxedType<EtsDouble>(linkExt, arg, paramClass, argValue, ClassRoot::F64);
            break;
        case EtsClass::BoxedType::FLOAT:
            checked = CheckAndUnpackBoxedType<EtsFloat>(linkExt, arg, paramClass, argValue, ClassRoot::F32);
            break;
        case EtsClass::BoxedType::INT:
            checked = CheckAndUnpackBoxedType<EtsInt>(linkExt, arg, paramClass, argValue, ClassRoot::I32);
            break;
        case EtsClass::BoxedType::LONG:
            checked = CheckAndUnpackBoxedType<EtsLong>(linkExt, arg, paramClass, argValue, ClassRoot::I64);
            break;
        case EtsClass::BoxedType::SHORT:
            checked = CheckAndUnpackBoxedType<EtsShort>(linkExt, arg, paramClass, argValue, ClassRoot::I16);
            break;
        default:
            ThrowEtsInvalidType(coro, argClass->GetDescriptor());
            return false;
    }

    if (!checked) {
        ThrowEtsInvalidType(coro, argClass->GetDescriptor());
        return false;
    }

    return true;
}

bool CheckReceiverType(EtsCoroutine *coro, EtsObject *arg, EtsClass *paramClass, Value *argValue)
{
    if (arg == nullptr) {
        if (paramClass->IsPrimitive()) {
            ThrowEtsException(coro, panda_file_items::class_descriptors::NULL_POINTER_ERROR,
                              "undefined argument is not allowed for primitive reciever");
            return false;
        }
        *argValue = Value(nullptr);
        return true;
    }

    EtsClass *argClass = arg->GetClass();

    if (paramClass->IsAssignableFrom(argClass)) {
        *argValue = Value(arg->GetCoreType());
        return true;
    }

    if (paramClass->IsPrimitive()) {
        return CheckPrimitiveReciever(coro, arg, argClass, paramClass, argValue);
    }

    ThrowEtsInvalidType(coro, argClass->GetDescriptor());
    return false;
}

EtsObject *InvokeAndResolveReturnValue(EtsMethod *method, EtsCoroutine *coro, Value *args)
{
    Value result = method->GetPandaMethod()->Invoke(coro, args);

    if (coro->HasPendingException()) {
        return nullptr;
    }

    // Value is already in args[0]
    if (method->IsInstanceConstructor()) {
        return nullptr;
    }

    EtsClass *resolvedType = method->ResolveReturnType();
    if (resolvedType->IsPrimitive() && !resolvedType->IsVoid()) {
        return GetBoxedValue(coro, result, method->GetReturnValueType());
    }

    // If the expected return value is not a
    //  primitive - handle result as an object
    return EtsObject::FromCoreType((result.GetAs<ObjectHeader *>()));
}

EtsMethod *ValidateAndResolveInstanceMethod(EtsCoroutine *coro, EtsObject *thisObj, EtsMethod *method)
{
    // For instance methods, thisObj validation is required
    if (thisObj == nullptr) {
        ThrowEtsException(coro, panda_file_items::class_descriptors::NULL_POINTER_ERROR,
                          "Instance method invocation requires non-null 'thisObj'");
        return nullptr;
    }

    // Validate that thisObj is subtype of method owner
    if (!method->GetClass()->IsAssignableFrom(thisObj->GetClass())) {
        PandaOStringStream ss;
        ss << "Object type [" << thisObj->GetClass()->GetRuntimeClass()->GetName()
           << "] is not compatible with method owner type [" << method->GetClass()->GetRuntimeClass()->GetName() << ']';
        ThrowEtsException(coro, panda_file_items::class_descriptors::TYPE_ERROR, ss.str());
        return nullptr;
    }

    // Resolve virtual method - this is instance-method specific
    EtsMethod *resolved = thisObj->GetClass()->ResolveVirtualMethod(method);
    if (resolved == nullptr) {
        ThrowEtsException(coro, panda_file_items::class_descriptors::TYPE_ERROR, "Virtual method resolution failed");
        return nullptr;
    }
    return resolved;
}

bool ValidateInstanceField(EtsCoroutine *coro, EtsObject *thisObj, EtsField *field)
{
    // For instance methods, thisObj validation is required
    if (thisObj == nullptr) {
        ThrowEtsException(coro, panda_file_items::class_descriptors::NULL_POINTER_ERROR,
                          "Instance field manipulation requires non-null 'thisObj'");
        return false;
    }

    // Validate that thisObj is subtype of method owner
    if (!field->GetDeclaringClass()->IsAssignableFrom(thisObj->GetClass())) {
        ThrowEtsException(coro, panda_file_items::class_descriptors::TYPE_ERROR,
                          "Object type is not compatible with field owner type");
        return false;
    }

    return true;
}

// CC-OFFNXT(G.FUD.05) big switch vase
bool ResolveAndSetPrimitive(EtsCoroutine *coro, const PrimitiveFieldInfo &info)
{
    EtsClass::BoxedType argType = info.argClass->GetBoxedType();
    switch (info.fieldType) {
        case EtsType::BOOLEAN:
            if (argType == EtsClass::BoxedType::BOOLEAN) {
                return ReflectFieldSetPrimitive(coro, info.thisObj, GetUnboxedValue(coro, info.arg).GetAs<EtsBoolean>(),
                                                info.field);
            }
            break;
        case EtsType::BYTE:
            if (argType == EtsClass::BoxedType::BYTE) {
                return ReflectFieldSetPrimitive(coro, info.thisObj, GetUnboxedValue(coro, info.arg).GetAs<EtsByte>(),
                                                info.field);
            }
            break;
        case EtsType::CHAR:
            if (argType == EtsClass::BoxedType::CHAR) {
                return ReflectFieldSetPrimitive(coro, info.thisObj, GetUnboxedValue(coro, info.arg).GetAs<EtsChar>(),
                                                info.field);
            }
            break;
        case EtsType::DOUBLE:
            if (argType == EtsClass::BoxedType::DOUBLE) {
                return ReflectFieldSetPrimitive(coro, info.thisObj, GetUnboxedValue(coro, info.arg).GetAs<EtsDouble>(),
                                                info.field);
            }
            break;
        case EtsType::FLOAT:
            if (argType == EtsClass::BoxedType::FLOAT) {
                return ReflectFieldSetPrimitive(coro, info.thisObj, GetUnboxedValue(coro, info.arg).GetAs<EtsFloat>(),
                                                info.field);
            }
            break;
        case EtsType::INT:
            if (argType == EtsClass::BoxedType::INT) {
                return ReflectFieldSetPrimitive(coro, info.thisObj, GetUnboxedValue(coro, info.arg).GetAs<EtsInt>(),
                                                info.field);
            }
            break;
        case EtsType::LONG:
            if (argType == EtsClass::BoxedType::LONG) {
                return ReflectFieldSetPrimitive(coro, info.thisObj, GetUnboxedValue(coro, info.arg).GetAs<EtsLong>(),
                                                info.field);
            }
            break;
        case EtsType::SHORT:
            if (argType == EtsClass::BoxedType::SHORT) {
                return ReflectFieldSetPrimitive(coro, info.thisObj, GetUnboxedValue(coro, info.arg).GetAs<EtsShort>(),
                                                info.field);
                return true;
            }
            break;
        case EtsType::OBJECT:
        case EtsType::VOID:
        case EtsType::UNKNOWN:
            break;
    }
    return false;
}

Value GetPrimitiveValue(EtsCoroutine *coro, EtsObject *thisObj, EtsType fieldType, EtsField *field)
{
    switch (fieldType) {
        case EtsType::BOOLEAN:
            return ReflectFieldGetPrimitive<EtsBoolean>(thisObj, field);
        case EtsType::BYTE:
            return ReflectFieldGetPrimitive<EtsByte>(thisObj, field);
        case EtsType::CHAR:
            return ReflectFieldGetPrimitive<EtsChar>(thisObj, field);
        case EtsType::DOUBLE:
            return ReflectFieldGetPrimitive<EtsDouble>(thisObj, field);
        case EtsType::FLOAT:
            return ReflectFieldGetPrimitive<EtsFloat>(thisObj, field);
        case EtsType::INT:
            return ReflectFieldGetPrimitive<EtsInt>(thisObj, field);
        case EtsType::LONG:
            return ReflectFieldGetPrimitive<EtsLong>(thisObj, field);
        case EtsType::SHORT:
            return ReflectFieldGetPrimitive<EtsShort>(thisObj, field);
        case EtsType::OBJECT:
        case EtsType::VOID:
        case EtsType::UNKNOWN:
            break;
    }
    ThrowEtsException(coro, panda_file_items::class_descriptors::TYPE_ERROR, "Failed to resolve primitive field type");
    return Value(0);
}

EtsBoolean IsLiteralInitializedInterface(EtsObject *target)
{
    ASSERT(target != nullptr);
    auto *etsClass = target->GetClass();
    auto *runtimeClass = etsClass->GetRuntimeClass();
    if (runtimeClass->GetPandaFile() == nullptr) {
        return ToEtsBoolean(false);
    }

    const panda_file::File &pf = *runtimeClass->GetPandaFile();
    panda_file::ClassDataAccessor cda(pf, runtimeClass->GetFileId());
    bool retBoolVal = false;

    cda.EnumerateAnnotation(panda_file_items::class_descriptors::INTERFACE_OBJ_LITERAL.data(),
                            [&retBoolVal](panda_file::AnnotationDataAccessor &) {
                                retBoolVal = true;
                                return true;
                            });

    return ToEtsBoolean(retBoolVal);
}

}  // namespace ark::ets::intrinsics::helpers
