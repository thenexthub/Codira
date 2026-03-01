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

#ifndef PANDA_RUNTIME_MEM_NEW_OBJECT_HELPERS_INL_H
#define PANDA_RUNTIME_MEM_NEW_OBJECT_HELPERS_INL_H

#include "runtime/mem/object-references-iterator.h"
#include "runtime/include/coretypes/array-inl.h"
#include "runtime/include/class-inl.h"

namespace ark::mem {
class ObjectArrayIterator {
public:
    template <typename T, bool INTERRUPTIBLE, typename Handler>
    static bool Iterate(coretypes::Array *array, Handler *handler);

    template <typename T, bool INTERRUPTIBLE, typename Handler>
    static bool Iterate(coretypes::Array *array, Handler *handler, void *begin, void *end);
};

template <typename T, bool INTERRUPTIBLE, typename Handler>
bool ObjectArrayIterator::Iterate(coretypes::Array *array, Handler *handler)
{
    auto *arrayStart = array->GetBase<T *>();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto *arrayEnd = arrayStart + array->GetLength();

    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    for (auto *p = arrayStart; p < arrayEnd; ++p) {
        [[maybe_unused]] auto cont = handler->ProcessObjectPointer(array, p);
        if constexpr (INTERRUPTIBLE) {
            if (!cont) {
                return false;
            }
        }
    }

    return true;
}

template <typename T, bool INTERRUPTIBLE, typename Handler>
bool ObjectArrayIterator::Iterate(coretypes::Array *array, Handler *handler, void *begin, void *end)
{
    ASSERT(IsAligned(ToUintPtr(begin), DEFAULT_ALIGNMENT_IN_BYTES));

    auto *arrayStart = array->GetBase<T *>();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto *arrayEnd = arrayStart + array->GetLength();
    auto *p = begin < arrayStart ? arrayStart : reinterpret_cast<T *>(begin);

    if (end > arrayEnd) {
        end = arrayEnd;
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    for (; p < end; ++p) {
        [[maybe_unused]] auto cont = handler->ProcessObjectPointer(array, p);
        if constexpr (INTERRUPTIBLE) {
            if (!cont) {
                return false;
            }
        }
    }

    return true;
}

template <bool INTERRUPTIBLE, typename Handler>
bool ObjectIterator<LANG_TYPE_STATIC>::Iterate(ObjectHeader *obj, Handler *handler, void *begin, void *end)
{
    auto *cls = obj->ClassAddr<Class>();
    ASSERT(cls != nullptr);
    return Iterate<INTERRUPTIBLE>(cls, obj, handler, begin, end);
}

template <typename Handler>
bool ObjectIterator<LANG_TYPE_STATIC>::IterateAndDiscoverReferences(GC *gc, ObjectHeader *obj, Handler *handler)
{
    auto *cls = obj->ClassAddr<Class>();
    ASSERT(cls != nullptr);

    if (gc->IsReference(cls, obj, [gc](auto *o) { return gc->InGCSweepRange(o); })) {
        gc->ProcessReferenceForSinglePassCompaction(cls, obj, [obj, handler](void *o) {
            handler->ProcessObjectPointer(obj, reinterpret_cast<ObjectPointerType *>(o));
        });
        return true;
    }

    return Iterate<false>(cls, obj, handler);
}

template <typename Handler>
bool ObjectIterator<LANG_TYPE_STATIC>::IterateAndDiscoverReferences(GC *gc, ObjectHeader *obj, Handler *handler,
                                                                    void *begin, void *end)
{
    auto *cls = obj->ClassAddr<Class>();
    ASSERT(cls != nullptr);

    if (gc->IsReference(cls, obj, [gc](auto *o) { return gc->InGCSweepRange(o); })) {
        gc->ProcessReferenceForSinglePassCompaction(cls, obj, [obj, handler](void *o) {
            handler->ProcessObjectPointer(obj, reinterpret_cast<ObjectPointerType *>(o));
        });
        return true;
    }

    return Iterate<false>(cls, obj, handler, begin, end);
}

template <bool INTERRUPTIBLE, typename Handler>
bool ObjectIterator<LANG_TYPE_STATIC>::Iterate(const Class *cls, ObjectHeader *obj, Handler *handler)
{
    if (cls->IsObjectArrayClass()) {
        return ObjectArrayIterator::Iterate<ObjectPointerType, INTERRUPTIBLE>(static_cast<coretypes::Array *>(obj),
                                                                              handler);
    }
    if (cls->IsClassClass()) {
        auto *objectClass = ark::Class::FromClassObject(obj);
        if (objectClass->IsInitializing() || objectClass->IsInitialized()) {
            if (!IterateClassReferences<INTERRUPTIBLE>(objectClass, handler)) {
                return false;
            }
        }
    }

    return IterateObjectReferences<INTERRUPTIBLE>(obj, cls, handler);
}

template <bool INTERRUPTIBLE, typename Handler>
bool ObjectIterator<LANG_TYPE_STATIC>::Iterate(Class *cls, ObjectHeader *obj, Handler *handler, void *begin, void *end)
{
    if (cls->IsObjectArrayClass()) {
        return ObjectArrayIterator::Iterate<ObjectPointerType, INTERRUPTIBLE>(static_cast<coretypes::Array *>(obj),
                                                                              handler, begin, end);
    }
    if (cls->IsClassClass()) {
        auto *objectClass = ark::Class::FromClassObject(obj);
        if (objectClass->IsInitializing() || objectClass->IsInitialized()) {
            if (!IterateClassReferences<INTERRUPTIBLE>(objectClass, handler, begin, end)) {
                return false;
            }
        }
    }

    if (obj >= begin && !cls->IsVariableSize() && ToUintPtr(obj) + cls->GetObjectSize() <= ToUintPtr(end)) {
        return IterateObjectReferences<INTERRUPTIBLE>(obj, cls, handler);
    }

    return IterateObjectReferences<INTERRUPTIBLE>(obj, cls, handler, begin, end);
}

template <bool INTERRUPTIBLE, typename Handler>
bool ObjectIterator<LANG_TYPE_STATIC>::IterateClassReferences(Class *cls, Handler *handler)
{
    auto refNum = cls->GetRefFieldsNum<true>();
    if (refNum > 0) {
        auto offset = cls->GetRefFieldsOffset<true>();
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto *refStart = reinterpret_cast<ObjectPointerType *>(ToUintPtr(cls) + offset);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto *refEnd = refStart + refNum;
        return IterateRange<INTERRUPTIBLE>(cls->GetManagedObject(), refStart, refEnd, handler);
    }
    return true;
}

template <bool INTERRUPTIBLE, typename Handler>
bool ObjectIterator<LANG_TYPE_STATIC>::IterateClassReferences(Class *cls, Handler *handler, void *begin, void *end)
{
    auto refNum = cls->GetRefFieldsNum<true>();
    if (refNum > 0) {
        auto offset = cls->GetRefFieldsOffset<true>();
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto *refStart = reinterpret_cast<ObjectPointerType *>(ToUintPtr(cls) + offset);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto *refEnd = refStart + refNum;
        return IterateRange<INTERRUPTIBLE>(cls->GetManagedObject(), refStart, refEnd, handler, begin, end);
    }
    return true;
}

template <bool INTERRUPTIBLE, typename Handler>
bool ObjectIterator<LANG_TYPE_STATIC>::IterateObjectReferences(ObjectHeader *object, const Class *objClass,
                                                               Handler *handler)
{
    ASSERT(objClass != nullptr);
    ASSERT(!objClass->IsDynamicClass());
    auto *cls = objClass;
    // CC-OFFNXT(G.CTL.03): false positive
    while (true) {
        auto refNum = cls->GetRefFieldsNum<false>();
        if (refNum == 0) {
            if (cls->HaveNoRefsInParents()) {
                break;
            }
            cls = cls->GetBase();
            continue;
        }

        auto offset = cls->GetRefFieldsOffset<false>();
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto *refStart = reinterpret_cast<ObjectPointerType *>(ToUintPtr(object) + offset);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto *refEnd = refStart + refNum;
        ASSERT_PRINT(ToUintPtr(refEnd) <= ToUintPtr(object) + objClass->GetObjectSize(),
                     "cls " << objClass->GetName() << " obj " << object << " size " << objClass->GetObjectSize()
                            << " refs " << refStart << ".." << refEnd);
        [[maybe_unused]] auto cont = IterateRange<INTERRUPTIBLE>(object, refStart, refEnd, handler);
        if constexpr (INTERRUPTIBLE) {
            if (!cont) {
                return false;
            }
        }

        if (cls->HaveNoRefsInParents()) {
            break;
        }
        cls = cls->GetBase();
    }
    return true;
}

template <bool INTERRUPTIBLE, typename Handler>
bool ObjectIterator<LANG_TYPE_STATIC>::IterateObjectReferences(ObjectHeader *object, const Class *cls, Handler *handler,
                                                               void *begin, void *end)
{
    ASSERT(cls != nullptr);
    ASSERT(!cls->IsDynamicClass());
    // CC-OFFNXT(G.CTL.03): false positive
    while (true) {
        auto refNum = cls->GetRefFieldsNum<false>();
        if (refNum == 0) {
            if (cls->HaveNoRefsInParents()) {
                break;
            }
            cls = cls->GetBase();
            continue;
        }

        auto offset = cls->GetRefFieldsOffset<false>();
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto *refStart = reinterpret_cast<ObjectPointerType *>(ToUintPtr(object) + offset);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto *refEnd = refStart + refNum;
        [[maybe_unused]] auto cont = IterateRange<INTERRUPTIBLE>(object, refStart, refEnd, handler, begin, end);
        if constexpr (INTERRUPTIBLE) {
            if (!cont) {
                return false;
            }
        }

        if (cls->HaveNoRefsInParents()) {
            break;
        }
        cls = cls->GetBase();
    }
    return true;
}

template <bool INTERRUPTIBLE, typename Handler>
bool ObjectIterator<LANG_TYPE_STATIC>::IterateRange(ObjectHeader *fromObject, ObjectPointerType *refStart,
                                                    const ObjectPointerType *refEnd, Handler *handler)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    for (auto *p = refStart; p < refEnd; p++) {
        [[maybe_unused]] auto cont = handler->ProcessObjectPointer(fromObject, p);
        if constexpr (INTERRUPTIBLE) {
            if (!cont) {
                return false;
            }
        }
    }
    return true;
}

template <bool INTERRUPTIBLE, typename Handler>
bool ObjectIterator<LANG_TYPE_STATIC>::IterateRange(ObjectHeader *fromObject, ObjectPointerType *refStart,
                                                    ObjectPointerType *refEnd, Handler *handler, void *begin, void *end)
{
    auto *p = begin < refStart ? refStart : reinterpret_cast<ObjectPointerType *>(begin);
    if (end > refEnd) {
        end = refEnd;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    for (; p < end; p++) {
        [[maybe_unused]] auto cont = handler->ProcessObjectPointer(fromObject, p);
        if constexpr (INTERRUPTIBLE) {
            if (!cont) {
                return false;
            }
        }
    }
    return true;
}

template <bool INTERRUPTIBLE, typename Handler>
bool ObjectIterator<LANG_TYPE_DYNAMIC>::Iterate(ObjectHeader *obj, Handler *handler, void *begin, void *end)
{
    auto *cls = obj->ClassAddr<HClass>();
    ASSERT(cls != nullptr && cls->IsDynamicClass());
    return Iterate<INTERRUPTIBLE>(cls, obj, handler, begin, end);
}

template <typename Handler>
bool ObjectIterator<LANG_TYPE_DYNAMIC>::IterateAndDiscoverReferences(GC *gc, ObjectHeader *obj, Handler *handler)
{
    auto *cls = obj->ClassAddr<HClass>();
    ASSERT(cls != nullptr && cls->IsDynamicClass());

    if (gc->IsReference(cls, obj, [gc](auto *o) { return gc->InGCSweepRange(o); })) {
        gc->ProcessReferenceForSinglePassCompaction(cls, obj, [obj, handler](void *o) {
            handler->ProcessObjectPointer(obj, reinterpret_cast<TaggedType *>(o));
        });
        return true;
    }

    return Iterate<false>(cls, obj, handler);
}

template <typename Handler>
bool ObjectIterator<LANG_TYPE_DYNAMIC>::IterateAndDiscoverReferences(GC *gc, ObjectHeader *obj, Handler *handler,
                                                                     void *begin, void *end)
{
    auto *cls = obj->ClassAddr<HClass>();
    ASSERT(cls != nullptr && cls->IsDynamicClass());

    if (gc->IsReference(cls, obj, [gc](auto *o) { return gc->InGCSweepRange(o); })) {
        gc->ProcessReferenceForSinglePassCompaction(cls, obj, [obj, handler](void *o) {
            handler->ProcessObjectPointer(obj, reinterpret_cast<TaggedType *>(o));
        });
        return true;
    }

    return Iterate<false>(cls, obj, handler, begin, end);
}

template <bool INTERRUPTIBLE, typename Handler>
bool ObjectIterator<LANG_TYPE_DYNAMIC>::Iterate(HClass *cls, ObjectHeader *obj, Handler *handler)
{
    if (cls->IsString() || cls->IsNativePointer()) {
        return true;
    }
    if (cls->IsArray()) {
        return ObjectArrayIterator::Iterate<TaggedType, INTERRUPTIBLE>(static_cast<coretypes::Array *>(obj), handler);
    }
    if (cls->IsHClass()) {
        return IterateClassReferences<INTERRUPTIBLE>(coretypes::DynClass::Cast(obj), handler);
    }

    return IterateObjectReferences<INTERRUPTIBLE>(obj, cls, handler);
}

template <bool INTERRUPTIBLE, typename Handler>
bool ObjectIterator<LANG_TYPE_DYNAMIC>::Iterate(HClass *cls, ObjectHeader *obj, Handler *handler, void *begin,
                                                void *end)
{
    if (cls->IsString() || cls->IsNativePointer()) {
        return true;
    }
    if (cls->IsArray()) {
        return ObjectArrayIterator::Iterate<TaggedType, INTERRUPTIBLE>(static_cast<coretypes::Array *>(obj), handler,
                                                                       begin, end);
    }
    if (cls->IsHClass()) {
        return IterateClassReferences<INTERRUPTIBLE>(coretypes::DynClass::Cast(obj), handler, begin, end);
    }

    return IterateObjectReferences<INTERRUPTIBLE>(obj, cls, handler, begin, end);
}

template <bool INTERRUPTIBLE, typename Handler>
bool ObjectIterator<LANG_TYPE_DYNAMIC>::IterateClassReferences(coretypes::DynClass *dynClass, Handler *handler)
{
    auto hklassSize = dynClass->ClassAddr<HClass>()->GetObjectSize() - sizeof(coretypes::DynClass);
    auto bodySize = hklassSize - sizeof(HClass);
    auto numOfFields = bodySize / TaggedValue::TaggedTypeSize();
    auto fieldOffset = sizeof(ObjectHeader) + sizeof(HClass);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto *fieldStart = reinterpret_cast<TaggedType *>(ToUintPtr(dynClass) + fieldOffset);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto *fieldEnd = fieldStart + numOfFields;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    for (auto *p = fieldStart; p < fieldEnd; p++) {
        [[maybe_unused]] auto cont = handler->ProcessObjectPointer(dynClass, p);
        if constexpr (INTERRUPTIBLE) {
            if (!cont) {
                return false;
            }
        }
    }
    return true;
}

template <bool INTERRUPTIBLE, typename Handler>
bool ObjectIterator<LANG_TYPE_DYNAMIC>::IterateClassReferences(coretypes::DynClass *dynClass, Handler *handler,
                                                               void *begin, void *end)
{
    auto hklassSize = dynClass->ClassAddr<HClass>()->GetObjectSize() - sizeof(coretypes::DynClass);
    auto bodySize = hklassSize - sizeof(HClass);
    auto numOfFields = bodySize / TaggedValue::TaggedTypeSize();
    auto fieldOffset = sizeof(ObjectHeader) + sizeof(HClass);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto *fieldStart = reinterpret_cast<TaggedType *>(ToUintPtr(dynClass) + fieldOffset);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto *fieldEnd = fieldStart + numOfFields;
    auto *p = begin < fieldStart ? fieldStart : reinterpret_cast<TaggedType *>(begin);
    if (end > fieldEnd) {
        end = fieldEnd;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    for (; p < end; p++) {
        [[maybe_unused]] auto cont = handler->ProcessObjectPointer(dynClass, p);
        if constexpr (INTERRUPTIBLE) {
            if (!cont) {
                return false;
            }
        }
    }
    return true;
}

template <bool INTERRUPTIBLE, typename Handler>
bool ObjectIterator<LANG_TYPE_DYNAMIC>::IterateObjectReferences(ObjectHeader *object, HClass *cls, Handler *handler)
{
    ASSERT(cls->IsDynamicClass());
    LOG(DEBUG, GC) << "TraverseObject Current object: " << GetDebugInfoAboutObject(object);
    auto objBodySize = cls->GetObjectSize() - ObjectHeader::ObjectHeaderSize();
    ASSERT(objBodySize % TaggedValue::TaggedTypeSize() == 0);
    auto numOfFields = objBodySize / TaggedValue::TaggedTypeSize();
    auto dataOffset = ObjectHeader::ObjectHeaderSize();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto *fieldStart = reinterpret_cast<TaggedType *>(ToUintPtr(object) + dataOffset);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto *fieldEnd = fieldStart + numOfFields;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    for (auto *p = fieldStart; p < fieldEnd; p++) {
        if (cls->IsNativeField(ToUintPtr(p) - ToUintPtr(object))) {
            continue;
        }
        [[maybe_unused]] auto cont = handler->ProcessObjectPointer(object, p);
        if constexpr (INTERRUPTIBLE) {
            if (!cont) {
                return false;
            }
        }
    }
    return true;
}

template <bool INTERRUPTIBLE, typename Handler>
bool ObjectIterator<LANG_TYPE_DYNAMIC>::IterateObjectReferences(ObjectHeader *object, HClass *cls, Handler *handler,
                                                                void *begin, void *end)
{
    ASSERT(cls->IsDynamicClass());
    LOG(DEBUG, GC) << "TraverseObject Current object: " << GetDebugInfoAboutObject(object);
    auto objBodySize = cls->GetObjectSize() - ObjectHeader::ObjectHeaderSize();
    ASSERT(objBodySize % TaggedValue::TaggedTypeSize() == 0);
    auto numOfFields = objBodySize / TaggedValue::TaggedTypeSize();
    auto dataOffset = ObjectHeader::ObjectHeaderSize();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto *fieldStart = reinterpret_cast<TaggedType *>(ToUintPtr(object) + dataOffset);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto *fieldEnd = fieldStart + numOfFields;
    auto *p = begin < fieldStart ? fieldStart : reinterpret_cast<TaggedType *>(begin);
    if (end > fieldEnd) {
        end = fieldEnd;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    for (; p < end; p++) {
        if (cls->IsNativeField(ToUintPtr(p) - ToUintPtr(object))) {
            continue;
        }
        [[maybe_unused]] auto cont = handler->ProcessObjectPointer(object, p);
        if constexpr (INTERRUPTIBLE) {
            if (!cont) {
                return false;
            }
        }
    }
    return true;
}
}  // namespace ark::mem

#endif  // PANDA_RUNTIME_MEM_NEW_OBJECT_HELPERS_INL_H
