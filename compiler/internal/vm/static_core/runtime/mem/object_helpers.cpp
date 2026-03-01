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
#include <algorithm>
#include <cstdio>
#include <cstdint>
#include <cinttypes>

#include "runtime/mem/object_helpers-inl.h"

#include "libarkbase/utils/utf.h"
#include "runtime/include/thread.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/coretypes/string.h"
#include "runtime/mem/free_object.h"
#include "runtime/mem/gc/dynamic/gc_dynamic_data.h"

namespace ark::mem {

using DynClass = coretypes::DynClass;
using TaggedValue = coretypes::TaggedValue;
using TaggedType = coretypes::TaggedType;

Logger::Buffer GetDebugInfoAboutObject(const ObjectHeader *header)
{
    ValidateObject(nullptr, header);

    Logger::Buffer buffer;
// For Security reason we hide address
#if !defined(PANDA_TARGET_OHOS) || !defined(NDEBUG)
    auto *baseClass = header->ClassAddr<BaseClass>();
    const uint8_t *descriptor = nullptr;
    if (baseClass->IsDynamicClass()) {
        descriptor = utf::CStringAsMutf8("Dynamic");
    } else {
        descriptor = static_cast<Class *>(baseClass)->GetDescriptor();
    }

    const void *rawptr = static_cast<const void *>(header);
    uintmax_t mark = header->AtomicGetMark().GetValue();
    size_t size = header->ObjectSize();

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    buffer.Printf("(\"%s\" %p %zu bytes) mword = %" PRIuMAX, descriptor, rawptr, size, mark);
#endif

    return buffer;
}

static void DumpArrayClassObject(ObjectHeader *objectHeader, std::basic_ostream<char, std::char_traits<char>> *oStream)
{
    auto *cls = objectHeader->ClassAddr<Class>();
    ASSERT(cls->IsArrayClass());
    auto array = static_cast<coretypes::Array *>(objectHeader);
    *oStream << "Array " << std::hex << objectHeader << " " << cls->GetComponentType()->GetName()
             << " length = " << std::dec << array->GetLength() << std::endl;
}

static void DumpStringClass(ObjectHeader *objectHeader, std::basic_ostream<char, std::char_traits<char>> *oStream)
{
    auto *strObject = static_cast<coretypes::String *>(objectHeader);
    if (strObject->GetLength() > 0 && strObject->IsUtf8()) {
        *oStream << "length = " << std::dec << strObject->GetLength() << std::endl;
        constexpr size_t BUFF_SIZE = 256;
        std::array<char, BUFF_SIZE> buff {0};
        PandaVector<uint8_t> srcVector(strObject->GetUtf8Length());
        strObject->CopyDataRegionUtf8(srcVector.data(), 0, srcVector.size(), srcVector.size());

        auto strRes = strncpy_s(&buff[0], BUFF_SIZE, reinterpret_cast<const char *>(srcVector.data()),
                                std::min(BUFF_SIZE - 1, static_cast<size_t>(srcVector.size())));
        if (UNLIKELY(strRes != EOK)) {
            LOG(ERROR, RUNTIME) << "Couldn't copy string by strncpy_s, error code: " << strRes;
        }
        *oStream << "String data: " << &buff[0] << std::endl;
    }
}

static void DumpReferenceField(ObjectHeader *objectHeader, const Field &field,
                               std::basic_ostream<char, std::char_traits<char>> *oStream)
{
    size_t offset = field.GetOffset();
    ObjectHeader *fieldObject = objectHeader->GetFieldObject(offset);
    if (fieldObject != nullptr) {
        *oStream << std::hex << fieldObject << std::endl;
    } else {
        *oStream << "NULL" << std::endl;
    }
}

// CC-OFFNXT(G.FUN.01-CPP) big switch-case
static void DumpPrimitivesField(ObjectHeader *objectHeader, const Field &field,
                                std::basic_ostream<char, std::char_traits<char>> *oStream)
{
    size_t offset = field.GetOffset();
    panda_file::Type::TypeId typeId = field.GetTypeId();
    *oStream << std::dec;
    switch (typeId) {
        case panda_file::Type::TypeId::U1: {
            auto val = objectHeader->GetFieldPrimitive<bool>(offset);
            *oStream << val << std::endl;
            break;
        }
        case panda_file::Type::TypeId::I8: {
            auto val = objectHeader->GetFieldPrimitive<int8_t>(offset);
            *oStream << val << std::endl;
            break;
        }
        case panda_file::Type::TypeId::U8: {
            auto val = objectHeader->GetFieldPrimitive<uint8_t>(offset);
            *oStream << val << std::endl;
            break;
        }
        case panda_file::Type::TypeId::I16: {
            auto val = objectHeader->GetFieldPrimitive<int16_t>(offset);
            *oStream << val << std::endl;
            break;
        }
        case panda_file::Type::TypeId::U16: {
            auto val = objectHeader->GetFieldPrimitive<uint16_t>(offset);
            *oStream << val << std::endl;
            break;
        }
        case panda_file::Type::TypeId::I32: {
            auto val = objectHeader->GetFieldPrimitive<int32_t>(offset);
            *oStream << val << std::endl;
            break;
        }
        case panda_file::Type::TypeId::U32: {
            auto val = objectHeader->GetFieldPrimitive<uint32_t>(offset);
            *oStream << val << std::endl;
            break;
        }
        case panda_file::Type::TypeId::F32: {
            auto val = objectHeader->GetFieldPrimitive<float>(offset);
            *oStream << val << std::endl;
            break;
        }
        case panda_file::Type::TypeId::F64: {
            auto val = objectHeader->GetFieldPrimitive<double>(offset);
            *oStream << val << std::endl;
            break;
        }
        case panda_file::Type::TypeId::I64: {
            auto val = objectHeader->GetFieldPrimitive<int64_t>(offset);
            *oStream << val << std::endl;
            break;
        }
        case panda_file::Type::TypeId::U64: {
            auto val = objectHeader->GetFieldPrimitive<uint64_t>(offset);
            *oStream << val << std::endl;
            break;
        }
        default:
            LOG(FATAL, COMMON) << "Error at object dump - wrong type id";
    }
}

void DumpObjectFields(ObjectHeader *objectHeader, std::basic_ostream<char, std::char_traits<char>> *oStream)
{
    auto *cls = objectHeader->ClassAddr<Class>();
    Span<Field> fields = cls->GetInstanceFields();
    for (Field &field : fields) {
        *oStream << "\tfield \"" << GetFieldName(field) << "\" ";
        panda_file::Type::TypeId typeId = field.GetTypeId();
        if (typeId == panda_file::Type::TypeId::REFERENCE) {
            DumpReferenceField(objectHeader, field, oStream);
        } else if (typeId != panda_file::Type::TypeId::VOID) {
            DumpPrimitivesField(objectHeader, field, oStream);
        }
    }
}

void DumpObject(ObjectHeader *objectHeader, std::basic_ostream<char, std::char_traits<char>> *oStream)
{
    auto *cls = objectHeader->ClassAddr<Class>();
    ASSERT(cls != nullptr);
    *oStream << "Dump object object_header = " << std::hex << objectHeader << ", cls = " << std::hex << cls->GetName()
             << std::endl;

    if (cls->IsArrayClass()) {
        DumpArrayClassObject(objectHeader, oStream);
    } else {
        while (cls != nullptr) {
            if (cls->IsStringClass()) {
                DumpStringClass(objectHeader, oStream);
            }
            DumpObjectFields(objectHeader, oStream);
            cls = cls->GetBase();
        }
    }
}

template <typename FieldVisitor>
void TraverseFields(const Span<Field> &fields, const Class *cls, const ObjectHeader *objectHeader,
                    const FieldVisitor &fieldVisitor)
{
    for (const Field &field : fields) {
        LOG(DEBUG, GC) << " current field \"" << GetFieldName(field) << "\"";
        size_t offset = field.GetOffset();
        panda_file::Type::TypeId typeId = field.GetTypeId();
        if (typeId == panda_file::Type::TypeId::REFERENCE) {
            ObjectHeader *fieldObject = objectHeader->GetFieldObject(offset);
            if (fieldObject != nullptr) {
                LOG(DEBUG, GC) << " field val = " << std::hex << fieldObject;
                fieldVisitor(cls, objectHeader, &field, fieldObject);
            } else {
                LOG(DEBUG, GC) << " field val = nullptr";
            }
        }
    }
}

void DumpClass(const Class *cls, std::basic_ostream<char, std::char_traits<char>> *oStream)
{
    if (UNLIKELY(cls == nullptr)) {
        return;
    }
    std::function<void(const Class *, const ObjectHeader *, const Field *, ObjectHeader *)> fieldDump(
        [oStream]([[maybe_unused]] const Class *kls, [[maybe_unused]] const ObjectHeader *obj, const Field *field,
                  ObjectHeader *fieldObject) {
            *oStream << "field = " << GetFieldName(*field) << std::hex << " " << fieldObject << std::endl;
        });
    // Dump class static fields
    *oStream << "Dump class: addr = " << std::hex << cls << ", cls = " << cls->GetDescriptor() << std::endl;
    *oStream << "Dump static fields:" << std::endl;
    const Span<Field> &fields = cls->GetStaticFields();
    ObjectHeader *clsObject = cls->GetManagedObject();
    TraverseFields(fields, cls, clsObject, fieldDump);
    *oStream << "Dump cls object fields:" << std::endl;
    DumpObject(clsObject);
}

ObjectHeader *GetForwardAddress(const ObjectHeader *objectHeader)
{
    ASSERT(objectHeader->IsForwarded());
    MarkWord markWord = objectHeader->AtomicGetMark();
    MarkWord::MarkWordSize addr = markWord.GetForwardingAddress();
    return reinterpret_cast<ObjectHeader *>(addr);
}

const char *GetFieldName(const Field &field)
{
    static const char *emptyString = "";
    const char *ret = emptyString;
    bool isProxy = field.GetClass()->IsProxy();
    // For proxy class it is impossible to get field name in standard manner
    if (!isProxy) {
        ret = reinterpret_cast<const char *>(field.GetName().data);
    }
    return ret;
}

class StdFunctionAdapter {
public:
    explicit StdFunctionAdapter(const std::function<void(ObjectHeader *, ObjectHeader *)> &callback)
        : callback_(callback)
    {
    }

