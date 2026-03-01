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
#ifndef PANDA_INTERPRETER_VREGISTER_H_
#define PANDA_INTERPRETER_VREGISTER_H_

#include <cstddef>
#include <cstdint>

#include "libarkbase/macros.h"
#include "libarkbase/utils/bit_helpers.h"
#include "libarkbase/utils/bit_utils.h"
#include "libarkbase/utils/logger.h"
#include "runtime/include/coretypes/tagged_value.h"
#include "runtime/include/mem/panda_string.h"
#include "runtime/mem/object_helpers.h"

namespace ark {
class ObjectHeader;
}  // namespace ark

namespace ark::interpreter {
// An uint64_t value is used for storing the tags of values. This kind of tags is compatible with static and dynamic
// languages, and the tag is encoded as below.
// tag bits | [63-7] |     [6-4]     |      [3-1]      |      [0]        |
// usage    | unused |  object type  | primitive type  | IsObject flag   |
// details  | unused | @000: default | @011: INT       | @0: value is a  |
//          |        | @001: STRING  | @100: DOUBLE    | primitive value |
//          |        |               |                 | @1: value is a  |
//          |        |               |                 | object pointer  |

//
// All the fields' bits occupancy should be adaptive. For example, if we extend the 'IsObject flag' field by 1 bit,
// the 'IsObject flag' field will take bits [1-0] and the 'primitive type' field should take bits [4-2].
//
// This kind of tags is compatible with static and dynamic languages, and that means if the lowest bit is 1,
// the value is a object pointer, otherwise, the value is a primitive value for both static and dynamic languages.

// [0]
static constexpr uint8_t OBJECT_FLAG_SHIFT = 0;
static constexpr uint8_t OBJECT_FLAG_BITS = 1;
// [3-1]
static constexpr uint8_t PRIMITIVE_FIRST_SHIFT = OBJECT_FLAG_SHIFT + OBJECT_FLAG_BITS;
static constexpr uint8_t PRIMITIVE_TYPE_BITS = 3;
// [6-4]
static constexpr uint8_t OBJECT_FIRST_SHIFT = PRIMITIVE_FIRST_SHIFT + PRIMITIVE_TYPE_BITS;
static constexpr uint8_t OBJECT_TYPE_BITS = 3;

// OBJECT_FLAG_MASK is compatible with static and dynamic languages, and 0x1 means the value is 'reference type' in
// static and 'HeapObject' type in dynamic language.
static constexpr coretypes::TaggedType OBJECT_FLAG_MASK = 0x1;

// PrimitiveIndex's max capacity is (2 ^ PRIMITIVE_TYPE_BITS). If the number of values in PrimitiveIndex
// exceeds the capacity, PRIMITIVE_TYPE_BITS should be increased.
enum PrimitiveIndex : uint8_t { INT_IDX = 3, DOUBLE_IDX };

// ObjectIndex's max capacity is (2 ^ OBJECT_TYPE_BITS). If the number of values in ObjectIndex
// exceeds the capacity, ObjectIndex should be increased.
enum ObjectIndex : uint8_t { STRING_IDX = 1 };

template <class T>
class VRegisterIface {
public:
    ALWAYS_INLINE inline void SetValue(int64_t v)
    {
        static_cast<T *>(this)->SetValue(v);
    }

    ALWAYS_INLINE inline int64_t GetValue() const
    {
        return static_cast<const T *>(this)->GetValue();
    }

    ALWAYS_INLINE inline void Set(int32_t value)
    {
        SetValue(value);
    }

    ALWAYS_INLINE inline void Set(uint32_t value)
    {
        SetValue(value);
    }

    ALWAYS_INLINE inline void Set(int64_t value)
    {
        SetValue(value);
    }

    ALWAYS_INLINE inline void Set(uint64_t value)
    {
        auto v = bit_cast<int64_t>(value);
        SetValue(v);
    }

    ALWAYS_INLINE inline void Set(float value)
    {
        auto v = bit_cast<int32_t>(value);
        SetValue(v);
    }

    ALWAYS_INLINE inline void Set(double value)
    {
        auto v = bit_cast<int64_t>(value);
        SetValue(v);
    }

    template <bool NEED_TO_VALIDATE = true>
    ALWAYS_INLINE inline void Set(ObjectHeader *value)
    {
        if constexpr (NEED_TO_VALIDATE) {
            mem::ValidateObject(mem::RootType::ROOT_THREAD, value);
        }
        auto v = down_cast<helpers::TypeHelperT<OBJECT_POINTER_SIZE * BYTE_SIZE, true>>(value);
        SetValue(v);
    }

    ALWAYS_INLINE inline int32_t Get() const
    {
        return GetAs<int32_t>();
    }

