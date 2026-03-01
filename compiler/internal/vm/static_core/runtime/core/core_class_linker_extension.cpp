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

#include "runtime/core/core_class_linker_extension.h"

#include "include/class_root.h"
#include "runtime/include/coretypes/class.h"
#include "runtime/include/exceptions.h"
#include "runtime/include/panda_vm.h"
#include "libarkbase/utils/utf.h"

namespace ark {

using SourceLang = panda_file::SourceLang;
using Type = panda_file::Type;

void CoreClassLinkerExtension::ErrorHandler::OnError(ClassLinker::Error error, const PandaString &message)
{
    auto *thread = ManagedThread::GetCurrent();
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);

    switch (error) {
        case ClassLinker::Error::CLASS_NOT_FOUND: {
            ThrowException(ctx, thread, ctx.GetClassNotFoundExceptionDescriptor(),
                           utf::CStringAsMutf8(message.c_str()));
            break;
        }
        case ClassLinker::Error::FIELD_NOT_FOUND: {
            ThrowException(ctx, thread, ctx.GetNoSuchFieldErrorDescriptor(), utf::CStringAsMutf8(message.c_str()));
            break;
        }
        case ClassLinker::Error::METHOD_NOT_FOUND: {
            ThrowException(ctx, thread, ctx.GetNoSuchMethodErrorDescriptor(), utf::CStringAsMutf8(message.c_str()));
            break;
        }
        case ClassLinker::Error::NO_CLASS_DEF: {
            ThrowException(ctx, thread, ctx.GetNoClassDefFoundErrorDescriptor(), utf::CStringAsMutf8(message.c_str()));
            break;
        }
        case ClassLinker::Error::CLASS_CIRCULARITY: {
            ThrowException(ctx, thread, ctx.GetClassCircularityErrorDescriptor(), utf::CStringAsMutf8(message.c_str()));
            break;
        }
        default:
            LOG(FATAL, CLASS_LINKER) << "Unhandled error (" << static_cast<size_t>(error) << "): " << message;
            break;
    }
}

void CoreClassLinkerExtension::InitializeClassRoots(const LanguageContext &ctx)
{
    InitializePrimitiveClassRoot(ClassRoot::U1, Type::TypeId::U1, "Z");
    InitializePrimitiveClassRoot(ClassRoot::I8, Type::TypeId::I8, "B");
    InitializePrimitiveClassRoot(ClassRoot::U8, Type::TypeId::U8, "H");
    InitializePrimitiveClassRoot(ClassRoot::I16, Type::TypeId::I16, "S");
    InitializePrimitiveClassRoot(ClassRoot::U16, Type::TypeId::U16, "C");
    InitializePrimitiveClassRoot(ClassRoot::I32, Type::TypeId::I32, "I");
    InitializePrimitiveClassRoot(ClassRoot::U32, Type::TypeId::U32, "U");
    InitializePrimitiveClassRoot(ClassRoot::I64, Type::TypeId::I64, "J");
    InitializePrimitiveClassRoot(ClassRoot::U64, Type::TypeId::U64, "Q");
    InitializePrimitiveClassRoot(ClassRoot::F32, Type::TypeId::F32, "F");
    InitializePrimitiveClassRoot(ClassRoot::F64, Type::TypeId::F64, "D");
    InitializePrimitiveClassRoot(ClassRoot::TAGGED, Type::TypeId::TAGGED, "A");

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
                             utf::Mutf8AsCString(ctx.GetStringArrayClassDescriptor()));

    InitializeSyntheticClassRoot(ClassRoot::ANY, "Y");
    InitializeSyntheticClassRoot(ClassRoot::NEVER, "N");
}

void CoreClassLinkerExtension::FillStringClass(Class *strCls, ClassRoot flag)
{
    strCls->SetState(Class::State::INITIALIZING);
    switch (flag) {
        case ClassRoot::LINE_STRING: {
            strCls->SetLineStringClass();
            break;
        }
        case ClassRoot::SLICED_STRING: {
            // used for gc
            strCls->SetSlicedStringClass();
            strCls->SetRefFieldsNum(common::SlicedString::REF_FIELDS_COUNT, false);
            strCls->SetRefFieldsOffset(common::SlicedString::PARENT_OFFSET, false);
            (static_cast<BaseClass *>(strCls))->SetObjectSize(common::SlicedString::SIZE);
            break;
        }
        case ClassRoot::TREE_STRING: {
            // used for gc
            strCls->SetTreeStringClass();
            strCls->SetRefFieldsNum(common::TreeString::REF_FIELDS_COUNT, false);
            strCls->SetRefFieldsOffset(common::TreeString::LEFT_OFFSET, false);
            (static_cast<BaseClass *>(strCls))->SetObjectSize(common::TreeString::SIZE);
            break;
        }
        default: {
            UNREACHABLE();
        }
    }
    strCls->SetStringClass();
    strCls->SetState(Class::State::INITIALIZED);
    strCls->SetFinal();
}

