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
#ifndef PANDA_RUNTIME_MEM_OBJECT_HELPERS_H
#define PANDA_RUNTIME_MEM_OBJECT_HELPERS_H

#include <functional>

#include "libarkbase/utils/logger.h"
#include "libarkbase/mem/mem.h"

#include "runtime/mem/gc/gc_root_type.h"
#include "runtime/include/object_header.h"

namespace ark {
class Class;
class HClass;
class Field;
class ManagedThread;
class PandaVM;
}  // namespace ark

namespace ark::coretypes {
class DynClass;
class Array;
}  // namespace ark::coretypes

namespace ark::mem {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_DEBUG_OBJ_HELPERS LOG(DEBUG, GC) << vm->GetGC()->GetLogPrefix()

class GC;

inline size_t GetObjectSize(const void *mem)
{
    return static_cast<const ObjectHeader *>(mem)->ObjectSize();
}

Logger::Buffer GetDebugInfoAboutObject(const ObjectHeader *header);

constexpr size_t GetMinimalObjectSize()
{
    return GetAlignedObjectSize(ObjectHeader::ObjectHeaderSize());
}

/**
 * Validate that object is correct from point of view of GC.
 * For example it checks that class of the object is not nullptr.
 * @param from_object object from which we found object by reference
 * @param object object which we want to validate
 */
inline void ValidateObject([[maybe_unused]] const ObjectHeader *fromObject, [[maybe_unused]] const ObjectHeader *object)
{
#ifndef NDEBUG
    if (object == nullptr) {
        return;
    }
    // from_object can be null, because sometimes we call Validate when we don't know previous object (for example when
    // we extract it from stack)
    if (object->template ClassAddr<BaseClass>() == nullptr) {
        LOG(ERROR, GC) << " Broken object doesn't have class: " << object << " accessed from object: " << fromObject;
        UNREACHABLE();
    }
#endif  // !NDEBUG
}

/**
 * Validate that object (which is gc-root) is correct from point of view of GC
 * See ValidateObject(from_object, object) for further explanation
 * @param root_type type of the root
 * @param object object (root) which we want to validate
 */
inline void ValidateObject([[maybe_unused]] RootType rootType, [[maybe_unused]] const ObjectHeader *object)
{
#ifndef NDEBUG
    if (object == nullptr) {
        return;
    }
    ASSERT_DO(object->template ClassAddr<BaseClass>() != nullptr,
              LOG(FATAL, GC) << " Broken object doesn't have class: " << object << " accessed from root: " << rootType);
#endif  // !NDEBUG
}

void DumpObject(ObjectHeader *objectHeader, std::basic_ostream<char, std::char_traits<char>> *oStream = &std::cerr);

void DumpClass(const Class *cls, std::basic_ostream<char, std::char_traits<char>> *oStream = &std::cerr);

[[nodiscard]] ObjectHeader *GetForwardAddress(const ObjectHeader *objectHeader);

const char *GetFieldName(const Field &field);

size_t GetDynClassInstanceSize(coretypes::DynClass *object);

class GCStaticObjectHelpers {
public:
    /// Check the object is an instance of class
    static inline bool IsClassObject(ObjectHeader *obj);

    /// Traverse all kinds of object_header and call obj_visitor for each reference field.
    static void TraverseAllObjects(ObjectHeader *objectHeader,
                                   const std::function<void(ObjectHeader *, ObjectHeader *)> &objVisitor);

    /**
     * Traverse all kinds of object_header and call handler for each reference field.
     * The handler accepts the object, the reference value, offset to the reference in the object and
     * the flag whether the field is volatile. Stop traverse if handler return false for any field.
     * INTERRUPTIBLE means that traverse over objects can be stopped (it is possible when handler may return false)
     * This template is used to decrease possible performance influence of checking handler's returned value on GC pause
     * time
     * Return true if object was fully traversed, otherwise false.
     */
    template <bool INTERRUPTIBLE, typename Handler>
    static bool TraverseAllObjectsWithInfo(ObjectHeader *object, Handler &handler, void *begin = ToVoidPtr(0),
                                           void *end = ToVoidPtr(UINTPTR_MAX));

