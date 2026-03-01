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

#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "common_interfaces/objects/string/base_string-inl.h"
#include "runtime/include/method.h"
#include "runtime/include/thread_scopes.h"
#include "libarkbase/macros.h"
#include "libarkbase/utils/logger.h"
#include "plugins/ets/runtime/ets_annotation.h"
#include "plugins/ets/runtime/ets_class_linker_context.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/ets_vtable_builder.h"
#include "plugins/ets/runtime/ani/ani_helpers.h"
#include "plugins/ets/runtime/types/ets_abc_runtime_linker.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "runtime/class_linker_context.h"
#include "runtime/include/class_linker_extension.h"
#include "runtime/include/class_linker-inl.h"
#include "runtime/include/language_context.h"
#include "runtime/include/mem/panda_string.h"
#include "runtime/include/panda_vm.h"
#include "runtime/mem/heap_manager.h"
#include "plugins/ets/runtime/entrypoints/entrypoints.h"

namespace ark::ets {
namespace {
enum class EtsNapiType {
    GENERIC,  // - Switches the coroutine to native mode (GC is allowed)
              // - Prepends the argument list with two additional arguments (NAPI environment and this / class object)

    FAST,  // - Leaves the coroutine in managed mode (GC is not allowed)
           // - Prepends the argument list with two additional arguments (NAPI environment and this / class object)
           // - !!! The native function should not make any allocations (GC may be triggered during an allocation)

