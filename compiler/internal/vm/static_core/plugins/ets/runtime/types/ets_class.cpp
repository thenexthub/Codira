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

#include "ets_panda_file_items.h"
#include "include/language_context.h"
#include "include/mem/panda_containers.h"
#include "libarkbase/utils/utf.h"
#include "libarkbase/macros.h"
#include "plugins/ets/runtime/ets_class_linker_context.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_utils.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_field.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/types/ets_method_signature.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/types/ets_value.h"
#include "plugins/ets/runtime/types/ets_class.h"
#include "runtime/include/runtime.h"
#include "runtime/mem/local_object_handle.h"

namespace ark::ets {

static bool MatchesValueClassStorage(const ark::panda_file::Type &prim, const ark::Class *owner)
{
    auto *pt = ark::ets::PlatformTypes();
    auto eq = [owner](ark::ets::EtsClass *c) -> bool { return c != nullptr && owner == c->GetRuntimeClass(); };

    switch (prim.GetId()) {
        case ark::panda_file::Type::TypeId::I32:
            return eq(pt->coreInt);
        case ark::panda_file::Type::TypeId::I64:
            return eq(pt->coreLong);
        case ark::panda_file::Type::TypeId::F64:
            return eq(pt->coreDouble);
        case ark::panda_file::Type::TypeId::F32:
            return eq(pt->coreFloat);
        case ark::panda_file::Type::TypeId::I8:
            return eq(pt->coreByte);
        case ark::panda_file::Type::TypeId::I16:
            return eq(pt->coreShort);
        case ark::panda_file::Type::TypeId::U16:
            return eq(pt->coreChar);
        case ark::panda_file::Type::TypeId::U1:
            return eq(pt->coreBoolean);
        default:
            return false;
    }
}

static bool VerifyLambdaClass(EtsClass *etsClass, Method *method, ClassLinkerErrorHandler *errorHandler)
{
    ASSERT(etsClass != nullptr);
    ASSERT(method != nullptr);
    auto fields = etsClass->GetFields();
    if (method->IsStatic()) {
        return fields.empty();
    }
    if (fields.size() != 1) {
        LOG(ERROR, CLASS_LINKER) << "Invalid LambdaClass: Expected at most 1 field, but got " << fields.size();
        return false;
    }
    auto *rf = etsClass->GetFieldByIndex(0)->GetRuntimeField();
    auto ftype = rf->GetType();
    auto *owner = method->GetClass();

    if (ftype.IsPrimitive()) {
        if (!MatchesValueClassStorage(ftype, owner)) {
            LOG(ERROR, CLASS_LINKER) << "FR: primitive $this does not match value-class storage";
            return false;
        }
        return true;
    }
    auto klass = rf->ResolveTypeClass(errorHandler);
    if (klass == nullptr) {
        return false;
    }
    return owner->IsAssignableFrom(klass);
}

static void ReportInvalidLambdaClass(const uint8_t *descriptor, [[maybe_unused]] ClassLinkerErrorHandler *errorHandler)
{
    if (errorHandler != nullptr) {
        PandaStringStream ss;
        ss << "Found invalid lambda class " << descriptor;
        errorHandler->OnError(ClassLinker::Error::INVALID_LAMBDA_CLASS, ss.str());
    }
}

static void FunctionalReferenceAnnotationCallBack(EtsClass *etsClass, const panda_file::File *pfile,
                                                  panda_file::AnnotationDataAccessor *ada,
                                                  ClassLinkerErrorHandler *errorHandler)
{
    // methodOffset is passed by FE
    auto implMethod = ada->GetElement(0).GetScalarValue().Get<panda_file::File::EntityId>();
    auto methodOffset = implMethod.GetOffset();
    auto *linker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto *method = linker->GetMethod(*pfile, panda_file::File::EntityId(methodOffset), etsClass->GetLoadContext());

    etsClass->SetTypeMetaData(reinterpret_cast<EtsLong>(method));
    if (!VerifyLambdaClass(etsClass, method, errorHandler)) {
        auto descriptor = utf::CStringAsMutf8(etsClass->GetDescriptor());
        ReportInvalidLambdaClass(descriptor, errorHandler);
    }
}

uint32_t EtsClass::GetFieldsNumber()
{
    uint32_t fnumber = 0;
    EnumerateBaseClasses([&](EtsClass *c) {
        fnumber += c->GetRuntimeClass()->GetFields().Size();
        return false;
    });
    return fnumber;
}

// Without inherited fields
uint32_t EtsClass::GetOwnFieldsNumber()
{
    return GetRuntimeClass()->GetFields().Size();
}

PandaVector<EtsField *> EtsClass::GetFields()
{
    auto etsFields = PandaVector<EtsField *>(Runtime::GetCurrent()->GetInternalAllocator()->Adapter());
    EnumerateBaseClasses([&](EtsClass *c) {
        auto fields = c->GetRuntimeClass()->GetFields();
        auto fnum = fields.Size();
        for (uint32_t i = 0; i < fnum; i++) {
            etsFields.push_back(EtsField::FromRuntimeField(&fields[i]));
        }
        return false;
    });
    return etsFields;
}

EtsField *EtsClass::GetFieldByIndex(uint32_t i)
{
    EtsField *res = nullptr;
    EnumerateBaseClasses([&](EtsClass *c) {
        auto fields = c->GetRuntimeClass()->GetFields();
        auto fnum = fields.Size();
        if (i >= fnum) {
            i -= fnum;
            return false;
        }
        res = EtsField::FromRuntimeField(&fields[i]);
        return true;
    });
    return res;
}

EtsField *EtsClass::GetOwnFieldByIndex(uint32_t i)
{
    return EtsField::FromRuntimeField(&GetRuntimeClass()->GetFields()[i]);
}

EtsMethod *EtsClass::GetDirectMethod(const char *name, const char *signature)
{
    auto coreName = reinterpret_cast<const uint8_t *>(name);
    return GetDirectMethod(coreName, signature);
}

EtsMethod *EtsClass::GetDirectMethod(const char *name)
{
    const uint8_t *mutf8Name = utf::CStringAsMutf8(name);
    Method *rtMethod = GetRuntimeClass()->GetDirectMethod(mutf8Name);
    return EtsMethod::FromRuntimeMethod(rtMethod);
}

EtsMethod *EtsClass::GetDirectMethod(const uint8_t *name, const char *signature)
{
    EtsMethodSignature methodSignature(signature);
    if (!methodSignature.IsValid()) {
        LOG(ERROR, RUNTIME) << "Wrong method signature: " << signature;
        return nullptr;
    }

    auto coreMethod = GetRuntimeClass()->GetDirectMethod(name, methodSignature.GetProto());
    return reinterpret_cast<EtsMethod *>(coreMethod);
}

EtsMethod *EtsClass::GetDirectMethod(const char *name, const Method::Proto &proto) const
{
    Method *method = klass_.GetDirectMethod(utf::CStringAsMutf8(name), proto);
    return EtsMethod::FromRuntimeMethod(method);
}

EtsMethod *EtsClass::GetDirectMethod(bool isStatic, const char *name, const char *signature) const
{
    EtsMethodSignature methodSignature(signature);
    if (!methodSignature.IsValid()) {
        LOG(ERROR, ANI) << "Wrong method signature: " << signature;
        return nullptr;
    }
    return GetDirectMethod(isStatic, name, methodSignature);
}

EtsMethod *EtsClass::GetDirectMethod(bool isStatic, const char *name, const EtsMethodSignature &methodSignature) const
{
    const Class *rtClass = GetRuntimeClass();
    Span<Method> methods = isStatic ? rtClass->GetStaticMethods() : rtClass->GetVirtualMethods();
    const uint8_t *mutf8Name = utf::CStringAsMutf8(name);
    const MethodNameComp comp;
    panda_file::File::StringData key = {static_cast<uint32_t>(ark::utf::MUtf8ToUtf16Size(mutf8Name)), mutf8Name};
    auto it = std::lower_bound(methods.begin(), methods.end(), key, comp);

    const Method::Proto &proto = methodSignature.GetProto();
    while (it != methods.end()) {
        auto &m = *it;
        if (!comp.equal(m, key)) {
            break;
        }
        if (m.GetProto() == proto) {
            return EtsMethod::FromRuntimeMethod(it);
        }
        ++it;
    }
    return nullptr;
}

EtsMethod *EtsClass::GetDirectMethod(bool isStatic, const char *name, bool *outIsUnique) const
{
    ASSERT(outIsUnique != nullptr);
    *outIsUnique = true;

    const Class *rtClass = GetRuntimeClass();
    Span<Method> methods = isStatic ? rtClass->GetStaticMethods() : rtClass->GetVirtualMethods();
    const uint8_t *mutf8Name = utf::CStringAsMutf8(name);
    const MethodNameComp comp;
    panda_file::File::StringData key = {static_cast<uint32_t>(ark::utf::MUtf8ToUtf16Size(mutf8Name)), mutf8Name};
    auto it = std::lower_bound(methods.begin(), methods.end(), key, comp);
    if (it == methods.cend() || !comp.equal(*it, key)) {
        return nullptr;
    }
    auto next = std::next(it);
    // Check that class has more than one method with that name.
    if (next != methods.cend() && comp.equal(*next, key)) {
        *outIsUnique = false;
    }
    return EtsMethod::FromRuntimeMethod(it);
}

uint32_t EtsClass::GetMethodsNum()
{
    // Atomic with relaxed order reason: get the latest value
    uint32_t currentNum = methodsNum_.load(std::memory_order_relaxed);
    if (currentNum == 0) {
        uint32_t newNum = GetMethods().size();
        methodsNum_.compare_exchange_strong(currentNum, newNum);
        return newNum;
    }
    return currentNum;
}

EtsMethod *EtsClass::GetMethodByIndex(uint32_t ind)
{
    EtsMethod *res = nullptr;
    auto methods = GetMethods();
    ASSERT(ind < methods.size());
    res = methods[ind];
    return res;
}

// NOTE(kirill-mitkin): Cache in EtsClass field later
PandaVector<EtsMethod *> EtsClass::GetMethods()
{
    PandaUnorderedMap<PandaString, EtsMethod *> uniqueMethods;

    auto addDirectMethods = [&](const EtsClass *c) {
        auto directMethods = c->GetRuntimeClass()->GetMethods();
        for (auto &method : directMethods) {
            PandaString methodFullName = utf::Mutf8AsCString(method.GetName().data);
            methodFullName += method.GetProto().GetSignature();
            if (uniqueMethods.find(methodFullName) == uniqueMethods.end()) {
                uniqueMethods[methodFullName] = EtsMethod::FromRuntimeMethod(&method);
            }
        }
    };

    auto addDirectMethodsForBaseClass = [&uniqueMethods](EtsClass *c) {
        auto directMethods = c->GetRuntimeClass()->GetMethods();
        auto fnum = directMethods.Size();
        for (uint32_t i = 0; i < fnum; i++) {
            Method *method = &directMethods[i];
            // Skip constructors
            if (method->IsConstructor()) {
                continue;
            }

            PandaString methodFullName = utf::Mutf8AsCString((method->GetName().data));
            methodFullName += method->GetProto().GetSignature();

            uniqueMethods[methodFullName] = EtsMethod::FromRuntimeMethod(method);
        }
    };

    if (IsInterface()) {
        addDirectMethods(this);
        EnumerateInterfaces([&](const EtsClass *c) {
            addDirectMethods(c);
            return false;
        });
    } else {
        EnumerateBaseClasses([&](EtsClass *c) {
            addDirectMethodsForBaseClass(c);
            return false;
        });
    }
    auto etsMethods = PandaVector<EtsMethod *>();
    for (auto &iter : uniqueMethods) {
        etsMethods.push_back(iter.second);
    }
    return etsMethods;
}

PandaVector<EtsMethod *> EtsClass::GetConstructors()
{
    auto constructors = PandaVector<EtsMethod *>();
    auto methods = GetRuntimeClass()->GetMethods();
    // NOTE(kirill-mitkin): cache in ets_class field
    for (auto &method : methods) {
        // Skip constructors
        if (method.IsInstanceConstructor()) {
            constructors.emplace_back(EtsMethod::FromRuntimeMethod(&method));
        }
    }
    return constructors;
}

EtsMethod *EtsClass::ResolveVirtualMethod(const EtsMethod *method) const
{
    return reinterpret_cast<EtsMethod *>(GetRuntimeClass()->ResolveVirtualMethod(method->GetPandaMethod()));
}

EtsString *EtsClass::CreateEtsClassName([[maybe_unused]] const char *descriptor)
{
    ASSERT_HAVE_ACCESS_TO_MANAGED_OBJECTS();

    if (*descriptor == 'L') {
        std::string_view tmpName(descriptor);
        tmpName.remove_prefix(1);
        tmpName.remove_suffix(1);
        PandaString etsName(tmpName);
        std::replace(etsName.begin(), etsName.end(), '/', '.');
        return EtsString::CreateFromMUtf8(etsName.data(), etsName.length());
    }

    ark::ets::RuntimeDescriptorParser nameParser(descriptor);
    return EtsString::CreateFromMUtf8(nameParser.Resolve().c_str());
}

EtsString *EtsClass::GetName()
{
    ASSERT_HAVE_ACCESS_TO_MANAGED_OBJECTS();

    EtsString *name = nullptr;

    name = reinterpret_cast<EtsString *>(GetObjectHeader()->GetFieldObject(GetNameOffset()));
    if (name != nullptr) {
        return name;
    }

    name = CreateEtsClassName(GetDescriptor());
    if (name == nullptr) {
        ASSERT(EtsCoroutine::GetCurrent()->HasPendingException());
        return nullptr;
    }
    return name;
}

bool EtsClass::IsInSamePackage(std::string_view className1, std::string_view className2)
{
    size_t i = 0;
    size_t minLength = std::min(className1.size(), className2.size());
    while (i < minLength && className1[i] == className2[i]) {
        ++i;
    }
    return className1.find('/', i) == std::string::npos && className2.find('/', i) == std::string::npos;
}

bool EtsClass::IsInSamePackage(EtsClass *that)
{
    if (this == that) {
        return true;
    }

    EtsClass *klass1 = this;
    EtsClass *klass2 = that;
    while (klass1->IsArrayClass()) {
        klass1 = klass1->GetComponentType();
    }
    while (klass2->IsArrayClass()) {
        klass2 = klass2->GetComponentType();
    }
    if (klass1 == klass2) {
        return true;
    }

    // Compare the package part of the descriptor string.
    return IsInSamePackage(klass1->GetDescriptor(), klass2->GetDescriptor());
}

void EtsClass::SetWeakReference()
{
    flags_ = flags_ | IS_WEAK_REFERENCE;
    ASSERT(IsWeakReference() && IsReference());
}
void EtsClass::SetFinalizeReference()
{
    flags_ = flags_ | IS_FINALIZE_REFERENCE;
    ASSERT(IsFinalizerReference() && IsReference());
}

void EtsClass::SetValueTyped()
{
    flags_ = flags_ | IS_VALUE_TYPED;
    ASSERT(IsValueTyped());
}
void EtsClass::SetNullValue()
{
    flags_ = flags_ | IS_NULLVALUE;
    ASSERT(IsNullValue());
}
void EtsClass::SetBoxedKind(BoxedType boxedKind)
{
    flags_ = flags_ | IS_BOXED;
    ASSERT(IsBoxed());
    SetValueTyped();
    auto kind = helpers::ToUnderlying(boxedKind);
    BoxedTypeField::Set(kind, &flags_);
}
void EtsClass::SetFunction()
{
    flags_ = flags_ | IS_FUNCTION;
    ASSERT(IsFunction());
}
void EtsClass::SetEtsEnum()
{
    flags_ = flags_ | IS_ETS_ENUM;
    ASSERT(IsEtsEnum());
}
void EtsClass::SetBigInt()
{
    flags_ = flags_ | IS_BIGINT;
    ASSERT(IsBigInt());
}

static bool HasFunctionTypeInSuperClasses(EtsClass *cls)
{
    if (EtsClass *base = cls->GetBase(); base != nullptr) {
        if (UNLIKELY(base->IsFunction())) {
            return true;
        }
    }
    for (Class *iface : cls->GetRuntimeClass()->GetInterfaces()) {
        if (UNLIKELY(EtsClass::FromRuntimeClass(iface)->IsFunction())) {
            return true;
        }
    }
    return false;
}

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP, G.FUD.05) solid logic
void EtsClass::Initialize(EtsClass *superClass, uint16_t accessFlags, bool isPrimitiveType,
                          ClassLinkerErrorHandler *errorHandler)
{
    ASSERT_HAVE_ACCESS_TO_MANAGED_OBJECTS();
    SetName(nullptr);
    // Class of interface extends Object, but we should not expose this information to user.
    IsInterface() ? SetSuperClass(nullptr) : SetSuperClass(superClass);

    // Save reference to defining RuntimeLinker in order to prevent its collection
    auto *ctx = GetLoadContext();
    ASSERT(ctx != nullptr);
    SetLinker(ctx->IsBootContext() ? nullptr
                                   : EtsClassLinkerContext::FromCoreType(ctx)->GetRuntimeLinker()->GetCoreType());

    uint32_t flags = accessFlags;
    if (isPrimitiveType) {
        flags |= ETS_ACC_PRIMITIVE;
    }
    if (superClass != nullptr) {
        static constexpr uint32_t COPIED_MASK = IS_WEAK_REFERENCE | IS_FINALIZE_REFERENCE;
        flags |= superClass->GetFlags() & COPIED_MASK;
        ASSERT(!superClass->IsValueTyped() || superClass->IsEtsEnum());
    }
    if (UNLIKELY(HasFunctionTypeInSuperClasses(this))) {
        flags |= IS_FUNCTION;
    }
    if (UNLIKELY(GetBase() != nullptr && GetBase()->IsEtsEnum())) {
        flags |= (IS_ETS_ENUM | IS_VALUE_TYPED);
    }
    if (UNLIKELY(GetRuntimeClass()->IsXRefClass())) {
        flags |= IS_VALUE_TYPED;
    }

    auto *runtimeClass = GetRuntimeClass();
    auto *pfile = runtimeClass->GetPandaFile();
    if (pfile != nullptr) {
        panda_file::ClassDataAccessor cda(*pfile, runtimeClass->GetFileId());
        cda.EnumerateAnnotations([this, &pfile, &flags, &errorHandler](panda_file::File::EntityId annotationId) {
            panda_file::AnnotationDataAccessor ada(*pfile, annotationId);
            auto *annotationName = pfile->GetStringData(ada.GetClassId()).data;
            auto *annotationModuleName = panda_file_items::class_descriptors::ANNOTATION_MODULE.data();
            auto *annotationFunctionalReferenceName =
                panda_file_items::class_descriptors::ANNOTATION_FUNCTIONAL_REFERENCE.data();
            if (utf::IsEqual(utf::CStringAsMutf8(annotationModuleName), annotationName)) {
                flags |= IS_MODULE;
            } else if (utf::IsEqual(utf::CStringAsMutf8(annotationFunctionalReferenceName), annotationName)) {
                flags |= (IS_FUNCTION_REFERENCE | IS_VALUE_TYPED);
                FunctionalReferenceAnnotationCallBack(this, pfile, &ada, errorHandler);
            }
        });
    }
    SetFlags(flags);
}

