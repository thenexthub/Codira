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

#ifndef PANDA_INTERPRETER_INSTRUCTION_HANDLER_BASE_H_
#define PANDA_INTERPRETER_INSTRUCTION_HANDLER_BASE_H_

#include <isa_constants_gen.h>
#include "runtime/include/safepoint_timer.h"
#include "runtime/include/method.h"
#include "runtime/include/runtime.h"
#include "runtime/interpreter/instruction_handler_state.h"

namespace ark::interpreter {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_INST()                                                                           \
    LOG(DEBUG, INTERPRETER) << std::hex << std::setw(sizeof(uintptr_t)) << std::setfill('0') \
                            << reinterpret_cast<uintptr_t>(this->GetInst().GetAddress()) << std::dec << ": "

#ifdef PANDA_ENABLE_GLOBAL_REGISTER_VARIABLES
#include "arch/global_regs.h"

class StaticFrameHandlerT : public StaticFrameHandler {
public:
    ALWAYS_INLINE inline explicit StaticFrameHandlerT(Frame *frame) : StaticFrameHandler(frame) {}

private:
    ALWAYS_INLINE inline interpreter::VRegister *GetVRegisters()
    {
        return reinterpret_cast<interpreter::VRegister *>(arch::regs::GetFp());
    }

    ALWAYS_INLINE inline interpreter::VRegister *GetMirrorVRegisters()
    {
        return reinterpret_cast<interpreter::VRegister *>(arch::regs::GetMirrorFp());
    }
};

class DynamicFrameHandlerT : public DynamicFrameHandler {
public:
    ALWAYS_INLINE inline explicit DynamicFrameHandlerT(Frame *frame) : DynamicFrameHandler(frame) {}

private:
    ALWAYS_INLINE inline interpreter::VRegister *GetVRegisters()
    {
        return reinterpret_cast<interpreter::VRegister *>(arch::regs::GetFp());
    }
};

#else

using StaticFrameHandlerT = StaticFrameHandler;
using DynamicFrameHandlerT = DynamicFrameHandler;

#endif  // PANDA_ENABLE_GLOBAL_REGISTER_VARIABLES

template <class RuntimeIfaceT, bool IS_DYNAMIC>
class InstructionHandlerBase {
public:
    ALWAYS_INLINE explicit InstructionHandlerBase(InstructionHandlerState *state) : state_(state) {}

    ALWAYS_INLINE uint16_t GetExceptionOpcode() const
    {
        // Need to call GetInst().GetOpcode() in this case too, otherwise compiler can generate non optimal code
        return static_cast<unsigned>(GetInst().GetPrimaryOpcode()) + state_->GetOpcodeExtension();
    }

    ALWAYS_INLINE uint8_t GetPrimaryOpcode() const
    {
        return static_cast<unsigned>(GetInst().GetPrimaryOpcode());
    }

    ALWAYS_INLINE uint8_t GetSecondaryOpcode() const
    {
        return static_cast<unsigned>(GetInst().GetSecondaryOpcode());
    }

    ALWAYS_INLINE bool IsPrimaryOpcodeValid() const
    {
        return GetInst().IsPrimaryOpcodeValid();
    }

    void DumpVRegs()
    {
#if PANDA_ENABLE_SLOW_DEBUG
        // Skip dump if logger is disable. This allows us to speed up interpretation in the 'Debug' build.
        if (!Logger::IsLoggingOn(Logger::Level::DEBUG, Logger::Component::INTERPRETER)) {
            return;
        }

        static constexpr uint64_t STANDARD_DEBUG_INDENT = 5;
        LOG(DEBUG, INTERPRETER) << PandaString(STANDARD_DEBUG_INDENT, ' ') << "acc."
                                << GetAccAsVReg<IS_DYNAMIC>().DumpVReg();
        auto frameHandler = GetFrameHandler();
        for (size_t i = 0; i < GetFrame()->GetSize(); ++i) {
            LOG(DEBUG, INTERPRETER) << PandaString(STANDARD_DEBUG_INDENT, ' ') << "v" << i << "."
                                    << frameHandler.GetVReg(i).DumpVReg();
        }
#endif
    }

    ALWAYS_INLINE uint32_t UpdateBytecodeOffset()
    {
        auto pc = GetBytecodeOffset();
        GetFrame()->SetBytecodeOffset(pc);
        return pc;
    }