    CRITICAL  // - Leaves the coroutine in managed mode (GC is not allowed)
              // - Passes the arguments as is (the callee method should be static)
              // - !!! The native function should not make any allocations (GC may be triggered during an allocation)
};
}  // namespace

extern "C" void EtsAsyncEntryPoint();

static EtsNapiType GetEtsNapiType(Method *method)
{
    EtsNapiType napiType = EtsNapiType::GENERIC;
    const panda_file::File &pf = *method->GetPandaFile();
    panda_file::MethodDataAccessor mda(pf, method->GetFileId());
    mda.EnumerateAnnotations([&pf, &napiType](panda_file::File::EntityId annId) {
        panda_file::AnnotationDataAccessor ada(pf, annId);
        const char *className = utf::Mutf8AsCString(pf.GetStringData(ada.GetClassId()).data);
        if (className == panda_file_items::class_descriptors::ANI_UNSAFE_QUICK) {
            napiType = EtsNapiType::FAST;
        } else if (className == panda_file_items::class_descriptors::ANI_UNSAFE_DIRECT) {
            napiType = EtsNapiType::CRITICAL;
        }
    });
    return napiType;
}

static std::string_view GetClassLinkerErrorDescriptor(ClassLinker::Error error)
{
    switch (error) {
        case ClassLinker::Error::CLASS_NOT_FOUND:
            return panda_file_items::class_descriptors::LINKER_CLASS_NOT_FOUND_ERROR;
        case ClassLinker::Error::FIELD_NOT_FOUND:
            return panda_file_items::class_descriptors::LINKER_UNRESOLVED_FIELD_ERROR;
        case ClassLinker::Error::METHOD_NOT_FOUND:
            return panda_file_items::class_descriptors::LINKER_UNRESOLVED_METHOD_ERROR;
        case ClassLinker::Error::NO_CLASS_DEF:
            return panda_file_items::class_descriptors::LINKER_UNRESOLVED_CLASS_ERROR;
        case ClassLinker::Error::CLASS_CIRCULARITY:
            return panda_file_items::class_descriptors::LINKER_TYPE_CIRCULARITY_ERROR;
        case ClassLinker::Error::OVERRIDES_FINAL:
        case ClassLinker::Error::MULTIPLE_OVERRIDE:
        case ClassLinker::Error::MULTIPLE_IMPLEMENT:
            return panda_file_items::class_descriptors::LINKER_METHOD_CONFLICT_ERROR;
        default:
            LOG(FATAL, CLASS_LINKER) << "Unhandled class linker error (" << helpers::ToUnderlying(error) << "): ";
            UNREACHABLE();
    }
}

void EtsClassLinkerExtension::ErrorHandler::OnError(ClassLinker::Error error, const PandaString &message)
{
    ThrowEtsException(EtsCoroutine::GetCurrent(), GetClassLinkerErrorDescriptor(error), message);
}

void EtsClassLinkerExtension::InitializeClassRoots()
{
    InitializeArrayClassRoot(ClassRoot::ARRAY_CLASS, ClassRoot::CLASS,
                             utf::Mutf8AsCString(langCtx_.GetClassArrayClassDescriptor()));

    InitializePrimitiveClassRoot(ClassRoot::V, panda_file::Type::TypeId::VOID, "V");
    InitializePrimitiveClassRoot(ClassRoot::U1, panda_file::Type::TypeId::U1, "Z");
    InitializePrimitiveClassRoot(ClassRoot::I8, panda_file::Type::TypeId::I8, "B");
    InitializePrimitiveClassRoot(ClassRoot::U8, panda_file::Type::TypeId::U8, "H");
    InitializePrimitiveClassRoot(ClassRoot::I16, panda_file::Type::TypeId::I16, "S");
    InitializePrimitiveClassRoot(ClassRoot::U16, panda_file::Type::TypeId::U16, "C");
    InitializePrimitiveClassRoot(ClassRoot::I32, panda_file::Type::TypeId::I32, "I");
    InitializePrimitiveClassRoot(ClassRoot::U32, panda_file::Type::TypeId::U32, "U");
    InitializePrimitiveClassRoot(ClassRoot::I64, panda_file::Type::TypeId::I64, "J");
    InitializePrimitiveClassRoot(ClassRoot::U64, panda_file::Type::TypeId::U64, "Q");
    InitializePrimitiveClassRoot(ClassRoot::F32, panda_file::Type::TypeId::F32, "F");
    InitializePrimitiveClassRoot(ClassRoot::F64, panda_file::Type::TypeId::F64, "D");
    InitializePrimitiveClassRoot(ClassRoot::TAGGED, panda_file::Type::TypeId::TAGGED, "A");

    InitializeArrayClassRoot(ClassRoot::ARRAY_U1, ClassRoot::U1, "[Z");
    InitializeArrayClassRoot(ClassRoot::ARRAY_I8, ClassRoot::I8, "[B");
    InitializeArrayClassRoot(ClassRoot::ARRAY_U8, ClassRoot::U8, "[H");
    InitializeArrayClassRoot(ClassRoot::ARRAY_I16, ClassRoot::I16, "[S");
    InitializeArrayClassRoot(ClassRoot::ARRAY_U16, ClassRoot::U16, "[C");
    InitializeArrayClassRoot(ClassRoot::ARRAY_I32, ClassRoot::I32, "[I");
    InitializeArrayClassRoot(ClassRoot::ARRAY_U32, ClassRoot::U32, "[U");
    InitializeArrayClassRoot(ClassRoot::ARRAY_I64, ClassRoot::I64, "[J");
    InitializeArrayClassRoot(ClassRoot::ARRAY_U64, ClassRoot::U64, "[Q");
    InitializeArrayClassRoot(ClassRoot::ARRAY_F32, ClassRoot::F32, "[F");
    InitializeArrayClassRoot(ClassRoot::ARRAY_F64, ClassRoot::F64, "[D");
    InitializeArrayClassRoot(ClassRoot::ARRAY_TAGGED, ClassRoot::TAGGED, "[A");
    InitializeArrayClassRoot(ClassRoot::ARRAY_STRING, ClassRoot::STRING,
                             utf::Mutf8AsCString(langCtx_.GetStringArrayClassDescriptor()));

    InitializeSyntheticClassRoot(ClassRoot::ANY, "Y");
    InitializeSyntheticClassRoot(ClassRoot::NEVER, "N");
}

Class *EtsClassLinkerExtension::CreateStringSubClass(const uint8_t *descriptor, Class *stringClass, ClassRoot type)
{
    ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();
    ClassLinkerContext *context = stringClass->GetLoadContext();

    uint32_t accessFlags = stringClass->GetAccessFlags() | ACC_FINAL;

    Span<Field> fields {};
    Span<Method> methodsSpan {};
    Span<Class *> interfacesSpan {};
    Class *subClass = classLinker->BuildClass(descriptor, true, accessFlags, methodsSpan, fields, stringClass,
                                              interfacesSpan, context, false);
    if (UNLIKELY(subClass == nullptr)) {
        LOG(FATAL, CLASS_LINKER) << "Cannot build string subclass '" << descriptor << "'";
        return nullptr;
    }

    subClass->SetState(Class::State::INITIALIZING);
    subClass->ClearHaveNoRefsInParentsFlag();
    subClass->SetStringClass();
    switch (type) {
        case ClassRoot::LINE_STRING: {
            subClass->SetLineStringClass();
            break;
        }

        case ClassRoot::SLICED_STRING: {
            // used for gc
            subClass->SetSlicedStringClass();
            subClass->SetRefFieldsNum(common::SlicedString::REF_FIELDS_COUNT, false);
            subClass->SetRefFieldsOffset(common::SlicedString::PARENT_OFFSET, false);
            (static_cast<BaseClass *>(subClass))->SetObjectSize(common::SlicedString::SIZE);
            break;
        }

        case ClassRoot::TREE_STRING: {
            // used for gc
            subClass->SetTreeStringClass();
            subClass->SetRefFieldsNum(common::TreeString::REF_FIELDS_COUNT, false);
            subClass->SetRefFieldsOffset(common::TreeString::LEFT_OFFSET, false);
            (static_cast<BaseClass *>(subClass))->SetObjectSize(common::TreeString::SIZE);
            break;
        }

        default:
            UNREACHABLE();
    }

    subClass->SetState(Class::State::INITIALIZED);
    return subClass;
}

const uint8_t *EtsClassLinkerExtension::GetStringClassDescriptor(ClassRoot strCls)
{
    switch (strCls) {
        case ClassRoot::LINE_STRING:
            return utf::CStringAsMutf8(panda_file_items::class_descriptors::LINE_STRING.data());

        case ClassRoot::SLICED_STRING:
            return utf::CStringAsMutf8(panda_file_items::class_descriptors::SLICED_STRING.data());

        case ClassRoot::TREE_STRING:
            return utf::CStringAsMutf8(panda_file_items::class_descriptors::TREE_STRING.data());

        default:
            UNREACHABLE();
    }
}

bool EtsClassLinkerExtension::InitializeStringClass([[maybe_unused]] Class *classClass)
{
    // 1. create StringClass
    auto *strCls = GetClassLinker()->GetClass(langCtx_.GetStringClassDescriptor(), false, GetBootContext());
    if (strCls == nullptr) {
        LOG(ERROR, CLASS_LINKER) << "Cannot create string class '" << langCtx_.GetStringClassDescriptor() << "'";
        return false;
    }
    strCls->RemoveFinal();
    strCls->SetStringClass();
    EtsClass::FromRuntimeClass(strCls)->AsObject()->GetCoreType()->SetClass(classClass);
    SetClassRoot(ClassRoot::STRING, strCls);

    // 2. create LineStringClass / SlicedStringClass / TreeStringClass
    auto first = static_cast<int>(ClassRoot::LINE_STRING);
    auto last = static_cast<int>(ClassRoot::TREE_STRING);
    for (auto i = first; i <= last; i++) {
        auto flag = static_cast<ClassRoot>(i);
        auto *cls = CreateStringSubClass(GetStringClassDescriptor(flag), strCls, flag);
        if (cls == nullptr) {
            LOG(ERROR, CLASS_LINKER) << "Cannot create sub string class '" << GetStringClassDescriptor(flag) << "'";
            return false;
        }
        cls->SetFinal();
        EtsClass::FromRuntimeClass(cls)->AsObject()->GetCoreType()->SetClass(classClass);
        SetClassRoot(flag, cls);
    }

    return true;
}

bool EtsClassLinkerExtension::InitializeImpl(bool compressedStringEnabled)
{
    // NOLINTNEXTLINE(google-build-using-namespace)
    using namespace panda_file_items::class_descriptors;

    auto *coroutine = ets::EtsCoroutine::GetCurrent();
    langCtx_ = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
    ASSERT(coroutine != nullptr);
    heapManager_ = coroutine->GetVM()->GetHeapManager();

    auto *objectClass = GetClassLinker()->GetClass(langCtx_.GetObjectClassDescriptor(), false, GetBootContext());
    if (objectClass == nullptr) {
        LOG(ERROR, CLASS_LINKER) << "Cannot create class '" << langCtx_.GetObjectClassDescriptor() << "'";
        return false;
    }
    SetClassRoot(ClassRoot::OBJECT, objectClass);

    auto *classClass = GetClassLinker()->GetClass(langCtx_.GetClassClassDescriptor(), false, GetBootContext());
    if (classClass == nullptr) {
        LOG(ERROR, CLASS_LINKER) << "Cannot create class class '" << langCtx_.GetClassClassDescriptor() << "'";
        return false;
    }
    SetClassRoot(ClassRoot::CLASS, classClass);

    EtsClass::FromRuntimeClass(classClass)->AsObject()->GetCoreType()->SetClass(classClass);
    EtsClass::FromRuntimeClass(objectClass)->AsObject()->GetCoreType()->SetClass(classClass);

    coretypes::String::SetCompressedStringsEnabled(compressedStringEnabled);

    // Set String Classes
    if (!InitializeStringClass(classClass)) {
        LOG(ERROR, CLASS_LINKER) << "Cannot create String classes";
        return false;
    }

    auto *jsValueClass = GetClassLinker()->GetClass(utf::CStringAsMutf8(JS_VALUE.data()), false, GetBootContext());
    if (jsValueClass == nullptr) {
        LOG(ERROR, CLASS_LINKER) << "Cannot create class '" << JS_VALUE << "'";
        return false;
    }
    jsValueClass->SetXRefClass();
    InitializeClassRoots();
    return true;
}

bool EtsClassLinkerExtension::InitializeArrayClass(Class *arrayClass, Class *componentClass)
{
    ASSERT(IsInitialized());

    ASSERT(!arrayClass->IsInitialized());
    ASSERT(arrayClass->GetComponentType() == nullptr);

    auto *objectClass = GetClassRoot(ClassRoot::OBJECT);
    arrayClass->SetBase(objectClass);
    EtsClass::FromRuntimeClass(arrayClass)->SetSuperClass(EtsClass::FromRuntimeClass(objectClass));
    arrayClass->SetComponentType(componentClass);

    auto accessFlags = componentClass->GetAccessFlags() & ACC_FILE_MASK;
    accessFlags &= ~ACC_INTERFACE;
    accessFlags |= ACC_FINAL | ACC_ABSTRACT;

    arrayClass->SetAccessFlags(accessFlags);

    auto objectClassVtable = objectClass->GetVTable();
    auto arrayClassVtable = arrayClass->GetVTable();
    for (size_t i = 0; i < objectClassVtable.size(); i++) {
        arrayClassVtable[i] = objectClassVtable[i];
    }

    arrayClass->SetState(Class::State::INITIALIZED);

    ASSERT(arrayClass->IsArrayClass());  // After init, we give out a well-formed array class.
    return true;
}

bool EtsClassLinkerExtension::InitializeUnionClass(Class *unionClass, Span<Class *> constituentClasses)
{
    ASSERT(IsInitialized());

    ASSERT(!unionClass->IsInitialized());
    ASSERT(unionClass->GetConstituentTypes().begin() == nullptr);

    unionClass->SetBase(nullptr);
    EtsClass::FromRuntimeClass(unionClass)->SetSuperClass(nullptr);
    unionClass->SetConstituentTypes(constituentClasses);
    unionClass->SetState(Class::State::INITIALIZED);

    ASSERT(unionClass->IsUnionClass());  // After init, we give out a well-formed union class.
    return true;
}

bool EtsClassLinkerExtension::InitializeClass(Class *klass)
{
    return InitializeClass(klass, GetErrorHandler());
}

bool EtsClassLinkerExtension::InitializeClass(Class *klass, [[maybe_unused]] ClassLinkerErrorHandler *handler)
{
    ASSERT(IsInitialized());
    ASSERT_HAVE_ACCESS_TO_MANAGED_OBJECTS();

    constexpr uint32_t ETS_ACCESS_FLAGS_MASK = 0xFFFFU;

    EtsClass::FromRuntimeClass(klass)->Initialize(
        klass->GetBase() != nullptr ? EtsClass::FromRuntimeClass(klass->GetBase()) : nullptr,
        klass->GetAccessFlags() & ETS_ACCESS_FLAGS_MASK, klass->IsPrimitive(), handler);

    return true;
}

void EtsClassLinkerExtension::InitializePrimitiveClass(Class *primitiveClass)
{
    ASSERT(IsInitialized());

    ASSERT(!primitiveClass->IsInitialized());
    ASSERT(primitiveClass->IsPrimitive());

    primitiveClass->SetAccessFlags(ACC_PUBLIC | ACC_FINAL | ACC_ABSTRACT);
    primitiveClass->SetState(Class::State::INITIALIZED);
}

void EtsClassLinkerExtension::InitializeSyntheticClass(Class *synClass)
{
    ASSERT(IsInitialized());

    ASSERT(!synClass->IsInitialized());

    synClass->SetAccessFlags(ACC_PUBLIC | ACC_FINAL | ACC_ABSTRACT);
    synClass->SetState(Class::State::INITIALIZED);
}

size_t EtsClassLinkerExtension::GetClassVTableSize(ClassRoot root)
{
    ASSERT(IsInitialized());

    switch (root) {
        case ClassRoot::V:
        case ClassRoot::U1:
        case ClassRoot::I8:
        case ClassRoot::U8:
        case ClassRoot::I16:
        case ClassRoot::U16:
        case ClassRoot::I32:
        case ClassRoot::U32:
        case ClassRoot::I64:
        case ClassRoot::U64:
        case ClassRoot::F32:
        case ClassRoot::F64:
        case ClassRoot::TAGGED:
        case ClassRoot::ANY:
        case ClassRoot::NEVER:
            return 0;
        case ClassRoot::ARRAY_U1:
        case ClassRoot::ARRAY_I8:
        case ClassRoot::ARRAY_U8:
        case ClassRoot::ARRAY_I16:
        case ClassRoot::ARRAY_U16:
        case ClassRoot::ARRAY_I32:
        case ClassRoot::ARRAY_U32:
        case ClassRoot::ARRAY_I64:
        case ClassRoot::ARRAY_U64:
        case ClassRoot::ARRAY_F32:
        case ClassRoot::ARRAY_F64:
        case ClassRoot::ARRAY_TAGGED:
        case ClassRoot::ARRAY_CLASS:
        case ClassRoot::ARRAY_STRING:
            return GetArrayClassVTableSize();
        case ClassRoot::OBJECT:
        case ClassRoot::STRING:
        case ClassRoot::LINE_STRING:
        case ClassRoot::SLICED_STRING:
        case ClassRoot::TREE_STRING:
            return GetClassRoot(root)->GetVTableSize();
        case ClassRoot::CLASS:
            return 0;
        default: {
            break;
        }
    }

    UNREACHABLE();
    return 0;
}

size_t EtsClassLinkerExtension::GetClassIMTSize(ClassRoot root)
{
    ASSERT(IsInitialized());

    switch (root) {
        case ClassRoot::V:
        case ClassRoot::U1:
        case ClassRoot::I8:
        case ClassRoot::U8:
        case ClassRoot::I16:
        case ClassRoot::U16:
        case ClassRoot::I32:
        case ClassRoot::U32:
        case ClassRoot::I64:
        case ClassRoot::U64:
        case ClassRoot::F32:
        case ClassRoot::F64:
        case ClassRoot::TAGGED:
        case ClassRoot::ANY:
        case ClassRoot::NEVER:
            return 0;
        case ClassRoot::ARRAY_U1:
        case ClassRoot::ARRAY_I8:
        case ClassRoot::ARRAY_U8:
        case ClassRoot::ARRAY_I16:
        case ClassRoot::ARRAY_U16:
        case ClassRoot::ARRAY_I32:
        case ClassRoot::ARRAY_U32:
        case ClassRoot::ARRAY_I64:
        case ClassRoot::ARRAY_U64:
        case ClassRoot::ARRAY_F32:
        case ClassRoot::ARRAY_F64:
        case ClassRoot::ARRAY_TAGGED:
        case ClassRoot::ARRAY_CLASS:
        case ClassRoot::ARRAY_STRING:
            return GetArrayClassIMTSize();
        case ClassRoot::OBJECT:
        case ClassRoot::CLASS:
        case ClassRoot::STRING:
        case ClassRoot::LINE_STRING:
        case ClassRoot::SLICED_STRING:
        case ClassRoot::TREE_STRING:
            return 0;
        default: {
            break;
        }
    }

    UNREACHABLE();
    return 0;
}

size_t EtsClassLinkerExtension::GetClassSize(ClassRoot root)
{
    ASSERT(IsInitialized());

    switch (root) {
        case ClassRoot::V:
        case ClassRoot::U1:
        case ClassRoot::I8:
        case ClassRoot::U8:
        case ClassRoot::I16:
        case ClassRoot::U16:
        case ClassRoot::I32:
        case ClassRoot::U32:
        case ClassRoot::I64:
        case ClassRoot::U64:
        case ClassRoot::F32:
        case ClassRoot::F64:
        case ClassRoot::ANY:
        case ClassRoot::NEVER:
        case ClassRoot::TAGGED:
            return Class::ComputeClassSize(GetClassVTableSize(root), GetClassIMTSize(root), 0, 0, 0, 0, 0, 0);
        case ClassRoot::ARRAY_U1:
        case ClassRoot::ARRAY_I8:
        case ClassRoot::ARRAY_U8:
        case ClassRoot::ARRAY_I16:
        case ClassRoot::ARRAY_U16:
        case ClassRoot::ARRAY_I32:
        case ClassRoot::ARRAY_U32:
        case ClassRoot::ARRAY_I64:
        case ClassRoot::ARRAY_U64:
        case ClassRoot::ARRAY_F32:
        case ClassRoot::ARRAY_F64:
        case ClassRoot::ARRAY_TAGGED:
        case ClassRoot::ARRAY_CLASS:
        case ClassRoot::ARRAY_STRING:
            return GetArrayClassSize();
        case ClassRoot::OBJECT:
        case ClassRoot::CLASS:
        case ClassRoot::STRING:
        case ClassRoot::LINE_STRING:
        case ClassRoot::SLICED_STRING:
        case ClassRoot::TREE_STRING:
            return Class::ComputeClassSize(GetClassVTableSize(root), GetClassIMTSize(root), 0, 0, 0, 0, 0, 0);
        default: {
            break;
        }
    }

    UNREACHABLE();
    return 0;
}

size_t EtsClassLinkerExtension::GetArrayClassVTableSize()
{
    ASSERT(IsInitialized());

    return GetClassVTableSize(ClassRoot::OBJECT);
}

size_t EtsClassLinkerExtension::GetArrayClassIMTSize()
{
    ASSERT(IsInitialized());

    return GetClassIMTSize(ClassRoot::OBJECT);
}

size_t EtsClassLinkerExtension::GetArrayClassSize()
{
    ASSERT(IsInitialized());

    return GetClassSize(ClassRoot::OBJECT);
}

Class *EtsClassLinkerExtension::InitializeClass(ObjectHeader *objectHeader, const uint8_t *descriptor,
                                                size_t vtableSize, size_t imtSize, size_t size)
{
    auto managedClass = reinterpret_cast<EtsClass *>(objectHeader);
    ASSERT(managedClass != nullptr);
    managedClass->InitClass(descriptor, vtableSize, imtSize, size);
    auto klass = managedClass->GetRuntimeClass();
    klass->SetManagedObject(objectHeader);
    klass->SetSourceLang(GetLanguage());

    AddCreatedClass(klass);

    return klass;
}

Class *EtsClassLinkerExtension::CreateClass(const uint8_t *descriptor, size_t vtableSize, size_t imtSize, size_t size)
{
    ASSERT(IsInitialized());

    Class *classClassRoot = GetClassRoot(ClassRoot::CLASS);
    ObjectHeader *classObject;
    if (UNLIKELY(classClassRoot == nullptr)) {
        ASSERT(utf::IsEqual(descriptor, langCtx_.GetObjectClassDescriptor()) ||
               utf::IsEqual(descriptor, langCtx_.GetClassClassDescriptor()));
        classObject = heapManager_->AllocateNonMovableObject<true>(classClassRoot, EtsClass::GetSize(size));
    } else {
        classObject = heapManager_->AllocateNonMovableObject<false>(classClassRoot, EtsClass::GetSize(size));
    }
    if (UNLIKELY(classObject == nullptr)) {
        return nullptr;
    }

    return InitializeClass(classObject, descriptor, vtableSize, imtSize, size);
}

void EtsClassLinkerExtension::FreeClass(Class *klass)
{
    ASSERT(IsInitialized());

    RemoveCreatedClass(klass);
}

EtsClassLinkerExtension::~EtsClassLinkerExtension()
{
    if (!IsInitialized()) {
        return;
    }

    FreeLoadedClasses();
}

bool EtsClassLinkerExtension::IsMethodNativeApi(const Method *method) const
{
    // intrinsics and async methods are marked as native, but they are not native api calls
    return method->IsNative() && !method->IsIntrinsic() && !EtsAnnotation::FindAsyncAnnotation(method).IsValid();
}

bool EtsClassLinkerExtension::CanThrowException(const Method *method) const
{
    if (!method->IsNative()) {
        return true;
    }

    const EtsMethod *etsMethod = EtsMethod::FromRuntimeMethod(method);
    return !etsMethod->IsCriticalNative();
}

bool EtsClassLinkerExtension::IsNecessarySwitchThreadState(const Method *method) const
{
    if (!IsMethodNativeApi(method)) {
        return false;
    }

    const EtsMethod *etsMethod = EtsMethod::FromRuntimeMethod(method);
    return !(etsMethod->IsFastNative() || etsMethod->IsCriticalNative());
}

bool EtsClassLinkerExtension::CanNativeMethodUseObjects(const Method *method) const
{
    if (!IsMethodNativeApi(method)) {
        return false;
    }

    const EtsMethod *etsMethod = EtsMethod::FromRuntimeMethod(method);
    return !etsMethod->IsCriticalNative();
}

const void *EtsClassLinkerExtension::GetNativeEntryPointFor(Method *method) const
{
    panda_file::File::EntityId asyncAnnId = EtsAnnotation::FindAsyncAnnotation(method);
    if (asyncAnnId.IsValid()) {
        return reinterpret_cast<const void *>(EtsAsyncEntryPoint);
    }
    switch (GetEtsNapiType(method)) {
        case EtsNapiType::GENERIC: {
            return ani::GetANIEntryPoint();
        }
        case EtsNapiType::FAST: {
            auto flags = method->GetAccessFlags();
            flags |= ACC_FAST_NATIVE;
            method->SetAccessFlags(flags);

            return ani::GetANIEntryPoint();
        }
        case EtsNapiType::CRITICAL: {
            auto flags = method->GetAccessFlags();
            flags |= ACC_CRITICAL_NATIVE;
            method->SetAccessFlags(flags);

            return ani::GetANICriticalEntryPoint();
        }
    }

    UNREACHABLE();
}

Class *EtsClassLinkerExtension::FromClassObject(ark::ObjectHeader *obj)
{
    return obj != nullptr ? reinterpret_cast<EtsClass *>(obj)->GetRuntimeClass() : nullptr;
}

size_t EtsClassLinkerExtension::GetClassObjectSizeFromClassSize(uint32_t size)
{
    return EtsClass::GetSize(size);
}

Class *EtsClassLinkerExtension::CacheClass(std::string_view descriptor, bool forceInit)
{
    Class *cls = GetClassLinker()->GetClass(utf::CStringAsMutf8(descriptor.data()), false, GetBootContext());
    if (cls == nullptr) {
        LOG(ERROR, CLASS_LINKER) << "Cannot create class " << descriptor;
        return nullptr;
    }
    if (forceInit && !GetClassLinker()->InitializeClass(EtsCoroutine::GetCurrent(), cls)) {
        LOG(ERROR, CLASS_LINKER) << "Cannot initialize class " << descriptor;
        return nullptr;
    }
    return cls;
}

template <typename F>
Class *EtsClassLinkerExtension::CacheClass(std::string_view descriptor, F const &setup, bool forceInit)
{
    Class *cls = CacheClass(descriptor, forceInit);
    if (cls != nullptr) {
        setup(EtsClass::FromRuntimeClass(cls));
    }
    return cls;
}

void EtsClassLinkerExtension::InitializeBuiltinSpecialClasses()
{
    CacheClass("Lstd/core/String;", [](auto *c) { c->SetValueTyped(); });
    CacheClass("Lstd/core/LineString;", [](auto *c) { c->SetValueTyped(); });
    CacheClass("Lstd/core/SlicedString;", [](auto *c) { c->SetValueTyped(); });
    CacheClass("Lstd/core/TreeString;", [](auto *c) { c->SetValueTyped(); });
    CacheClass("Lstd/core/Null;", [](auto *c) {
        c->SetNullValue();
        c->SetValueTyped();
    });

    CacheClass("Lstd/core/Boolean;", [](auto *c) { c->SetBoxedKind(EtsClass::BoxedType::BOOLEAN); });
    CacheClass("Lstd/core/Byte;", [](auto *c) { c->SetBoxedKind(EtsClass::BoxedType::BYTE); });
    CacheClass("Lstd/core/Char;", [](auto *c) { c->SetBoxedKind(EtsClass::BoxedType::CHAR); });
    CacheClass("Lstd/core/Short;", [](auto *c) { c->SetBoxedKind(EtsClass::BoxedType::SHORT); });
    CacheClass("Lstd/core/Int;", [](auto *c) { c->SetBoxedKind(EtsClass::BoxedType::INT); });
    CacheClass("Lstd/core/Long;", [](auto *c) { c->SetBoxedKind(EtsClass::BoxedType::LONG); });
    CacheClass("Lstd/core/Float;", [](auto *c) { c->SetBoxedKind(EtsClass::BoxedType::FLOAT); });
    CacheClass("Lstd/core/Double;", [](auto *c) { c->SetBoxedKind(EtsClass::BoxedType::DOUBLE); });

    CacheClass("Lstd/core/BigInt;", [](auto *c) {
        c->SetBigInt();
        c->SetValueTyped();
    });
    CacheClass("Lstd/core/Function;", [](auto *c) { c->SetFunction(); });
    CacheClass("Lstd/core/BaseEnum;", [](auto *c) {
        c->SetEtsEnum();
        c->SetValueTyped();
    });

    CacheClass("Lstd/core/FinalizableWeakRef;", [](auto *c) {
        c->SetFinalizeReference();
        c->SetWeakReference();
    });
    CacheClass("Lstd/core/BaseWeakRef;", [](auto *c) { c->SetWeakReference(); });
}

void EtsClassLinkerExtension::InitializeBuiltinClasses()
{
    // NOLINTNEXTLINE(google-build-using-namespace)
    using namespace panda_file_items::class_descriptors;

    InitializeBuiltinSpecialClasses();

    auto *coro = EtsCoroutine::GetCurrent();
    plaformTypes_ =
        PandaUniquePtr<EtsPlatformTypes>(Runtime::GetCurrent()->GetInternalAllocator()->New<EtsPlatformTypes>(coro));

    // NOTE (electronick, #15938): Refactor the managed class-related pseudo TLS fields
    // initialization in MT ManagedThread ctor and EtsCoroutine::Initialize
    coro->SetPromiseClass(GetPlatformTypes()->corePromise->GetRuntimeClass());
    coro->SetJobClass(GetPlatformTypes()->coreJob->GetRuntimeClass());
    coro->SetStringClassPtr(GetClassRoot(ClassRoot::LINE_STRING));
    coro->SetArrayU16ClassPtr(GetClassRoot(ClassRoot::ARRAY_U16));
    coro->SetArrayU8ClassPtr(GetClassRoot(ClassRoot::ARRAY_U8));
}

void EtsClassLinkerExtension::InitializeFinish()
{
    plaformTypes_->InitializeClasses(EtsCoroutine::GetCurrent());
}

ClassLinkerContext *EtsClassLinkerExtension::CreateApplicationClassLinkerContext(const PandaVector<PandaString> &path)
{
    ClassLinkerContext *ctx = PandaEtsVM::GetCurrent()->CreateApplicationRuntimeLinker(path);
    ASSERT(ctx != nullptr);
    return ctx;
}

/* static */
ClassLinkerContext *EtsClassLinkerExtension::GetParentContext(ClassLinkerContext *ctx)
{
    // Boot context is the root, no need to get parent from it
    if (ctx->IsBootContext()) {
        return nullptr;
    }
    auto *etsLinkerContext = EtsClassLinkerContext::FromCoreType(ctx);
    auto *linker = etsLinkerContext->GetRuntimeLinker();
    ASSERT(linker != nullptr);
    // NOTE(dslynko, #30466): remove this assertion after moving `parentLinker` to `RuntimeLinker`
    ASSERT(linker->GetClass()->IsAssignableFrom(PlatformTypes()->coreAbcRuntimeLinker));
    auto *abcRuntimeLinker = EtsAbcRuntimeLinker::FromEtsObject(linker);
    auto *parentLinker = abcRuntimeLinker->GetParentLinker();
    return parentLinker->GetClassLinkerContext();
}

ClassLinkerContext *EtsClassLinkerExtension::GetCommonContext(Span<Class *> classes)
{
    ASSERT(!classes.Empty());
    auto *commonCtx = classes[0]->GetLoadContext();

    size_t foundClassesNum = 0;
    while (!commonCtx->IsBootContext()) {
        for (auto *klass : classes) {
            if (commonCtx->FindClass(klass->GetDescriptor()) != nullptr) {
                foundClassesNum++;
            }
        }
        if (foundClassesNum == classes.Size()) {
            break;
        }
        commonCtx = GetParentContext(commonCtx);
    }
    return commonCtx;
}

std::optional<PandaVector<Method *>> EtsClassLinkerExtension::BuildProxyClassMethodsSpan(ITable itable)
{
    constexpr size_t PROXY_BASE_CLASS_NUM_METHODS = 2U;

    size_t methodsNum = PROXY_BASE_CLASS_NUM_METHODS;
    for (auto &entry : itable.Get()) {
        methodsNum += entry.GetInterface()->GetMethods().Size();
    }

    auto *allocator = mem::InternalAllocator<>::GetInternalAllocatorFromRuntime();
    PandaVector<Method *> methods(methodsNum, allocator->Adapter());

    size_t idx = 0;
    methods[idx++] = GetPlatformTypes()->coreReflectProxyConstructor->GetPandaMethod();
    methods[idx++] = GetPlatformTypes()->coreReflectProxyGetHandler->GetPandaMethod();
    ASSERT(idx == PROXY_BASE_CLASS_NUM_METHODS);

    for (auto &entry : itable.Get()) {
        for (auto &method : entry.GetInterface()->GetMethods()) {
            methods[idx++] = &method;
        }
    }
    ASSERT(idx == methodsNum);

    return methods;
}

static void CreateProxyMethodAsCopyFromBase(Class *klass, EtsMethod *method, Method *out)
{
    ASSERT(klass != nullptr);
    ASSERT(out != nullptr);
    ASSERT(method != nullptr);

    new (out) Method(EtsMethod::ToRuntimeMethod(method));
    out->SetClass(klass);

    out->SetAccessFlags((out->GetAccessFlags() & ~ACC_PROTECTED) | ACC_PUBLIC);
    // NOTE(kurnevichstanislav): possibly need additional support of some cases #30568.
}

static void CreateProxyMethod(Class *klass, EtsMethod *method, Method *out)
{
    ASSERT(klass != nullptr);
    ASSERT(method != nullptr);
    ASSERT(out != nullptr);

    new (out) Method(method->GetPandaMethod());

    constexpr uint32_t FLAGS_TO_REMOVE = ACC_ABSTRACT | ACC_DEFAULT_INTERFACE_METHOD;

    ASSERT(out->IsPublic());
    out->SetAccessFlags((out->GetAccessFlags() & ~FLAGS_TO_REMOVE) | ACC_FINAL);
    out->SetClass(klass);
    out->SetCompiledEntryPoint(entrypoints::GetEtsProxyEntryPoint());

    // Set original method of interface for which we create proxy method.
    // This pointer will be passed to the proxy handler invoke/set/get.
    EtsMethod::FromRuntimeMethod(out)->SetProxyPointer(method);
}

std::optional<Span<Method>> EtsClassLinkerExtension::GenerateProxyClassMethods(Class *proxyKlass,
                                                                               Span<Method *> rawMethods)
{
    ASSERT(proxyKlass != nullptr);

    auto *allocator = mem::InternalAllocator<>::GetInternalAllocatorFromRuntime();
    Span<Method> proxyMethods(allocator->AllocArray<Method>(rawMethods.Size()), rawMethods.Size());

    auto *ctor = GetPlatformTypes()->coreReflectProxyConstructor;
    auto *getHandler = GetPlatformTypes()->coreReflectProxyGetHandler;

    for (size_t idx = 0; idx < rawMethods.Size(); ++idx) {
        if (rawMethods[idx] == ctor->GetPandaMethod()) {
            ASSERT(rawMethods[idx]->IsConstructor());
            CreateProxyMethodAsCopyFromBase(proxyKlass, ctor, &proxyMethods[idx]);
        } else if (rawMethods[idx] == getHandler->GetPandaMethod()) {
            CreateProxyMethodAsCopyFromBase(proxyKlass, getHandler, &proxyMethods[idx]);
        } else {
            CreateProxyMethod(proxyKlass, EtsMethod::FromRuntimeMethod(rawMethods[idx]), &proxyMethods[idx]);
        }
    }

    return proxyMethods;
}

}  // namespace ark::ets
