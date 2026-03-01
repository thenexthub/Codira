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

#ifndef PANDA_INTERPRETER_INL_H_
#define PANDA_INTERPRETER_INL_H_

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <limits>
#include <memory>
#include <type_traits>
#include <unordered_map>

#include "libarkfile/bytecode_instruction.h"
#include "intrinsics.h"
#include "libarkbase/events/events.h"
#include "libarkbase/macros.h"
#include "libarkbase/utils/logger.h"
#include "libarkbase/utils/type_helpers.h"
#include "libarkfile/bytecode_instruction-inl.h"
#include "libarkfile/file_items.h"
#include "libarkfile/shorty_iterator.h"
#include "runtime/bridge/bridge.h"
#include "runtime/include/class.h"
#include "runtime/include/coretypes/array-inl.h"
#include "runtime/include/coretypes/dyn_objects.h"
#include "runtime/include/coretypes/line_string.h"
#include "runtime/include/exceptions.h"
#include "runtime/include/locks.h"
#include "runtime/include/method-inl.h"
#include "runtime/include/object_header-inl.h"
#include "runtime/include/runtime.h"
#include "runtime/include/runtime_notification.h"
#include "runtime/include/thread-inl.h"
#include "runtime/include/value-inl.h"
#include "runtime/interpreter/acc_vregister.h"
#include "runtime/interpreter/arch/macros.h"
#include "runtime/interpreter/frame.h"
#include "runtime/interpreter/instruction_handler_base.h"
#include "runtime/interpreter/math_helpers.h"
#include "runtime/interpreter/runtime_interface.h"
#include "runtime/interpreter/vregister_iterator.h"
#include "runtime/jit/profiling_data.h"
#include "runtime/mem/vm_handle.h"
#include "runtime/handle_base-inl.h"

// ALWAYS_INLINE is mandatory attribute for handlers. There are cases which will be failed without it.

namespace ark::interpreter {

template <class RuntimeIfaceT, bool IS_DYNAMIC, bool IS_PROFILE_ENABLED = false>
void ExecuteImpl(ManagedThread *thread, const uint8_t *pc, Frame *frame, bool jumpToEh);
template <class RuntimeIfaceT, bool IS_DYNAMIC, bool IS_PROFILE_ENABLED = false>
void ExecuteImplDebug(ManagedThread *thread, const uint8_t *pc, Frame *frame, bool jumpToEh);

template <class RuntimeIfaceT, bool IS_DYNAMIC, bool IS_PROFILE_ENABLED = false>
void ExecuteImplInner(ManagedThread *thread, const uint8_t *pc, Frame *frame, bool jumpToEh);

class FrameHelperDefault {
public:
    template <BytecodeInstruction::Format FORMAT, class InstructionHandler>
    ALWAYS_INLINE static uint32_t GetNumberActualArgsDyn(InstructionHandler *instrHandler)
    {
        // +1 means function object itself
        return instrHandler->GetInst().template GetImm<FORMAT, 0>() + 1;
    }

    template <BytecodeInstruction::Format FORMAT, class InstructionHandler>
    ALWAYS_INLINE static void CopyArgumentsDyn([[maybe_unused]] InstructionHandler *instrHandler,
                                               [[maybe_unused]] Frame *newFrame, [[maybe_unused]] uint32_t numVregs,
                                               [[maybe_unused]] uint32_t numActualArgs)
    {
    }

