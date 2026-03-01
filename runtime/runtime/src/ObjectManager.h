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


#ifndef MRT_OBJECT_MANAGER_H
#define MRT_OBJECT_MANAGER_H

// object model components
#include "ObjectModel/MArray.h"
#include "ObjectModel/MClass.h"
#include "ObjectModel/MObject.h"

// Note: general field accessor is a common facility

namespace MapleRuntime {
//  Motivation: all interfaces related to object model implementation should have a separated control plane
class ObjectManager {
public:
    // used to regulate accepted fixed width element type
    enum class ArrayElemBits : U32 { ELEM_8B = 8, ELEM_16B = 16, ELEM_32B = 32, ELEM_64B = 64 };

    // Runtime module lifetime interfaces
    void Init() const {};
    void Fini() const {};

    static inline ObjRef NewObject(const TypeInfo* ti, MSize size, AllocType allocType = AllocType::MOVEABLE_OBJECT);
    static inline ObjRef NewWeakRefObject(const TypeInfo* ti, MSize size,
                                          AllocType allocType = AllocType::MOVEABLE_OBJECT);
    static inline ObjRef NewPinnedObject(const TypeInfo* ti, MSize size, bool isFinalizer);
    static inline ObjRef NewFinalizer(const TypeInfo* ti, MSize size);

    static inline GCTib GetGCInfo(const TypeInfo* ti);

    // general (slow) interface for array creation
    static inline ArrayRef NewArray(MIndex nElems, const TypeInfo* arrayTi,
                                    AllocType allocType = AllocType::MOVEABLE_OBJECT);

    // create object array: it needs special care.
    static inline ArrayRef NewObjArray(MIndex nElems, const TypeInfo* arrayTi,
                                       AllocType allocType = AllocType::MOVEABLE_OBJECT);

    static inline ArrayRef NewKnownWidthArray(MIndex nElems, const TypeInfo* arrayTi, ArrayElemBits elemBits,
                                              AllocType allocType = AllocType::MOVEABLE_OBJECT);
};
} // namespace MapleRuntime

#endif // MRT_OBJECT_MANAGER_H