void EtsClass::SetComponentType(EtsClass *componentType)
{
    if (componentType == nullptr) {
        GetRuntimeClass()->SetComponentType(nullptr);
        return;
    }
    GetRuntimeClass()->SetComponentType(componentType->GetRuntimeClass());
}

EtsClass *EtsClass::GetComponentType() const
{
    ark::Class *componentType = GetRuntimeClass()->GetComponentType();
    if (componentType == nullptr) {
        return nullptr;
    }
    return FromRuntimeClass(componentType);
}

void EtsClass::SetLinker(ObjectHeader *linker)
{
    GetObjectHeader()->SetFieldObject(MEMBER_OFFSET(EtsClass, linker_), linker);
}

void EtsClass::SetName(EtsString *name)
{
    GetObjectHeader()->SetFieldObject(GetNameOffset(), reinterpret_cast<ObjectHeader *>(name));
}

bool EtsClass::CompareAndSetName(EtsString *oldName, EtsString *newName)
{
    return GetObjectHeader()->CompareAndSetFieldObject(GetNameOffset(), reinterpret_cast<ObjectHeader *>(oldName),
                                                       reinterpret_cast<ObjectHeader *>(newName),
                                                       std::memory_order::memory_order_seq_cst, true);
}

EtsField *EtsClass::GetFieldIDByName(const char *name, const char *sig)
{
    ASSERT(name != nullptr);
    auto u8name = reinterpret_cast<const uint8_t *>(name);
    auto *field = reinterpret_cast<EtsField *>(GetRuntimeClass()->GetInstanceFieldByName(u8name));
    if (sig != nullptr && field != nullptr) {
        if (strcmp(field->GetTypeDescriptor(), sig) != 0) {
            return nullptr;
        }
    }

    return field;
}

