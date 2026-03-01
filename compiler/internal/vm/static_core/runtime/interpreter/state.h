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
#ifndef PANDA_INTERPRETER_STATE_H_
#define PANDA_INTERPRETER_STATE_H_

#include "libarkfile/bytecode_instruction-inl.h"
#include "runtime/interpreter/acc_vregister.h"
#include "runtime/interpreter/acc_vregister-inl.h"
#include "runtime/interpreter/frame.h"

#ifdef PANDA_ENABLE_GLOBAL_REGISTER_VARIABLES
#include "arch/global_regs.h"
#endif

namespace ark::interpreter {

template <class T>
class StateIface {
public:
    ALWAYS_INLINE inline explicit StateIface(Frame *frame) : acc_(frame->GetAcc()) {}

    ALWAYS_INLINE inline BytecodeInstruction GetInst() const
    {
        return static_cast<const T *>(this)->GetInst();
    }

    ALWAYS_INLINE inline void SetInst(BytecodeInstruction inst)
    {
        static_cast<T *>(this)->SetInst(inst);
    }

    ALWAYS_INLINE inline Frame *GetFrame() const
    {
        return static_cast<const T *>(this)->GetFrame();
    }

    ALWAYS_INLINE inline void SetFrame(Frame *frame)
    {
        static_cast<T *>(this)->SetFrame(frame);
    }

    ALWAYS_INLINE inline ManagedThread *GetThread() const
    {
        return static_cast<const T *>(this)->GetThread();
    }

    ALWAYS_INLINE inline void SetThread(ManagedThread *thread)
    {
        static_cast<T *>(this)->SetThread(thread);
    }

    ALWAYS_INLINE inline void SaveState()
    {
        static_cast<T *>(this)->SaveState();
    }

    ALWAYS_INLINE inline void RestoreState()
    {
        static_cast<T *>(this)->RestoreState();
    }

    ALWAYS_INLINE inline AccVRegisterT &GetAcc()
    {
        return acc_;
    }

    ALWAYS_INLINE inline const AccVRegisterT &GetAcc() const
    {
        return acc_;
    }

private:
    AccVRegisterT acc_;
};

#ifdef PANDA_ENABLE_GLOBAL_REGISTER_VARIABLES

class State : public StateIface<State> {
public:
    ALWAYS_INLINE inline State(ManagedThread *thread, const uint8_t *pc, Frame *frame, const void *const *dispatchTable)
        : StateIface(frame)
    {
        SetInst(BytecodeInstruction(pc));
        SetFrame(frame);
        SetThread(thread);
        SetDispatchTable(dispatchTable);
    }

    ALWAYS_INLINE inline void UpdateState(const uint8_t *pc, Frame *frame)
    {
        SetInst(BytecodeInstruction(pc));
        SetFrame(frame);
    }

    ALWAYS_INLINE inline BytecodeInstruction GetInst() const
    {
        return BytecodeInstruction(arch::regs::GetPc());
    }

    ALWAYS_INLINE inline void SetInst(BytecodeInstruction inst)
    {
        arch::regs::SetPc(inst.GetAddress());
    }

    ALWAYS_INLINE inline Frame *GetFrame() const
    {
        return arch::regs::GetFrame();
    }

    ALWAYS_INLINE inline void SetFrame(Frame *frame)
    {
        arch::regs::SetFrame(frame);
        arch::regs::SetMirrorFp(
            reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(frame) + Frame::GetVregsOffset() + frame->GetSize()));
    }

    ALWAYS_INLINE inline const void *const *GetDispatchTable() const
    {
        return arch::regs::GetDispatchTable();
    }

    ALWAYS_INLINE inline void SetDispatchTable(const void *const *dispatchTable)
    {
        return arch::regs::SetDispatchTable(dispatchTable);
    }

    ALWAYS_INLINE inline ManagedThread *GetThread() const
    {
        return arch::regs::GetThread();
    }

    ALWAYS_INLINE inline void SetThread(ManagedThread *thread)
    {
        arch::regs::SetThread(thread);
    }

    ALWAYS_INLINE inline void SaveState()
    {
        instSpill_ = GetInst();
        accSpill_ = GetAcc();
        fpSpill_ = arch::regs::GetFp();
        mFpSpill_ = arch::regs::GetMirrorFp();
        threadSpill_ = GetThread();
    }

    ALWAYS_INLINE inline void RestoreState()
    {
        SetInst(instSpill_);
        GetAcc() = accSpill_;
        arch::regs::SetFp(fpSpill_);
        arch::regs::SetMirrorFp(mFpSpill_);
        SetThread(threadSpill_);
    }

private:
    BytecodeInstruction instSpill_;
    void *fpSpill_ {nullptr};
    void *mFpSpill_ {nullptr};
    ManagedThread *threadSpill_ {nullptr};
    AccVRegister accSpill_;
};

#else

class State : public StateIface<State> {
public:
    ALWAYS_INLINE inline State(ManagedThread *thread, const uint8_t *pc, Frame *frame, const void *const *dispatchTable)
        : StateIface(frame)
    {
        SetInst(BytecodeInstruction(pc));
        SetFrame(frame);
        SetThread(thread);
        SetDispatchTable(dispatchTable);
    }

    ALWAYS_INLINE inline void UpdateState(const uint8_t *pc, Frame *frame)
    {
        SetInst(BytecodeInstruction(pc));
        SetFrame(frame);
    }

    ALWAYS_INLINE inline BytecodeInstruction GetInst() const
    {
        return inst_;
    }

    ALWAYS_INLINE inline void SetInst(BytecodeInstruction inst)
    {
        inst_ = inst;
    }

    ALWAYS_INLINE inline Frame *GetFrame() const
    {
        return frame_;
    }

    ALWAYS_INLINE inline void SetFrame(Frame *frame)
    {
        frame_ = frame;
    }

    ALWAYS_INLINE inline const void *const *GetDispatchTable() const
    {
        return dispatchTable_;
    }

    ALWAYS_INLINE inline ManagedThread *GetThread() const
    {
        return thread_;
    }

    ALWAYS_INLINE inline void SetThread(ManagedThread *thread)
    {
        thread_ = thread;
    }

    void SetDispatchTable(const void *const *dispatchTable)
    {
        dispatchTable_ = dispatchTable;
    }

    void SaveState() {}

    void RestoreState() {}

private:
    BytecodeInstruction inst_;
    Frame *frame_ {nullptr};
    ManagedThread *thread_ {nullptr};
    const void *const *dispatchTable_ {nullptr};
};

#endif  // PANDA_ENABLE_GLOBAL_REGISTER_VARIABLES

}  // namespace ark::interpreter

#endif  // PANDA_INTERPRETER_VREGISTER_H_
