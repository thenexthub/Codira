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


#ifndef MRT_MOBJECT_H
#define MRT_MOBJECT_H

#include "Common/BaseObject.h"

namespace MapleRuntime {
// MObject is the peer structure of MArray, thus not "the" generic structure for runtime.
// refer to BaseObject instead.
class ATTR_PACKED(4) MObject : public BaseObject {
public:
    static MObject* NewFinalizer(const TypeInfo* ti, MSize size);

    // no need to retrieve object size via class metadata, i.e. GetInstanceSize().
    static MObject* NewObject(TypeInfo* ti, MSize objectSize, AllocType);
    static MObject* NewPinnedObject(TypeInfo* ti, MSize objectSize);

    // inlined functions
    // Property query
    inline bool IsSubType(TypeInfo& ti);
    inline bool IsArray() const;
    inline bool IsPrimitiveArray() const;
    inline bool IsStructArray() const;

    // Field access
    template<typename T>
    inline T Load(size_t offset) const;
    template<typename T>
    inline void Store(size_t offset, T value);
    inline MObject* LoadRef(size_t offset);
    inline void StoreRef(size_t offset, MObject* value);

    // Type conversion: need to be safe enough
    template<typename T0, typename T1>
    static inline T0* Cast(T1 o);

    template<typename T0, typename T1>
    static inline T0* CastNonNull(T1 o);

private:
    MObject() = delete;
    ~MObject() = delete;
    MObject(MObject&&) = delete;
    MObject& operator=(MObject&&) = delete;
}; // class MObject
} // namespace MapleRuntime
#endif // MRT_MOBJECT_H