uint32_t EtsClass::GetFieldIndexByName(const char *name)
{
    auto u8name = ark::utf::CStringAsMutf8(name);
    auto fields = GetFields();
    panda_file::File::StringData sd = {static_cast<uint32_t>(ark::utf::MUtf8ToUtf16Size(u8name)), u8name};
    for (uint32_t i = 0; i < GetFieldsNumber(); i++) {
        if (fields[i]->GetCoreType()->GetName() == sd) {
            return i;
        }
    }
    return -1;
}

EtsField *EtsClass::GetStaticFieldIDByName(const char *name, const char *sig)
{
    auto u8name = reinterpret_cast<const uint8_t *>(name);
    auto *field = reinterpret_cast<EtsField *>(GetRuntimeClass()->GetStaticFieldByName(u8name));

    if (sig != nullptr && field != nullptr) {
        if (strcmp(field->GetTypeDescriptor(), sig) != 0) {
            return nullptr;
        }
    }

    return field;
}

EtsField *EtsClass::GetDeclaredFieldIDByName(std::string_view name)
{
    return reinterpret_cast<EtsField *>(GetRuntimeClass()->FindDeclaredField([name](const ark::Field &field) -> bool {
        auto *efield = EtsField::FromRuntimeField(&field);
        ASSERT(efield != nullptr);
        return std::string_view(efield->GetName()) == name;
    }));
}

