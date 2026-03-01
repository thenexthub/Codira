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
#ifndef PANDA_INTERPRETER_FRAME_H_
#define PANDA_INTERPRETER_FRAME_H_

#include <cstddef>
#include <cstdint>

#include "libarkbase/macros.h"
#include "libarkbase/utils/bit_helpers.h"
#include "libarkbase/utils/bit_utils.h"
#include "libarkbase/utils/logger.h"
#include "runtime/interpreter/acc_vregister.h"
#include "runtime/mem/frame_allocator-inl.h"
#include "libarkfile/bytecode_instruction-inl.h"

namespace ark {

// ========== Compatible Frame Layout ==========
// Now we have a variable vregisters list, it's compatible with static language(Java etc.) and dynamic language(JS etc.)
// Frame layout - Static Languages
// +---------------------------------+
// |          ark::Frame           |
// +---------------------------------+ <-------- payload
// |           vregs[0]: v_          |
// +---------------------------------+
// |              ...                |
// +---------------------------------+
// |         vregs[nregs-1] : v_     |
// +---------------------------------+ <-------- mirror
// |         vregs[0]: v_            |
// +---------------------------------+
// |               ...               |
// +---------------------------------+
// |         vregs[nregs-1] : v_     |
// +---------------------------------+
//
// Frame layout - Dynamic Languages
// +---------------------------------+
// |          ark::Frame           |
// +---------------------------------+ <-------- payload
// |           vregs[0]: v_          |
// +---------------------------------+
// |              ...                |
// +---------------------------------+
// |         vregs[nregs-1] : v_     |
// +---------------------------------+
//
// Vregister has no type info now, called tagless vregister. For dynamic Language, vregister's value is Tagged with type
// info, which does not need the mirror part to store extra tag. For static Language, we alloc a mirror part for every
// vregister to retain the tag info. And the mirror part offset will be `nregs_ * sizeof(Vregister)`.
// You can use the `FrameHandler` to access the vregisters list and GetVReg will return `VRegisterRef`
// AccVRegister is different from VRegister, it contains both value and tag like before.

class Method;
class ObjectHeader;
template <class ExtData>
class ExtFrame;

class Frame {
public:
    // Instrumentation: indicate what the frame must be force poped
    static constexpr size_t FORCE_POP = 1U;
    // Instrumentation: indicate what the frame must retry last instruction
    static constexpr size_t RETRY_INSTRUCTION = 2U;
    // Instrumentation: indicate what the frame must notify when poped
    static constexpr size_t NOTIFY_POP = 4U;
    // Indicate that the frame was created after deoptimization.
    // This flag is needed to avoid OSR for deoptimized frames. Because the OSR consumes stack that isn't released after
    // deoptimization, stack overflow can be occurred. This constrain may be removed once asm interpreter is introduced.
    static constexpr size_t IS_DEOPTIMIZED = 8U;
    // Indicate whether this frame is stackless frame, only take effects under stackless interpreter mode.
    static constexpr size_t IS_STACKLESS = 16U;
    // Indicate whether this frame is initobj frame, only take effects under stackless interpreter mode.
    static constexpr size_t IS_INITOBJ = 32U;

    // Indicate whether this frame is created in Method::Invoke
    static constexpr size_t IS_INVOKE = 64U;

    // Indicate whether this frame is static or dynamic, which decides the frame layout
    static constexpr size_t IS_DYNAMIC = 128U;

    ALWAYS_INLINE inline Frame(void *ext, Method *method, Frame *prev, uint32_t nregs)
        : prev_(prev),
          method_(method),
          nregs_(nregs),
          numActualArgs_(0),
          bcOffset_(0),
          flags_(0),
          ext_(ext),
          nextInst_(nullptr),
          inst_(nullptr)
    {
    }
    ALWAYS_INLINE inline Frame(void *ext, Method *method, Frame *prev, uint32_t nregs, uint32_t numActualArgs)
        : prev_(prev),
          method_(method),
          nregs_(nregs),
          numActualArgs_(numActualArgs),
          bcOffset_(0),
          flags_(0),
          ext_(ext),
          nextInst_(nullptr),
          inst_(nullptr)
    {
    }