const uint8_t *CoreClassLinkerExtension::GetStringClassDescriptor(ClassRoot flag)
{
    switch (flag) {
        case ClassRoot::LINE_STRING:
            return utf::CStringAsMutf8("Lpanda/LineString;");

        case ClassRoot::SLICED_STRING:
            return utf::CStringAsMutf8("Lpanda/SlicedString;");

        case ClassRoot::TREE_STRING:
            return utf::CStringAsMutf8("Lpanda/TreeString;");

        default:
            UNREACHABLE();
    }
}

bool CoreClassLinkerExtension::InitializeStringClass()
{
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(GetLanguage());
    auto *objCls = GetClassRoot(ClassRoot::OBJECT);
    // 1. create StringClass
    ClassRoot flag = ClassRoot::STRING;
    auto *strCls = CreateClass(ctx.GetStringClassDescriptor(), GetClassVTableSize(flag), GetClassIMTSize(flag),
                               GetClassSize(flag));
    if (strCls == nullptr) {
        LOG(ERROR, CLASS_LINKER) << "Cannot create string class '" << ctx.GetStringClassDescriptor() << "'";
        return false;
    }
    strCls->SetBase(objCls);
    strCls->SetStringClass();
    strCls->RemoveFinal();
    strCls->SetState(Class::State::LOADED);
    strCls->SetLoadContext(GetBootContext());
    GetClassLinker()->AddClassRoot(flag, strCls);

    // 2. create LineStringClass / SlicedStringClass / TreeStringClass
    uint32_t accessFlags = strCls->GetAccessFlags() | ACC_FINAL;
    Span<Field> fields {};
    Span<Method> methodsSpan {};
    Span<Class *> interfacesSpan {};
    auto first = static_cast<int>(ClassRoot::LINE_STRING);
    auto last = static_cast<int>(ClassRoot::TREE_STRING);
    for (auto i = first; i <= last; i++) {
        flag = static_cast<ClassRoot>(i);
        const uint8_t *descriptor = GetStringClassDescriptor(flag);
        Class *subStrCls = GetClassLinker()->BuildClass(descriptor, true, accessFlags, methodsSpan, fields, strCls,
                                                        interfacesSpan, GetBootContext(), false);
        if (subStrCls == nullptr) {
            LOG(ERROR, CLASS_LINKER) << "Cannot create string class '" << descriptor << "'";
            return false;
        }
        FillStringClass(subStrCls, flag);
        GetClassLinker()->AddClassRoot(flag, subStrCls);
    }
    strCls->CalcHaveNoRefsInParents();
    return true;
}

bool CoreClassLinkerExtension::InitializeImpl(bool compressedStringEnabled)
{
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(GetLanguage());

    auto *classClass = CreateClass(ctx.GetClassClassDescriptor(), GetClassVTableSize(ClassRoot::CLASS),
                                   GetClassIMTSize(ClassRoot::CLASS), GetClassSize(ClassRoot::CLASS));
    coretypes::Class::FromRuntimeClass(classClass)->SetClass(classClass);
    classClass->SetState(Class::State::LOADED);
    classClass->SetLoadContext(GetBootContext());
    GetClassLinker()->AddClassRoot(ClassRoot::CLASS, classClass);

    auto *objClass = GetClassLinker()->GetClass(ctx.GetObjectClassDescriptor(), false, GetBootContext());
    if (objClass == nullptr) {  // Happens when we work without pandastdlib
        objClass = CreateClass(ctx.GetObjectClassDescriptor(), GetClassVTableSize(ClassRoot::OBJECT),
                               GetClassIMTSize(ClassRoot::OBJECT), GetClassSize(ClassRoot::OBJECT));
        objClass->CalcHaveNoRefsInParents();
        objClass->SetObjectSize(ObjectHeader::ObjectHeaderSize());
        objClass->SetState(Class::State::LOADED);
        objClass->SetLoadContext(GetBootContext());
        GetClassLinker()->AddClassRoot(ClassRoot::OBJECT, objClass);
    } else {
        SetClassRoot(ClassRoot::OBJECT, objClass);
    }
    classClass->SetBase(objClass);
    classClass->CalcHaveNoRefsInParents();

    coretypes::String::SetCompressedStringsEnabled(compressedStringEnabled);
    if (!InitializeStringClass()) {
        LOG(ERROR, CLASS_LINKER) << "Cannot create string classes";
        return false;
    }

    InitializeArrayClassRoot(ClassRoot::ARRAY_CLASS, ClassRoot::CLASS,
                             utf::Mutf8AsCString(ctx.GetClassArrayClassDescriptor()));

    InitializeClassRoots(ctx);
    return true;
}

