/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef PANDA_INTERPRETER_ACC_VREGISTER_INL_H_
#define PANDA_INTERPRETER_ACC_VREGISTER_INL_H_

#include <cstddef>
#include <cstdint>

#include "runtime/interpreter/acc_vregister.h"

#ifdef PANDA_ENABLE_GLOBAL_REGISTER_VARIABLES
#include "arch/global_regs.h"
#endif

namespace ark::interpreter {
#ifdef PANDA_ENABLE_GLOBAL_REGISTER_VARIABLES

class AccVRegisterT : public VRegisterIface<AccVRegisterT> {
public:
    // NOLINTNEXTLINE(google-explicit-constructor)
    ALWAYS_INLINE inline AccVRegisterT(const AccVRegister &other)
    {
        SetValue(other.GetValue());
        SetTag(other.GetTag());
    }

    // NOLINTNEXTLINE(google-explicit-constructor)
    ALWAYS_INLINE inline operator AccVRegister() const
    {
        return AccVRegister(GetValue(), GetTag());
    }

    ALWAYS_INLINE inline int64_t GetValue() const
    {
        return arch::regs::GetAccValue();
    }

    ALWAYS_INLINE inline void SetValue(int64_t value)
    {
        arch::regs::SetAccValue(value);
    }

    ALWAYS_INLINE inline int64_t GetTag() const
    {
        return arch::regs::GetAccTag();
    }

    ALWAYS_INLINE inline void SetTag(int64_t value)
    {
        arch::regs::SetAccTag(value);
    }

    ~AccVRegisterT() = default;

    DEFAULT_COPY_SEMANTIC(AccVRegisterT);
    DEFAULT_MOVE_SEMANTIC(AccVRegisterT);
};

template <bool IS_DYNAMIC>
class AccVRegisterTRef : public VRegisterRef<AccVRegisterTRef<IS_DYNAMIC>, AccVRegisterT> {
public:
    ALWAYS_INLINE inline explicit AccVRegisterTRef(AccVRegisterT *payload)
        : VRegisterRef<AccVRegisterTRef<IS_DYNAMIC>, AccVRegisterT>(payload)
    {
    }

    ALWAYS_INLINE inline void Move(const StaticVRegisterRef &other)
    {
        this->payload_->SetValue(other.GetValue());
        this->payload_->SetTag(other.GetTag());
    }

    ALWAYS_INLINE inline void Move(const DynamicVRegisterRef &other)
    {
        this->payload_->SetValue(other.GetValue());
    }

    // NOLINTNEXTLINE(google-explicit-constructor)
    ALWAYS_INLINE inline operator std::pair<int64_t, int64_t>() const
    {
        return std::pair<int64_t, int64_t>(this->payload_->GetValue(), this->payload_->GetTag());
    }

    // NOLINTNEXTLINE(bugprone-unhandled-self-assignment, cert-oop54-cpp)
    ALWAYS_INLINE inline AccVRegisterTRef &operator=(const AccVRegisterTRef &other)
    {
        this->payload_->SetValue(other.GetValue());
        this->payload_->SetTag(other.GetTag());
        return *this;
    }

    // NOLINTNEXTLINE(bugprone-unhandled-self-assignment, cert-oop54-cpp)
    ALWAYS_INLINE inline AccVRegisterTRef &operator=(AccVRegisterTRef &&other)
    {
        this->payload_->SetValue(other.GetValue());
        this->payload_->SetTag(other.GetTag());
        return *this;
    }

    ALWAYS_INLINE inline bool HasObject() const
    {
        if constexpr (IS_DYNAMIC) {
            coretypes::TaggedValue v(this->payload_->template GetAs<uint64_t>());
            return v.IsHeapObject();
        }
        return this->payload_->GetTag() == BASE::GC_OBJECT_TYPE;
    }

    ALWAYS_INLINE inline void MovePrimitive(const AccVRegisterTRef &other)
    {
        ASSERT(!other.HasObject());
        this->payload_->SetValue(other.payload_->GetValue());
        if constexpr (!IS_DYNAMIC) {
            this->payload_->SetTag(BASE::PRIMITIVE_TYPE);
        }
    }

    ALWAYS_INLINE inline void MoveReference(const AccVRegisterTRef &other)
    {
        ASSERT(other.HasObject());
        this->payload_->SetValue(other.payload_->GetValue());
        if constexpr (!IS_DYNAMIC) {
            this->payload_->SetTag(BASE::GC_OBJECT_TYPE);
        }
    }

    ALWAYS_INLINE inline void Move(const AccVRegisterTRef &other)
    {
        this->payload_->SetValue(other.payload_->GetValue());
        if constexpr (!IS_DYNAMIC) {
            this->payload_->SetTag(other.payload_->GetTag());
        }
    }

    ALWAYS_INLINE inline void SetPrimitive(int32_t value)
    {
        this->payload_->Set(value);
        if constexpr (!IS_DYNAMIC) {
            this->payload_->SetTag(BASE::PRIMITIVE_TYPE);
        }
    }

    ALWAYS_INLINE inline void SetPrimitive(int64_t value)
    {
        this->payload_->Set(value);
        if constexpr (!IS_DYNAMIC) {
            this->payload_->SetTag(BASE::PRIMITIVE_TYPE);
        }
    }

    ALWAYS_INLINE inline void SetPrimitive(float value)
    {
        this->payload_->Set(value);
        if constexpr (!IS_DYNAMIC) {
            this->payload_->SetTag(BASE::PRIMITIVE_TYPE);
        }
    }

    ALWAYS_INLINE inline void SetPrimitive(double value)
    {
        this->payload_->Set(value);
        if constexpr (!IS_DYNAMIC) {
            this->payload_->SetTag(BASE::PRIMITIVE_TYPE);
        }
    }

    ALWAYS_INLINE inline void SetPrimitive(uint64_t value)
    {
        this->payload_->Set(value);
        if constexpr (!IS_DYNAMIC) {
            this->payload_->SetTag(BASE::PRIMITIVE_TYPE);
        }
    }

    ALWAYS_INLINE inline void SetReference(ObjectHeader *obj)
    {
        this->payload_->Set(obj);
        if constexpr (!IS_DYNAMIC) {
            this->payload_->SetTag(BASE::GC_OBJECT_TYPE);
        }
    }

    ~AccVRegisterTRef() = default;

    DEFAULT_COPY_CTOR(AccVRegisterTRef);
    DEFAULT_MOVE_CTOR(AccVRegisterTRef);

    using BASE = VRegisterRef<AccVRegisterTRef<IS_DYNAMIC>, AccVRegisterT>;
};

#else

using AccVRegisterT = AccVRegister;

#endif  // PANDA_ENABLE_GLOBAL_REGISTER_VARIABLES
}  // namespace ark::interpreter

#endif  // PANDA_INTERPRETER_ACC_VREGISTER_INL_H_
