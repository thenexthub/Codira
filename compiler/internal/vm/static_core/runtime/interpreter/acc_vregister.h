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
#ifndef PANDA_INTERPRETER_ACC_VREGISTER_H_
#define PANDA_INTERPRETER_ACC_VREGISTER_H_

#include <cstddef>
#include <cstdint>

#include "runtime/interpreter/vregister.h"

namespace ark::interpreter {

class AccVRegister : public VRegisterIface<AccVRegister> {
public:
    ALWAYS_INLINE inline AccVRegister() = default;

    ALWAYS_INLINE inline AccVRegister(int64_t payload, int64_t mirror) : payload_(payload), mirror_(mirror) {}

    ALWAYS_INLINE inline int64_t GetValue() const
    {
        return payload_.GetValue();
    }

    ALWAYS_INLINE inline void SetValue(int64_t value)
    {
        payload_.SetValue(value);
    }

    ALWAYS_INLINE inline int64_t GetTag() const
    {
        return mirror_.GetValue();
    }

    ALWAYS_INLINE inline void SetTag(int64_t value)
    {
        mirror_.SetValue(value);
    }

    template <bool IS_DYNAMIC = false>
    ALWAYS_INLINE inline typename std::enable_if<IS_DYNAMIC, DynamicVRegisterRef>::type AsVRegRef()
    {
        return DynamicVRegisterRef(&payload_);
    }

    template <bool IS_DYNAMIC = false>
    ALWAYS_INLINE inline typename std::enable_if<!IS_DYNAMIC, StaticVRegisterRef>::type AsVRegRef()
    {
        return StaticVRegisterRef(&payload_, &mirror_);
    }

    ALWAYS_INLINE static inline constexpr uint32_t GetMirrorOffset()
    {
        return MEMBER_OFFSET(AccVRegister, mirror_);
    }

    ~AccVRegister() = default;

    DEFAULT_COPY_SEMANTIC(AccVRegister);
    DEFAULT_MOVE_SEMANTIC(AccVRegister);

private:
    VRegister payload_;
    VRegister mirror_;
};

}  // namespace ark::interpreter

#endif  // PANDA_INTERPRETER_ACC_VREGISTER_H_