    void InstrumentInstruction()
    {
        // Should set ACC to Frame, so that ACC will be marked when GC
        GetFrame()->SetAcc(GetAcc());

        auto pc = UpdateBytecodeOffset();
        RuntimeIfaceT::GetNotificationManager()->BytecodePcChangedEvent(GetThread(), GetFrame()->GetMethod(), pc);

        // BytecodePcChangedEvent hook can call the GC, so we need to update the ACC
        GetAcc() = GetFrame()->GetAcc();
    }

    void InstrumentForceReturn()
    {
        interpreter::AccVRegister result;  // empty result, because force exit
        GetAcc() = result;
        GetFrame()->GetAcc() = result;
    }

    ALWAYS_INLINE const AccVRegisterT &GetAcc() const
    {
        return state_->GetAcc();
    }

    ALWAYS_INLINE AccVRegisterT &GetAcc()
    {
        return state_->GetAcc();
    }

#ifdef PANDA_ENABLE_GLOBAL_REGISTER_VARIABLES
    template <bool IS_DYNAMIC_T = IS_DYNAMIC>
    ALWAYS_INLINE AccVRegisterTRef<IS_DYNAMIC_T> GetAccAsVReg()
    {
        return AccVRegisterTRef<IS_DYNAMIC_T>(&state_->GetAcc());
    }
#else
    template <bool IS_DYNAMIC_T = IS_DYNAMIC>
    ALWAYS_INLINE typename std::enable_if<IS_DYNAMIC_T, DynamicVRegisterRef>::type GetAccAsVReg()
    {
        return state_->GetAcc().template AsVRegRef<IS_DYNAMIC_T>();
    }

    template <bool IS_DYNAMIC_T = IS_DYNAMIC>
    ALWAYS_INLINE typename std::enable_if<!IS_DYNAMIC_T, StaticVRegisterRef>::type GetAccAsVReg()
    {
        return state_->GetAcc().template AsVRegRef<IS_DYNAMIC_T>();
    }
#endif  // PANDA_ENABLE_GLOBAL_REGISTER_VARIABLES

    ALWAYS_INLINE BytecodeInstruction GetInst() const
    {
        return state_->GetInst();
    }

    void DebugDump();

    template <bool IS_DYNAMIC_T = IS_DYNAMIC>
    ALWAYS_INLINE typename std::enable_if<IS_DYNAMIC_T, DynamicFrameHandlerT>::type GetFrameHandler()
    {
        return DynamicFrameHandlerT(state_->GetFrame());
    }

    template <bool IS_DYNAMIC_T = IS_DYNAMIC>
    ALWAYS_INLINE typename std::enable_if<!IS_DYNAMIC_T, StaticFrameHandlerT>::type GetFrameHandler()
    {
        return StaticFrameHandlerT(state_->GetFrame());
    }

    template <bool IS_DYNAMIC_T = IS_DYNAMIC>
    ALWAYS_INLINE typename std::enable_if<IS_DYNAMIC_T, DynamicFrameHandler>::type GetFrameHandler(Frame *frame)
    {
        return DynamicFrameHandler(frame);
    }

    template <bool IS_DYNAMIC_T = IS_DYNAMIC>
    ALWAYS_INLINE typename std::enable_if<!IS_DYNAMIC_T, StaticFrameHandler>::type GetFrameHandler(Frame *frame)
    {
        return StaticFrameHandler(frame);
    }

    ALWAYS_INLINE Frame *GetFrame() const
    {
        return state_->GetFrame();
    }

    ALWAYS_INLINE void SetFrame(Frame *frame)
    {
        state_->SetFrame(frame);
    }

    ALWAYS_INLINE ManagedThread *GetThread() const
    {
        return state_->GetThread();
    }

    ALWAYS_INLINE const void *const *GetDispatchTable() const
    {
        return state_->GetDispatchTable();
    }

protected:
    template <BytecodeInstruction::Format FORMAT, bool CAN_THROW>
    ALWAYS_INLINE void MoveToNextInst()
    {
        SetInst(GetInst().template GetNext<FORMAT>());

        if (CAN_THROW) {
            SetOpcodeExtension(0);
        }
    }

    template <bool CAN_THROW>
    ALWAYS_INLINE void JumpToInst(int32_t offset)
    {
        SetInst(GetInst().JumpTo(offset));

        if (CAN_THROW) {
            SetOpcodeExtension(0);
        }
    }

    template <bool CAN_THROW>
    ALWAYS_INLINE void JumpTo(const uint8_t *pc)
    {
        SetInst(BytecodeInstruction(pc));

        if (CAN_THROW) {
            SetOpcodeExtension(0);
        }
    }