    ALWAYS_INLINE static void *ToExt(Frame *frame, size_t extSz)
    {
        return reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(frame) - extSz);
    }

    ALWAYS_INLINE static Frame *FromExt(void *ext, size_t extSz)
    {
        return reinterpret_cast<Frame *>(reinterpret_cast<uintptr_t>(ext) + extSz);
    }

    ALWAYS_INLINE inline void *GetExt() const
    {
        return ext_;
    }

    ALWAYS_INLINE inline const interpreter::VRegister &GetVReg(size_t i) const
    {
        return vregs_[i];
    }

    ALWAYS_INLINE inline interpreter::VRegister &GetVReg(size_t i)
    {
        return vregs_[i];
    }

    ALWAYS_INLINE inline void SetAcc(const interpreter::AccVRegister &acc)
    {
        acc_ = acc;
    }

    ALWAYS_INLINE inline interpreter::AccVRegister &GetAcc()
    {
        return acc_;
    }

    ALWAYS_INLINE inline const interpreter::AccVRegister &GetAcc() const
    {
        return acc_;
    }

    ALWAYS_INLINE inline void SetMethod(Method *method)
    {
        method_ = method;
    }

    ALWAYS_INLINE inline Method *GetMethod() const
    {
        return method_;
    }

    ALWAYS_INLINE const uint8_t *GetInstrOffset();

    ALWAYS_INLINE inline void SetPrevFrame(Frame *prev)
    {
        prev_ = prev;
    }

    ALWAYS_INLINE inline Frame *GetPrevFrame() const
    {
        return prev_;
    }

    ALWAYS_INLINE inline uint32_t GetSize() const
    {
        return nregs_;
    }

    ALWAYS_INLINE inline uint32_t GetNumActualArgs() const
    {
        return numActualArgs_;
    }

    ALWAYS_INLINE inline void SetBytecodeOffset(uint32_t bcOffset)
    {
        bcOffset_ = bcOffset;
    }

    ALWAYS_INLINE inline uint32_t GetBytecodeOffset() const
    {
        return bcOffset_;
    }

    ALWAYS_INLINE inline void SetNextInstruction(BytecodeInstruction inst)
    {
        nextInst_ = inst;
    }

    ALWAYS_INLINE inline BytecodeInstruction GetNextInstruction() const
    {
        return nextInst_;
    }

    ALWAYS_INLINE inline void SetInstruction(const uint8_t *inst)
    {
        inst_ = inst;
    }

    ALWAYS_INLINE inline const uint8_t *GetInstruction() const
    {
        return inst_;
    }

    ALWAYS_INLINE static inline size_t GetAllocSize(size_t size, uint32_t extSz)
    {
        return AlignUp(sizeof(Frame) + sizeof(interpreter::VRegister) * size + extSz,
                       GetAlignmentInBytes(DEFAULT_FRAME_ALIGNMENT));
    }

    ALWAYS_INLINE inline bool IsForcePop() const
    {
        return (flags_ & FORCE_POP) != 0;
    }

    ALWAYS_INLINE inline void ClearForcePop()
    {
        flags_ = flags_ & ~FORCE_POP;
    }

    ALWAYS_INLINE inline void SetForcePop()
    {
        flags_ = flags_ | FORCE_POP;
    }

    ALWAYS_INLINE inline bool IsRetryInstruction() const
    {
        return (flags_ & RETRY_INSTRUCTION) != 0;
    }

    ALWAYS_INLINE inline void ClearRetryInstruction()
    {
        flags_ = flags_ & ~RETRY_INSTRUCTION;
    }

    ALWAYS_INLINE inline void SetRetryInstruction()
    {
        flags_ = flags_ | RETRY_INSTRUCTION;
    }

    ALWAYS_INLINE inline bool IsNotifyPop() const
    {
        return (flags_ & NOTIFY_POP) != 0;
    }

    ALWAYS_INLINE inline void ClearNotifyPop()
    {
        flags_ = flags_ & ~NOTIFY_POP;
    }