    template <class RuntimeIfaceT>
    ALWAYS_INLINE static Frame *CreateFrame([[maybe_unused]] ManagedThread *thread, uint32_t nregsSize, Method *method,
                                            Frame *prev, uint32_t nregs, uint32_t numActualArgs)
    {
        return RuntimeIfaceT::CreateFrameWithActualArgsAndSize(nregsSize, nregs, numActualArgs, method, prev);
    }
};

template <class RuntimeIfaceT, bool IS_DYNAMIC, bool IS_DEBUG, bool IS_PROFILE_ENABLED = false>
class InstructionHandler : public InstructionHandlerBase<RuntimeIfaceT, IS_DYNAMIC> {
public:
    ALWAYS_INLINE inline explicit InstructionHandler(InstructionHandlerState *state)
        : InstructionHandlerBase<RuntimeIfaceT, IS_DYNAMIC>(state)
    {
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleNop()
    {
        LOG_INST() << "nop";
        this->template MoveToNextInst<FORMAT, false>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleFldaiDyn()
    {
        auto imm = bit_cast<double>(this->GetInst().template GetImm<FORMAT>());
        LOG_INST() << "fldai.dyn " << imm;
        this->GetAcc().SetValue(coretypes::TaggedValue(imm).GetRawData());
        this->template MoveToNextInst<FORMAT, false>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleLdaiDyn()
    {
        int32_t imm = this->GetInst().template GetImm<FORMAT>();
        LOG_INST() << "ldai.dyn " << std::hex << "0x" << imm;
        this->GetAcc().SetValue(coretypes::TaggedValue(imm).GetRawData());
        this->template MoveToNextInst<FORMAT, false>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleMov()
    {
        uint16_t vd = this->GetInst().template GetVReg<FORMAT, 0>();
        uint16_t vs = this->GetInst().template GetVReg<FORMAT, 1>();
        LOG_INST() << "mov v" << vd << ", v" << vs;
        auto curFrameHandler = this->GetFrameHandler();
        curFrameHandler.GetVReg(vd).MovePrimitive(curFrameHandler.GetVReg(vs));
        this->template MoveToNextInst<FORMAT, false>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleMovWide()
    {
        uint16_t vd = this->GetInst().template GetVReg<FORMAT, 0>();
        uint16_t vs = this->GetInst().template GetVReg<FORMAT, 1>();
        LOG_INST() << "mov.64 v" << vd << ", v" << vs;
        auto curFrameHandler = this->GetFrameHandler();
        curFrameHandler.GetVReg(vd).MovePrimitive(curFrameHandler.GetVReg(vs));
        this->template MoveToNextInst<FORMAT, false>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleMovObj()
    {
        uint16_t vd = this->GetInst().template GetVReg<FORMAT, 0>();
        uint16_t vs = this->GetInst().template GetVReg<FORMAT, 1>();
        LOG_INST() << "mov.obj v" << vd << ", v" << vs;
        auto curFrameHandler = this->GetFrameHandler();
        curFrameHandler.GetVReg(vd).MoveReference(curFrameHandler.GetVReg(vs));
        this->template MoveToNextInst<FORMAT, false>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleMovDyn()
    {
        uint16_t vd = this->GetInst().template GetVReg<FORMAT, 0>();
        uint16_t vs = this->GetInst().template GetVReg<FORMAT, 1>();
        LOG_INST() << "mov.dyn v" << vd << ", v" << vs;
        auto curFrameHandler = this->GetFrameHandler();
        curFrameHandler.GetVReg(vd).Move(curFrameHandler.GetVReg(vs));
        this->template MoveToNextInst<FORMAT, false>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleMovi()
    {
        int32_t imm = this->GetInst().template GetImm<FORMAT>();
        uint16_t vd = this->GetInst().template GetVReg<FORMAT>();
        LOG_INST() << "movi v" << vd << ", " << std::hex << "0x" << imm;
        this->GetFrameHandler().GetVReg(vd).SetPrimitive(imm);
        this->template MoveToNextInst<FORMAT, false>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleMoviWide()
    {
        int64_t imm = this->GetInst().template GetImm<FORMAT>();
        uint16_t vd = this->GetInst().template GetVReg<FORMAT>();
        LOG_INST() << "movi.64 v" << vd << ", " << std::hex << "0x" << imm;
        this->GetFrameHandler().GetVReg(vd).SetPrimitive(imm);
        this->template MoveToNextInst<FORMAT, false>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleFmovi()
    {
        auto imm = bit_cast<float>(this->GetInst().template GetImm<FORMAT>());
        uint16_t vd = this->GetInst().template GetVReg<FORMAT>();
        LOG_INST() << "fmovi v" << vd << ", " << imm;
        this->GetFrameHandler().GetVReg(vd).SetPrimitive(imm);
        this->template MoveToNextInst<FORMAT, false>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleFmoviWide()
    {
        auto imm = bit_cast<double>(this->GetInst().template GetImm<FORMAT>());
        uint16_t vd = this->GetInst().template GetVReg<FORMAT>();
        LOG_INST() << "fmovi.64 v" << vd << ", " << imm;
        this->GetFrameHandler().GetVReg(vd).SetPrimitive(imm);
        this->template MoveToNextInst<FORMAT, false>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleMovNull()
    {
        uint16_t vd = this->GetInst().template GetVReg<FORMAT>();
        LOG_INST() << "mov.null v" << vd;
        this->GetFrameHandler().GetVReg(vd).SetReference(nullptr);
        this->template MoveToNextInst<FORMAT, false>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleLda()
    {
        uint16_t vs = this->GetInst().template GetVReg<FORMAT>();
        LOG_INST() << "lda v" << vs;
        this->GetAccAsVReg().SetPrimitive(this->GetFrame()->GetVReg(vs).Get());
        this->template MoveToNextInst<FORMAT, false>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleLdaWide()
    {
        uint16_t vs = this->GetInst().template GetVReg<FORMAT>();
        LOG_INST() << "lda.64 v" << vs;
        this->GetAccAsVReg().SetPrimitive(this->GetFrame()->GetVReg(vs).GetLong());
        this->template MoveToNextInst<FORMAT, false>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleLdaObj()
    {
        uint16_t vs = this->GetInst().template GetVReg<FORMAT>();
        LOG_INST() << "lda.obj v" << vs;
        this->GetAccAsVReg().SetReference(this->GetFrame()->GetVReg(vs).GetReference());
        this->template MoveToNextInst<FORMAT, false>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleLdai()
    {
        int32_t imm = this->GetInst().template GetImm<FORMAT>();
        LOG_INST() << "ldai " << std::hex << "0x" << imm;
        this->GetAccAsVReg().SetPrimitive(imm);
        this->template MoveToNextInst<FORMAT, false>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleLdaiWide()
    {
        int64_t imm = this->GetInst().template GetImm<FORMAT>();
        LOG_INST() << "ldai.64 " << std::hex << "0x" << imm;
        this->GetAccAsVReg().SetPrimitive(imm);
        this->template MoveToNextInst<FORMAT, false>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleFldai()
    {
        auto imm = bit_cast<float>(this->GetInst().template GetImm<FORMAT>());
        LOG_INST() << "fldai " << imm;
        this->GetAccAsVReg().SetPrimitive(imm);
        this->template MoveToNextInst<FORMAT, false>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleFldaiWide()
    {
        auto imm = bit_cast<double>(this->GetInst().template GetImm<FORMAT>());
        LOG_INST() << "fldai.64 " << imm;
        this->GetAccAsVReg().SetPrimitive(imm);
        this->template MoveToNextInst<FORMAT, false>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleLdaStr()
    {
        auto stringId = this->GetInst().template GetId<FORMAT>();
        LOG_INST() << "lda.str " << std::hex << "0x" << stringId;

        this->GetFrame()->SetAcc(this->GetAcc());
        PandaVM *vm = this->GetThread()->GetVM();
        vm->HandleLdaStr(this->GetFrame(), stringId);
        if (UNLIKELY(this->GetThread()->HasPendingException())) {
            this->MoveToExceptionHandler();
        } else {
            this->GetAcc() = this->GetFrame()->GetAcc();
            this->template MoveToNextInst<FORMAT, false>();
        }
    }

    template <class T>
    ALWAYS_INLINE bool CheckLoadConstOp(coretypes::Array *array, T elem)
    {
        if constexpr (std::is_same_v<T, ObjectHeader *>) {
            if (elem != nullptr) {
                auto *arrayClass = array->ClassAddr<Class>();
                auto *elementClass = arrayClass->GetComponentType();
                if (UNLIKELY(!elem->IsInstanceOf(elementClass))) {
                    RuntimeIfaceT::ThrowArrayStoreException(arrayClass, elem->template ClassAddr<Class>());
                    return false;
                }
            }
        }
        return true;
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleLdaType()
    {
        auto typeId = this->GetInst().template GetId<FORMAT>();
        LOG_INST() << "lda.type " << std::hex << "0x" << typeId;
        Class *type = ResolveType(typeId);
        if (LIKELY(type != nullptr)) {
            this->GetAccAsVReg().SetReference(type->GetManagedObject());
            this->template MoveToNextInst<FORMAT, false>();
        } else {
            this->MoveToExceptionHandler();
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleLdaNull()
    {
        LOG_INST() << "lda.null";
        this->GetAccAsVReg().SetReference(nullptr);
        this->template MoveToNextInst<FORMAT, false>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleSta()
    {
        uint16_t vd = this->GetInst().template GetVReg<FORMAT>();
        LOG_INST() << "sta v" << vd;
        this->GetFrameHandler().GetVReg(vd).SetPrimitive(this->GetAcc().Get());
        this->template MoveToNextInst<FORMAT, false>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleStaWide()
    {
        uint16_t vd = this->GetInst().template GetVReg<FORMAT>();
        LOG_INST() << "sta.64 v" << vd;
        this->GetFrameHandler().GetVReg(vd).SetPrimitive(this->GetAcc().GetValue());
        this->template MoveToNextInst<FORMAT, false>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleStaObj()
    {
        uint16_t vd = this->GetInst().template GetVReg<FORMAT>();
        LOG_INST() << "sta.obj v" << vd;
        this->GetFrameHandler().GetVReg(vd).SetReference(this->GetAcc().GetReference());
        this->template MoveToNextInst<FORMAT, false>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleStaDyn()
    {
        uint16_t vd = this->GetInst().template GetVReg<FORMAT>();
        LOG_INST() << "sta.dyn v" << vd;
        this->GetFrameHandler().GetVReg(vd).Move(this->GetAccAsVReg());
        this->template MoveToNextInst<FORMAT, false>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleJmp()
    {
        int32_t imm = this->GetInst().template GetImm<FORMAT>();
        LOG_INST() << "jmp " << std::hex << "0x" << imm;
        if (!this->InstrumentBranches(imm)) {
            this->template JumpToInst<false>(imm);
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleCmpWide()
    {
        LOG_INST() << "cmp.64 ->";
        HandleBinaryOp2<FORMAT, int64_t, math_helpers::cmp>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleUcmp()
    {
        LOG_INST() << "ucmp ->";
        HandleBinaryOp2<FORMAT, uint32_t, math_helpers::cmp>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleUcmpWide()
    {
        LOG_INST() << "ucmp.64 ->";
        HandleBinaryOp2<FORMAT, uint64_t, math_helpers::cmp>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleFcmpl()
    {
        LOG_INST() << "fcmpl ->";
        HandleBinaryOp2<FORMAT, float, math_helpers::fcmpl>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleFcmplWide()
    {
        LOG_INST() << "fcmpl.64 ->";
        HandleBinaryOp2<FORMAT, double, math_helpers::fcmpl>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleFcmpg()
    {
        LOG_INST() << "fcmpg ->";
        HandleBinaryOp2<FORMAT, float, math_helpers::fcmpg>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleFcmpgWide()
    {
        LOG_INST() << "fcmpg.64 ->";
        HandleBinaryOp2<FORMAT, double, math_helpers::fcmpg>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleJeqz()
    {
        LOG_INST() << "jeqz ->";
        HandleCondJmpz<FORMAT, std::equal_to>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleJnez()
    {
        LOG_INST() << "jnez ->";
        HandleCondJmpz<FORMAT, std::not_equal_to>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleJltz()
    {
        LOG_INST() << "jltz ->";
        HandleCondJmpz<FORMAT, std::less>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleJgtz()
    {
        LOG_INST() << "jgtz ->";
        HandleCondJmpz<FORMAT, std::greater>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleJlez()
    {
        LOG_INST() << "jlez ->";
        HandleCondJmpz<FORMAT, std::less_equal>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleJgez()
    {
        LOG_INST() << "jgez ->";
        HandleCondJmpz<FORMAT, std::greater_equal>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleJeq()
    {
        LOG_INST() << "jeq ->";
        HandleCondJmp<FORMAT, std::equal_to>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleJne()
    {
        LOG_INST() << "jne ->";
        HandleCondJmp<FORMAT, std::not_equal_to>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleJlt()
    {
        LOG_INST() << "jlt ->";
        HandleCondJmp<FORMAT, std::less>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleJgt()
    {
        LOG_INST() << "jgt ->";
        HandleCondJmp<FORMAT, std::greater>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleJle()
    {
        LOG_INST() << "jle ->";
        HandleCondJmp<FORMAT, std::less_equal>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleJge()
    {
        LOG_INST() << "jge ->";
        HandleCondJmp<FORMAT, std::greater_equal>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleJeqzObj()
    {
        LOG_INST() << "jeqz.obj ->";
        HandleCondJmpzObj<FORMAT, std::equal_to>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleJnezObj()
    {
        LOG_INST() << "jnez.obj ->";
        HandleCondJmpzObj<FORMAT, std::not_equal_to>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleJeqObj()
    {
        LOG_INST() << "jeq.obj ->";
        HandleCondJmpObj<FORMAT, std::equal_to>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleJneObj()
    {
        LOG_INST() << "jne.obj ->";
        HandleCondJmpObj<FORMAT, std::not_equal_to>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleAdd2()
    {
        LOG_INST() << "add2 ->";
        HandleBinaryOp2<FORMAT, int32_t, math_helpers::Plus>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleAdd2Wide()
    {
        LOG_INST() << "add2.64 ->";
        HandleBinaryOp2<FORMAT, int64_t, math_helpers::Plus>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleFadd2()
    {
        LOG_INST() << "fadd2 ->";
        HandleBinaryOp2<FORMAT, float, std::plus>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleFadd2Wide()
    {
        LOG_INST() << "fadd2.64 ->";
        HandleBinaryOp2<FORMAT, double, std::plus>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleSub2()
    {
        LOG_INST() << "sub2 ->";
        HandleBinaryOp2<FORMAT, int32_t, math_helpers::Minus>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleSub2Wide()
    {
        LOG_INST() << "sub2.64 ->";
        HandleBinaryOp2<FORMAT, int64_t, math_helpers::Minus>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleFsub2()
    {
        LOG_INST() << "fsub2 ->";
        HandleBinaryOp2<FORMAT, float, std::minus>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleFsub2Wide()
    {
        LOG_INST() << "fsub2.64 ->";
        HandleBinaryOp2<FORMAT, double, std::minus>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleMul2()
    {
        LOG_INST() << "mul2 ->";
        HandleBinaryOp2<FORMAT, int32_t, math_helpers::Multiplies>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleMul2Wide()
    {
        LOG_INST() << "mul2.64 ->";
        HandleBinaryOp2<FORMAT, int64_t, math_helpers::Multiplies>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleFmul2()
    {
        LOG_INST() << "fmul2 ->";
        HandleBinaryOp2<FORMAT, float, std::multiplies>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleFmul2Wide()
    {
        LOG_INST() << "fmul2.64 ->";
        HandleBinaryOp2<FORMAT, double, std::multiplies>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleFdiv2()
    {
        LOG_INST() << "fdiv2 ->";
        HandleBinaryOp2<FORMAT, float, std::divides>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleFdiv2Wide()
    {
        LOG_INST() << "fdiv2.64 ->";
        HandleBinaryOp2<FORMAT, double, std::divides>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleFmod2()
    {
        LOG_INST() << "fmod2 ->";
        HandleBinaryOp2<FORMAT, float, math_helpers::fmodulus>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleFmod2Wide()
    {
        LOG_INST() << "fmod2.64 ->";
        HandleBinaryOp2<FORMAT, double, math_helpers::fmodulus>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleAnd2()
    {
        LOG_INST() << "and2 ->";
        HandleBinaryOp2<FORMAT, int32_t, std::bit_and>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleAnd2Wide()
    {
        LOG_INST() << "and2.64 ->";
        HandleBinaryOp2<FORMAT, int64_t, std::bit_and>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleOr2()
    {
        LOG_INST() << "or2 ->";
        HandleBinaryOp2<FORMAT, int32_t, std::bit_or>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleOr2Wide()
    {
        LOG_INST() << "or2.64 ->";
        HandleBinaryOp2<FORMAT, int64_t, std::bit_or>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleXor2()
    {
        LOG_INST() << "xor2 ->";
        HandleBinaryOp2<FORMAT, int32_t, std::bit_xor>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleXor2Wide()
    {
        LOG_INST() << "xor2.64 ->";
        HandleBinaryOp2<FORMAT, int64_t, std::bit_xor>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleShl2()
    {
        LOG_INST() << "shl2 ->";
        HandleBinaryOp2<FORMAT, int32_t, math_helpers::bit_shl>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleShl2Wide()
    {
        LOG_INST() << "shl2.64 ->";
        HandleBinaryOp2<FORMAT, int64_t, math_helpers::bit_shl>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleShr2()
    {
        LOG_INST() << "shr2 ->";
        HandleBinaryOp2<FORMAT, int32_t, math_helpers::bit_shr>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleShr2Wide()
    {
        LOG_INST() << "shr2.64 ->";
        HandleBinaryOp2<FORMAT, int64_t, math_helpers::bit_shr>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleAshr2()
    {
        LOG_INST() << "ashr2 ->";
        HandleBinaryOp2<FORMAT, int32_t, math_helpers::bit_ashr>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleAshr2Wide()
    {
        LOG_INST() << "ashr2.64 ->";
        HandleBinaryOp2<FORMAT, int64_t, math_helpers::bit_ashr>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleDiv2()
    {
        LOG_INST() << "div2 ->";
        HandleBinaryOp2<FORMAT, int32_t, math_helpers::idivides, true>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleDiv2Wide()
    {
        LOG_INST() << "div2.64 ->";
        HandleBinaryOp2<FORMAT, int64_t, math_helpers::idivides, true>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleMod2()
    {
        LOG_INST() << "mod2 ->";
        HandleBinaryOp2<FORMAT, int32_t, math_helpers::imodulus, true>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleMod2Wide()
    {
        LOG_INST() << "mod2.64 ->";
        HandleBinaryOp2<FORMAT, int64_t, math_helpers::imodulus, true>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleDivu2()
    {
        LOG_INST() << "divu2 ->";
        HandleBinaryOp2<FORMAT, uint32_t, math_helpers::idivides, true>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleDivu2Wide()
    {
        LOG_INST() << "divu2.64 ->";
        HandleBinaryOp2<FORMAT, uint64_t, math_helpers::idivides, true>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleModu2()
    {
        LOG_INST() << "modu2 ->";
        HandleBinaryOp2<FORMAT, uint32_t, math_helpers::imodulus, true>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleModu2Wide()
    {
        LOG_INST() << "modu2.64 ->";
        HandleBinaryOp2<FORMAT, uint64_t, math_helpers::imodulus, true>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleAdd2v()
    {
        LOG_INST() << "add2v ->";
        HandleBinaryOp2V<FORMAT, int32_t, math_helpers::Plus>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleAdd2vWide()
    {
        LOG_INST() << "add2v.64 ->";
        HandleBinaryOp2V<FORMAT, int64_t, math_helpers::Plus>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleFadd2v()
    {
        LOG_INST() << "fadd2v ->";
        HandleBinaryOp2V<FORMAT, float, std::plus>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleFadd2vWide()
    {
        LOG_INST() << "fadd2v.64 ->";
        HandleBinaryOp2V<FORMAT, double, std::plus>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleSub2v()
    {
        LOG_INST() << "sub2v ->";
        HandleBinaryOp2V<FORMAT, int32_t, math_helpers::Minus>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleSub2vWide()
    {
        LOG_INST() << "sub2v.64 ->";
        HandleBinaryOp2V<FORMAT, int64_t, math_helpers::Minus>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleFsub2v()
    {
        LOG_INST() << "fsub2v ->";
        HandleBinaryOp2V<FORMAT, float, std::minus>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleFsub2vWide()
    {
        LOG_INST() << "fsub2v.64 ->";
        HandleBinaryOp2V<FORMAT, double, std::minus>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleMul2v()
    {
        LOG_INST() << "mul2v ->";
        HandleBinaryOp2V<FORMAT, int32_t, math_helpers::Multiplies>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleMul2vWide()
    {
        LOG_INST() << "mul2v.64 ->";
        HandleBinaryOp2V<FORMAT, int64_t, math_helpers::Multiplies>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleFmul2v()
    {
        LOG_INST() << "fmul2v ->";
        HandleBinaryOp2V<FORMAT, float, std::multiplies>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleFmul2vWide()
    {
        LOG_INST() << "fmul2v.64 ->";
        HandleBinaryOp2V<FORMAT, double, std::multiplies>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleFdiv2v()
    {
        LOG_INST() << "fdiv2v ->";
        HandleBinaryOp2V<FORMAT, float, std::divides>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleFdiv2vWide()
    {
        LOG_INST() << "fdiv2v.64 ->";
        HandleBinaryOp2V<FORMAT, double, std::divides>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleFmod2v()
    {
        LOG_INST() << "fmod2v ->";
        HandleBinaryOp2V<FORMAT, float, math_helpers::fmodulus>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleFmod2vWide()
    {
        LOG_INST() << "fmod2v.64 ->";
        HandleBinaryOp2V<FORMAT, double, math_helpers::fmodulus>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleAnd2v()
    {
        LOG_INST() << "and2v ->";
        HandleBinaryOp2V<FORMAT, int32_t, std::bit_and>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleAnd2vWide()
    {
        LOG_INST() << "and2v.64 ->";
        HandleBinaryOp2V<FORMAT, int64_t, std::bit_and>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleOr2v()
    {
        LOG_INST() << "or2v ->";
        HandleBinaryOp2V<FORMAT, int32_t, std::bit_or>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleOr2vWide()
    {
        LOG_INST() << "or2v.64 ->";
        HandleBinaryOp2V<FORMAT, int64_t, std::bit_or>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleXor2v()
    {
        LOG_INST() << "xor2v ->";
        HandleBinaryOp2V<FORMAT, int32_t, std::bit_xor>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleXor2vWide()
    {
        LOG_INST() << "xor2v.64 ->";
        HandleBinaryOp2V<FORMAT, int64_t, std::bit_xor>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleShl2v()
    {
        LOG_INST() << "shl2v ->";
        HandleBinaryOp2V<FORMAT, int32_t, math_helpers::bit_shl>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleShl2vWide()
    {
        LOG_INST() << "shl2v.64 ->";
        HandleBinaryOp2V<FORMAT, int64_t, math_helpers::bit_shl>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleShr2v()
    {
        LOG_INST() << "shr2v ->";
        HandleBinaryOp2V<FORMAT, int32_t, math_helpers::bit_shr>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleShr2vWide()
    {
        LOG_INST() << "shr2v.64 ->";
        HandleBinaryOp2V<FORMAT, int64_t, math_helpers::bit_shr>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleAshr2v()
    {
        LOG_INST() << "ashr2v ->";
        HandleBinaryOp2V<FORMAT, int32_t, math_helpers::bit_ashr>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleAshr2vWide()
    {
        LOG_INST() << "ashr2v.64 ->";
        HandleBinaryOp2V<FORMAT, int64_t, math_helpers::bit_ashr>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleDiv2v()
    {
        LOG_INST() << "div2v ->";
        HandleBinaryOp2V<FORMAT, int32_t, math_helpers::idivides, true>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleDiv2vWide()
    {
        LOG_INST() << "div2v.64 ->";
        HandleBinaryOp2V<FORMAT, int64_t, math_helpers::idivides, true>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleMod2v()
    {
        LOG_INST() << "mod2v ->";
        HandleBinaryOp2V<FORMAT, int32_t, math_helpers::imodulus, true>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleMod2vWide()
    {
        LOG_INST() << "mod2v.64 ->";
        HandleBinaryOp2V<FORMAT, int64_t, math_helpers::imodulus, true>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleDivu2v()
    {
        LOG_INST() << "divu2v ->";
        HandleBinaryOp2V<FORMAT, uint32_t, math_helpers::idivides, true>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleDivu2vWide()
    {
        LOG_INST() << "divu2v.64 ->";
        HandleBinaryOp2V<FORMAT, uint64_t, math_helpers::idivides, true>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleModu2v()
    {
        LOG_INST() << "modu2v ->";
        HandleBinaryOp2V<FORMAT, uint32_t, math_helpers::imodulus, true>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleModu2vWide()
    {
        LOG_INST() << "modu2v.64 ->";
        HandleBinaryOp2V<FORMAT, uint64_t, math_helpers::imodulus, true>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleAdd()
    {
        LOG_INST() << "add ->";
        HandleBinaryOp<FORMAT, int32_t, math_helpers::Plus>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleSub()
    {
        LOG_INST() << "sub ->";
        HandleBinaryOp<FORMAT, int32_t, math_helpers::Minus>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleMul()
    {
        LOG_INST() << "mul ->";
        HandleBinaryOp<FORMAT, int32_t, math_helpers::Multiplies>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleAnd()
    {
        LOG_INST() << "and ->";
        HandleBinaryOp<FORMAT, int32_t, std::bit_and>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleOr()
    {
        LOG_INST() << "or ->";
        HandleBinaryOp<FORMAT, int32_t, std::bit_or>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleXor()
    {
        LOG_INST() << "xor ->";
        HandleBinaryOp<FORMAT, int32_t, std::bit_xor>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleShl()
    {
        LOG_INST() << "shl ->";
        HandleBinaryOp<FORMAT, int32_t, math_helpers::bit_shl>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleShr()
    {
        LOG_INST() << "shr ->";
        HandleBinaryOp<FORMAT, int32_t, math_helpers::bit_shr>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleAshr()
    {
        LOG_INST() << "ashr ->";
        HandleBinaryOp<FORMAT, int32_t, math_helpers::bit_ashr>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleDiv()
    {
        LOG_INST() << "div ->";
        HandleBinaryOp<FORMAT, int32_t, math_helpers::idivides, true>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleMod()
    {
        LOG_INST() << "mod ->";
        HandleBinaryOp<FORMAT, int32_t, math_helpers::imodulus, true>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleAddv()
    {
        LOG_INST() << "add.v ->";
        HandleBinaryOpV<FORMAT, int32_t, math_helpers::Plus>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleSubv()
    {
        LOG_INST() << "sub.v ->";
        HandleBinaryOpV<FORMAT, int32_t, math_helpers::Minus>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleMulv()
    {
        LOG_INST() << "mul.v ->";
        HandleBinaryOpV<FORMAT, int32_t, math_helpers::Multiplies>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleAndv()
    {
        LOG_INST() << "and.v ->";
        HandleBinaryOpV<FORMAT, int32_t, std::bit_and>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleOrv()
    {
        LOG_INST() << "or.v ->";
        HandleBinaryOpV<FORMAT, int32_t, std::bit_or>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleXorv()
    {
        LOG_INST() << "xor.v ->";
        HandleBinaryOpV<FORMAT, int32_t, std::bit_xor>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleShlv()
    {
        LOG_INST() << "shl.v ->";
        HandleBinaryOpV<FORMAT, int32_t, math_helpers::bit_shl>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleShrv()
    {
        LOG_INST() << "shr.v ->";
        HandleBinaryOpV<FORMAT, int32_t, math_helpers::bit_shr>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleAshrv()
    {
        LOG_INST() << "ashr.v ->";
        HandleBinaryOpV<FORMAT, int32_t, math_helpers::bit_ashr>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleDivv()
    {
        LOG_INST() << "div.v ->";
        HandleBinaryOpV<FORMAT, int32_t, math_helpers::idivides, true>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleModv()
    {
        LOG_INST() << "mod.v ->";
        HandleBinaryOpV<FORMAT, int32_t, math_helpers::imodulus, true>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleAnyCall0()
    {
        uint16_t vs1 = this->GetInst().template GetVReg<FORMAT, 0x0>();
        LOG_INST() << "any.call.0 v" << vs1;
        ObjectHeader *funcObj = this->GetFrame()->GetVReg(vs1).template GetAs<ObjectHeader *>();
        auto ret = intrinsics::AnyCall0(this->GetThread(), funcObj);
        if (UNLIKELY(this->GetThread()->HasPendingException())) {
            this->MoveToExceptionHandler();
        } else {
            this->GetAccAsVReg().SetReference(ret);
            this->template MoveToNextInst<FORMAT, true>();
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleAnyCallRange()
    {
        uint16_t vs1 = this->GetInst().template GetVReg<FORMAT, 0x0>();
        auto argc = this->GetInst().template GetImm<FORMAT>();
        uint16_t argStart = this->GetInst().template GetVReg<FORMAT, 0x1>();
        LOG_INST() << "any.call.range v" << vs1 << ", v" << argStart << ", " << argc;
        ObjectHeader *funcObj = this->GetFrame()->GetVReg(vs1).template GetAs<ObjectHeader *>();
        auto ret = intrinsics::AnyCallRange(this->GetThread(), this->GetFrame(), funcObj, argStart, argc);
        if (UNLIKELY(this->GetThread()->HasPendingException())) {
            this->MoveToExceptionHandler();
        } else {
            this->GetAccAsVReg().SetReference(ret);
            this->template MoveToNextInst<FORMAT, true>();
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleAnyCallShort()
    {
        // This should be generated from plugin.erb file
        uint16_t vs1 = this->GetInst().template GetVReg<FORMAT, 0x0>();
        uint16_t vs2 = this->GetInst().template GetVReg<FORMAT, 0x1>();
        LOG_INST() << "any.call.short v" << vs1 << ", v" << vs2;
        ObjectHeader *funcObj = this->GetFrame()->GetVReg(vs1).template GetAs<ObjectHeader *>();
        ObjectHeader *arg = this->GetFrame()->GetVReg(vs2).template GetAs<ObjectHeader *>();
        auto ret = intrinsics::AnyCallShort(this->GetThread(), funcObj, arg);
        if (UNLIKELY(this->GetThread()->HasPendingException())) {
            this->MoveToExceptionHandler();
        } else {
            this->GetAccAsVReg().SetReference(ret);
            this->template MoveToNextInst<FORMAT, true>();
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleAnyCallThis0()
    {
        uint16_t vs1 = this->GetInst().template GetVReg<FORMAT, 0x0>();
        auto stringId = this->GetInst().template GetId<FORMAT>().AsRawValue();
        LOG_INST() << "any.call.this.0 v" << vs1 << ", " << stringId;
        ObjectHeader *thisObj = this->GetFrame()->GetVReg(vs1).template GetAs<ObjectHeader *>();
        auto ret = intrinsics::AnyCallThis0(this->GetThread(), this->GetFrame(), thisObj, stringId);
        if (UNLIKELY(this->GetThread()->HasPendingException())) {
            this->MoveToExceptionHandler();
        } else {
            this->GetAccAsVReg().SetReference(ret);
            this->template MoveToNextInst<FORMAT, true>();
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleAnyCallThisRange()
    {
        uint16_t vs1 = this->GetInst().template GetVReg<FORMAT, 0x0>();
        auto argc = this->GetInst().template GetImm<FORMAT>();
        auto stringId = this->GetInst().template GetId<FORMAT>().AsRawValue();
        uint16_t argStart = this->GetInst().template GetVReg<FORMAT, 0x1>();
        LOG_INST() << "any.call.this.range v" << vs1 << ", v" << argStart << ", " << argc << ", " << stringId;
        ObjectHeader *thisObj = this->GetFrame()->GetVReg(vs1).template GetAs<ObjectHeader *>();
        auto ret = intrinsics::AnyCallThisRange(this->GetThread(), this->GetFrame(), thisObj, stringId, argStart, argc);
        if (UNLIKELY(this->GetThread()->HasPendingException())) {
            this->MoveToExceptionHandler();
        } else {
            this->GetAccAsVReg().SetReference(ret);
            this->template MoveToNextInst<FORMAT, true>();
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleAnyCallThisShort()
    {
        uint16_t vs1 = this->GetInst().template GetVReg<FORMAT, 0x0>();
        uint16_t vs2 = this->GetInst().template GetVReg<FORMAT, 0x1>();
        auto stringId = this->GetInst().template GetId<FORMAT>().AsRawValue();
        LOG_INST() << "any.call.this.short v" << vs1 << ", v" << vs2 << ", " << stringId;
        ObjectHeader *thisObj = this->GetFrame()->GetVReg(vs1).template GetAs<ObjectHeader *>();
        ObjectHeader *arg = this->GetFrame()->GetVReg(vs2).template GetAs<ObjectHeader *>();
        auto ret = intrinsics::AnyCallThisShort(this->GetThread(), this->GetFrame(), thisObj, stringId, arg);
        if (UNLIKELY(this->GetThread()->HasPendingException())) {
            this->MoveToExceptionHandler();
        } else {
            this->GetAccAsVReg().SetReference(ret);
            this->template MoveToNextInst<FORMAT, true>();
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleAnyCallNew0()
    {
        uint16_t vs1 = this->GetInst().template GetVReg<FORMAT, 0x0>();
        LOG_INST() << "any.call.new.0 v" << vs1;
        ObjectHeader *ctor = this->GetFrame()->GetVReg(vs1).template GetAs<ObjectHeader *>();
        auto ret = intrinsics::AnyCallNew0(this->GetThread(), ctor);
        if (UNLIKELY(this->GetThread()->HasPendingException())) {
            this->MoveToExceptionHandler();
        } else {
            this->GetAccAsVReg().SetReference(ret);
            this->template MoveToNextInst<FORMAT, true>();
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleAnyCallNewRange()
    {
        uint16_t vs1 = this->GetInst().template GetVReg<FORMAT, 0x0>();
        auto argc = this->GetInst().template GetImm<FORMAT>();
        uint16_t argStart = this->GetInst().template GetVReg<FORMAT, 0x1>();
        LOG_INST() << "any.call.new.range v" << vs1 << ", v" << argStart << ", " << argc;
        ObjectHeader *ctor = this->GetFrame()->GetVReg(vs1).template GetAs<ObjectHeader *>();
        auto ret = intrinsics::AnyCallNewRange(this->GetThread(), this->GetFrame(), ctor, argStart, argc);
        if (UNLIKELY(this->GetThread()->HasPendingException())) {
            this->MoveToExceptionHandler();
        } else {
            this->GetAccAsVReg().SetReference(ret);
            this->template MoveToNextInst<FORMAT, true>();
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleAnyCallNewShort()
    {
        uint16_t vs1 = this->GetInst().template GetVReg<FORMAT, 0x0>();
        uint16_t vs2 = this->GetInst().template GetVReg<FORMAT, 0x1>();
        LOG_INST() << "any.call.new.short v" << vs1 << ", v" << vs2;
        ObjectHeader *ctor = this->GetFrame()->GetVReg(vs1).template GetAs<ObjectHeader *>();
        ObjectHeader *arg = this->GetFrame()->GetVReg(vs2).template GetAs<ObjectHeader *>();
        auto ret = intrinsics::AnyCallNewShort(this->GetThread(), ctor, arg);
        if (UNLIKELY(this->GetThread()->HasPendingException())) {
            this->MoveToExceptionHandler();
        } else {
            this->GetAccAsVReg().SetReference(ret);
            this->template MoveToNextInst<FORMAT, true>();
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleAnyIsinstance()
    {
        uint16_t vs1 = this->GetInst().template GetVReg<FORMAT, 0x0>();
        LOG_INST() << "any.isinstance v" << vs1;

        ObjectHeader *lhsObj = this->GetAcc().template GetAs<ObjectHeader *>();
        ObjectHeader *rhsObj = this->GetFrame()->GetVReg(vs1).template GetAs<ObjectHeader *>();
        bool ret = intrinsics::AnyIsinstance(this->GetThread(), lhsObj, rhsObj);
        if (UNLIKELY(this->GetThread()->HasPendingException())) {
            this->MoveToExceptionHandler();
        } else {
            this->GetAccAsVReg().SetPrimitive(ret);
            this->template MoveToNextInst<FORMAT, true>();
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleAddi()
    {
        LOG_INST() << "addi ->";
        HandleBinaryOp2Imm<FORMAT, int32_t, math_helpers::Plus>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleSubi()
    {
        LOG_INST() << "subi ->";
        HandleBinaryOp2Imm<FORMAT, int32_t, math_helpers::Minus>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleMuli()
    {
        LOG_INST() << "muli ->";
        HandleBinaryOp2Imm<FORMAT, int32_t, math_helpers::Multiplies>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleAndi()
    {
        LOG_INST() << "andi ->";
        HandleBinaryOp2Imm<FORMAT, int32_t, std::bit_and>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleOri()
    {
        LOG_INST() << "ori ->";
        HandleBinaryOp2Imm<FORMAT, int32_t, std::bit_or>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleXori()
    {
        LOG_INST() << "xori ->";
        HandleBinaryOp2Imm<FORMAT, int32_t, std::bit_xor>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleShli()
    {
        LOG_INST() << "shli ->";
        HandleBinaryOp2Imm<FORMAT, int32_t, math_helpers::bit_shl>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleShri()
    {
        LOG_INST() << "shri ->";
        HandleBinaryOp2Imm<FORMAT, int32_t, math_helpers::bit_shr>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleAshri()
    {
        LOG_INST() << "ashri ->";
        HandleBinaryOp2Imm<FORMAT, int32_t, math_helpers::bit_ashr>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleDivi()
    {
        LOG_INST() << "divi ->";
        HandleBinaryOp2Imm<FORMAT, int32_t, math_helpers::idivides, true>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleModi()
    {
        LOG_INST() << "modi ->";
        HandleBinaryOp2Imm<FORMAT, int32_t, math_helpers::imodulus, true>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleAddiv()
    {
        LOG_INST() << "addiv ->";
        HandleBinaryOpImmV<FORMAT, int32_t, math_helpers::Plus>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleSubiv()
    {
        LOG_INST() << "subiv ->";
        HandleBinaryOpImmV<FORMAT, int32_t, math_helpers::Minus>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleMuliv()
    {
        LOG_INST() << "muliv ->";
        HandleBinaryOpImmV<FORMAT, int32_t, math_helpers::Multiplies>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleAndiv()
    {
        LOG_INST() << "andiv ->";
        HandleBinaryOpImmV<FORMAT, int32_t, std::bit_and>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleOriv()
    {
        LOG_INST() << "oriv ->";
        HandleBinaryOpImmV<FORMAT, int32_t, std::bit_or>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleXoriv()
    {
        LOG_INST() << "xoriv ->";
        HandleBinaryOpImmV<FORMAT, int32_t, std::bit_xor>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleShliv()
    {
        LOG_INST() << "shliv ->";
        HandleBinaryOpImmV<FORMAT, int32_t, math_helpers::bit_shl>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleShriv()
    {
        LOG_INST() << "shriv ->";
        HandleBinaryOpImmV<FORMAT, int32_t, math_helpers::bit_shr>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleAshriv()
    {
        LOG_INST() << "ashriv ->";
        HandleBinaryOpImmV<FORMAT, int32_t, math_helpers::bit_ashr>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleDiviv()
    {
        LOG_INST() << "diviv ->";
        HandleBinaryOpImmV<FORMAT, int32_t, math_helpers::idivides, true>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleModiv()
    {
        LOG_INST() << "modiv ->";
        HandleBinaryOpImmV<FORMAT, int32_t, math_helpers::imodulus, true>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleNeg()
    {
        LOG_INST() << "neg";
        HandleUnaryOp<FORMAT, int32_t, std::negate>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleNegWide()
    {
        LOG_INST() << "neg.64";
        HandleUnaryOp<FORMAT, int64_t, std::negate>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleFneg()
    {
        LOG_INST() << "fneg";
        HandleUnaryOp<FORMAT, float, std::negate>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleFnegWide()
    {
        LOG_INST() << "fneg.64";
        HandleUnaryOp<FORMAT, double, std::negate>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleNot()
    {
        LOG_INST() << "not";
        HandleUnaryOp<FORMAT, int32_t, std::bit_not>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleNotWide()
    {
        LOG_INST() << "not.64";
        HandleUnaryOp<FORMAT, int64_t, std::bit_not>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleInci()
    {
        int32_t imm = this->GetInst().template GetImm<FORMAT>();
        uint16_t vx = this->GetInst().template GetVReg<FORMAT>();
        LOG_INST() << "inci v" << vx << ", " << std::hex << "0x" << imm;
        auto &reg = this->GetFrame()->GetVReg(vx);
        auto value = reg.template GetAs<int32_t>();
        reg.Set(value + imm);
        this->template MoveToNextInst<FORMAT, false>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleU32toi64()
    {
        LOG_INST() << "u32toi64";
        HandleConversion<FORMAT, uint32_t, int64_t>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleU32toi16()
    {
        LOG_INST() << "u32toi16";
        HandleConversion<FORMAT, uint32_t, int16_t>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleU32tou16()
    {
        LOG_INST() << "u32tou16";
        HandleConversion<FORMAT, uint32_t, uint16_t>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleU32toi8()
    {
        LOG_INST() << "u32toi8";
        HandleConversion<FORMAT, uint32_t, int8_t>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleU32tou8()
    {
        LOG_INST() << "u32tou8";
        HandleConversion<FORMAT, uint32_t, uint8_t>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleU32tou1()
    {
        LOG_INST() << "u32tou1";
        HandleConversion<FORMAT, uint32_t, bool>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleI32toi64()
    {
        LOG_INST() << "i32toi64";
        HandleConversion<FORMAT, int32_t, int64_t>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleI32tou16()
    {
        LOG_INST() << "i32tou16";
        HandleConversion<FORMAT, int32_t, uint16_t>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleI32toi16()
    {
        LOG_INST() << "i32toi16";
        HandleConversion<FORMAT, int32_t, int16_t>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleI32toi8()
    {
        LOG_INST() << "i32toi8";
        HandleConversion<FORMAT, int32_t, int8_t>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleI32tou8()
    {
        LOG_INST() << "i32tou8";
        HandleConversion<FORMAT, int32_t, uint8_t>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleI32tou1()
    {
        LOG_INST() << "i32tou1";
        HandleConversion<FORMAT, int32_t, bool>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleI32tof32()
    {
        LOG_INST() << "i32tof32";
        HandleConversion<FORMAT, int32_t, float>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleI32tof64()
    {
        LOG_INST() << "i32tof64";
        HandleConversion<FORMAT, int32_t, double>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleU32tof32()
    {
        LOG_INST() << "u32tof32";
        HandleConversion<FORMAT, uint32_t, float>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleU32tof64()
    {
        LOG_INST() << "u32tof64";
        HandleConversion<FORMAT, uint32_t, double>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleI64toi32()
    {
        LOG_INST() << "i64toi32";
        HandleConversion<FORMAT, int64_t, int32_t>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleI64tou1()
    {
        LOG_INST() << "i64tou1";
        HandleConversion<FORMAT, int64_t, bool>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleI64tof32()
    {
        LOG_INST() << "i64tof32";
        HandleConversion<FORMAT, int64_t, float>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleI64tof64()
    {
        LOG_INST() << "i64tof64";
        HandleConversion<FORMAT, int64_t, double>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleU64toi32()
    {
        LOG_INST() << "u64toi32";
        HandleConversion<FORMAT, uint64_t, int32_t>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleU64tou32()
    {
        LOG_INST() << "u64tou32";
        HandleConversion<FORMAT, uint64_t, uint32_t>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleU64tou1()
    {
        LOG_INST() << "u64tou1";
        HandleConversion<FORMAT, uint64_t, bool>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleU64tof32()
    {
        LOG_INST() << "u64tof32";
        HandleConversion<FORMAT, uint64_t, float>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleU64tof64()
    {
        LOG_INST() << "u64tof64";
        HandleConversion<FORMAT, uint64_t, double>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleF32tof64()
    {
        LOG_INST() << "f32tof64";
        HandleConversion<FORMAT, float, double>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleF32toi32()
    {
        LOG_INST() << "f32toi32";
        HandleFloatToIntConversion<FORMAT, float, int32_t>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleF32toi64()
    {
        LOG_INST() << "f32toi64";
        HandleFloatToIntConversion<FORMAT, float, int64_t>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleF32tou32()
    {
        LOG_INST() << "f32tou32";
        HandleFloatToIntConversion<FORMAT, float, uint32_t>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleF32tou64()
    {
        LOG_INST() << "f32tou64";
        HandleFloatToIntConversion<FORMAT, float, uint64_t>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleF64tof32()
    {
        LOG_INST() << "f64tof32";
        HandleConversion<FORMAT, double, float>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleF64toi64()
    {
        LOG_INST() << "f64toi64";
        HandleFloatToIntConversion<FORMAT, double, int64_t>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleF64toi32()
    {
        LOG_INST() << "f64toi32";
        HandleFloatToIntConversion<FORMAT, double, int32_t>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleF64tou64()
    {
        LOG_INST() << "f64tou64";
        HandleFloatToIntConversion<FORMAT, double, uint64_t>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleF64tou32()
    {
        LOG_INST() << "f64tou32";
        HandleFloatToIntConversion<FORMAT, double, uint32_t>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleLdarr8()
    {
        LOG_INST() << "ldarr.8";
        HandleArrayPrimitiveLoad<FORMAT, int8_t>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleLdarr16()
    {
        LOG_INST() << "ldarr.16";
        HandleArrayPrimitiveLoad<FORMAT, int16_t>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleLdarr()
    {
        LOG_INST() << "ldarr";
        HandleArrayPrimitiveLoad<FORMAT, int32_t>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleLdarrWide()
    {
        LOG_INST() << "ldarr.64";
        HandleArrayPrimitiveLoad<FORMAT, int64_t>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleLdarru8()
    {
        LOG_INST() << "ldarru.8";
        HandleArrayPrimitiveLoad<FORMAT, uint8_t>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleLdarru16()
    {
        LOG_INST() << "ldarru.16";
        HandleArrayPrimitiveLoad<FORMAT, uint16_t>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleFldarr32()
    {
        LOG_INST() << "fldarr.32";
        HandleArrayPrimitiveLoad<FORMAT, float>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleFldarrWide()
    {
        LOG_INST() << "fldarr.64";
        HandleArrayPrimitiveLoad<FORMAT, double>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleLdarrObj()
    {
        uint16_t vs = this->GetInst().template GetVReg<FORMAT>();

        LOG_INST() << "ldarr.obj v" << vs;
        auto *array = static_cast<coretypes::Array *>(this->GetFrame()->GetVReg(vs).GetReference());
        int32_t idx = this->GetAcc().Get();

        if (LIKELY(CheckLoadArrayOp(array, idx))) {
            this->GetAccAsVReg().SetReference(
                array->Get<ObjectHeader *, RuntimeIfaceT::NEED_READ_BARRIER>(this->GetThread(), idx));
            this->template MoveToNextInst<FORMAT, true>();
        } else {
            this->MoveToExceptionHandler();
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleLdaDyn()
    {
        uint16_t vs = this->GetInst().template GetVReg<FORMAT>();
        LOG_INST() << "lda.dyn v" << vs;
        this->GetAccAsVReg().Move(this->GetFrameHandler().GetVReg(vs));
        this->template MoveToNextInst<FORMAT, false>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleStarr8()
    {
        LOG_INST() << "starr.8";
        HandleArrayStore<FORMAT, uint8_t>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleStarr16()
    {
        LOG_INST() << "starr.16";
        HandleArrayStore<FORMAT, uint16_t>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleStarr()
    {
        LOG_INST() << "starr";
        HandleArrayStore<FORMAT, uint32_t>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleStarrWide()
    {
        LOG_INST() << "starr.64";
        HandleArrayStore<FORMAT, uint64_t>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleFstarr32()
    {
        LOG_INST() << "fstarr.32";
        HandleArrayStore<FORMAT, float>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleFstarrWide()
    {
        LOG_INST() << "fstarr.64";
        HandleArrayStore<FORMAT, double>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleStarrObj()
    {
        LOG_INST() << "starr.obj";
        HandleArrayStore<FORMAT, ObjectHeader *>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleLenarr()
    {
        uint16_t vs = this->GetInst().template GetVReg<FORMAT>();

        LOG_INST() << "lenarr v" << vs;

        ObjectHeader *obj = this->GetFrame()->GetVReg(vs).GetReference();
        if (UNLIKELY(obj == nullptr)) {
            RuntimeIfaceT::ThrowNullPointerException();
            this->MoveToExceptionHandler();
        } else {
            auto *array = static_cast<coretypes::Array *>(obj);
            this->GetAccAsVReg().SetPrimitive(static_cast<int32_t>(array->GetLength()));
            this->template MoveToNextInst<FORMAT, true>();
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleLdaConst()
    {
        auto litarrId = this->GetInst().template GetId<FORMAT>();
        uint16_t vd = this->GetInst().template GetVReg<FORMAT>();

        LOG_INST() << "lda.const v" << vd << ", " << std::hex << "0x" << litarrId;

        this->GetFrame()->SetAcc(this->GetAcc());
        auto array = ResolveLiteralArray(litarrId);
        this->GetAcc() = this->GetFrame()->GetAcc();

        if (UNLIKELY(array == nullptr)) {
            this->MoveToExceptionHandler();
        } else {
            this->GetFrameHandler().GetVReg(vd).SetReference(array);
            this->template MoveToNextInst<FORMAT, false>();
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleNewarr()
    {
        uint16_t vd = this->GetInst().template GetVReg<FORMAT, 0>();
        uint16_t vs = this->GetInst().template GetVReg<FORMAT, 1>();
        auto id = this->GetInst().template GetId<FORMAT>();

        LOG_INST() << "newarr v" << vd << ", v" << vs << ", " << std::hex << "0x" << id;

        int32_t size = this->GetFrame()->GetVReg(vs).Get();

        if (UNLIKELY(size < 0)) {
            RuntimeIfaceT::ThrowNegativeArraySizeException(size);
            this->MoveToExceptionHandler();
        } else {
            Class *klass = ResolveType<true>(id);
            if (LIKELY(klass != nullptr)) {
                this->GetFrame()->GetAcc() = this->GetAcc();
                coretypes::Array *array = RuntimeIfaceT::CreateArray(klass, size);
                this->GetAcc() = this->GetFrame()->GetAcc();
                this->GetFrameHandler().GetVReg(vd).SetReference(array);
                if (UNLIKELY(array == nullptr)) {
                    this->MoveToExceptionHandler();
                } else {
                    this->template MoveToNextInst<FORMAT, true>();
                }
            } else {
                this->MoveToExceptionHandler();
            }
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleNewobj()
    {
        uint16_t vd = this->GetInst().template GetVReg<FORMAT, 0>();
        auto id = this->GetInst().template GetId<FORMAT>();

        LOG_INST() << "newobj v" << vd << std::hex << "0x" << id;

        Class *klass = ResolveType<true>(id);
        if (LIKELY(klass != nullptr)) {
            this->GetFrame()->GetAcc() = this->GetAcc();
            ObjectHeader *obj = RuntimeIfaceT::CreateObject(this->GetThread(), klass);
            this->GetAcc() = this->GetFrame()->GetAcc();
            if (LIKELY(obj != nullptr)) {
                this->GetFrameHandler().GetVReg(vd).SetReference(obj);
                this->template MoveToNextInst<FORMAT, false>();
            } else {
                this->MoveToExceptionHandler();
            }
        } else {
            this->MoveToExceptionHandler();
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleInitobj()
    {
        auto id = this->GetInst().template GetId<FORMAT>();

        LOG_INST() << "initobj " << this->GetInst().template GetVReg<FORMAT, 0>() << ", v"
                   << this->GetInst().template GetVReg<FORMAT, 1>() << ", v"
                   << this->GetInst().template GetVReg<FORMAT, 2>() << ", v"
                   << this->GetInst().template GetVReg<FORMAT, 3>() << ", " << std::hex << "0x" << id;

        InitializeObject<FORMAT>(id);
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleInitobjShort()
    {
        auto id = this->GetInst().template GetId<FORMAT>();

        LOG_INST() << "initobj.short v" << this->GetInst().template GetVReg<FORMAT, 0>() << ", v"
                   << this->GetInst().template GetVReg<FORMAT, 1>() << ", " << std::hex << "0x" << id;

        InitializeObject<FORMAT>(id);
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleInitobjRange()
    {
        auto id = this->GetInst().template GetId<FORMAT>();

        LOG_INST() << "initobj.range v" << this->GetInst().template GetVReg<FORMAT, 0>() << ", " << std::hex << "0x"
                   << id;

        InitializeObject<FORMAT>(id);
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleLdobj()
    {
        uint16_t vs = this->GetInst().template GetVReg<FORMAT>();
        auto id = this->GetInst().template GetId<FORMAT>();

        LOG_INST() << "ldobj v" << vs << ", " << std::hex << "0x" << id;

        ObjectHeader *obj = this->GetFrame()->GetVReg(vs).GetReference();
        if (UNLIKELY(obj == nullptr)) {
            RuntimeIfaceT::ThrowNullPointerException();
            this->MoveToExceptionHandler();
        } else {
            Field *field = ResolveField(id);
            obj = this->GetFrame()->GetVReg(vs).GetReference();
            if (LIKELY(field != nullptr)) {
                ASSERT(!field->IsStatic());
                ASSERT(!field->IsVolatile());
                LoadPrimitiveField(obj, field);
                this->template MoveToNextInst<FORMAT, true>();
            } else {
                this->MoveToExceptionHandler();
            }
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleLdobjWide()
    {
        uint16_t vs = this->GetInst().template GetVReg<FORMAT>();
        auto id = this->GetInst().template GetId<FORMAT>();

        LOG_INST() << "ldobj.64 v" << vs << ", " << std::hex << "0x" << id;

        ObjectHeader *obj = this->GetFrame()->GetVReg(vs).GetReference();
        if (UNLIKELY(obj == nullptr)) {
            RuntimeIfaceT::ThrowNullPointerException();
            this->MoveToExceptionHandler();
        } else {
            Field *field = ResolveField(id);
            obj = this->GetFrame()->GetVReg(vs).GetReference();
            if (LIKELY(field != nullptr)) {
                ASSERT(!field->IsStatic());
                ASSERT(!field->IsVolatile());
                LoadPrimitiveField(obj, field);
                this->template MoveToNextInst<FORMAT, true>();
            } else {
                this->MoveToExceptionHandler();
            }
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleLdobjObj()
    {
        uint16_t vs = this->GetInst().template GetVReg<FORMAT>();
        auto id = this->GetInst().template GetId<FORMAT>();

        LOG_INST() << "ldobj.obj v" << vs << ", " << std::hex << "0x" << id;

        ObjectHeader *obj = this->GetFrame()->GetVReg(vs).GetReference();
        if (UNLIKELY(obj == nullptr)) {
            RuntimeIfaceT::ThrowNullPointerException();
            this->MoveToExceptionHandler();
        } else {
            Field *field = ResolveField(id);
            obj = this->GetFrame()->GetVReg(vs).GetReference();
            if (LIKELY(field != nullptr)) {
                ASSERT(!field->IsStatic());
                ASSERT(!field->IsVolatile());
                ASSERT(field->GetType().IsReference());
                this->GetAccAsVReg().SetReference(
                    obj->GetFieldObject<RuntimeIfaceT::NEED_READ_BARRIER>(this->GetThread(), *field));
                this->template MoveToNextInst<FORMAT, true>();
            } else {
                this->MoveToExceptionHandler();
            }
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleLdobjV()
    {
        uint16_t vd = this->GetInst().template GetVReg<FORMAT, 0>();
        uint16_t vs = this->GetInst().template GetVReg<FORMAT, 1>();
        auto id = this->GetInst().template GetId<FORMAT>();

        LOG_INST() << "ldobj.v v" << vd << ", v" << vs << ", " << std::hex << "0x" << id;

        ObjectHeader *obj = this->GetFrame()->GetVReg(vs).GetReference();
        if (UNLIKELY(obj == nullptr)) {
            RuntimeIfaceT::ThrowNullPointerException();
            this->MoveToExceptionHandler();
        } else {
            Field *field = ResolveField(id);
            obj = this->GetFrame()->GetVReg(vs).GetReference();
            if (LIKELY(field != nullptr)) {
                ASSERT(!field->IsStatic());
                ASSERT(!field->IsVolatile());
                auto vreg = this->GetFrameHandler().GetVReg(vd);
                LoadPrimitiveFieldReg(vreg, obj, field);
                this->template MoveToNextInst<FORMAT, true>();
            } else {
                this->MoveToExceptionHandler();
            }
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleLdobjVWide()
    {
        uint16_t vd = this->GetInst().template GetVReg<FORMAT, 0>();
        uint16_t vs = this->GetInst().template GetVReg<FORMAT, 1>();
        auto id = this->GetInst().template GetId<FORMAT>();

        LOG_INST() << "ldobj.v.64 v" << vd << ", v" << vs << ", " << std::hex << "0x" << id;

        ObjectHeader *obj = this->GetFrame()->GetVReg(vs).GetReference();
        if (UNLIKELY(obj == nullptr)) {
            RuntimeIfaceT::ThrowNullPointerException();
            this->MoveToExceptionHandler();
        } else {
            Field *field = ResolveField(id);
            obj = this->GetFrame()->GetVReg(vs).GetReference();
            if (LIKELY(field != nullptr)) {
                ASSERT(!field->IsStatic());
                ASSERT(!field->IsVolatile());
                auto vreg = this->GetFrameHandler().GetVReg(vd);
                LoadPrimitiveFieldReg(vreg, obj, field);
                this->template MoveToNextInst<FORMAT, true>();
            } else {
                this->MoveToExceptionHandler();
            }
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleLdobjVObj()
    {
        uint16_t vd = this->GetInst().template GetVReg<FORMAT, 0>();
        uint16_t vs = this->GetInst().template GetVReg<FORMAT, 1>();
        auto id = this->GetInst().template GetId<FORMAT>();

        LOG_INST() << "ldobj.v.obj v" << vd << ", v" << vs << ", " << std::hex << "0x" << id;

        ObjectHeader *obj = this->GetFrame()->GetVReg(vs).GetReference();
        if (UNLIKELY(obj == nullptr)) {
            RuntimeIfaceT::ThrowNullPointerException();
            this->MoveToExceptionHandler();
        } else {
            Field *field = ResolveField(id);
            obj = this->GetFrame()->GetVReg(vs).GetReference();
            if (LIKELY(field != nullptr)) {
                ASSERT(!field->IsStatic());
                ASSERT(!field->IsVolatile());
                ASSERT(field->GetType().IsReference());
                this->GetFrameHandler().GetVReg(vd).SetReference(
                    obj->GetFieldObject<RuntimeIfaceT::NEED_READ_BARRIER>(this->GetThread(), *field));
                this->template MoveToNextInst<FORMAT, true>();
            } else {
                this->MoveToExceptionHandler();
            }
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleStobj()
    {
        uint16_t vs = this->GetInst().template GetVReg<FORMAT>();
        auto id = this->GetInst().template GetId<FORMAT>();

        LOG_INST() << "stobj v" << vs << ", " << std::hex << "0x" << id;

        ObjectHeader *obj = this->GetFrame()->GetVReg(vs).GetReference();
        if (UNLIKELY(obj == nullptr)) {
            RuntimeIfaceT::ThrowNullPointerException();
            this->MoveToExceptionHandler();
        } else {
            Field *field = ResolveField(id);
            obj = this->GetFrame()->GetVReg(vs).GetReference();
            if (LIKELY(field != nullptr)) {
                ASSERT(!field->IsStatic());
                ASSERT(!field->IsVolatile());
                StorePrimitiveField(obj, field);
                this->template MoveToNextInst<FORMAT, true>();
            } else {
                this->MoveToExceptionHandler();
            }
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleStobjWide()
    {
        uint16_t vs = this->GetInst().template GetVReg<FORMAT>();
        auto id = this->GetInst().template GetId<FORMAT>();

        LOG_INST() << "stobj.64 v" << vs << ", " << std::hex << "0x" << id;

        ObjectHeader *obj = this->GetFrame()->GetVReg(vs).GetReference();
        if (UNLIKELY(obj == nullptr)) {
            RuntimeIfaceT::ThrowNullPointerException();
            this->MoveToExceptionHandler();
        } else {
            Field *field = ResolveField(id);
            obj = this->GetFrame()->GetVReg(vs).GetReference();
            if (LIKELY(field != nullptr)) {
                ASSERT(!field->IsStatic());
                ASSERT(!field->IsVolatile());
                StorePrimitiveField(obj, field);
                this->template MoveToNextInst<FORMAT, true>();
            } else {
                this->MoveToExceptionHandler();
            }
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleStobjObj()
    {
        uint16_t vs = this->GetInst().template GetVReg<FORMAT>();
        auto id = this->GetInst().template GetId<FORMAT>();

        LOG_INST() << "stobj.obj v" << vs << ", " << std::hex << "0x" << id;

        ObjectHeader *obj = this->GetFrame()->GetVReg(vs).GetReference();
        if (UNLIKELY(obj == nullptr)) {
            RuntimeIfaceT::ThrowNullPointerException();
            this->MoveToExceptionHandler();
        } else {
            Field *field = ResolveField(id);
            obj = this->GetFrame()->GetVReg(vs).GetReference();
            if (LIKELY(field != nullptr)) {
                ASSERT(!field->IsStatic());
                ASSERT(!field->IsVolatile());
                ASSERT(field->GetType().IsReference());
                obj->SetFieldObject<RuntimeIfaceT::NEED_WRITE_BARRIER>(this->GetThread(), *field,
                                                                       this->GetAcc().GetReference());
                this->template MoveToNextInst<FORMAT, true>();
            } else {
                this->MoveToExceptionHandler();
            }
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleStobjV()
    {
        uint16_t vd = this->GetInst().template GetVReg<FORMAT, 0>();
        uint16_t vs = this->GetInst().template GetVReg<FORMAT, 1>();
        auto id = this->GetInst().template GetId<FORMAT>();

        LOG_INST() << "stobj.v v" << vd << ", v" << vs << ", " << std::hex << "0x" << id;

        ObjectHeader *obj = this->GetFrame()->GetVReg(vs).GetReference();
        if (UNLIKELY(obj == nullptr)) {
            RuntimeIfaceT::ThrowNullPointerException();
            this->MoveToExceptionHandler();
        } else {
            Field *field = ResolveField(id);
            obj = this->GetFrame()->GetVReg(vs).GetReference();
            if (LIKELY(field != nullptr)) {
                ASSERT(!field->IsStatic());
                ASSERT(!field->IsVolatile());
                StorePrimitiveFieldReg(this->GetFrame()->GetVReg(vd), obj, field);
                this->template MoveToNextInst<FORMAT, true>();
            } else {
                this->MoveToExceptionHandler();
            }
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleStobjVWide()
    {
        uint16_t vd = this->GetInst().template GetVReg<FORMAT, 0>();
        uint16_t vs = this->GetInst().template GetVReg<FORMAT, 1>();
        auto id = this->GetInst().template GetId<FORMAT>();

        LOG_INST() << "stobj.v.64 v" << vd << ", v" << vs << ", " << std::hex << "0x" << id;

        ObjectHeader *obj = this->GetFrame()->GetVReg(vs).GetReference();
        if (UNLIKELY(obj == nullptr)) {
            RuntimeIfaceT::ThrowNullPointerException();
            this->MoveToExceptionHandler();
        } else {
            Field *field = ResolveField(id);
            obj = this->GetFrame()->GetVReg(vs).GetReference();
            if (LIKELY(field != nullptr)) {
                ASSERT(!field->IsStatic());
                ASSERT(!field->IsVolatile());
                StorePrimitiveFieldReg(this->GetFrame()->GetVReg(vd), obj, field);
                this->template MoveToNextInst<FORMAT, true>();
            } else {
                this->MoveToExceptionHandler();
            }
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleStobjVObj()
    {
        uint16_t vd = this->GetInst().template GetVReg<FORMAT, 0>();
        uint16_t vs = this->GetInst().template GetVReg<FORMAT, 1>();
        auto id = this->GetInst().template GetId<FORMAT>();

        LOG_INST() << "stobj.v.obj v" << vd << ", v" << vs << ", " << std::hex << "0x" << id;

        ObjectHeader *obj = this->GetFrame()->GetVReg(vs).GetReference();
        if (UNLIKELY(obj == nullptr)) {
            RuntimeIfaceT::ThrowNullPointerException();
            this->MoveToExceptionHandler();
        } else {
            Field *field = ResolveField(id);
            obj = this->GetFrame()->GetVReg(vs).GetReference();
            if (LIKELY(field != nullptr)) {
                ASSERT(!field->IsStatic());
                ASSERT(!field->IsVolatile());
                ASSERT(field->GetType().IsReference());
                obj->SetFieldObject<RuntimeIfaceT::NEED_WRITE_BARRIER>(this->GetThread(), *field,
                                                                       this->GetFrame()->GetVReg(vd).GetReference());
                this->template MoveToNextInst<FORMAT, true>();
            } else {
                this->MoveToExceptionHandler();
            }
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleLdstatic()
    {
        auto id = this->GetInst().template GetId<FORMAT>();

        LOG_INST() << "ldstatic " << std::hex << "0x" << id;

        Field *field = ResolveField<true>(id);
        if (LIKELY(field != nullptr)) {
            ASSERT(field->IsStatic());
            ASSERT(!field->IsVolatile());
            LoadPrimitiveField(GetClass(field), field);
            this->template MoveToNextInst<FORMAT, false>();
        } else {
            this->MoveToExceptionHandler();
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleLdstaticWide()
    {
        auto id = this->GetInst().template GetId<FORMAT>();

        LOG_INST() << "ldstatic.64 " << std::hex << "0x" << id;

        Field *field = ResolveField<true>(id);
        if (LIKELY(field != nullptr)) {
            ASSERT(field->IsStatic());
            ASSERT(!field->IsVolatile());
            LoadPrimitiveField(GetClass(field), field);
            this->template MoveToNextInst<FORMAT, false>();
        } else {
            this->MoveToExceptionHandler();
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleLdstaticObj()
    {
        auto id = this->GetInst().template GetId<FORMAT>();

        LOG_INST() << "ldstatic.obj " << std::hex << "0x" << id;

        Field *field = ResolveField<true>(id);
        if (LIKELY(field != nullptr)) {
            ASSERT(field->IsStatic());
            ASSERT(!field->IsVolatile());
            Class *klass = GetClass(field);
            ASSERT(field->GetType().IsReference());
            this->GetAccAsVReg().SetReference(
                klass->GetFieldObject<RuntimeIfaceT::NEED_READ_BARRIER>(this->GetThread(), *field));
            this->template MoveToNextInst<FORMAT, false>();
        } else {
            this->MoveToExceptionHandler();
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleStstatic()
    {
        auto id = this->GetInst().template GetId<FORMAT>();

        LOG_INST() << "ststatic " << std::hex << "0x" << id;

        Field *field = ResolveField<true>(id);
        if (LIKELY(field != nullptr)) {
            ASSERT(field->IsStatic());
            ASSERT(!field->IsVolatile());
            Class *klass = GetClass(field);
            StorePrimitiveField(klass, field);
            this->template MoveToNextInst<FORMAT, false>();
        } else {
            this->MoveToExceptionHandler();
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleStstaticWide()
    {
        auto id = this->GetInst().template GetId<FORMAT>();

        LOG_INST() << "ststatic.64 " << std::hex << "0x" << id;

        Field *field = ResolveField<true>(id);
        if (LIKELY(field != nullptr)) {
            ASSERT(field->IsStatic());
            ASSERT(!field->IsVolatile());
            Class *klass = GetClass(field);
            StorePrimitiveField(klass, field);
            this->template MoveToNextInst<FORMAT, false>();
        } else {
            this->MoveToExceptionHandler();
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleStstaticObj()
    {
        auto id = this->GetInst().template GetId<FORMAT>();

        LOG_INST() << "ststatic.obj " << std::hex << "0x" << id;

        Field *field = ResolveField<true>(id);
        if (LIKELY(field != nullptr)) {
            ASSERT(field->IsStatic());
            ASSERT(!field->IsVolatile());
            Class *klass = GetClass(field);
            ASSERT(field->GetType().IsReference());
            klass->SetFieldObject<RuntimeIfaceT::NEED_WRITE_BARRIER>(this->GetThread(), *field,
                                                                     this->GetAcc().GetReference());
            this->template MoveToNextInst<FORMAT, false>();
        } else {
            this->MoveToExceptionHandler();
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleReturn()
    {
        LOG_INST() << "return";
        this->GetFrame()->GetAccAsVReg().SetPrimitive(this->GetAcc().Get());
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleReturnWide()
    {
        LOG_INST() << "return.64";
        this->GetFrame()->GetAccAsVReg().SetPrimitive(this->GetAcc().GetValue());
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleReturnObj()
    {
        LOG_INST() << "return.obj";
        this->GetFrame()->GetAccAsVReg().SetReference(this->GetAcc().GetReference());
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleReturnVoid()
    {
        LOG_INST() << "return.void";
    }

    NO_UB_SANITIZE ALWAYS_INLINE void HandleReturnStackless()
    {
        Frame *frame = this->GetFrame();
        Frame *prev = frame->GetPrevFrame();

        ASSERT(frame->IsStackless());

        Method *method = frame->GetMethod();
        ManagedThread *thread = this->GetThread();

#if EVENT_METHOD_EXIT_ENABLED
        EVENT_METHOD_EXIT(frame->GetMethod()->GetFullName(), events::MethodExitKind::INTERP,
                          thread->RecordMethodExit());
#endif

        Runtime::GetCurrent()->GetNotificationManager()->MethodExitEvent(thread, method);

        this->GetInstructionHandlerState()->UpdateInstructionHandlerState(
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            prev->GetInstruction() + prev->GetBytecodeOffset(), prev);

        this->SetDispatchTable(this->GetThread()->template GetCurrentDispatchTable<IS_DEBUG>());
        RuntimeIfaceT::SetCurrentFrame(thread, prev);

        if (UNLIKELY(this->GetThread()->HasPendingException())) {
            this->MoveToExceptionHandler();
        } else {
            this->GetAcc() = frame->GetAcc();
            this->SetInst(prev->GetNextInstruction());
        }

        if constexpr (!IS_DYNAMIC) {  // delegated to dynamic return
            if (frame->IsInitobj()) {
                this->GetAcc() = prev->GetAcc();
            }
        }

        RuntimeIfaceT::FreeFrame(this->GetThread(), frame);

        LOG(DEBUG, INTERPRETER) << "Exit: Runtime Call.";
    }

    ALWAYS_INLINE void HandleInstrumentForceReturn()
    {
        HandleReturnStackless();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleCheckcast()
    {
        auto typeId = this->GetInst().template GetId<FORMAT>();

        LOG_INST() << "checkcast " << std::hex << "0x" << typeId;

        Class *type = ResolveType(typeId);
        if (LIKELY(type != nullptr)) {
            ObjectHeader *obj = this->GetAcc().GetReference();

            if (UNLIKELY(obj != nullptr && !type->IsAssignableFrom(obj->ClassAddr<Class>()))) {
                RuntimeIfaceT::ThrowClassCastException(type, obj->ClassAddr<Class>());
                this->MoveToExceptionHandler();
            } else {
                this->template MoveToNextInst<FORMAT, true>();
            }
        } else {
            this->MoveToExceptionHandler();
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleIsinstance()
    {
        auto typeId = this->GetInst().template GetId<FORMAT>();

        LOG_INST() << "isinstance " << std::hex << "0x" << typeId;

        Class *type = ResolveType(typeId);
        if (LIKELY(type != nullptr)) {
            ObjectHeader *obj = this->GetAcc().GetReference();

            if (obj != nullptr && type->IsAssignableFrom(obj->ClassAddr<Class>())) {
                this->GetAccAsVReg().SetPrimitive(1);
            } else {
                this->GetAccAsVReg().SetPrimitive(0);
            }
            this->template MoveToNextInst<FORMAT, false>();
        } else {
            this->MoveToExceptionHandler();
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleCallShort()
    {
        auto id = this->GetInst().template GetId<FORMAT>();

        LOG_INST() << "call.short v" << this->GetInst().template GetVReg<FORMAT, 0>() << ", v"
                   << this->GetInst().template GetVReg<FORMAT, 1>() << ", " << std::hex << "0x" << id;

        auto *method = ResolveMethod(id);
        if (LIKELY(method != nullptr)) {
            if (!method->IsStatic() && this->GetCallerObject<FORMAT>() == nullptr) {
                return;
            }
            HandleCall<FrameHelperDefault, FORMAT>(method);
        } else {
            this->MoveToExceptionHandler();
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleCallAccShort()
    {
        auto id = this->GetInst().template GetId<FORMAT>();

        LOG_INST() << "call.acc.short v" << this->GetInst().template GetVReg<FORMAT, 0>() << ", "
                   << this->GetInst().template GetImm<FORMAT, 0>() << ", " << std::hex << "0x" << id;

        auto *method = ResolveMethod(id);
        if (LIKELY(method != nullptr)) {
            if (!method->IsStatic() && this->GetCallerObject<FORMAT, true>() == nullptr) {
                return;
            }
            HandleCall<FrameHelperDefault, FORMAT, false, false, true>(method);
        } else {
            this->MoveToExceptionHandler();
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleCall()
    {
        auto id = this->GetInst().template GetId<FORMAT>();

        LOG_INST() << "call v" << this->GetInst().template GetVReg<FORMAT, 0>() << ", v"
                   << this->GetInst().template GetVReg<FORMAT, 1>() << ", v"
                   << this->GetInst().template GetVReg<FORMAT, 2>() << ", v"
                   << this->GetInst().template GetVReg<FORMAT, 3>() << ", " << std::hex << "0x" << id;

        auto *method = ResolveMethod(id);
        if (LIKELY(method != nullptr)) {
            if (!method->IsStatic() && this->GetCallerObject<FORMAT>() == nullptr) {
                return;
            }
            HandleCall<FrameHelperDefault, FORMAT>(method);
        } else {
            this->MoveToExceptionHandler();
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleCallAcc()
    {
        auto id = this->GetInst().template GetId<FORMAT>();

        LOG_INST() << "call.acc v" << this->GetInst().template GetVReg<FORMAT, 0>() << ", v"
                   << this->GetInst().template GetVReg<FORMAT, 1>() << ", v"
                   << this->GetInst().template GetVReg<FORMAT, 2>() << ", "
                   << this->GetInst().template GetImm<FORMAT, 0>() << ", " << std::hex << "0x" << id;

        auto *method = ResolveMethod(id);
        if (LIKELY(method != nullptr)) {
            if (!method->IsStatic() && this->GetCallerObject<FORMAT, true>() == nullptr) {
                return;
            }
            HandleCall<FrameHelperDefault, FORMAT, false, false, true>(method);
        } else {
            this->MoveToExceptionHandler();
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleCallRange()
    {
        auto id = this->GetInst().template GetId<FORMAT>();

        LOG_INST() << "call.range v" << this->GetInst().template GetVReg<FORMAT, 0>() << ", " << std::hex << "0x" << id;

        auto *method = ResolveMethod(id);
        if (LIKELY(method != nullptr)) {
            if (!method->IsStatic() && this->GetCallerObject<FORMAT>() == nullptr) {
                return;
            }
            HandleCall<FrameHelperDefault, FORMAT, false, true>(method);
        } else {
            this->MoveToExceptionHandler();
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleCallVirtShort()
    {
        auto id = this->GetInst().template GetId<FORMAT>();

        LOG_INST() << "call.virt.short v" << this->GetInst().template GetVReg<FORMAT, 0>() << ", v"
                   << this->GetInst().template GetVReg<FORMAT, 1>() << ", " << std::hex << "0x" << id;

        auto *method = ResolveMethod(id);
        if (LIKELY(method != nullptr)) {
            HandleVirtualCall<FORMAT>(method);
        } else {
            this->MoveToExceptionHandler();
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleCallVirtAccShort()
    {
        auto id = this->GetInst().template GetId<FORMAT>();

        LOG_INST() << "call.virt.acc.short v" << this->GetInst().template GetVReg<FORMAT, 0>() << ", "
                   << this->GetInst().template GetImm<FORMAT, 0>() << ", " << std::hex << "0x" << id;

        auto *method = ResolveMethod(id);
        if (LIKELY(method != nullptr)) {
            HandleVirtualCall<FORMAT, false, true>(method);
        } else {
            this->MoveToExceptionHandler();
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleCallVirt()
    {
        auto id = this->GetInst().template GetId<FORMAT>();

        LOG_INST() << "call.virt v" << this->GetInst().template GetVReg<FORMAT, 0>() << ", v"
                   << this->GetInst().template GetVReg<FORMAT, 1>() << ", v"
                   << this->GetInst().template GetVReg<FORMAT, 2>() << ", v"
                   << this->GetInst().template GetVReg<FORMAT, 3>() << ", " << std::hex << "0x" << id;

        auto *method = ResolveMethod(id);
        if (LIKELY(method != nullptr)) {
            HandleVirtualCall<FORMAT>(method);
        } else {
            this->MoveToExceptionHandler();
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleCallVirtAcc()
    {
        auto id = this->GetInst().template GetId<FORMAT>();

        LOG_INST() << "call.virt.acc v" << this->GetInst().template GetVReg<FORMAT, 0>() << ", v"
                   << this->GetInst().template GetVReg<FORMAT, 1>() << ", v"
                   << this->GetInst().template GetVReg<FORMAT, 2>() << ", "
                   << this->GetInst().template GetImm<FORMAT, 0>() << ", " << std::hex << "0x" << id;

        auto *method = ResolveMethod(id);
        if (LIKELY(method != nullptr)) {
            HandleVirtualCall<FORMAT, false, true>(method);
        } else {
            this->MoveToExceptionHandler();
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleCallVirtRange()
    {
        auto id = this->GetInst().template GetId<FORMAT>();

        LOG_INST() << "call.virt.range v" << this->GetInst().template GetVReg<FORMAT, 0>() << ", " << std::hex << "0x"
                   << id;

        auto *method = ResolveMethod(id);
        if (LIKELY(method != nullptr)) {
            HandleVirtualCall<FORMAT, true>(method);
        } else {
            this->MoveToExceptionHandler();
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleThrow()
    {
        uint16_t vs = this->GetInst().template GetVReg<FORMAT>();

        LOG_INST() << "throw v" << vs;

        ObjectHeader *exception = this->GetFrame()->GetVReg(vs).GetReference();
        if (UNLIKELY(exception == nullptr)) {
            RuntimeIfaceT::ThrowNullPointerException();
        } else {
            this->GetThread()->SetException(exception);
        }

        this->UpdateThrowStatistics();

        this->MoveToExceptionHandler();
    }

    ALWAYS_INLINE uint32_t FindCatchBlockStackless()
    {
        Frame *frame = this->GetFrame();
        while (frame != nullptr) {
            ManagedThread *thread = this->GetThread();
            Frame *prev = frame->GetPrevFrame();

            ASSERT(thread->HasPendingException());

            uint32_t pcOffset = this->FindCatchBlock(thread->GetException(), this->GetBytecodeOffset());
            if (pcOffset != panda_file::INVALID_OFFSET) {
                return pcOffset;
            }

            if (!frame->IsStackless() || prev == nullptr ||
                StackWalker::IsBoundaryFrame<FrameKind::INTERPRETER>(prev)) {
                return pcOffset;
            }

            Method *method = frame->GetMethod();
            EVENT_METHOD_EXIT(method->GetFullName(), events::MethodExitKind::INTERP, thread->RecordMethodExit());

            Runtime::GetCurrent()->GetNotificationManager()->MethodExitEvent(thread, method);

            this->GetInstructionHandlerState()->UpdateInstructionHandlerState(
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                prev->GetInstruction() + prev->GetBytecodeOffset(), prev);

            thread->GetVM()->HandleReturnFrame();

            RuntimeIfaceT::SetCurrentFrame(thread, prev);

            ASSERT(thread->HasPendingException());

            RuntimeIfaceT::FreeFrame(this->GetThread(), frame);

            LOG(DEBUG, INTERPRETER) << "Exit: Runtime Call.";

            frame = prev;
        }
        return panda_file::INVALID_OFFSET;
    }

    ALWAYS_INLINE static bool IsCompilerEnableJit()
    {
        return !Runtime::GetCurrent()->IsDebugMode() && RuntimeIfaceT::IsCompilerEnableJit();
    }

    uint32_t FindCatchBlock(ObjectHeader *exception, uint32_t pc) const
    {
        auto *method = this->GetFrame()->GetMethod();
        return RuntimeIfaceT::FindCatchBlock(*method, exception, pc);
    }

    template <class T>
    ALWAYS_INLINE Class *GetClass(const T *entity)
    {
        auto *klass = entity->GetClass();

        // Whenever we obtain a class by a field, method, etc.,
        // we expect it to be either fully initialized or in process
        // of initialization (e.g. during executing a cctor).
        ASSERT(klass != nullptr);
        ASSERT(klass->IsInitializing() || klass->IsInitialized());

        return klass;
    }

    template <class F, class T, class R>
    ALWAYS_INLINE void LoadPrimitiveFieldReg(R &vreg, T *obj, Field *field)
    {
        auto value = static_cast<int64_t>(obj->template GetFieldPrimitive<F>(*field));
        vreg.SetPrimitive(value);
    }

    template <class T, class R>
    ALWAYS_INLINE void LoadPrimitiveFieldReg(R &vreg, T *obj, Field *field)
    {
        switch (field->GetTypeId()) {
            case panda_file::Type::TypeId::U1:
            case panda_file::Type::TypeId::U8:
                LoadPrimitiveFieldReg<uint8_t>(vreg, obj, field);
                break;
            case panda_file::Type::TypeId::I8:
                LoadPrimitiveFieldReg<int8_t>(vreg, obj, field);
                break;
            case panda_file::Type::TypeId::I16:
                LoadPrimitiveFieldReg<int16_t>(vreg, obj, field);
                break;
            case panda_file::Type::TypeId::U16:
                LoadPrimitiveFieldReg<uint16_t>(vreg, obj, field);
                break;
            case panda_file::Type::TypeId::I32:
                LoadPrimitiveFieldReg<int32_t>(vreg, obj, field);
                break;
            case panda_file::Type::TypeId::U32:
                LoadPrimitiveFieldReg<uint32_t>(vreg, obj, field);
                break;
            case panda_file::Type::TypeId::I64:
                LoadPrimitiveFieldReg<int64_t>(vreg, obj, field);
                break;
            case panda_file::Type::TypeId::U64:
                LoadPrimitiveFieldReg<uint64_t>(vreg, obj, field);
                break;
            case panda_file::Type::TypeId::F32:
                vreg.SetPrimitive(obj->template GetFieldPrimitive<float>(*field));
                break;
            case panda_file::Type::TypeId::F64:
                vreg.SetPrimitive(obj->template GetFieldPrimitive<double>(*field));
                break;
            default:
                UNREACHABLE();
                break;
        }
    }

    template <class F, class T>
    ALWAYS_INLINE void LoadPrimitiveField(T *obj, Field *field)
    {
        auto value = static_cast<int64_t>(obj->template GetFieldPrimitive<F>(*field));
        this->GetAccAsVReg().SetPrimitive(value);
    }

    template <class T>
    ALWAYS_INLINE void LoadPrimitiveField(T *obj, Field *field)
    {
        switch (field->GetTypeId()) {
            case panda_file::Type::TypeId::U1:
            case panda_file::Type::TypeId::U8:
                LoadPrimitiveField<uint8_t>(obj, field);
                break;
            case panda_file::Type::TypeId::I8:
                LoadPrimitiveField<int8_t>(obj, field);
                break;
            case panda_file::Type::TypeId::I16:
                LoadPrimitiveField<int16_t>(obj, field);
                break;
            case panda_file::Type::TypeId::U16:
                LoadPrimitiveField<uint16_t>(obj, field);
                break;
            case panda_file::Type::TypeId::I32:
                LoadPrimitiveField<int32_t>(obj, field);
                break;
            case panda_file::Type::TypeId::U32:
                LoadPrimitiveField<uint32_t>(obj, field);
                break;
            case panda_file::Type::TypeId::I64:
                LoadPrimitiveField<int64_t>(obj, field);
                break;
            case panda_file::Type::TypeId::U64:
                LoadPrimitiveField<uint64_t>(obj, field);
                break;
            case panda_file::Type::TypeId::F32:
                this->GetAccAsVReg().SetPrimitive(obj->template GetFieldPrimitive<float>(*field));
                break;
            case panda_file::Type::TypeId::F64:
                this->GetAccAsVReg().SetPrimitive(obj->template GetFieldPrimitive<double>(*field));
                break;
            default:
                UNREACHABLE();
                break;
        }
    }

    template <class T, class R>
    ALWAYS_INLINE void StorePrimitiveFieldReg(R &vreg, T *obj, Field *field)
    {
        switch (field->GetTypeId()) {
            case panda_file::Type::TypeId::U1:
            case panda_file::Type::TypeId::U8: {
                obj->SetFieldPrimitive(*field, vreg.template GetAs<uint8_t>());
                break;
            }
            case panda_file::Type::TypeId::I8: {
                obj->SetFieldPrimitive(*field, vreg.template GetAs<int8_t>());
                break;
            }
            case panda_file::Type::TypeId::I16: {
                obj->SetFieldPrimitive(*field, vreg.template GetAs<int16_t>());
                break;
            }
            case panda_file::Type::TypeId::U16: {
                obj->SetFieldPrimitive(*field, vreg.template GetAs<uint16_t>());
                break;
            }
            case panda_file::Type::TypeId::I32: {
                obj->SetFieldPrimitive(*field, vreg.template GetAs<int32_t>());
                break;
            }
            case panda_file::Type::TypeId::U32: {
                obj->SetFieldPrimitive(*field, vreg.template GetAs<uint32_t>());
                break;
            }
            case panda_file::Type::TypeId::I64: {
                obj->SetFieldPrimitive(*field, vreg.template GetAs<int64_t>());
                break;
            }
            case panda_file::Type::TypeId::U64: {
                obj->SetFieldPrimitive(*field, vreg.template GetAs<uint64_t>());
                break;
            }
            case panda_file::Type::TypeId::F32: {
                obj->SetFieldPrimitive(*field, vreg.template GetAs<float>());
                break;
            }
            case panda_file::Type::TypeId::F64: {
                obj->SetFieldPrimitive(*field, vreg.template GetAs<double>());
                break;
            }
            default: {
                UNREACHABLE();
                break;
            }
        }
    }

    template <class T>
    ALWAYS_INLINE void StorePrimitiveField(T *obj, Field *field)
    {
        switch (field->GetTypeId()) {
            case panda_file::Type::TypeId::U1:
            case panda_file::Type::TypeId::U8: {
                obj->SetFieldPrimitive(*field, this->GetAcc().template GetAs<uint8_t>());
                break;
            }
            case panda_file::Type::TypeId::I8: {
                obj->SetFieldPrimitive(*field, this->GetAcc().template GetAs<int8_t>());
                break;
            }
            case panda_file::Type::TypeId::I16: {
                obj->SetFieldPrimitive(*field, this->GetAcc().template GetAs<int16_t>());
                break;
            }
            case panda_file::Type::TypeId::U16: {
                obj->SetFieldPrimitive(*field, this->GetAcc().template GetAs<uint16_t>());
                break;
            }
            case panda_file::Type::TypeId::I32: {
                obj->SetFieldPrimitive(*field, this->GetAcc().template GetAs<int32_t>());
                break;
            }
            case panda_file::Type::TypeId::U32: {
                obj->SetFieldPrimitive(*field, this->GetAcc().template GetAs<uint32_t>());
                break;
            }
            case panda_file::Type::TypeId::I64: {
                obj->SetFieldPrimitive(*field, this->GetAcc().template GetAs<int64_t>());
                break;
            }
            case panda_file::Type::TypeId::U64: {
                obj->SetFieldPrimitive(*field, this->GetAcc().template GetAs<uint64_t>());
                break;
            }
            case panda_file::Type::TypeId::F32: {
                obj->SetFieldPrimitive(*field, this->GetAcc().template GetAs<float>());
                break;
            }
            case panda_file::Type::TypeId::F64: {
                obj->SetFieldPrimitive(*field, this->GetAcc().template GetAs<double>());
                break;
            }
            default: {
                UNREACHABLE();
                break;
            }
        }
    }

    template <BytecodeInstruction::Format FORMAT, class T>
    ALWAYS_INLINE void HandleArrayPrimitiveLoad()
    {
        static_assert(std::is_integral_v<T> || std::is_floating_point_v<T>,
                      "T should be either integral or floating point type");
        uint16_t vs = this->GetInst().template GetVReg<FORMAT>();

        LOG_INST() << "\t"
                   << "load v" << vs;

        auto *array = static_cast<coretypes::Array *>(this->GetFrame()->GetVReg(vs).GetReference());
        int32_t idx = this->GetAcc().Get();

        if (LIKELY(CheckLoadArrayOp(array, idx))) {
            this->GetAcc().Set(array->Get<T>(idx));
            this->template MoveToNextInst<FORMAT, true>();
        } else {
            this->MoveToExceptionHandler();
        }
    }

    template <BytecodeInstruction::Format FORMAT, class T>
    ALWAYS_INLINE void HandleArrayStore()
    {
        uint16_t vs1 = this->GetInst().template GetVReg<FORMAT, 0>();
        uint16_t vs2 = this->GetInst().template GetVReg<FORMAT, 1>();

        LOG_INST() << "\t"
                   << "store v" << vs1 << ", v" << vs2;

        auto *array = static_cast<coretypes::Array *>(this->GetFrame()->GetVReg(vs1).GetReference());
        int32_t idx = this->GetFrame()->GetVReg(vs2).Get();

        auto elem = this->GetAcc().template GetAs<T>();
        if (LIKELY(CheckStoreArrayOp(array, idx, elem))) {
            array->Set<T, RuntimeIfaceT::NEED_WRITE_BARRIER>(this->GetThread(), idx, elem);
            this->template MoveToNextInst<FORMAT, true>();
        } else {
            this->MoveToExceptionHandler();
        }
    }

    template <class T>
    ALWAYS_INLINE bool CheckStoreArrayOp(coretypes::Array *array, int32_t idx, [[maybe_unused]] T elem)
    {
        if (UNLIKELY(array == nullptr)) {
            RuntimeIfaceT::ThrowNullPointerException();
            return false;
        }

        if (UNLIKELY(idx < 0 || helpers::ToUnsigned(idx) >= array->GetLength())) {
            RuntimeIfaceT::ThrowArrayIndexOutOfBoundsException(idx, array->GetLength());
            return false;
        }

        if constexpr (std::is_same_v<T, ObjectHeader *>) {
            if (elem != nullptr) {
                auto *arrayClass = array->ClassAddr<Class>();
                auto *elementClass = arrayClass->GetComponentType();
                if (UNLIKELY(!elem->IsInstanceOf(elementClass))) {
                    RuntimeIfaceT::ThrowArrayStoreException(arrayClass, elem->template ClassAddr<Class>());
                    return false;
                }
            }
        }

        return true;
    }

    ALWAYS_INLINE bool CheckLoadArrayOp(coretypes::Array *array, int32_t idx)
    {
        if (UNLIKELY(array == nullptr)) {
            RuntimeIfaceT::ThrowNullPointerException();
            return false;
        }

        if (UNLIKELY(idx < 0 || helpers::ToUnsigned(idx) >= array->GetLength())) {
            RuntimeIfaceT::ThrowArrayIndexOutOfBoundsException(idx, array->GetLength());
            return false;
        }

        return true;
    }

    ALWAYS_INLINE coretypes::Array *ResolveLiteralArray(BytecodeId id)
    {
        return RuntimeIfaceT::ResolveLiteralArray(this->GetThread()->GetVM(), *this->GetFrame()->GetMethod(), id);
    }

    ALWAYS_INLINE Method *ResolveMethod(BytecodeId id)
    {
        this->UpdateBytecodeOffset();

        auto cache = this->GetThread()->GetInterpreterCache();
        auto *res = cache->template Get<Method>(this->GetInst().GetAddress(), this->GetFrame()->GetMethod());
        if (res != nullptr) {
            return res;
        }

        this->GetFrame()->SetAcc(this->GetAcc());
        auto *method = RuntimeIfaceT::ResolveMethod(this->GetThread(), *this->GetFrame()->GetMethod(), id);
        this->GetAcc() = this->GetFrame()->GetAcc();
        if (UNLIKELY(method == nullptr)) {
            ASSERT(this->GetThread()->HasPendingException());
            return nullptr;
        }

        cache->Set(this->GetInst().GetAddress(), method, this->GetFrame()->GetMethod());
        return method;
    }

    template <bool NEED_INIT = false>
    ALWAYS_INLINE Field *ResolveField(BytecodeId id)
    {
        auto cache = this->GetThread()->GetInterpreterCache();
        auto *res = cache->template Get<Field>(this->GetInst().GetAddress(), this->GetFrame()->GetMethod());
        if (res != nullptr) {
            return res;
        }

        if (NEED_INIT) {
            // Update bytecode offset in the current frame as RuntimeIfaceT::ResolveField can trigger class initializer
            this->UpdateBytecodeOffset();
        }

        this->GetFrame()->SetAcc(this->GetAcc());
        auto *field = RuntimeIfaceT::ResolveField(this->GetThread(), *this->GetFrame()->GetMethod(), id, NEED_INIT);
        this->GetAcc() = this->GetFrame()->GetAcc();
        if (UNLIKELY(field == nullptr)) {
            ASSERT(this->GetThread()->HasPendingException());
            return nullptr;
        }

        cache->Set(this->GetInst().GetAddress(), field, this->GetFrame()->GetMethod());
        return field;
    }

    template <bool NEED_INIT = false>
    ALWAYS_INLINE Class *ResolveType(BytecodeId id)
    {
        auto cache = this->GetThread()->GetInterpreterCache();
        auto *res = cache->template Get<Class>(this->GetInst().GetAddress(), this->GetFrame()->GetMethod());
        if (res != nullptr) {
            ASSERT(!NEED_INIT || res->IsInitializing() || res->IsInitialized());
            return res;
        }

        this->GetFrame()->SetAcc(this->GetAcc());
        auto *klass =
            RuntimeIfaceT::template ResolveClass<NEED_INIT>(this->GetThread(), *this->GetFrame()->GetMethod(), id);
        this->GetAcc() = this->GetFrame()->GetAcc();
        if (UNLIKELY(klass == nullptr)) {
            ASSERT(this->GetThread()->HasPendingException());
            return nullptr;
        }

        ASSERT(!NEED_INIT || klass->IsInitializing() || klass->IsInitialized());

        cache->Set(this->GetInst().GetAddress(), klass, this->GetFrame()->GetMethod());
        return klass;
    }

    template <BytecodeInstruction::Format FORMAT, bool IS_DYNAMIC_T>
    ALWAYS_INLINE inline void CopyCallAccShortArguments(Frame &frame, uint32_t numVregs)
    {
        auto curFrameHandler = this->template GetFrameHandler<IS_DYNAMIC_T>();
        auto frameHandler = this->template GetFrameHandler<IS_DYNAMIC_T>(&frame);
        static_assert(FORMAT == BytecodeInstruction::Format::V4_IMM4_ID16, "Invalid call acc short format");
        auto accPosition = static_cast<size_t>(this->GetInst().template GetImm<FORMAT, 0>());
        switch (accPosition) {
            case 0U:
                frameHandler.GetVReg(numVregs).Move(this->template GetAccAsVReg<IS_DYNAMIC_T>());
                frameHandler.GetVReg(numVregs + 1U) =
                    curFrameHandler.GetVReg(this->GetInst().template GetVReg<FORMAT, 0>());
                break;
            case 1U:
                frameHandler.GetVReg(numVregs) = curFrameHandler.GetVReg(this->GetInst().template GetVReg<FORMAT, 0>());
                frameHandler.GetVReg(numVregs + 1U).Move(this->template GetAccAsVReg<IS_DYNAMIC_T>());
                break;
            default:
                UNREACHABLE();
        }
    }

    template <BytecodeInstruction::Format FORMAT, bool IS_DYNAMIC_T>
    ALWAYS_INLINE inline void CopyCallAccArguments(Frame &frame, uint32_t numVregs)
    {
        auto curFrameHandler = this->template GetFrameHandler<IS_DYNAMIC_T>();
        auto frameHandler = this->template GetFrameHandler<IS_DYNAMIC_T>(&frame);
        static_assert(FORMAT == BytecodeInstruction::Format::V4_V4_V4_IMM4_ID16, "Invalid call acc format");
        auto accPosition = static_cast<size_t>(this->GetInst().template GetImm<FORMAT, 0>());
        switch (accPosition) {
            case 0U:
                frameHandler.GetVReg(numVregs).Move(this->template GetAccAsVReg<IS_DYNAMIC_T>());
                frameHandler.GetVReg(numVregs + 1U) =
                    curFrameHandler.GetVReg(this->GetInst().template GetVReg<FORMAT, 0>());
                frameHandler.GetVReg(numVregs + 2U) =
                    curFrameHandler.GetVReg(this->GetInst().template GetVReg<FORMAT, 1>());
                frameHandler.GetVReg(numVregs + 3U) =
                    curFrameHandler.GetVReg(this->GetInst().template GetVReg<FORMAT, 2>());
                break;
            case 1U:
                frameHandler.GetVReg(numVregs) = curFrameHandler.GetVReg(this->GetInst().template GetVReg<FORMAT, 0>());
                frameHandler.GetVReg(numVregs + 1U).Move(this->template GetAccAsVReg<IS_DYNAMIC_T>());
                frameHandler.GetVReg(numVregs + 2U) =
                    curFrameHandler.GetVReg(this->GetInst().template GetVReg<FORMAT, 1>());
                frameHandler.GetVReg(numVregs + 3U) =
                    curFrameHandler.GetVReg(this->GetInst().template GetVReg<FORMAT, 2>());
                break;
            case 2U:
                frameHandler.GetVReg(numVregs) = curFrameHandler.GetVReg(this->GetInst().template GetVReg<FORMAT, 0>());
                frameHandler.GetVReg(numVregs + 1U) =
                    curFrameHandler.GetVReg(this->GetInst().template GetVReg<FORMAT, 1>());
                frameHandler.GetVReg(numVregs + 2U).Move(this->template GetAccAsVReg<IS_DYNAMIC_T>());
                frameHandler.GetVReg(numVregs + 3U) =
                    curFrameHandler.GetVReg(this->GetInst().template GetVReg<FORMAT, 2>());
                break;
            case 3U:
                frameHandler.GetVReg(numVregs) = curFrameHandler.GetVReg(this->GetInst().template GetVReg<FORMAT, 0>());
                frameHandler.GetVReg(numVregs + 1U) =
                    curFrameHandler.GetVReg(this->GetInst().template GetVReg<FORMAT, 1>());
                frameHandler.GetVReg(numVregs + 2U) =
                    curFrameHandler.GetVReg(this->GetInst().template GetVReg<FORMAT, 2>());
                frameHandler.GetVReg(numVregs + 3U).Move(this->template GetAccAsVReg<IS_DYNAMIC_T>());
                break;
            default:
                UNREACHABLE();
        }
    }

    template <BytecodeInstruction::Format FORMAT, bool IS_DYNAMIC_T, bool INITOBJ>
    ALWAYS_INLINE inline void CopyCallShortArguments(Frame &frame, uint32_t numVregs)
    {
        static_assert(FORMAT == BytecodeInstruction::Format::V4_V4_ID16, "Invalid call short format");

        auto curFrameHandler = this->template GetFrameHandler<IS_DYNAMIC_T>();
        auto frameHandler = this->template GetFrameHandler<IS_DYNAMIC_T>(&frame);
        constexpr size_t SHIFT = INITOBJ ? 1U : 0;
        if constexpr (INITOBJ) {
            frameHandler.GetVReg(numVregs).Move(this->template GetAccAsVReg<IS_DYNAMIC_T>());
        }

        frameHandler.GetVReg(numVregs + SHIFT) = curFrameHandler.GetVReg(this->GetInst().template GetVReg<FORMAT, 0>());
        frameHandler.GetVReg(numVregs + SHIFT + 1U) =
            curFrameHandler.GetVReg(this->GetInst().template GetVReg<FORMAT, 1U>());
    }

    template <BytecodeInstruction::Format FORMAT, bool IS_DYNAMIC_T, bool INITOBJ>
    ALWAYS_INLINE inline void CopyCallArguments(Frame &frame, uint32_t numVregs)
    {
        static_assert(FORMAT == BytecodeInstruction::Format::V4_V4_V4_V4_ID16, "Invalid call format");

        auto curFrameHandler = this->template GetFrameHandler<IS_DYNAMIC_T>();
        auto frameHandler = this->template GetFrameHandler<IS_DYNAMIC_T>(&frame);
        constexpr size_t SHIFT = INITOBJ ? 1U : 0;
        if constexpr (INITOBJ) {
            frameHandler.GetVReg(numVregs).Move(this->template GetAccAsVReg<IS_DYNAMIC_T>());
        }

        frameHandler.GetVReg(numVregs + SHIFT) = curFrameHandler.GetVReg(this->GetInst().template GetVReg<FORMAT, 0>());
        frameHandler.GetVReg(numVregs + SHIFT + 1U) =
            curFrameHandler.GetVReg(this->GetInst().template GetVReg<FORMAT, 1U>());
        frameHandler.GetVReg(numVregs + SHIFT + 2U) =
            curFrameHandler.GetVReg(this->GetInst().template GetVReg<FORMAT, 2U>());
        frameHandler.GetVReg(numVregs + SHIFT + 3U) =
            curFrameHandler.GetVReg(this->GetInst().template GetVReg<FORMAT, 3U>());
    }

    template <bool IS_DYNAMIC_T, BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE inline void CopyCallArguments(Frame &frame, uint32_t numVregs, uint32_t numActualArgs)
    {
        auto curFrameHandler = this->template GetFrameHandler<IS_DYNAMIC_T>();
        auto frameHandler = this->template GetFrameHandler<IS_DYNAMIC_T>(&frame);

#ifdef PANDA_WITH_ETS
        if constexpr (FORMAT == BytecodeInstruction::Format::PREF_V4_V4_ID16) {
            frameHandler.GetVReg(numVregs) = curFrameHandler.GetVReg(this->GetInst().template GetVReg<FORMAT, 0>());
            frameHandler.GetVReg(numVregs + 1U) =
                curFrameHandler.GetVReg(this->GetInst().template GetVReg<FORMAT, 1U>());
            return;
        } else if constexpr (FORMAT == BytecodeInstruction::Format::PREF_V4_V4_V4_V4_ID16) {
            frameHandler.GetVReg(numVregs) = curFrameHandler.GetVReg(this->GetInst().template GetVReg<FORMAT, 0>());
            frameHandler.GetVReg(numVregs + 1U) =
                curFrameHandler.GetVReg(this->GetInst().template GetVReg<FORMAT, 1U>());
            frameHandler.GetVReg(numVregs + 2U) =
                curFrameHandler.GetVReg(this->GetInst().template GetVReg<FORMAT, 2U>());
            frameHandler.GetVReg(numVregs + 3U) =
                curFrameHandler.GetVReg(this->GetInst().template GetVReg<FORMAT, 3U>());
            return;
        }
#endif
        frameHandler.GetVReg(numVregs) = curFrameHandler.GetVReg(this->GetInst().GetVReg(0));
        if (numActualArgs == 2) {
            frameHandler.GetVReg(numVregs + 1U).Move(this->template GetAccAsVReg<IS_DYNAMIC_T>());
        }
    }

    template <BytecodeInstruction::Format FORMAT, bool IS_DYNAMIC_T, bool INITOBJ>
    ALWAYS_INLINE inline void CopyRangeArguments(Frame &frame, uint32_t numVregs, uint32_t numActualArgs)
    {
        auto curFrameHandler = this->template GetFrameHandler<IS_DYNAMIC_T>();
        auto frameHandler = this->template GetFrameHandler<IS_DYNAMIC_T>(&frame);
        constexpr size_t SHIFT = INITOBJ ? 1U : 0;
        if constexpr (INITOBJ) {
            frameHandler.GetVReg(numVregs).Move(this->template GetAccAsVReg<IS_DYNAMIC_T>());
        }

        uint16_t startReg = this->GetInst().template GetVReg<FORMAT, 0>();
        for (size_t i = 0; i < numActualArgs - SHIFT; i++) {
            frameHandler.GetVReg(numVregs + SHIFT + i) = curFrameHandler.GetVReg(startReg + i);
        }
    }

    template <class FrameHelper, BytecodeInstruction::Format FORMAT, bool IS_DYNAMIC_T, bool IS_RANGE, bool ACCEPT_ACC,
              bool INITOBJ, bool CALL>
    ALWAYS_INLINE inline void CopyArguments(Frame &frame, uint32_t numVregs, [[maybe_unused]] uint32_t numActualArgs,
                                            uint32_t numArgs)
    {
        if constexpr (IS_DYNAMIC_T) {
            FrameHelper::template CopyArgumentsDyn<FORMAT>(this, &frame, numVregs, numActualArgs);
            return;
        }
        if (numArgs == 0) {
            return;
        }

        if constexpr (IS_RANGE) {
            CopyRangeArguments<FORMAT, IS_DYNAMIC_T, INITOBJ>(frame, numVregs, numActualArgs);
        } else if constexpr (ACCEPT_ACC) {
            if constexpr (FORMAT == BytecodeInstruction::Format::V4_IMM4_ID16) {
                CopyCallAccShortArguments<FORMAT, IS_DYNAMIC_T>(frame, numVregs);
            } else if constexpr (FORMAT == BytecodeInstruction::Format::V4_V4_V4_IMM4_ID16) {
                CopyCallAccArguments<FORMAT, IS_DYNAMIC_T>(frame, numVregs);
            } else {
                UNREACHABLE();
            }
        } else if constexpr (!CALL) {
            CopyCallArguments<IS_DYNAMIC_T, FORMAT>(frame, numVregs, numActualArgs);
        } else {
            if constexpr (FORMAT == BytecodeInstruction::Format::V4_V4_ID16) {
                CopyCallShortArguments<FORMAT, IS_DYNAMIC_T, INITOBJ>(frame, numVregs);
            } else if constexpr (FORMAT == BytecodeInstruction::Format::V4_V4_V4_V4_ID16) {
                CopyCallArguments<FORMAT, IS_DYNAMIC_T, INITOBJ>(frame, numVregs);
            } else {
                UNREACHABLE();
            }
        }
    }

    template <class FrameHelper, BytecodeInstruction::Format FORMAT, bool IS_DYNAMIC_T, bool IS_RANGE, bool ACCEPT_ACC,
              bool INITOBJ, bool CALL, bool STACK_LESS>
    ALWAYS_INLINE inline bool CreateAndSetFrame(Method *method, Frame **frame, uint32_t numVregs)
    {
        uint32_t numDeclaredArgs = method->GetNumArgs();
        uint32_t numActualArgs;
        uint32_t nregs;

        if constexpr (IS_DYNAMIC_T) {
            numActualArgs = FrameHelper::template GetNumberActualArgsDyn<FORMAT>(this);
            nregs = numVregs + std::max(numDeclaredArgs, numActualArgs);
        } else {
            numActualArgs = numDeclaredArgs;
            if (FORMAT == BytecodeInstruction::Format::V4_V4_ID16 ||
                FORMAT == BytecodeInstruction::Format::V4_IMM4_ID16) {
                nregs = numVregs + (INITOBJ ? 3U : 2U);
            } else if (FORMAT == BytecodeInstruction::Format::V4_V4_V4_V4_ID16 ||
                       FORMAT == BytecodeInstruction::Format::V4_V4_V4_IMM4_ID16) {
                nregs = numVregs + (INITOBJ ? 5U : 4U);
            } else {
                nregs = numVregs + numDeclaredArgs;
            }
        }
        auto current = this->GetThread();
        *frame = FrameHelper::template CreateFrame<RuntimeIfaceT>(current, Frame::GetActualSize<IS_DYNAMIC>(nregs),
                                                                  method, this->GetFrame(), nregs, numActualArgs);

        if (UNLIKELY(*frame == nullptr)) {
            current->DisableStackOverflowCheck();
            ark::ThrowStackOverflowException(current);
            current->EnableStackOverflowCheck();
            this->MoveToExceptionHandler();
            return false;
        }

        (*frame)->SetAcc(this->GetAcc());
        if constexpr (IS_DYNAMIC_T) {
            (*frame)->SetDynamic();
        }

        CopyArguments<FrameHelper, FORMAT, IS_DYNAMIC_T, IS_RANGE, ACCEPT_ACC, INITOBJ, CALL>(
            **frame, numVregs, numActualArgs, numDeclaredArgs);

        RuntimeIfaceT::SetCurrentFrame(current, *frame);

        return true;
    }

    template <bool IS_DYNAMIC_T = false>
    ALWAYS_INLINE void HandleCallPrologue(Method *method)
    {
        ASSERT(method != nullptr);
        if constexpr (IS_DYNAMIC_T) {
            LOG(DEBUG, INTERPRETER) << "Entry: Runtime Call.";
        } else {
            LOG(DEBUG, INTERPRETER) << "Entry: " << method->GetFullName();
        }
        SAFEPOINT_TIME_CHECKER(SafepointTimerTable::ResetTimers(this->GetThread()->GetInternalId(), true));
        if (this->GetThread()->TestAllFlags()) {
            this->GetFrame()->SetAcc(this->GetAcc());
            RuntimeIfaceT::Safepoint();
            this->GetAcc() = this->GetFrame()->GetAcc();
        }
        if (!method->HasCompiledCode()) {
            this->template UpdateHotness<true>(method);
        }
    }

    template <class FrameHelper, BytecodeInstruction::Format FORMAT, bool IS_DYNAMIC_T, bool IS_RANGE, bool ACCEPT_ACC,
              bool INITOBJ, bool CALL>
    ALWAYS_INLINE inline void CallInterpreterStackless(Method *method)
    {
        uint32_t numVregs = method->GetNumVregs();
        auto *instructions = method->GetInstructions();

        Frame *frame = nullptr;
        if (!CreateAndSetFrame<FrameHelper, FORMAT, IS_DYNAMIC_T, IS_RANGE, ACCEPT_ACC, INITOBJ, CALL, true>(
                method, &frame, numVregs)) {
            return;
        }

        Runtime::GetCurrent()->GetNotificationManager()->MethodEntryEvent(this->GetThread(), method);

        frame->SetStackless();
        if constexpr (INITOBJ) {
            frame->SetInitobj();
            if constexpr (IS_DYNAMIC_T) {
                // Disabling OSR because there is a special logic in bytecode "return.*" instruction handlers for
                // INITOBJ frame.
                frame->DisableOsr();
            }
        }
        frame->SetInstruction(instructions);
        this->SetDispatchTable(this->GetThread()->template GetCurrentDispatchTable<IS_DEBUG>());
        this->template MoveToNextInst<FORMAT, false>();
        this->GetFrame()->SetNextInstruction(this->GetInst());
        this->GetInstructionHandlerState()->UpdateInstructionHandlerState(instructions, frame);
        EVENT_METHOD_ENTER(frame->GetMethod()->GetFullName(), events::MethodEnterKind::INTERP,
                           this->GetThread()->RecordMethodEnter());
    }

    template <BytecodeInstruction::Format FORMAT, bool IS_DYNAMIC_T>
    ALWAYS_INLINE inline void CallCompiledCode(Method *method)
    {
        this->GetFrame()->SetAcc(this->GetAcc());
        if constexpr (IS_DYNAMIC_T) {
            InterpreterToCompiledCodeBridgeDyn(this->GetInst().GetAddress(), this->GetFrame(), method,
                                               this->GetThread());
        } else {
            InterpreterToCompiledCodeBridge(this->GetInst().GetAddress(), this->GetFrame(), method, this->GetThread());
        }

        this->GetThread()->SetCurrentFrameIsCompiled(false);
        this->GetThread()->SetCurrentFrame(this->GetFrame());

        if (UNLIKELY(this->GetThread()->HasPendingException())) {
            this->MoveToExceptionHandler();
        } else {
            this->GetAcc() = this->GetFrame()->GetAcc();
            this->template MoveToNextInst<FORMAT, true>();
        }

        if constexpr (IS_DYNAMIC_T) {
            LOG(DEBUG, INTERPRETER) << "Exit: Runtime Call.";
        } else {
            LOG(DEBUG, INTERPRETER) << "Exit: " << method->GetFullName();
        }
    }

    template <class FrameHelper, BytecodeInstruction::Format FORMAT, bool IS_DYNAMIC_T = false, bool IS_RANGE = false,
              bool ACCEPT_ACC = false, bool INITOBJ = false, bool CALL = true>
    ALWAYS_INLINE void HandleCall(Method *method)
    {
        HandleCallPrologue<IS_DYNAMIC_T>(method);

        if (method->HasCompiledCode()) {
            CallCompiledCode<FORMAT, IS_DYNAMIC_T>(method);
        } else {
            CallInterpreterStackless<FrameHelper, FORMAT, IS_DYNAMIC_T, IS_RANGE, ACCEPT_ACC, INITOBJ, CALL>(method);
        }
    }

    template <BytecodeInstruction::Format FORMAT, bool IS_RANGE = false, bool ACCEPT_ACC = false>
    ALWAYS_INLINE void HandleVirtualCall(Method *method)
    {
        ASSERT(method != nullptr);
        ASSERT(!method->IsStatic());
        ASSERT(!method->IsConstructor());

        ObjectHeader *obj = this->GetCallerObject<FORMAT, ACCEPT_ACC>();
        if (UNLIKELY(obj == nullptr)) {
            return;
        }
        auto *cls = obj->ClassAddr<Class>();
        ASSERT(cls != nullptr);
        auto *resolved = cls->ResolveVirtualMethod(method);
        ASSERT(resolved != nullptr);

        ProfilingData *profData = this->GetFrame()->GetMethod()->GetProfilingData();
        if (profData != nullptr) {
            profData->UpdateInlineCaches(this->GetBytecodeOffset(), obj->ClassAddr<Class>());
        }

        HandleCall<FrameHelperDefault, FORMAT, false, IS_RANGE, ACCEPT_ACC>(resolved);
    }

    template <BytecodeInstruction::Format FORMAT, template <typename OpT> class Op>
    ALWAYS_INLINE void HandleCondJmpz()
    {
        auto imm = this->GetInst().template GetImm<FORMAT>();

        LOG_INST() << "\t"
                   << "cond jmpz " << std::hex << "0x" << imm;

        int32_t v1 = this->GetAcc().Get();

        if (Op<int32_t>()(v1, 0)) {
            this->template UpdateBranchStatistics<true>();
            if (!this->InstrumentBranches(imm)) {
                this->template JumpToInst<false>(imm);
            }
        } else {
            this->template UpdateBranchStatistics<false>();
            this->template MoveToNextInst<FORMAT, false>();
        }
    }

    template <BytecodeInstruction::Format FORMAT, template <typename OpT> class Op>
    ALWAYS_INLINE void HandleCondJmp()
    {
        auto imm = this->GetInst().template GetImm<FORMAT>();
        uint16_t vs = this->GetInst().template GetVReg<FORMAT>();

        LOG_INST() << "\t"
                   << "cond jmp v" << vs << ", " << std::hex << "0x" << imm;

        int32_t v1 = this->GetAcc().Get();
        int32_t v2 = this->GetFrame()->GetVReg(vs).Get();

        if (Op<int32_t>()(v1, v2)) {
            this->template UpdateBranchStatistics<true>();
            if (!this->InstrumentBranches(imm)) {
                this->template JumpToInst<false>(imm);
            }
        } else {
            this->template UpdateBranchStatistics<false>();
            this->template MoveToNextInst<FORMAT, false>();
        }
    }

    template <BytecodeInstruction::Format FORMAT, template <typename OpT> class Op>
    ALWAYS_INLINE void HandleCondJmpzObj()
    {
        auto imm = this->GetInst().template GetImm<FORMAT>();
        ObjectHeader *v1 = this->GetAcc().GetReference();

        LOG_INST() << "\t"
                   << "cond jmpz.obj " << std::hex << "0x" << imm;

        if (Op<ObjectHeader *>()(v1, nullptr)) {
            this->template UpdateBranchStatistics<true>();
            if (!this->InstrumentBranches(imm)) {
                this->template JumpToInst<false>(imm);
            }
        } else {
            this->template UpdateBranchStatistics<false>();
            this->template MoveToNextInst<FORMAT, false>();
        }
    }

    template <BytecodeInstruction::Format FORMAT, template <typename OpT> class Op>
    ALWAYS_INLINE void HandleCondJmpObj()
    {
        auto imm = this->GetInst().template GetImm<FORMAT>();
        uint16_t vs = this->GetInst().template GetVReg<FORMAT>();

        LOG_INST() << "\t"
                   << "cond jmp.obj v" << vs << ", " << std::hex << "0x" << imm;

        ObjectHeader *v1 = this->GetAcc().GetReference();
        ObjectHeader *v2 = this->GetFrame()->GetVReg(vs).GetReference();

        if (Op<ObjectHeader *>()(v1, v2)) {
            this->template UpdateBranchStatistics<true>();
            if (!this->InstrumentBranches(imm)) {
                this->template JumpToInst<false>(imm);
            }
        } else {
            this->template UpdateBranchStatistics<false>();
            this->template MoveToNextInst<FORMAT, false>();
        }
    }

    template <BytecodeInstruction::Format FORMAT, typename OpT, template <typename> class Op, bool IS_DIV = false>
    ALWAYS_INLINE void HandleBinaryOp2Imm()
    {
        OpT v1 = this->GetAcc().template GetAs<OpT>();
        OpT v2 = this->GetInst().template GetImm<FORMAT>();

        LOG_INST() << "\t"
                   << "binop2imm " << std::hex << "0x" << v2;

        if (IS_DIV && UNLIKELY(v2 == 0)) {
            RuntimeIfaceT::ThrowArithmeticException();
            this->MoveToExceptionHandler();
        } else {
            this->GetAcc().Set(Op<OpT>()(v1, v2));
            this->template MoveToNextInst<FORMAT, IS_DIV>();
        }
    }

    template <BytecodeInstruction::Format FORMAT, typename OpT, template <typename> class Op, bool IS_DIV = false>
    ALWAYS_INLINE void HandleBinaryOpImmV()
    {
        uint16_t vd = this->GetInst().template GetVReg<FORMAT, 0>();
        uint16_t vs = this->GetInst().template GetVReg<FORMAT, 1>();
        OpT imm = this->GetInst().template GetImm<FORMAT>();

        LOG_INST() << "\t"
                   << "binopimm.v v" << vd << ", v" << vs << ", " << std::hex << "0x" << imm;

        OpT v = this->GetFrame()->GetVReg(vs).template GetAs<OpT>();

        if (IS_DIV && UNLIKELY(imm == 0)) {
            RuntimeIfaceT::ThrowArithmeticException();
            this->MoveToExceptionHandler();
        } else {
            this->GetFrameHandler().GetVReg(vd).SetPrimitive(Op<OpT>()(v, imm));
            this->template MoveToNextInst<FORMAT, IS_DIV>();
        }
    }

    template <BytecodeInstruction::Format FORMAT, typename OpT, template <typename> class Op, bool IS_DIV = false>
    ALWAYS_INLINE void HandleBinaryOp2()
    {
        OpT v1 = this->GetAcc().template GetAs<OpT>();
        uint16_t vs1 = this->GetInst().template GetVReg<FORMAT>();

        LOG_INST() << "\t"
                   << "binop2 v" << vs1;

        OpT v2 = this->GetFrame()->GetVReg(vs1).template GetAs<OpT>();

        if (IS_DIV && UNLIKELY(v2 == 0)) {
            RuntimeIfaceT::ThrowArithmeticException();
            this->MoveToExceptionHandler();
        } else {
            this->GetAcc().Set(Op<OpT>()(v1, v2));
            this->template MoveToNextInst<FORMAT, IS_DIV>();
        }
    }

    template <BytecodeInstruction::Format FORMAT, typename OpT, template <typename> class Op, bool IS_DIV = false>
    ALWAYS_INLINE void HandleBinaryOp2V()
    {
        OpT v1 = this->GetAcc().template GetAs<OpT>();
        uint16_t vd = this->GetInst().template GetVReg<FORMAT, 0x0>();
        uint16_t vs = this->GetInst().template GetVReg<FORMAT, 0x1>();

        LOG_INST() << "\t"
                   << "binop2v v" << vd << ", v" << vs;

        OpT v2 = this->GetFrame()->GetVReg(vs).template GetAs<OpT>();

        if (IS_DIV && UNLIKELY(v2 == 0)) {
            RuntimeIfaceT::ThrowArithmeticException();
            this->MoveToExceptionHandler();
        } else {
            if constexpr (std::is_floating_point<OpT>::value) {
                this->GetFrameHandler().GetVReg(vd).SetPrimitive(Op<OpT>()(v1, v2));
            } else {
                this->GetFrameHandler().GetVReg(vd).SetPrimitive(
                    static_cast<std::make_signed_t<OpT>>(Op<OpT>()(v1, v2)));
            }
            this->template MoveToNextInst<FORMAT, IS_DIV>();
        }
    }

    template <BytecodeInstruction::Format FORMAT, typename OpT, template <typename> class Op, bool IS_DIV = false>
    ALWAYS_INLINE void HandleBinaryOp()
    {
        uint16_t vs1 = this->GetInst().template GetVReg<FORMAT, 0>();
        uint16_t vs2 = this->GetInst().template GetVReg<FORMAT, 1>();

        LOG_INST() << "\t"
                   << "binop2 v" << vs1 << ", v" << vs2;

        OpT v1 = this->GetFrame()->GetVReg(vs1).template GetAs<OpT>();
        OpT v2 = this->GetFrame()->GetVReg(vs2).template GetAs<OpT>();

        if (IS_DIV && UNLIKELY(v2 == 0)) {
            RuntimeIfaceT::ThrowArithmeticException();
            this->MoveToExceptionHandler();
        } else {
            this->GetAccAsVReg().SetPrimitive(Op<OpT>()(v1, v2));
            this->template MoveToNextInst<FORMAT, IS_DIV>();
        }
    }

    template <BytecodeInstruction::Format FORMAT, typename OpT, template <typename> class Op, bool IS_DIV = false>
    ALWAYS_INLINE void HandleBinaryOpV()
    {
        uint16_t vs1 = this->GetInst().template GetVReg<FORMAT, 0>();
        uint16_t vs2 = this->GetInst().template GetVReg<FORMAT, 1>();

        LOG_INST() << "\t"
                   << "binop.v v" << vs1 << ", v" << vs2;

        OpT v1 = this->GetFrame()->GetVReg(vs1).template GetAs<OpT>();
        OpT v2 = this->GetFrame()->GetVReg(vs2).template GetAs<OpT>();

        if (IS_DIV && UNLIKELY(v2 == 0)) {
            RuntimeIfaceT::ThrowArithmeticException();
            this->MoveToExceptionHandler();
        } else {
            this->GetFrameHandler().GetVReg(vs1).SetPrimitive(Op<OpT>()(v1, v2));
            this->template MoveToNextInst<FORMAT, IS_DIV>();
        }
    }

    template <BytecodeInstruction::Format FORMAT, typename OpT, template <typename> class Op>
    ALWAYS_INLINE void HandleUnaryOp()
    {
        OpT v = this->GetAcc().template GetAs<OpT>();
        this->GetAcc().Set(Op<OpT>()(v));
        this->template MoveToNextInst<FORMAT, false>();
    }

    template <BytecodeInstruction::Format FORMAT, typename From, typename To>
    ALWAYS_INLINE void HandleConversion()
    {
        this->GetAcc().Set(static_cast<To>(this->GetAcc().template GetAs<From>()));
        this->template MoveToNextInst<FORMAT, false>();
    }

    template <BytecodeInstruction::Format FORMAT, typename From, typename To>
    ALWAYS_INLINE void HandleFloatToIntConversion()
    {
        auto value = this->GetAcc().template GetAs<From>();
        this->GetAcc().Set(CastFloatToInt<From, To>(value));
        this->template MoveToNextInst<FORMAT, false>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void InitializeObject(Class *klass, Method *method)
    {
        if (UNLIKELY(method == nullptr)) {
            this->MoveToExceptionHandler();
            return;
        }

        auto *obj = RuntimeIfaceT::CreateObject(this->GetThread(), klass);
        if (UNLIKELY(obj == nullptr)) {
            this->MoveToExceptionHandler();
            return;
        }

        this->GetAccAsVReg().SetReference(obj);
        this->GetFrame()->GetAcc() = this->GetAcc();

        constexpr bool IS_RANGE = FORMAT == BytecodeInstruction::Format::V8_ID16;
        HandleCall<FrameHelperDefault, FORMAT, false, IS_RANGE, false, true>(method);
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void InitializeObject(BytecodeId &methodId)
    {
        Class *klass;
        auto cache = this->GetThread()->GetInterpreterCache();
        auto *method = cache->template Get<Method>(this->GetInst().GetAddress(), this->GetFrame()->GetMethod());
        if (method != nullptr) {
            klass = method->GetClass();
        } else {
            klass = RuntimeIfaceT::GetMethodClass(this->GetFrame()->GetMethod(), methodId);
            this->GetAccAsVReg().SetPrimitive(0);
            if (UNLIKELY(klass == nullptr)) {
                this->MoveToExceptionHandler();
                return;
            }
        }

        if (UNLIKELY(klass->IsArrayClass())) {
            DimIterator<FORMAT> dimIter {this->GetInst(), this->GetFrame()};
            auto nargs = RuntimeIfaceT::GetMethodArgumentsCount(this->GetFrame()->GetMethod(), methodId);
            auto obj = coretypes::Array::CreateMultiDimensionalArray<DimIterator<FORMAT>>(this->GetThread(), klass,
                                                                                          nargs, dimIter);
            if (LIKELY(obj != nullptr)) {
                this->GetAccAsVReg().SetReference(obj);
                this->template MoveToNextInst<FORMAT, false>();
            } else {
                this->MoveToExceptionHandler();
            }
            return;
        }

        if (UNLIKELY(method == nullptr)) {
            method = ResolveMethod(methodId);
        }
        if (UNLIKELY(klass->IsStringClass())) {
            if (UNLIKELY(method == nullptr)) {
                this->MoveToExceptionHandler();
                return;
            }

            // StringClass constructor extensions for InitObj instruction are considered as legacy and should not be
            // extended
            uint16_t objVreg = this->GetInst().template GetVReg<FORMAT>();
            ObjectHeader *ctorArg = this->GetFrame()->GetVReg(objVreg).template GetAs<ObjectHeader *>();

            auto str = this->GetThread()->GetVM()->CreateString(method, ctorArg);
            if (LIKELY(str != nullptr)) {
                this->GetAccAsVReg().SetReference(str);
                this->template MoveToNextInst<FORMAT, false>();
            } else {
                this->MoveToExceptionHandler();
            }
        } else {
            this->UpdateBytecodeOffset();
            InitializeObject<FORMAT>(klass, method);
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleAnyLdbyname()
    {
        uint16_t vs = this->GetInst().template GetVReg<FORMAT>();
        auto id = this->GetInst().template GetId<FORMAT>();

        LOG_INST() << "any.ldbyname v" << vs << ", " << std::hex << "0x" << id;

        ObjectHeader *obj = this->GetFrame()->GetVReg(vs).GetReference();
        auto ret = intrinsics::AnyLdbyname(this->GetThread(), this->GetFrame(), obj, id.AsRawValue());
        if (UNLIKELY(this->GetThread()->HasPendingException())) {
            this->MoveToExceptionHandler();
        } else {
            this->GetAccAsVReg().SetReference(ret);
            this->template MoveToNextInst<FORMAT, true>();
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleAnyLdbynameV()
    {
        uint16_t vd = this->GetInst().template GetVReg<FORMAT, 0x0>();
        uint16_t vs = this->GetInst().template GetVReg<FORMAT, 0x1>();
        auto id = this->GetInst().template GetId<FORMAT>();

        LOG_INST() << "any.ldbyname.v v" << vd << ", v" << vs << ", " << std::hex << "0x" << id;

        ObjectHeader *obj = this->GetFrame()->GetVReg(vs).GetReference();
        auto ret = intrinsics::AnyLdbyname(this->GetThread(), this->GetFrame(), obj, id.AsRawValue());
        if (UNLIKELY(this->GetThread()->HasPendingException())) {
            this->MoveToExceptionHandler();
        } else {
            this->GetFrameHandler().GetVReg(vd).SetReference(ret);
            this->template MoveToNextInst<FORMAT, true>();
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleAnyStbyname()
    {
        uint16_t vs = this->GetInst().template GetVReg<FORMAT>();
        auto id = this->GetInst().template GetId<FORMAT>();

        LOG_INST() << "any.stbyname v" << vs << ", " << std::hex << "0x" << id;

        ObjectHeader *obj = this->GetFrame()->GetVReg(vs).GetReference();
        ObjectHeader *val = this->GetAccAsVReg().GetReference();
        intrinsics::AnyStbyname(this->GetThread(), this->GetFrame(), obj, id.AsRawValue(), val);
        if (UNLIKELY(this->GetThread()->HasPendingException())) {
            this->MoveToExceptionHandler();
        } else {
            this->template MoveToNextInst<FORMAT, true>();
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleAnyStbynameV()
    {
        uint16_t vs1 = this->GetInst().template GetVReg<FORMAT, 0x0>();
        uint16_t vs2 = this->GetInst().template GetVReg<FORMAT, 0x1>();
        auto id = this->GetInst().template GetId<FORMAT>();

        LOG_INST() << "any.stbyname.v v" << vs1 << ", v" << vs2 << ", " << std::hex << "0x" << id;

        ObjectHeader *obj = this->GetFrame()->GetVReg(vs2).GetReference();
        ObjectHeader *val = this->GetFrame()->GetVReg(vs1).GetReference();
        intrinsics::AnyStbyname(this->GetThread(), this->GetFrame(), obj, id.AsRawValue(), val);
        if (UNLIKELY(this->GetThread()->HasPendingException())) {
            this->MoveToExceptionHandler();
        } else {
            this->template MoveToNextInst<FORMAT, true>();
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleAnyLdbyidx()
    {
        uint16_t vs = this->GetInst().template GetVReg<FORMAT>();

        LOG_INST() << "any.ldbyidx v" << vs << ", " << std::hex;

        ObjectHeader *obj = this->GetFrame()->GetVReg(vs).GetReference();
        ASSERT(!this->GetAccAsVReg().HasObject());

        double index = this->GetAccAsVReg().GetDouble();
        auto ldObj = intrinsics::AnyLdbyidx(this->GetThread(), obj, index);
        this->GetAccAsVReg().SetReference(ldObj);
        if (UNLIKELY(this->GetThread()->HasPendingException())) {
            this->MoveToExceptionHandler();
        } else {
            this->template MoveToNextInst<FORMAT, true>();
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleAnyStbyidx()
    {
        uint16_t vs1 = this->GetInst().template GetVReg<FORMAT, 0x0>();
        uint16_t vs2 = this->GetInst().template GetVReg<FORMAT, 0x1>();

        LOG_INST() << "any.stbyidx v" << vs1 << ", " << vs2 << std::hex;

        ObjectHeader *obj = this->GetFrame()->GetVReg(vs1).GetReference();
        [[maybe_unused]] double index = this->GetFrame()->GetVReg(vs2).GetDouble();
        ObjectHeader *val = this->GetAccAsVReg().GetReference();
        intrinsics::AnyStbyidx(this->GetThread(), obj, index, val);
        if (UNLIKELY(this->GetThread()->HasPendingException())) {
            this->MoveToExceptionHandler();
        } else {
            this->template MoveToNextInst<FORMAT, true>();
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleAnyLdbyval()
    {
        uint16_t vs1 = this->GetInst().template GetVReg<FORMAT, 0x0>();
        uint16_t vs2 = this->GetInst().template GetVReg<FORMAT, 0x1>();

        LOG_INST() << "any.ldbyval v" << vs1 << ", v" << vs2 << ", " << std::hex;
        ObjectHeader *obj = this->GetFrame()->GetVReg(vs1).GetReference();
        ObjectHeader *valObj = this->GetFrame()->GetVReg(vs2).GetReference();
        auto ldObj = intrinsics::AnyLdbyval(this->GetThread(), obj, valObj);
        if (UNLIKELY(this->GetThread()->HasPendingException())) {
            this->MoveToExceptionHandler();
        } else {
            this->GetAccAsVReg().SetReference(ldObj);
            this->template MoveToNextInst<FORMAT, true>();
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleAnyStbyval()
    {
        uint16_t vs1 = this->GetInst().template GetVReg<FORMAT, 0x0>();
        uint16_t vs2 = this->GetInst().template GetVReg<FORMAT, 0x1>();

        LOG_INST() << "any.stbyval v" << vs1 << ", v" << vs2 << ", " << std::hex;

        ObjectHeader *obj = this->GetFrame()->GetVReg(vs1).GetReference();
        ObjectHeader *key = this->GetFrame()->GetVReg(vs2).GetReference();
        ObjectHeader *val = this->GetAccAsVReg().GetReference();
        intrinsics::AnyStbyval(this->GetThread(), obj, key, val);
        if (UNLIKELY(this->GetThread()->HasPendingException())) {
            this->MoveToExceptionHandler();
        } else {
            this->template MoveToNextInst<FORMAT, true>();
        }
    }

private:
    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE ObjectHeader *GetObjHelper()
    {
        uint16_t objVreg = this->GetInst().template GetVReg<FORMAT, 0>();
        return this->GetFrame()->GetVReg(objVreg).GetReference();
    }

    template <BytecodeInstruction::Format FORMAT, bool ACCEPT_ACC = false>
    ALWAYS_INLINE ObjectHeader *GetCallerObject()
    {
        ObjectHeader *obj = nullptr;
        if constexpr (ACCEPT_ACC) {
            if (this->GetInst().template GetImm<FORMAT, 0>() == 0) {
                obj = this->GetAcc().GetReference();
            } else {
                obj = GetObjHelper<FORMAT>();
            }
        } else {
            obj = GetObjHelper<FORMAT>();
        }

        if (UNLIKELY(obj == nullptr)) {
            RuntimeIfaceT::ThrowNullPointerException();
            this->MoveToExceptionHandler();
            return nullptr;
        }
        return obj;
    }
};

extern "C" void ExecuteImplStub(ManagedThread *thread, const uint8_t *pc, Frame *frame, bool jumpToEh, void *impl);

template <class RuntimeIfaceT, bool IS_DYNAMIC, bool IS_PROFILE_ENABLED>
void ExecuteImplInner(ManagedThread *thread, const uint8_t *pc, Frame *frame, bool jumpToEh)
{
    void *impl = reinterpret_cast<void *>(&ExecuteImpl<RuntimeIfaceT, IS_DYNAMIC, IS_PROFILE_ENABLED>);
    ExecuteImplStub(thread, pc, frame, jumpToEh, impl);
}

}  // namespace ark::interpreter

#endif  // PANDA_INTERPRETER_INL_H_