EtsField *EtsClass::GetFieldIDByOffset(uint32_t fieldOffset)
{
    auto pred = [fieldOffset](const ark::Field &f) { return f.GetOffset() == fieldOffset; };
    return reinterpret_cast<EtsField *>(GetRuntimeClass()->FindInstanceField(pred));
}

EtsField *EtsClass::GetStaticFieldIDByOffset(uint32_t fieldOffset)
{
    auto pred = [fieldOffset](const ark::Field &f) { return f.GetOffset() == fieldOffset; };
    return reinterpret_cast<EtsField *>(GetRuntimeClass()->FindStaticField(pred));
}

EtsClass *EtsClass::GetBase()
{
    auto *base = GetRuntimeClass()->GetBase();
    if (base == nullptr) {
        return nullptr;
    }
    return FromRuntimeClass(base);
}

void EtsClass::GetInterfaces(PandaUnorderedSet<EtsClass *> &ifaces, EtsClass *iface)
{
    ifaces.insert(iface);
    EnumerateDirectInterfaces([&](EtsClass *runtimeInterface) {
        if (ifaces.find(runtimeInterface) == ifaces.end()) {
            runtimeInterface->GetInterfaces(ifaces, runtimeInterface);
        }
        return false;
    });
}

EtsObject *EtsClass::GetStaticFieldObject(EtsField *field)
{
    return reinterpret_cast<EtsObject *>(GetRuntimeClass()->GetFieldObject(*field->GetRuntimeField()));
}

