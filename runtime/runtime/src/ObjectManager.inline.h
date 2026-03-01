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


#ifndef MRT_OBJECT_MANAGER_INLINE_H
#define MRT_OBJECT_MANAGER_INLINE_H

// inlined object model functions
// common headers
// inter-module headers
#include "ExceptionManager.h"

// module internal headers
#include "ObjectManager.h"
#include "ObjectModel/MArray.inline.h"
#include "ObjectModel/MClass.inline.h"
#include "ObjectModel/MObject.inline.h"

namespace MapleRuntime {
inline ObjRef ObjectManager::NewObject(const TypeInfo* ti, MSize size, AllocType allocType)
{
    CHECK_DETAIL(ti != nullptr, "ti is nullptr");
    auto obj = MObject::NewObject(const_cast<TypeInfo*>(ti), size, allocType);
    return static_cast<ObjRef>(obj);
}

inline ObjRef ObjectManager::NewWeakRefObject(const TypeInfo* ti, MSize size, AllocType allocType)
{
    CHECK_DETAIL(ti != nullptr, "klass is nullptr");
    auto obj = MObject::NewObject(const_cast<TypeInfo*>(ti), size, allocType);
    return static_cast<ObjRef>(obj);
}

inline ObjRef ObjectManager::NewPinnedObject(const TypeInfo* ti, MSize size, bool isFinalizer)
{
    CHECK_DETAIL(ti != nullptr, "ti is nullptr");
    auto obj = MObject::NewPinnedObject(const_cast<TypeInfo*>(ti), size);
    if (isFinalizer && obj != nullptr) {
        static_cast<ObjRef>(obj)->OnFinalizerCreated();
    }
    return static_cast<ObjRef>(obj);
}

inline ObjRef ObjectManager::NewFinalizer(const TypeInfo* ti, MSize size)
{
    CHECK_DETAIL(ti != nullptr, "ti is nullptr");
    auto obj = MObject::NewFinalizer(ti, size);
    return static_cast<ObjRef>(obj);
}

inline GCTib ObjectManager::GetGCInfo(const TypeInfo* ti) { return ti->GetGCTib(); }

// general array creation
inline ArrayRef ObjectManager::NewArray(MIndex nElems, const TypeInfo* arrayTi, AllocType allocType)
{
    CHECK_DETAIL(arrayTi != nullptr, "arrayTi is nullptr");
    return MArray::NewArray(nElems, *const_cast<TypeInfo*>(arrayTi), allocType);
}

inline ArrayRef ObjectManager::NewObjArray(MIndex nElems, const TypeInfo* arrayTi, AllocType allocType)
{
    CHECK_DETAIL(arrayTi != nullptr, "arrayTi is nullptr");
    return MArray::NewRefArray(nElems, *const_cast<TypeInfo*>(arrayTi), allocType);
}

inline ArrayRef ObjectManager::NewKnownWidthArray(MIndex nElems, const TypeInfo* arrayTi, ArrayElemBits elemBits,
                                                  AllocType allocType)
{
    CHECK_DETAIL(arrayTi != nullptr, "arrayTi is nullptr");
    constexpr U32 bitsToByte = 3;
    // Note here we need Bytes instead of Bits
    return MArray::NewKnownWidthArray(nElems, *const_cast<TypeInfo*>(arrayTi),
                                      (static_cast<U32>(elemBits) >> bitsToByte), allocType);
}
} // namespace MapleRuntime

#endif // MRT_OBJECT_MANAGER_INLINE_H
