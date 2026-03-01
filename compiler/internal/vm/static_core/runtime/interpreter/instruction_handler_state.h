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

#ifndef PANDA_INTERPRETER_INSTRUCTION_HANDLER_STATE_H_
#define PANDA_INTERPRETER_INSTRUCTION_HANDLER_STATE_H_

#include "runtime/interpreter/state.h"
#include "runtime/jit/profiling_data.h"

namespace ark::interpreter {

class InstructionHandlerState {
public:
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    ALWAYS_INLINE InstructionHandlerState(ManagedThread *thread, const uint8_t *pc, Frame *frame,
                                          const void *const *dispatchTable)
        : state_(thread, pc, frame, dispatchTable)
    {
    }

    ALWAYS_INLINE void UpdateInstructionHandlerState(const uint8_t *pc, Frame *frame)
    {
        state_.UpdateState(pc, frame);
    }

    ALWAYS_INLINE ManagedThread *GetThread() const
    {
        return state_.GetThread();
    }

    ALWAYS_INLINE void SetThread(ManagedThread *thread)
    {
        state_.SetThread(thread);
    }

    ALWAYS_INLINE void SetInst(BytecodeInstruction inst)
    {
        state_.SetInst(inst);
    }

    ALWAYS_INLINE Frame *GetFrame() const
    {
        return state_.GetFrame();
    }

    ALWAYS_INLINE void SetFrame(Frame *frame)
    {
        state_.SetFrame(frame);
    }

    ALWAYS_INLINE const void *const *GetDispatchTable() const
    {
        return state_.GetDispatchTable();
    }

    ALWAYS_INLINE void SetDispatchTable(const void *const *dispatchTable)
    {
        return state_.SetDispatchTable(dispatchTable);
    }

    ALWAYS_INLINE void SaveState()
    {
        state_.SaveState();
    }

    ALWAYS_INLINE void RestoreState()
    {
        state_.RestoreState();
    }

    ALWAYS_INLINE uint16_t GetOpcodeExtension() const
    {
        return opcodeExtension_;
    }

    ALWAYS_INLINE void SetOpcodeExtension(uint16_t opcodeExtension)
    {
        opcodeExtension_ = opcodeExtension;
    }

    ALWAYS_INLINE uint8_t GetPrimaryOpcode() const
    {
        return static_cast<unsigned>(GetInst().GetOpcode()) & OPCODE_MASK;
    }

    ALWAYS_INLINE uint8_t GetSecondaryOpcode() const
    {
        return (static_cast<unsigned>(GetInst().GetSecondaryOpcode())) & OPCODE_MASK;
    }

    ALWAYS_INLINE bool IsPrimaryOpcodeValid() const
    {
        return GetInst().IsPrimaryOpcodeValid();
    }

    ALWAYS_INLINE BytecodeInstruction GetInst() const
    {
        return state_.GetInst();
    }

    ALWAYS_INLINE const AccVRegisterT &GetAcc() const
    {
        return state_.GetAcc();
    }

    ALWAYS_INLINE AccVRegisterT &GetAcc()
    {
        return state_.GetAcc();
    }

    ALWAYS_INLINE auto &GetFakeInstBuf()
    {
        return fakeInstBuf_;
    }

    ALWAYS_INLINE uint32_t GetBytecodeOffset() const
    {
        return GetInst().GetAddress() - GetFrame()->GetInstruction();
    }

private:
    static constexpr size_t FAKE_INST_BUF_SIZE = 4;
    static constexpr uint8_t OPCODE_MASK = 0xFFU;

    State state_;
    std::array<uint8_t, FAKE_INST_BUF_SIZE> fakeInstBuf_;
    uint16_t opcodeExtension_ {0};
};

}  // namespace ark::interpreter

#endif  // PANDA_INTERPRETER_INSTRUCTION_HANDLER_STATE_H_