    ALWAYS_INLINE void MoveToExceptionHandler()
    {
        SetOpcodeExtension(UINT8_MAX + NUM_PREFIXED + 1);
        SetOpcodeExtension(GetOpcodeExtension() - GetPrimaryOpcode());
    }

    ALWAYS_INLINE void SetThread(ManagedThread *thread)
    {
        state_->SetThread(thread);
    }

    ALWAYS_INLINE void SetInst(BytecodeInstruction inst)
    {
        state_->SetInst(inst);
    }

    ALWAYS_INLINE void SetDispatchTable(const void *const *dispatchTable)
    {
        return state_->SetDispatchTable(dispatchTable);
    }

    ALWAYS_INLINE void SaveState()
    {
        state_->SaveState();
    }

    ALWAYS_INLINE void RestoreState()
    {
        state_->RestoreState();
    }

    ALWAYS_INLINE uint16_t GetOpcodeExtension() const
    {
        return state_->GetOpcodeExtension();
    }

    ALWAYS_INLINE void SetOpcodeExtension(uint16_t opcodeExtension)
    {
        state_->SetOpcodeExtension(opcodeExtension);
    }

    ALWAYS_INLINE auto &GetFakeInstBuf()
    {
        return state_->GetFakeInstBuf();
    }

    template <bool IS_CALL>
    ALWAYS_INLINE void UpdateHotness(Method *method)
    {
        method->DecrementHotnessCounter<IS_CALL>(0, nullptr);
    }

    ALWAYS_INLINE uint32_t GetBytecodeOffset() const
    {
        return state_->GetBytecodeOffset();
    }

    ALWAYS_INLINE InstructionHandlerState *GetInstructionHandlerState()
    {
        return state_;
    }

    template <bool TAKEN>
    ALWAYS_INLINE void UpdateBranchStatistics()
    {
        ProfilingData *profData = this->GetFrame()->GetMethod()->GetProfilingDataWithoutCheck();
        if (profData != nullptr && profData->IsBranchProfilingEnabled()) {
            auto pc = this->GetBytecodeOffset();
            if constexpr (TAKEN) {
                profData->UpdateBranchTaken(pc);
            } else {
                profData->UpdateBranchNotTaken(pc);
            }
        }
    }

    ALWAYS_INLINE void UpdateThrowStatistics()
    {
        ProfilingData *profData = this->GetFrame()->GetMethod()->GetProfilingDataWithoutCheck();
        if (profData != nullptr) {
            auto pc = this->GetBytecodeOffset();
            profData->UpdateThrowTaken(pc);
        }
    }

    ALWAYS_INLINE bool UpdateHotnessOSR(Method *method, int offset)
    {
        ASSERT(ArchTraits<RUNTIME_ARCH>::SUPPORT_OSR);
        if (this->GetFrame()->IsDeoptimized() || !Runtime::GetOptions().IsCompilerEnableOsr()) {
            method->DecrementHotnessCounter<false>(0, nullptr);
            return false;
        }
        return method->DecrementHotnessCounter<false>(this->GetBytecodeOffset() + offset, &this->GetAcc(), true);
    }

    ALWAYS_INLINE bool InstrumentBranches(int32_t offset)
    {
        // Offset may be 0 in case of infinite empty loops (see issue #5301)
        if (offset > 0) {
            return false;
        }
        SAFEPOINT_TIME_CHECKER(SafepointTimerTable::ResetTimers(GetThread()->GetInternalId(), true));
        if (this->GetThread()->TestAllFlags()) {
            this->GetFrame()->SetAcc(this->GetAcc());
            RuntimeIfaceT::Safepoint();
            this->GetAcc() = this->GetFrame()->GetAcc();
        }
        if constexpr (ArchTraits<RUNTIME_ARCH>::SUPPORT_OSR) {
            if (UpdateHotnessOSR(this->GetFrame()->GetMethod(), offset)) {
                static_assert(static_cast<unsigned>(BytecodeInstruction::Opcode::RETURN_VOID) <=
                              std::numeric_limits<uint8_t>::max());
                this->GetFakeInstBuf()[0] = static_cast<uint8_t>(BytecodeInstruction::Opcode::RETURN_VOID);
                this->SetInst(BytecodeInstruction(this->GetFakeInstBuf().data()));
                return true;
            }
        } else {
            this->UpdateHotness<false>(this->GetFrame()->GetMethod());
        }
        return false;
    }

private:
    InstructionHandlerState *state_;
};

}  // namespace ark::interpreter

#endif  // PANDA_INTERPRETER_INSTRUCTION_HANDLER_BASE_H_