    ALWAYS_INLINE inline void SetNotifyPop()
    {
        flags_ = flags_ | NOTIFY_POP;
    }

    ALWAYS_INLINE inline bool IsDeoptimized() const
    {
        return (flags_ & IS_DEOPTIMIZED) != 0;
    }

    ALWAYS_INLINE inline void SetDeoptimized()
    {
        flags_ |= IS_DEOPTIMIZED;
    }

    ALWAYS_INLINE inline void DisableOsr()
    {
        SetDeoptimized();
    }

    ALWAYS_INLINE inline bool IsStackless() const
    {
        return (flags_ & IS_STACKLESS) != 0;
    }

    ALWAYS_INLINE inline void SetStackless()
    {
        flags_ = flags_ | IS_STACKLESS;
    }

    ALWAYS_INLINE inline bool IsInitobj() const
    {
        return (flags_ & IS_INITOBJ) != 0;
    }

    ALWAYS_INLINE inline void SetInitobj()
    {
        flags_ = flags_ | IS_INITOBJ;
    }

    ALWAYS_INLINE inline void SetInvoke()
    {
        flags_ = flags_ | IS_INVOKE;
    }

    ALWAYS_INLINE inline bool IsInvoke() const
    {
        return (flags_ & IS_INVOKE) != 0;
    }

    ALWAYS_INLINE inline bool IsDynamic() const
    {
        return (flags_ & IS_DYNAMIC) != 0;
    }

    ALWAYS_INLINE inline void SetDynamic()
    {
        flags_ = flags_ | IS_DYNAMIC;
    }

    template <bool IS_DYNAMIC = false>
    ALWAYS_INLINE inline typename std::enable_if<IS_DYNAMIC, interpreter::DynamicVRegisterRef>::type GetAccAsVReg()
    {
        return GetAcc().template AsVRegRef<true>();
    }

    template <bool IS_DYNAMIC = false>
    ALWAYS_INLINE inline typename std::enable_if<!IS_DYNAMIC, interpreter::StaticVRegisterRef>::type GetAccAsVReg()
    {
        return GetAcc().template AsVRegRef<false>();
    }

    template <bool IS_DYNAMIC = false>
    ALWAYS_INLINE static inline uint32_t GetActualSize(uint32_t nregs)
    {
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (IS_DYNAMIC) {
            return nregs;
        }
        return nregs * 2U;
    }

    ALWAYS_INLINE static inline constexpr uint32_t GetMethodOffset()
    {
        return MEMBER_OFFSET(Frame, method_);
    }

    ALWAYS_INLINE static inline constexpr uint32_t GetPrevFrameOffset()
    {
        return MEMBER_OFFSET(Frame, prev_);
    }

    ALWAYS_INLINE static inline constexpr uint32_t GetNumVregsOffset()
    {
        return MEMBER_OFFSET(Frame, nregs_);
    }

    ALWAYS_INLINE static inline constexpr uint32_t GetVregsOffset()
    {
        return MEMBER_OFFSET(Frame, vregs_);
    }

    ALWAYS_INLINE static inline constexpr uint32_t GetAccOffset()
    {
        return MEMBER_OFFSET(Frame, acc_);
    }

    ALWAYS_INLINE static inline constexpr uint32_t GetNumActualArgsOffset()
    {
        return MEMBER_OFFSET(Frame, numActualArgs_);
    }

    ALWAYS_INLINE static inline constexpr uint32_t GetFlagsOffset()
    {
        return MEMBER_OFFSET(Frame, flags_);
    }

    ALWAYS_INLINE static inline constexpr uint32_t GetNextInstructionOffset()
    {
        return MEMBER_OFFSET(Frame, nextInst_);
    }

    ALWAYS_INLINE static inline constexpr uint32_t GetInstructionsOffset()
    {
        return MEMBER_OFFSET(Frame, inst_);
    }

    ALWAYS_INLINE static constexpr uint32_t GetBytecodeOffsetOffset()
    {
        return MEMBER_OFFSET(Frame, bcOffset_);
    }