EtsObject *EtsClass::GetStaticFieldObject(int32_t fieldOffset, bool isVolatile)
{
    if (isVolatile) {
        return reinterpret_cast<EtsObject *>(GetRuntimeClass()->GetFieldObject<true>(fieldOffset));
    }
    return reinterpret_cast<EtsObject *>(GetRuntimeClass()->GetFieldObject<false>(fieldOffset));
}

void EtsClass::SetStaticFieldObject(EtsField *field, EtsObject *value)
{
    GetRuntimeClass()->SetFieldObject(*field->GetRuntimeField(), reinterpret_cast<ObjectHeader *>(value));
}

void EtsClass::SetStaticFieldObject(int32_t fieldOffset, bool isVolatile, EtsObject *value)
{
    if (isVolatile) {
        GetRuntimeClass()->SetFieldObject<true>(fieldOffset, reinterpret_cast<ObjectHeader *>(value));
    }
    GetRuntimeClass()->SetFieldObject<false>(fieldOffset, reinterpret_cast<ObjectHeader *>(value));
}

EtsObject *EtsClass::CreateInstance()
{
    auto coro = EtsCoroutine::GetCurrent();
    const auto throwCreateInstanceErr = [coro, this](std::string_view msg) {
        ets::ThrowEtsException(coro, panda_file_items::class_descriptors::ERROR,
                               PandaString(msg) + " " + GetDescriptor());
    };

    if (UNLIKELY(!GetRuntimeClass()->IsInstantiable() || IsArrayClass())) {
        throwCreateInstanceErr("Cannot instantiate");
        return nullptr;
    }

    if (IsStringClass()) {
        auto emptyString = EtsString::CreateNewEmptyString();
        ASSERT(emptyString != nullptr);
        return emptyString->AsObject();
    }

    EtsMethod *ctor = GetDirectMethod(panda_file_items::CTOR.data(), ":V");
    if (UNLIKELY(ctor == nullptr) || !ctor->IsPublic()) {
        throwCreateInstanceErr("No default public constructor in");
        return nullptr;
    }

    EtsClassLinker *linker = coro->GetPandaVM()->GetClassLinker();
    if (UNLIKELY(!IsInitialized() && !linker->InitializeClass(coro, this))) {
        return nullptr;
    }
    EtsObject *obj = EtsObject::Create(this);
    if (UNLIKELY(obj == nullptr)) {
        return nullptr;
    }

    LocalObjectHandle objHandle(coro, obj);
    std::array<Value, 1> args {Value(obj->GetCoreType())};
    ctor->GetPandaMethod()->Invoke(coro, args.data());
    if (UNLIKELY(coro->HasPendingException())) {
        return nullptr;
    }
    return objHandle.GetPtr();
}

EtsClass *EtsClass::ResolveStringClass()
{
    ASSERT(IsStringClass());
    auto *coreClass = GetRuntimeClass();
    if (coreClass->IsLineStringClass() || coreClass->IsSlicedStringClass() || coreClass->IsTreeStringClass()) {
        EtsCoroutine *coroutine = EtsCoroutine::GetCurrent();
        ASSERT(coroutine != nullptr);
        const EtsPlatformTypes *ptypes = PlatformTypes(coroutine);
        return ptypes->coreString;
    }
    return this;
}

EtsClass *EtsClass::ResolvePublicClass()
{
    if (IsStringClass()) {
        return ResolveStringClass();
    }
    return this;
}

}  // namespace ark::ets
