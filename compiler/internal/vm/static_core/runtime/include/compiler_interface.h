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
#ifndef PANDA_RUNTIME_COMPILER_INTERFACE_H_
#define PANDA_RUNTIME_COMPILER_INTERFACE_H_

#include <cstdint>
#include <atomic>

#include "libarkbase/macros.h"
#include "libarkfile/file.h"
#include "runtime/include/coretypes/tagged_value.h"
#include "runtime/interpreter/frame.h"
#include "runtime/thread_pool_queue.h"

namespace ark {

namespace coretypes {
class Array;
class TaggedValue;
}  // namespace coretypes

class PandaVM;
class CompilerTask : public TaskInterface {
    NO_COPY_SEMANTIC(CompilerTask);

public:
    explicit CompilerTask(Method *method = nullptr, bool isOsr = false, PandaVM *vm = nullptr)
        : method_(method), isOsr_(isOsr), vm_(vm)
    {
    }
    inline ~CompilerTask();
    CompilerTask(CompilerTask &&task)
    {
        method_ = task.method_;
        isOsr_ = task.isOsr_;
        vm_ = task.vm_;
        task.vm_ = nullptr;
        task.method_ = nullptr;
    }

    CompilerTask &operator=(CompilerTask &&task)
    {
        method_ = task.method_;
        isOsr_ = task.isOsr_;
        vm_ = task.vm_;
        task.vm_ = nullptr;
        task.method_ = nullptr;
        return *this;
    }

    Method *GetMethod() const
    {
        return method_;
    }
    bool IsOsr() const
    {
        return isOsr_;
    }

    bool IsEmpty() const
    {
        return method_ == nullptr;
    }

    PandaVM *GetVM() const
    {
        return vm_;
    }

private:
    Method *method_ {nullptr};
    bool isOsr_ {false};
    PandaVM *vm_ {nullptr};
};

class Method;
class CompilerInterface {
public:
    enum class ReturnReason { RET_OK = 0, RET_DEOPTIMIZATION = 1 };

    class ExecState {
    public:
        ExecState(const uint8_t *pc, Frame *frame, Method *callee, size_t numArgs, const bool *spFlag)
            : pc_(pc), frame_(frame), calleeMethod_(callee), numArgs_(numArgs), spFlag_(spFlag)
        {
        }

        const uint8_t *GetPc() const
        {
            return pc_;
        }

        void SetPc(const uint8_t *pc)
        {
            pc_ = pc;
        }

        Frame *GetFrame() const
        {
            return frame_;
        }

        void SetFrame(Frame *frame)
        {
            frame_ = frame;
        }

        size_t GetNumArgs() const
        {
            return numArgs_;
        }

        const interpreter::VRegister &GetAcc() const
        {
            return acc_;
        }

        interpreter::VRegister &GetAcc()
        {
            return acc_;
        }

        void SetAcc(const interpreter::VRegister &acc)
        {
            acc_ = acc;
        }

        interpreter::VRegister &GetArg(size_t i)
        {
            return args_[i];
        }

        void SetArg(size_t i, const interpreter::VRegister &reg)
        {
            args_[i] = reg;
        }

        const uint8_t *GetMethodInst()
        {
            return frame_->GetInstrOffset();
        }

        const bool *GetSPFlag() const
        {
            return spFlag_;
        }

        Method *GetCalleeMethod()
        {
            return calleeMethod_;
        }

        static size_t GetSize(size_t nargs)
        {
            return sizeof(ExecState) + sizeof(interpreter::VRegister) * nargs;
        }

        static constexpr uint32_t GetExecStateAccOffset()
        {
            return MEMBER_OFFSET(ExecState, acc_);
        }

        static constexpr uint32_t GetExecStateArgsOffset()
        {
            return MEMBER_OFFSET(ExecState, args_);
        }

        static constexpr uint32_t GetExecStatePcOffset()
        {
            return MEMBER_OFFSET(ExecState, pc_);
        }

        static constexpr uint32_t GetExecStateFrameOffset()
        {
            return MEMBER_OFFSET(ExecState, frame_);
        }

        static constexpr uint32_t GetExecStateSPFlagAddrOffset()
        {
            return MEMBER_OFFSET(ExecState, spFlag_);
        }

        static constexpr uint32_t GetCalleeMethodOffset()
        {
            return MEMBER_OFFSET(ExecState, calleeMethod_);
        }

    private:
        const uint8_t *pc_;
        Frame *frame_;
        Method *calleeMethod_;
        size_t numArgs_;
        const bool *spFlag_;
        interpreter::VRegister acc_;
        __extension__ interpreter::VRegister args_[0];  // NOLINT(modernize-avoid-c-arrays)
    };

    CompilerInterface() = default;

    using CompiledEntryPoint = ReturnReason (*)(ExecState *);

    virtual bool CompileMethod(Method *method, uintptr_t bytecodeOffset, bool osr, coretypes::TaggedValue func) = 0;

    virtual void JoinWorker() = 0;

    virtual void FinalizeWorker() = 0;

    virtual void PreZygoteFork() = 0;

    virtual void PostZygoteFork() = 0;

    virtual void *GetOsrCode(const Method *method) = 0;

    virtual void SetOsrCode(const Method *method, void *ptr) = 0;

    virtual void RemoveOsrCode(const Method *method) = 0;

    virtual void SetNoAsyncJit(bool v) = 0;
    virtual bool IsNoAsyncJit() = 0;

    virtual ~CompilerInterface() = default;

    NO_COPY_SEMANTIC(CompilerInterface);
    NO_MOVE_SEMANTIC(CompilerInterface);
};

}  // namespace ark

#endif  // PANDA_RUNTIME_COMPILER_INTERFACE_H_