bool CoreClassLinkerExtension::InitializeArrayClass(Class *arrayClass, Class *componentClass)
{
    ASSERT(IsInitialized());

    auto *objectClass = GetClassRoot(ClassRoot::OBJECT);
    arrayClass->SetBase(objectClass);
    arrayClass->SetComponentType(componentClass);
    uint32_t accessFlags = componentClass->GetAccessFlags() & ACC_FILE_MASK;
    accessFlags &= ~ACC_INTERFACE;
    accessFlags |= ACC_FINAL | ACC_ABSTRACT;
    arrayClass->SetAccessFlags(accessFlags);
    arrayClass->SetState(Class::State::INITIALIZED);
    return true;
}

bool CoreClassLinkerExtension::InitializeUnionClass(Class *unionClass, Span<Class *> constituentClasses)
{
    ASSERT(IsInitialized());

    auto *objectClass = GetClassRoot(ClassRoot::OBJECT);
    unionClass->SetBase(objectClass);
    unionClass->SetConstituentTypes(constituentClasses);
    uint32_t accessFlags = ACC_FILE_MASK;
    for (auto cl : constituentClasses) {
        accessFlags &= cl->GetAccessFlags();
    }
    accessFlags &= ~ACC_INTERFACE;
    accessFlags |= ACC_FINAL | ACC_ABSTRACT;
    unionClass->SetAccessFlags(accessFlags);
    unionClass->SetState(Class::State::INITIALIZED);
    return true;
}

void CoreClassLinkerExtension::InitializePrimitiveClass(Class *primitiveClass)
{
    ASSERT(IsInitialized());

    primitiveClass->SetAccessFlags(ACC_PUBLIC | ACC_FINAL | ACC_ABSTRACT);
    primitiveClass->SetState(Class::State::INITIALIZED);
}

void CoreClassLinkerExtension::InitializeSyntheticClass(Class *synClass)
{
    ASSERT(IsInitialized());

    synClass->SetAccessFlags(ACC_PUBLIC | ACC_FINAL | ACC_ABSTRACT);
    synClass->SetState(Class::State::INITIALIZED);
}

size_t CoreClassLinkerExtension::GetClassVTableSize(ClassRoot root)
{
    ASSERT(IsInitialized());

    switch (root) {
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
        case ClassRoot::CLASS:
        case ClassRoot::STRING:
        case ClassRoot::LINE_STRING:
            return 0;
        default: {
            break;
        }
    }

    UNREACHABLE();
}

size_t CoreClassLinkerExtension::GetClassIMTSize(ClassRoot root)
{
    ASSERT(IsInitialized());

    switch (root) {
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
            return 0;
        default: {
            break;
        }
    }

    UNREACHABLE();
}

size_t CoreClassLinkerExtension::GetClassSize(ClassRoot root)
{
    ASSERT(IsInitialized());

    switch (root) {
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
            return Class::ComputeClassSize(GetClassVTableSize(root), GetClassIMTSize(root), 0, 0, 0, 0, 0, 0);
        default: {
            break;
        }
    }

    UNREACHABLE();
}

size_t CoreClassLinkerExtension::GetArrayClassVTableSize()
{
    ASSERT(IsInitialized());

    return GetClassVTableSize(ClassRoot::OBJECT);
}

size_t CoreClassLinkerExtension::GetArrayClassIMTSize()
{
    ASSERT(IsInitialized());

    return GetClassIMTSize(ClassRoot::OBJECT);
}

size_t CoreClassLinkerExtension::GetArrayClassSize()
{
    ASSERT(IsInitialized());

    return GetClassSize(ClassRoot::OBJECT);
}

Class *CoreClassLinkerExtension::CreateClass(const uint8_t *descriptor, size_t vtableSize, size_t imtSize, size_t size)
{
    ASSERT(IsInitialized());

    auto vm = Thread::GetCurrent()->GetVM();
    auto *heapManager = vm->GetHeapManager();

    auto *classRoot = GetClassRoot(ClassRoot::CLASS);
    ObjectHeader *objectHeader;
    if (classRoot == nullptr) {
        objectHeader = heapManager->AllocateNonMovableObject<true>(classRoot, coretypes::Class::GetSize(size));
    } else {
        objectHeader = heapManager->AllocateNonMovableObject<false>(classRoot, coretypes::Class::GetSize(size));
    }

    if (UNLIKELY(objectHeader == nullptr)) {
        return nullptr;
    }

    auto *res = reinterpret_cast<coretypes::Class *>(objectHeader);
    res->InitClass(descriptor, vtableSize, imtSize, size);
    auto *klass = res->GetRuntimeClass();
    klass->SetManagedObject(res);
    AddCreatedClass(klass);
    return klass;
}

void CoreClassLinkerExtension::FreeClass(Class *klass)
{
    ASSERT(IsInitialized());

    RemoveCreatedClass(klass);
}

CoreClassLinkerExtension::~CoreClassLinkerExtension()
{
    if (!IsInitialized()) {
        return;
    }

    FreeLoadedClasses();
}

ClassLinkerContext *CoreClassLinkerExtension::GetCommonContext(Span<Class *> classes)
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
        commonCtx = GetBootContext();  // yield the uppermost context conservatively
    }
    return commonCtx;
}

}  // namespace ark
