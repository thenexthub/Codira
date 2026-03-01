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


#ifndef MRT_MARRAY_H
#define MRT_MARRAY_H

#include "Common/BaseObject.h"
namespace MapleRuntime {
#ifdef POINTER_IS_64BIT
// align to 8 bytes for 64-bit platforms
class ATTR_PACKED(8) MArray : public BaseObject {
#else
class ATTR_PACKED(4) MArray : public BaseObject {
#endif
public:
    static constexpr MOffset GetContentOffset(); // in Bytes

    // inlined static functions to create arrays
    inline static MArray* NewArray(MIndex nElems, TypeInfo& arrayClass,
                                   AllocType allocType = AllocType::MOVEABLE_OBJECT);
    inline static MArray* NewRefArray(MIndex nElems, TypeInfo& arrayClass,
                                      AllocType allocType = AllocType::MOVEABLE_OBJECT);
    inline static MArray* NewKnownWidthArray(MIndex nElems, TypeInfo& arrayClass, const U32 elemBytes,
                                             AllocType allocType = AllocType::MOVEABLE_OBJECT);

    // inlined functions
    inline MIndex GetLength() const; // the number of elements
    inline void SetLength(MIndex number);

    inline MIndex GetMArraySize() const; // overall size in bytes of this array
    inline MSize GetElementSize() const; // in Bytes
    inline MIndex GetContentSize() const;
    inline bool IsPrimitiveArray() const;

    // access array elements
    template<typename T>
    inline T GetPrimitiveElement(MIndex index) const;

    template<typename T>
    inline void SetPrimitiveElement(MIndex index, T value);

    inline ObjectPtr GetRefElement(MIndex index);
    inline void SetRefElement(MIndex index, const ObjectPtr mObj);
    inline U8* ConvertToCArray() const;
    // this interface can only be called by array with reference fields.
    void ForEachRefFieldInRange(const RefFieldVisitor& visitor, MAddress fieldStart, MIndex fieldEnd) const;

private:
    // use MIndex because length is the upper boundary of all indices
    MIndex length;
    // array content is appended here.
};
} // namespace MapleRuntime
#endif // MRT_MARRAY_H