    ALWAYS_INLINE static constexpr uint32_t GetExtOffset()
    {
        return MEMBER_OFFSET(Frame, ext_);
    }

    ~Frame() = default;

    NO_COPY_SEMANTIC(Frame);
    NO_MOVE_SEMANTIC(Frame);

private:
    Frame *prev_;
    Method *method_;
    uint32_t nregs_;
    uint32_t numActualArgs_;
    uint32_t bcOffset_;
    size_t flags_;

    // ExtFrame<Ext> allocation ptr
    void *ext_;

    interpreter::AccVRegister acc_;
    BytecodeInstruction nextInst_;
    const uint8_t *inst_;

    __extension__ interpreter::VRegister vregs_[0];  // NOLINT(modernize-avoid-c-arrays)
};

class FrameHandler {
public:
    ALWAYS_INLINE inline explicit FrameHandler(Frame *frame) : frame_(frame) {}

    ALWAYS_INLINE inline void SetAcc(const interpreter::AccVRegister &acc)
    {
        frame_->SetAcc(acc);
    }

    ALWAYS_INLINE inline interpreter::AccVRegister &GetAcc()
    {
        return frame_->GetAcc();
    }

    ALWAYS_INLINE inline const interpreter::AccVRegister &GetAcc() const
    {
        return frame_->GetAcc();
    }

    ALWAYS_INLINE inline void SetMethod(Method *method)
    {
        frame_->SetMethod(method);
    }

    ALWAYS_INLINE inline Method *GetMethod() const
    {
        return frame_->GetMethod();
    }

    ALWAYS_INLINE const uint8_t *GetInstrOffset() const
    {
        return frame_->GetInstrOffset();
    }

    ALWAYS_INLINE inline void SetPrevFrame(Frame *prev)
    {
        frame_->SetPrevFrame(prev);
    }

    ALWAYS_INLINE inline Frame *GetPrevFrame() const
    {
        return frame_->GetPrevFrame();
    }

    ALWAYS_INLINE inline uint32_t GetSize() const
    {
        return frame_->GetSize();
    }

    ALWAYS_INLINE inline uint32_t GetNumActualArgs() const
    {
        return frame_->GetNumActualArgs();
    }

    ALWAYS_INLINE inline void SetBytecodeOffset(uint32_t bcOffset)
    {
        frame_->SetBytecodeOffset(bcOffset);
    }

    ALWAYS_INLINE inline uint32_t GetBytecodeOffset() const
    {
        return frame_->GetBytecodeOffset();
    }

    ALWAYS_INLINE inline void SetNextInstruction(BytecodeInstruction inst)
    {
        frame_->SetNextInstruction(inst);
    }

    ALWAYS_INLINE inline BytecodeInstruction GetNextInstruction() const
    {
        return frame_->GetNextInstruction();
    }

    ALWAYS_INLINE inline void SetInstruction(const uint8_t *inst)
    {
        frame_->SetInstruction(inst);
    }

    ALWAYS_INLINE inline const uint8_t *GetInstruction() const
    {
        return frame_->GetInstruction();
    }

    ALWAYS_INLINE inline bool IsForcePop() const
    {
        return frame_->IsForcePop();
    }

    ALWAYS_INLINE inline void ClearForcePop()
    {
        frame_->ClearForcePop();
    }

    ALWAYS_INLINE inline void SetForcePop()
    {
        frame_->SetForcePop();
    }

    ALWAYS_INLINE inline bool IsRetryInstruction() const
    {
        return frame_->IsRetryInstruction();
    }

    ALWAYS_INLINE inline void ClearRetryInstruction()
    {
        frame_->ClearRetryInstruction();
    }

    ALWAYS_INLINE inline void SetRetryInstruction()
    {
        frame_->SetRetryInstruction();
    }

    ALWAYS_INLINE inline bool IsNotifyPop() const
    {
        return frame_->IsNotifyPop();
    }

    ALWAYS_INLINE inline void ClearNotifyPop()
    {
        frame_->ClearNotifyPop();
    }

    ALWAYS_INLINE inline void SetNotifyPop()
    {
        frame_->SetNotifyPop();
    }