    ALWAYS_INLINE inline float GetFloat() const
    {
        return GetAs<float>();
    }

    ALWAYS_INLINE inline int64_t GetLong() const
    {
        return GetValue();
    }

    ALWAYS_INLINE inline double GetDouble() const
    {
        return GetAs<double>();
    }

    ALWAYS_INLINE inline ObjectHeader *GetReference() const
    {
        return GetAs<ObjectHeader *>();
    }

    template <typename M, std::enable_if_t<std::is_same_v<int8_t, M> || std::is_same_v<uint8_t, M> ||
                                           std::is_same_v<int16_t, M> || std::is_same_v<uint16_t, M> ||
                                           std::is_same_v<std::int32_t, M> || std::is_same_v<uint32_t, M> ||
                                           std::is_same_v<std::int64_t, M> || std::is_same_v<uint64_t, M>> * = nullptr>
    ALWAYS_INLINE inline M GetAs() const
    {
        return static_cast<M>(GetValue());
    }

    template <typename M, std::enable_if_t<std::is_same_v<float, M>> * = nullptr>
    ALWAYS_INLINE inline float GetAs() const
    {
        return bit_cast<float>(Get());
    }

    template <typename M, std::enable_if_t<std::is_same_v<double, M>> * = nullptr>
    ALWAYS_INLINE inline double GetAs() const
    {
        return bit_cast<double>(GetValue());
    }

    template <typename M, std::enable_if_t<std::is_same_v<ObjectHeader *, M>> * = nullptr>
    ALWAYS_INLINE inline ObjectHeader *GetAs() const
    {
        return reinterpret_cast<ObjectHeader *>(static_cast<ObjectPointerType>(GetValue()));
    }

private:
    static constexpr int8_t BYTE_SIZE = 8;
};

// ========== Tagless VRegister ==========
// VRegister is an independent module which only contains a 64-bit value, and previous tag info is held by Frame
// StaticVRegisterRef contains payload and mirror vregister ptr, while DynamicVRegisterRef contains payload ptr only
// They can help you to access the tag info, like `HasObject`, `SetPrimitive`
// More details please refer to the comment in `Frame.h`.

class VRegister : public VRegisterIface<VRegister> {
public:
    VRegister() = default;

    ALWAYS_INLINE inline explicit VRegister(int64_t v)
    {
        SetValue(v);
    }

    ALWAYS_INLINE inline void SetValue(int64_t v)
    {
        v_ = v;
    }

    ALWAYS_INLINE inline int64_t GetValue() const
    {
        return v_;
    }

    ~VRegister() = default;

    DEFAULT_COPY_SEMANTIC(VRegister);
    DEFAULT_MOVE_SEMANTIC(VRegister);

    ALWAYS_INLINE static inline constexpr uint32_t GetValueOffset()
    {
        return MEMBER_OFFSET(VRegister, v_);
    }

private:
    // Stores the bit representation of the register value, regardless of the real type.
    // It can contain int/uint 8/16/32/64, float, double and ObjectHeader *.
    int64_t v_ {0};
};

template <class T, class VRegT = VRegister>
class VRegisterRef {
public:
    ALWAYS_INLINE inline explicit VRegisterRef(VRegT *payload) : payload_(payload) {}

    ALWAYS_INLINE inline void SetValue(int64_t v)
    {
        payload_->SetValue(v);
    }

    ALWAYS_INLINE inline int64_t GetValue() const
    {
        return payload_->GetValue();
    }

    ALWAYS_INLINE inline bool HasObject() const
    {
        return static_cast<const T *>(this)->HasObject();
    }

    ALWAYS_INLINE inline void MovePrimitive(const T &other)
    {
        ASSERT(!other.HasObject());
        static_cast<T *>(this)->MovePrimitive(other);
    }

    ALWAYS_INLINE inline void MoveReference(const T &other)
    {
        ASSERT(other.HasObject());
        static_cast<T *>(this)->MoveReference(other);
    }

    ALWAYS_INLINE inline void Move(const T &other)
    {
        static_cast<T *>(this)->Move(other);
    }

    ALWAYS_INLINE inline void SetPrimitive(int32_t value)
    {
        static_cast<T *>(this)->SetPrimitive(value);
    }

    ALWAYS_INLINE inline void SetPrimitive(int64_t value)
    {
        static_cast<T *>(this)->SetPrimitive(value);
    }

    ALWAYS_INLINE inline void SetPrimitive(float value)
    {
        static_cast<T *>(this)->SetPrimitive(value);
    }

    ALWAYS_INLINE inline void SetPrimitive(double value)
    {
        static_cast<T *>(this)->SetPrimitive(value);
    }

