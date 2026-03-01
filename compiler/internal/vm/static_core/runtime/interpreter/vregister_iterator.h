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
#ifndef PANDA_RUNTIME_VREGISTER_ITERATOR_H_
#define PANDA_RUNTIME_VREGISTER_ITERATOR_H_

namespace ark::interpreter {

template <BytecodeInstruction::Format FORMAT>
class VRegisterIterator {
public:
    // NOLINTNEXTLINE(performance-move-const-arg)
    explicit VRegisterIterator(BytecodeInstruction insn, Frame *frame) : instn_(std::move(insn)), frame_(frame) {}

    ALWAYS_INLINE size_t GetVRegIdx(size_t paramIdx) const
    {
        if constexpr (BytecodeInstruction::IsVregArgsShort(FORMAT)) {
            switch (paramIdx) {
                case 0: {
                    return instn_.GetVReg<FORMAT, 0>();
                }
                case 1: {
                    return instn_.GetVReg<FORMAT, 1>();
                }
                default:
                    UNREACHABLE();
            }
        } else if constexpr (BytecodeInstruction::IsVregArgs(FORMAT)) {
            switch (paramIdx) {
                case 0U: {
                    return instn_.GetVReg<FORMAT, 0U>();
                }
                case 1U: {
                    return instn_.GetVReg<FORMAT, 1U>();
                }
                case 2U: {
                    return instn_.GetVReg<FORMAT, 2U>();
                }
                case 3U: {
                    return instn_.GetVReg<FORMAT, 3U>();
                }
                default:
                    UNREACHABLE();
            }
        } else if constexpr (BytecodeInstruction::IsVregArgsRange(FORMAT)) {
            return instn_.GetVReg<FORMAT, 0>() + paramIdx;
        } else {
            UNREACHABLE();
            return SIZE_MAX;
        }
    }

    template <class T>
    ALWAYS_INLINE T GetAs(size_t paramIdx) const
    {
        return frame_->GetVReg(GetVRegIdx(paramIdx)).template GetAs<T>();
    }

private:
    BytecodeInstruction instn_;
    Frame *frame_;
};

template <BytecodeInstruction::Format FORMAT, bool IS_DYNAMIC>
class DimIterator final : VRegisterIterator<FORMAT> {
public:
    // NOLINTNEXTLINE(performance-move-const-arg)
    explicit DimIterator(BytecodeInstruction insn, Frame *frame) : VRegisterIterator<FORMAT>(std::move(insn), frame) {}

    int32_t Get(size_t paramIdx) const
    {
        return this->template GetAs<int32_t>(paramIdx);
    }
};

}  // namespace ark::interpreter

#endif  // PANDA_RUNTIME_CORETYPES_ARRAY_H_
