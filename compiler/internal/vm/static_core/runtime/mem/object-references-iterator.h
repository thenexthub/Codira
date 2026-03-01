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

#ifndef PANDA_RUNTIME_MEM_NEW_OBJECT_HELPERS_H
#define PANDA_RUNTIME_MEM_NEW_OBJECT_HELPERS_H

#include "runtime/include/coretypes/array.h"
#include "runtime/include/coretypes/dyn_objects.h"
#include "runtime/include/class.h"
#include "runtime/include/hclass.h"

namespace ark::mem {
template <LangTypeT LANG_TYPE>
class ObjectIterator;

/// Provides functionality to iterate through references in object for static languages
template <>
class ObjectIterator<LANG_TYPE_STATIC> {
public:
    /// Iterates references in object and passes pointers of them to the handler
    template <bool INTERRUPTIBLE, typename Handler>
    static bool Iterate(ObjectHeader *obj, Handler *handler, void *begin, void *end);

    template <typename Handler>
    static bool IterateAndDiscoverReferences(GC *gc, ObjectHeader *obj, Handler *handler);

    template <typename Handler>
    static bool IterateAndDiscoverReferences(GC *gc, ObjectHeader *obj, Handler *handler, void *begin, void *end);

    template <bool INTERRUPTIBLE, typename Handler>
    static bool Iterate(const Class *cls, ObjectHeader *obj, Handler *handler);

private:
    template <bool INTERRUPTIBLE, typename Handler>
    static bool Iterate(Class *cls, ObjectHeader *obj, Handler *handler, void *begin, void *end);

    template <bool INTERRUPTIBLE, typename Handler>
    static bool IterateObjectReferences(ObjectHeader *object, const Class *objClass, Handler *handler);

    template <bool INTERRUPTIBLE, typename Handler>
    static bool IterateObjectReferences(ObjectHeader *object, const Class *cls, Handler *handler, void *begin,
                                        void *end);

    template <bool INTERRUPTIBLE, typename Handler>
    static bool IterateClassReferences(Class *cls, Handler *handler);

    template <bool INTERRUPTIBLE, typename Handler>
    static bool IterateClassReferences(Class *cls, Handler *handler, void *begin, void *end);

    template <bool INTERRUPTIBLE, typename Handler>
    static bool IterateRange(ObjectHeader *fromObject, ObjectPointerType *refStart, const ObjectPointerType *refEnd,
                             Handler *handler);

    template <bool INTERRUPTIBLE, typename Handler>
    static bool IterateRange(ObjectHeader *fromObject, ObjectPointerType *refStart, ObjectPointerType *refEnd,
                             Handler *handler, void *begin, void *end);
};

/// Provides functionality to iterate through references in object for dynamic languages
template <>
class ObjectIterator<LANG_TYPE_DYNAMIC> {
public:
    /// Iterates references in object and passes pointers of them to the handler
    template <bool INTERRUPTIBLE, typename Handler>
    static bool Iterate(ObjectHeader *obj, Handler *handler, void *begin, void *end);

    template <typename Handler>
    static bool IterateAndDiscoverReferences(GC *gc, ObjectHeader *obj, Handler *handler);

    template <typename Handler>
    static bool IterateAndDiscoverReferences(GC *gc, ObjectHeader *obj, Handler *handler, void *begin, void *end);

private:
    template <bool INTERRUPTIBLE, typename Handler>
    static bool Iterate(HClass *cls, ObjectHeader *obj, Handler *handler);

    template <bool INTERRUPTIBLE, typename Handler>
    static bool Iterate(HClass *cls, ObjectHeader *obj, Handler *handler, void *begin, void *end);

    template <bool INTERRUPTIBLE, typename Handler>
    static bool IterateObjectReferences(ObjectHeader *object, HClass *cls, Handler *handler);

    template <bool INTERRUPTIBLE, typename Handler>
    static bool IterateObjectReferences(ObjectHeader *object, HClass *cls, Handler *handler, void *begin, void *end);

    template <bool INTERRUPTIBLE, typename Handler>
    static bool IterateClassReferences(coretypes::DynClass *dynClass, Handler *handler);

    template <bool INTERRUPTIBLE, typename Handler>
    static bool IterateClassReferences(coretypes::DynClass *dynClass, Handler *handler, void *begin, void *end);
};
}  // namespace ark::mem

#endif  // PANDA_RUNTIME_MEM_NEW_OBJECT_HELPERS_H