    ALWAYS_INLINE inline void SetPrimitive(uint64_t value)
    {
        static_cast<T *>(this)->SetPrimitive(value);
    }

    template <bool NEED_TO_VALIDATE = true>
    ALWAYS_INLINE inline void SetReference(ObjectHeader *obj)
    {
        static_cast<T *>(this)->template SetReference<NEED_TO_VALIDATE>(obj);
    }

    ALWAYS_INLINE inline void Set(int32_t value)
    {
        payload_->Set(value);
    }

    ALWAYS_INLINE inline void Set(uint32_t value)
    {
        payload_->Set(value);
    }

    ALWAYS_INLINE inline void Set(int64_t value)
    {
        payload_->Set(value);
    }

    ALWAYS_INLINE inline void Set(uint64_t value)
    {
        payload_->Set(value);
    }

    ALWAYS_INLINE inline void Set(float value)
    {
        payload_->Set(value);
    }

    ALWAYS_INLINE inline void Set(double value)
    {
        payload_->Set(value);
    }

    template <bool NEED_TO_VALIDATE = true>
    ALWAYS_INLINE inline void Set(ObjectHeader *value)
    {
        payload_->template Set<NEED_TO_VALIDATE>(value);
    }

    ALWAYS_INLINE inline int32_t Get() const
    {
        return payload_->Get();
    }

    ALWAYS_INLINE inline int64_t GetLong() const
    {
        return payload_->GetLong();
    }

    ALWAYS_INLINE inline float GetFloat() const
    {
        return payload_->GetFloat();
    }

    ALWAYS_INLINE inline double GetDouble() const
    {
        return payload_->GetDouble();
    }

    ALWAYS_INLINE inline ObjectHeader *GetReference() const
    {
        return payload_->GetReference();
    }

    template <typename M>
    ALWAYS_INLINE inline M GetAs() const
    {
        return payload_->template GetAs<M>();
    }

#ifndef NDEBUG
    ALWAYS_INLINE inline PandaString DumpVReg() const
    {
        PandaStringStream values;
        if (HasObject()) {
            values << "obj = " << std::hex << GetValue();
        } else {
            values << "pri = (i64) " << GetValue() << " | "
                   << "(f32) " << GetFloat() << " | "
                   << "(f64) " << GetDouble() << " | "
                   << "(hex) " << std::hex << GetValue();
        }
        return values.str();
    }
#endif

    ~VRegisterRef() = default;

    DEFAULT_COPY_SEMANTIC(VRegisterRef);
    DEFAULT_MOVE_SEMANTIC(VRegisterRef);

    static constexpr int64_t GC_OBJECT_TYPE = 0x1;
    static constexpr int64_t PRIMITIVE_TYPE = 0x0;

protected:
    VRegT *payload_ {nullptr};  // NOLINT(misc-non-private-member-variables-in-classes)
};

class StaticVRegisterRef : public VRegisterRef<StaticVRegisterRef> {
public:
    ALWAYS_INLINE inline explicit StaticVRegisterRef(VRegister *payload, VRegister *mirror)
        : VRegisterRef(payload), mirror_(mirror)
    {
    }

    ALWAYS_INLINE inline void SetTag(int64_t value)
    {
        mirror_->SetValue(value);
    }

    ALWAYS_INLINE inline int64_t GetTag() const
    {
        return mirror_->GetValue();
    }

    ALWAYS_INLINE inline void Move(std::pair<int64_t, int64_t> value)
    {
        payload_->SetValue(value.first);
        mirror_->SetValue(value.second);
    }

    // NOLINTNEXTLINE(bugprone-unhandled-self-assignment, cert-oop54-cpp)
    ALWAYS_INLINE inline StaticVRegisterRef &operator=(const StaticVRegisterRef &other)
    {
        *payload_ = *other.payload_;
        *mirror_ = *other.mirror_;
        return *this;
    }

    // NOLINTNEXTLINE(bugprone-unhandled-self-assignment, cert-oop54-cpp)
    ALWAYS_INLINE inline StaticVRegisterRef &operator=(StaticVRegisterRef &&other)
    {
        *payload_ = *other.payload_;
        *mirror_ = *other.mirror_;
        return *this;
    }

    ALWAYS_INLINE inline bool HasObject() const
    {
        return mirror_->GetValue() == GC_OBJECT_TYPE;
    }

    ALWAYS_INLINE inline void MovePrimitive(const StaticVRegisterRef &other)
    {
        payload_->SetValue(other.payload_->GetValue());
        mirror_->SetValue(PRIMITIVE_TYPE);
    }