    static void UpdateRefsToMovedObjects(ObjectHeader *object);

    /**
     * Update a single reference field in the object to the moved value.
     * Return the moved value.
     */
    static ObjectHeader *UpdateRefToMovedObject(ObjectHeader *object, ObjectHeader *ref, uint32_t offset);

private:
    template <bool INTERRUPTIBLE, typename Handler>
    static bool TraverseClass(Class *cls, Handler &handler);
    template <bool INTERRUPTIBLE, typename Handler>
    static bool TraverseArray(coretypes::Array *array, Class *cls, void *begin, void *end, Handler &handler);
    template <bool INTERRUPTIBLE, typename Handler>
    static bool TraverseObject(ObjectHeader *objectHeader, Class *cls, Handler &handler);
};

class GCDynamicObjectHelpers {
public:
    /// Check the object is an instance of class
    static inline bool IsClassObject(ObjectHeader *obj);

    /// Traverse all kinds of object_header and call obj_visitor for each reference field.
    static void TraverseAllObjects(ObjectHeader *objectHeader,
                                   const std::function<void(ObjectHeader *, ObjectHeader *)> &objVisitor);

    /**
     * Traverse all kinds of object_header and call handler for each reference field.
     * The handler accepts the object, the reference value, offset to the reference in the object and
     * the flag whether the field is volatile. Stop traverse if handler return false for any field.
     * INTERRUPTIBLE means that traverse over objects can be stopped (it is possible when handler may return false)
     * This template is used to decrease possible performance influence of checking handler's returned value on GC pause
     * time
     * Return true if object was fully traversed, otherwise false.
     */
    template <bool INTERRUPTIBLE, typename Handler>
    static bool TraverseAllObjectsWithInfo(ObjectHeader *objectHeader, Handler &handler, void *begin = ToVoidPtr(0),
                                           void *end = ToVoidPtr(UINTPTR_MAX));

    static void UpdateRefsToMovedObjects(ObjectHeader *object);

    /**
     * Update a single reference field in the object to the moved value.
     * Return the moved value.
     */
    static ObjectHeader *UpdateRefToMovedObject(ObjectHeader *object, ObjectHeader *ref, uint32_t offset);

    static void RecordDynWeakReference(GC *gc, coretypes::TaggedType *value);
    static void HandleDynWeakReferences(GC *gc);

private:
    template <bool INTERRUPTIBLE, typename Handler>
    static bool TraverseArray(coretypes::Array *array, HClass *cls, void *begin, void *end, Handler &handler);
    template <bool INTERRUPTIBLE, typename Handler>
    static bool TraverseClass(coretypes::DynClass *dynClass, Handler &handler);
    template <bool INTERRUPTIBLE, typename Handler>
    static bool TraverseObject(ObjectHeader *objectHeader, HClass *cls, Handler &handler);

    static void UpdateDynArray(PandaVM *vm, coretypes::Array *array, ArraySizeT index, ObjectHeader *objRef);

    static void UpdateDynObjectRef(PandaVM *vm, ObjectHeader *objectHeader, size_t offset, ObjectHeader *fieldObjRef);
};

template <LangTypeT LANG_TYPE>
class GCObjectHelpers {
};

template <>
class GCObjectHelpers<LANG_TYPE_STATIC> {
public:
    using Value = GCStaticObjectHelpers;
};

template <>
class GCObjectHelpers<LANG_TYPE_DYNAMIC> {
public:
    using Value = GCDynamicObjectHelpers;
};

template <LangTypeT LANG_TYPE>
using ObjectHelpers = typename GCObjectHelpers<LANG_TYPE>::Value;

}  // namespace ark::mem

#endif  // PANDA_OBJECT_HELPERS_H