    ALWAYS_INLINE inline bool IsDeoptimized() const
    {
        return frame_->IsDeoptimized();
    }

    ALWAYS_INLINE inline void SetDeoptimized()
    {
        frame_->SetDeoptimized();
    }

    ALWAYS_INLINE inline void DisableOsr()
    {
        frame_->SetDeoptimized();
    }

    ALWAYS_INLINE inline bool IsStackless() const
    {
        return frame_->IsStackless();
    }

    ALWAYS_INLINE inline void SetStackless()
    {
        frame_->SetStackless();
    }

    ALWAYS_INLINE inline bool IsInitobj() const
    {
        return frame_->IsInitobj();
    }

    ALWAYS_INLINE inline void SetInitobj()
    {
        frame_->SetInitobj();
    }

    ALWAYS_INLINE inline bool IsInvoke() const
    {
        return frame_->IsInvoke();
    }

    ALWAYS_INLINE inline void SetInvoke()
    {
        frame_->SetInvoke();
    }

    ALWAYS_INLINE inline bool IsDynamic() const
    {
        return frame_->IsDynamic();
    }

    ALWAYS_INLINE inline void SetDynamic()
    {
        frame_->SetDynamic();
    }

    ALWAYS_INLINE inline Frame *operator*() const
    {
        return frame_;
    }

    ~FrameHandler() = default;

    DEFAULT_COPY_SEMANTIC(FrameHandler);
    DEFAULT_MOVE_SEMANTIC(FrameHandler);

protected:
    ALWAYS_INLINE inline interpreter::VRegister *GetVRegisters()
    {
        return reinterpret_cast<interpreter::VRegister *>(reinterpret_cast<uintptr_t>(frame_) +
                                                          Frame::GetVregsOffset());
    }

    Frame *frame_ {nullptr};  // NOLINT(misc-non-private-member-variables-in-classes)
};

class StaticFrameHandler : public FrameHandler {
public:
    ALWAYS_INLINE inline explicit StaticFrameHandler(Frame *frame) : FrameHandler(frame) {}

    ALWAYS_INLINE inline interpreter::StaticVRegisterRef GetVReg(size_t i)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return interpreter::StaticVRegisterRef(&GetVRegisters()[i], &GetMirrorVRegisters()[i]);
    }

    ALWAYS_INLINE inline interpreter::StaticVRegisterRef GetAccAsVReg()
    {
        return GetAcc().template AsVRegRef<false>();
    }

private:
    ALWAYS_INLINE inline interpreter::VRegister *GetMirrorVRegisters()
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return &GetVRegisters()[frame_->GetSize()];
    }
};

class DynamicFrameHandler : public FrameHandler {
public:
    ALWAYS_INLINE inline explicit DynamicFrameHandler(Frame *frame) : FrameHandler(frame) {}

    ALWAYS_INLINE inline interpreter::DynamicVRegisterRef GetVReg(size_t i)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return interpreter::DynamicVRegisterRef(&GetVRegisters()[i]);
    }

    ALWAYS_INLINE inline interpreter::DynamicVRegisterRef GetAccAsVReg()
    {
        return GetAcc().template AsVRegRef<true>();
    }
};

template <bool IS_DYNAMIC = false>
ALWAYS_INLINE inline typename std::enable_if<IS_DYNAMIC, DynamicFrameHandler>::type GetFrameHandler(Frame *frame)
{
    return DynamicFrameHandler(frame);
}

template <bool IS_DYNAMIC = false>
ALWAYS_INLINE inline typename std::enable_if<!IS_DYNAMIC, StaticFrameHandler>::type GetFrameHandler(Frame *frame)
{
    return StaticFrameHandler(frame);
}