    bool operator()(ObjectHeader *obj, ObjectHeader *field, [[maybe_unused]] uint32_t offset,
                    [[maybe_unused]] bool isVolatile)
    {
        callback_(obj, field);
        return true;
    }

private:
    const std::function<void(ObjectHeader *, ObjectHeader *)> &callback_;
};

void GCDynamicObjectHelpers::TraverseAllObjects(ObjectHeader *objectHeader,
                                                const std::function<void(ObjectHeader *, ObjectHeader *)> &objVisitor)
{
    StdFunctionAdapter handler(objVisitor);
    TraverseAllObjectsWithInfo<false>(objectHeader, handler);
}

void GCDynamicObjectHelpers::RecordDynWeakReference(GC *gc, coretypes::TaggedType *value)
{
    GCExtensionData *data = gc->GetExtensionData();
    ASSERT(data != nullptr);
    ASSERT(data->GetLangType() == LANG_TYPE_DYNAMIC);
    static_cast<GCDynamicData *>(data)->GetDynWeakReferences()->push(value);
}

void GCDynamicObjectHelpers::HandleDynWeakReferences(GC *gc)
{
    GCExtensionData *data = gc->GetExtensionData();
    ASSERT(data != nullptr);
    ASSERT(data->GetLangType() == LANG_TYPE_DYNAMIC);
    auto *weakRefs = static_cast<GCDynamicData *>(data)->GetDynWeakReferences();
    while (!weakRefs->empty()) {
        coretypes::TaggedType *objectPointer = weakRefs->top();
        weakRefs->pop();
        TaggedValue value(*objectPointer);
        if (value.IsUndefined()) {
            continue;
        }
        ASSERT(value.IsWeak());
        ObjectHeader *object = value.GetWeakReferent();
        /* Note: If it is in young GC, the weak reference whose referent is in tenured space will not be marked. The */
        /*       weak reference whose referent is in young space will be moved into the tenured space or reset in    */
        /*       CollecYoungAndMove. If the weak referent here is not moved in young GC, it shoule be cleared.       */
        if (gc->GetGCPhase() == GCPhase::GC_PHASE_MARK_YOUNG) {
            if (gc->GetObjectAllocator()->IsObjectInYoungSpace(object) && !gc->IsMarked(object)) {
                *objectPointer = TaggedValue::Undefined().GetRawData();
            }
        } else {
            /* Note: When it is in tenured GC, we check whether the referent has been marked. */
            if (!gc->IsMarked(object)) {
                *objectPointer = TaggedValue::Undefined().GetRawData();
            }
        }
    }
}

void GCStaticObjectHelpers::TraverseAllObjects(ObjectHeader *objectHeader,
                                               const std::function<void(ObjectHeader *, ObjectHeader *)> &objVisitor)
{
    StdFunctionAdapter handler(objVisitor);
    TraverseAllObjectsWithInfo<false>(objectHeader, handler);
}

class StaticUpdateHandler {
public:
    bool operator()(ObjectHeader *object, ObjectHeader *field, uint32_t offset, [[maybe_unused]] bool isVolatile)
    {
        GCStaticObjectHelpers::UpdateRefToMovedObject(object, field, offset);
        return true;
    }
};

void GCStaticObjectHelpers::UpdateRefsToMovedObjects(ObjectHeader *object)
{
    StaticUpdateHandler handler;
    TraverseAllObjectsWithInfo<false>(object, handler);
}

ObjectHeader *GCStaticObjectHelpers::UpdateRefToMovedObject(ObjectHeader *object, ObjectHeader *ref, uint32_t offset)
{
    MarkWord markWord = ref->GetMark();  // no need atomic because stw
    if (markWord.GetState() != MarkWord::ObjectState::STATE_GC) {
        return ref;
    }
    // update instance field without write barrier
    MarkWord::MarkWordSize addr = markWord.GetForwardingAddress();
    LOG(DEBUG, GC) << "update obj ref of object " << object << " from " << ref << " to " << ToVoidPtr(addr);
    auto *forwardedObject = reinterpret_cast<ObjectHeader *>(addr);
    object->SetFieldObject<false, false>(offset, forwardedObject);
    return forwardedObject;
}

class DynamicUpdateHandler {
public:
    bool operator()(ObjectHeader *object, ObjectHeader *field, uint32_t offset, [[maybe_unused]] bool isVolatile)
    {
        GCDynamicObjectHelpers::UpdateRefToMovedObject(object, field, offset);
        return true;
    }
};

void GCDynamicObjectHelpers::UpdateRefsToMovedObjects(ObjectHeader *object)
{
    ASSERT(object->ClassAddr<HClass>()->IsDynamicClass());
    DynamicUpdateHandler handler;
    TraverseAllObjectsWithInfo<false>(object, handler);
}

ObjectHeader *GCDynamicObjectHelpers::UpdateRefToMovedObject(ObjectHeader *object, ObjectHeader *ref, uint32_t offset)
{
    MarkWord markWord = ref->AtomicGetMark();
    if (markWord.GetState() != MarkWord::ObjectState::STATE_GC) {
        return ref;
    }
    MarkWord::MarkWordSize addr = markWord.GetForwardingAddress();
    LOG(DEBUG, GC) << "update obj ref for object " << object << " from "
                   << ObjectAccessor::GetDynValue<ObjectHeader *>(object, offset) << " to " << ToVoidPtr(addr);
    auto *forwardedObject = reinterpret_cast<ObjectHeader *>(addr);
    if (ObjectAccessor::GetDynValue<TaggedValue>(object, offset).IsWeak()) {
        forwardedObject = TaggedValue(forwardedObject).CreateAndGetWeakRef().GetRawHeapObject();
    }
    ObjectAccessor::SetDynValueWithoutBarrier(object, offset, TaggedValue(forwardedObject).GetRawData());
    return forwardedObject;
}

}  // namespace ark::mem