    ALWAYS_INLINE inline void MoveReference(const StaticVRegisterRef &other)
    {
        payload_->SetValue(other.payload_->GetValue());
        mirror_->SetValue(GC_OBJECT_TYPE);
    }

    ALWAYS_INLINE inline void Move(const StaticVRegisterRef &other)
    {
        payload_->SetValue(other.payload_->GetValue());
        mirror_->SetValue(other.mirror_->GetValue());
    }

    ALWAYS_INLINE inline void SetPrimitive(int32_t value)
    {
        payload_->Set(value);
        mirror_->SetValue(PRIMITIVE_TYPE);
    }

    ALWAYS_INLINE inline void SetPrimitive(int64_t value)
    {
        payload_->Set(value);
        mirror_->SetValue(PRIMITIVE_TYPE);
    }

    ALWAYS_INLINE inline void SetPrimitive(float value)
    {
        payload_->Set(value);
        mirror_->SetValue(PRIMITIVE_TYPE);
    }

    ALWAYS_INLINE inline void SetPrimitive(double value)
    {
        payload_->Set(value);
        mirror_->SetValue(PRIMITIVE_TYPE);
    }

    ALWAYS_INLINE inline void SetPrimitive(uint64_t value)
    {
        payload_->Set(value);
        mirror_->SetValue(PRIMITIVE_TYPE);
    }

    template <bool NEED_TO_VALIDATE = true>
    ALWAYS_INLINE inline void SetReference(ObjectHeader *obj)
    {
        payload_->Set<NEED_TO_VALIDATE>(obj);
        mirror_->SetValue(GC_OBJECT_TYPE);
    }

    ~StaticVRegisterRef() = default;

    DEFAULT_COPY_CTOR(StaticVRegisterRef);
    DEFAULT_MOVE_CTOR(StaticVRegisterRef);

private:
    VRegister *mirror_ {nullptr};
};

class DynamicVRegisterRef : public VRegisterRef<DynamicVRegisterRef> {
public:
    ALWAYS_INLINE inline explicit DynamicVRegisterRef(VRegister *payload) : VRegisterRef(payload) {}

    ALWAYS_INLINE inline void Move(std::pair<int64_t, int64_t> value)
    {
        payload_->SetValue(value.first);
    }

    // NOLINTNEXTLINE(bugprone-unhandled-self-assignment, cert-oop54-cpp)
    ALWAYS_INLINE inline DynamicVRegisterRef &operator=(const DynamicVRegisterRef &other)
    {
        *payload_ = *other.payload_;
        return *this;
    }

    // NOLINTNEXTLINE(bugprone-unhandled-self-assignment, cert-oop54-cpp)
    ALWAYS_INLINE inline DynamicVRegisterRef &operator=(DynamicVRegisterRef &&other)
    {
        *payload_ = *other.payload_;
        return *this;
    }

    ALWAYS_INLINE inline bool HasObject() const
    {
        coretypes::TaggedValue v(payload_->GetAs<uint64_t>());
        return v.IsHeapObject();
    }

    ALWAYS_INLINE inline void MovePrimitive(const DynamicVRegisterRef &other)
    {
        ASSERT(!other.HasObject());
        Move(other);
    }

    ALWAYS_INLINE inline void MoveReference(const DynamicVRegisterRef &other)
    {
        ASSERT(other.HasObject());
        Move(other);
    }

    ALWAYS_INLINE inline void Move(const DynamicVRegisterRef &other)
    {
        payload_->SetValue(other.payload_->GetValue());
    }

    ALWAYS_INLINE inline void SetPrimitive(int32_t value)
    {
        payload_->Set(value);
    }

    ALWAYS_INLINE inline void SetPrimitive(int64_t value)
    {
        payload_->Set(value);
    }

    ALWAYS_INLINE inline void SetPrimitive(float value)
    {
        payload_->Set(value);
    }

    ALWAYS_INLINE inline void SetPrimitive(double value)
    {
        payload_->Set(value);
    }

    ALWAYS_INLINE inline void SetPrimitive(uint64_t value)
    {
        payload_->Set(value);
    }

    template <bool NEED_TO_VALIDATE = false>
    ALWAYS_INLINE inline void SetReference(ObjectHeader *obj)
    {
        static_assert(!NEED_TO_VALIDATE);  // Not implemented
        coretypes::TaggedValue v(obj);
        payload_->Set(v.GetRawData());
    }

    ~DynamicVRegisterRef() = default;

    DEFAULT_COPY_CTOR(DynamicVRegisterRef);
    DEFAULT_MOVE_CTOR(DynamicVRegisterRef);
};

}  // namespace ark::interpreter

#endif  // PANDA_INTERPRETER_VREGISTER_H_