// ExtFrame<ExtData> is an extended ark::Frame. It's used to hold any
// language-specific data, which is required to be associated with ark::Frame.
//
//                      Allocated ExtFrame looks like that:
//
//   ExtFrame<ExtData>*  ---->  | ExtData.field1 |  <---- ExtFrame::ExtData*
//                              | ExtData.field2 |
//                              | ExtData.field3 |
//    inserted by compiler ---- |     padding    |
//                              |  Frame.field1  |  <---- ExtFame::Frame*
//                              |  Frame.field2  |
//               VRegs VLA --|--|     vreg[0]    |
//                           |  |     vreg[1]    |
//                           |  |      ....      |
//                           |--|  vreg[nregs-1] |
//
// Generic panda interpreter operates ark::Frames, while any language extension
// may access its LangSpecData via ark::Frame *base_frame
//     ExtFrame<LangSpecData>::FromFrame(base_frame)->GetExtData()
// if it's known that this frame was properly allocated.
template <class ExtData>
class ExtFrame {
public:
    ExtFrame() = delete;
    ~ExtFrame() = delete;
    NO_COPY_SEMANTIC(ExtFrame);
    NO_MOVE_SEMANTIC(ExtFrame);

    ALWAYS_INLINE inline Frame *GetFrame()
    {
        return &frame_;
    }

    ALWAYS_INLINE inline ExtData *GetExtData()
    {
        return &extData_;
    }

    ALWAYS_INLINE static inline constexpr uint32_t GetExtSize()
    {
        return FRAME_OFFSET;
    }

    ALWAYS_INLINE static inline constexpr uint32_t GetFrameOffset()
    {
        return FRAME_OFFSET;
    }

    ALWAYS_INLINE static inline constexpr uint32_t GetExtDataOffset()
    {
        return EXT_DATA_OFFSET;
    }

    ALWAYS_INLINE static inline ExtFrame<ExtData> *FromFrame(const Frame *base)
    {
        auto *res = reinterpret_cast<ExtFrame<ExtData> *>(reinterpret_cast<uintptr_t>(base) - FRAME_OFFSET);
        ASSERT(res == base->GetExt());
        return res;
    }

private:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
    ExtData extData_;
    __extension__ Frame frame_;
#pragma GCC diagnostic pop
    static constexpr uint32_t EXT_DATA_OFFSET = MEMBER_OFFSET(ExtFrame<ExtData>, extData_);
    static constexpr uint32_t FRAME_OFFSET = MEMBER_OFFSET(ExtFrame<ExtData>, frame_);
};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
// Zero-size template parameter for ExtFrame in case where no language
// extension required
union EmptyExtFrameData {
    __extension__ int v[0];  // NOLINT(modernize-avoid-c-arrays)
};
constexpr uint32_t EMPTY_EXT_FRAME_DATA_SIZE = ExtFrame<EmptyExtFrameData>::GetExtSize();
constexpr uint32_t CORE_EXT_FRAME_DATA_SIZE = 0;
static_assert(EMPTY_EXT_FRAME_DATA_SIZE == 0, "Nonzero EMPTY_EXT_FRAME_DATA_SIZE");
#pragma GCC diagnostic pop

template <class ExtT = EmptyExtFrameData>
ALWAYS_INLINE inline Frame *CreateFrame(mem::StackFrameAllocator *stackFrameAllocator, uint32_t nregsSize,
                                        Method *method, Frame *prev, uint32_t nregs, uint32_t numActualArgs)
{
    constexpr uint32_t EXT_SIZE = ExtFrame<ExtT>::GetExtSize();

    size_t extFrameSize = Frame::GetAllocSize(nregsSize, EXT_SIZE);
    void *mem = stackFrameAllocator->Alloc(extFrameSize);
    if (UNLIKELY(mem == nullptr)) {
        return nullptr;
    }
    // CODECHECK-NOLINTNEXTLINE(CPP_RULE_ID_SMARTPOINTER_INSTEADOF_ORIGINPOINTER)
    return new (Frame::FromExt(mem, EXT_SIZE)) Frame(mem, method, prev, nregs, numActualArgs);
}

ALWAYS_INLINE inline void DestroyFrame(mem::StackFrameAllocator *stackFrameAllocator, Frame *frame)
{
    ASSERT(frame->GetExt() != nullptr);
    stackFrameAllocator->Free(frame->GetExt());
}

}  // namespace ark

#endif  // PANDA_INTERPRETER_FRAME_H_
