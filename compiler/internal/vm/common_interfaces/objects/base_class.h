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

#ifndef COMMON_INTERFACE_OBJECTS_BASE_CLASS_H
#define COMMON_INTERFACE_OBJECTS_BASE_CLASS_H
#include <cstdint>
#include "base/bit_field.h"

namespace common {
class BaseObject;

enum class ObjectType : uint8_t {
    INVALID = 0,
    FIRST_OBJECT_TYPE,

    LINE_STRING = FIRST_OBJECT_TYPE,
    SLICED_STRING,
    TREE_STRING,

    LAST_OBJECT_TYPE = TREE_STRING,

    STRING_FIRST = LINE_STRING,
    STRING_LAST = TREE_STRING,
};

class BaseClass {
public:
    BaseClass() = delete;
    ~BaseClass() = delete;
    NO_MOVE_SEMANTIC_CC(BaseClass);
    NO_COPY_SEMANTIC_CC(BaseClass);

    using HeaderType = uint64_t;

    static constexpr size_t TYPE_BITFIELD_NUM = common::BITS_PER_BYTE * sizeof(ObjectType);
    using ObjectTypeBits = common::BitField<ObjectType, 0, TYPE_BITFIELD_NUM>; // 8

    ObjectType GetObjectType() const
    {
        return ObjectTypeBits::Decode(bitfield_);
    }

    void SetObjectType(ObjectType type)
    {
        bitfield_ = ObjectTypeBits::Update(bitfield_, type);
    }

    void ClearBitField()
    {
        bitfield_ = 0;
    }

    bool IsString() const
    {
        return GetObjectType() >= ObjectType::LINE_STRING && GetObjectType() <= ObjectType::TREE_STRING;
    }

    bool IsLineString() const
    {
        return GetObjectType() == ObjectType::LINE_STRING;
    }

    bool IsSlicedString() const
    {
        return GetObjectType() == ObjectType::SLICED_STRING;
    }

    bool IsTreeString() const
    {
        return GetObjectType() == ObjectType::TREE_STRING;
    }

protected:
    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    // header_ is a padding field in base class, it will be used to store the root class in ets_runtime.
    FIELD_UNUSED_CC HeaderType header_;
    // bitfield will be initialized as the bitfield_ and bitfield1_ of js_hclass.
    // Now only the low 8bits in bitfield are used as the common type encode. Other field has no specific means here
    // but should follow the bitfield and bitfield1_ defines in js_hclass.
    uint64_t bitfield_;
    // NOLINTEND(misc-non-private-member-variables-in-classes)
};
}  // namespace common
#endif //COMMON_INTERFACE_OBJECTS_BASE_CLASS_H

