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

#ifndef PANDA_RUNTIME_SIGNAL_HANDLER_H
#define PANDA_RUNTIME_SIGNAL_HANDLER_H

#include <csignal>
#include <cstdint>
#include <iostream>
#include <vector>
#include <libarkbase/macros.h>
#include "runtime/include/mem/panda_containers.h"
#include "libarkbase/os/sighook.h"

namespace ark {

class Method;
class SignalHandler;

bool IsInvalidPointer(uintptr_t addr);
bool InAllocatedCodeRange(uintptr_t pc);

#ifdef PANDA_TARGET_ARM32
#define CONTEXT_PC uc_->uc_mcontext.arm_pc  // NOLINT(cppcoreguidelines-macro-usage)
#define CONTEXT_SP uc_->uc_mcontext.arm_sp  // NOLINT(cppcoreguidelines-macro-usage)
#define CONTEXT_FP uc_->uc_mcontext.arm_fp  // NOLINT(cppcoreguidelines-macro-usage)
#define CONTEXT_LR uc_->uc_mcontext.arm_lr  // NOLINT(cppcoreguidelines-macro-usage)
#elif defined(PANDA_TARGET_ARM64)
#ifdef __APPLE__
#define CONTEXT_PC uc_->uc_mcontext->__ss.__pc  // NOLINT(cppcoreguidelines-macro-usage)
#define CONTEXT_FP uc_->uc_mcontext->__ss.__fp  // NOLINT(cppcoreguidelines-macro-usage)
#define CONTEXT_LR uc_->uc_mcontext->__ss.__lr  // NOLINT(cppcoreguidelines-macro-usage)
#else
#define CONTEXT_PC uc_->uc_mcontext.pc        // NOLINT(cppcoreguidelines-macro-usage)
#define CONTEXT_FP uc_->uc_mcontext.regs[29]  // NOLINT(cppcoreguidelines-macro-usage)
#define CONTEXT_LR uc_->uc_mcontext.regs[30]  // NOLINT(cppcoreguidelines-macro-usage)
#endif
#elif defined(PANDA_TARGET_AMD64)
#ifdef __APPLE__
#define CONTEXT_PC uc_->uc_mcontext->__ss.__rip  // NOLINT(cppcoreguidelines-macro-usage)
#define CONTEXT_SP uc_->uc_mcontext->__ss.__rsp  // NOLINT(cppcoreguidelines-macro-usage)
#define CONTEXT_FP uc_->uc_mcontext->__ss.__rbp  // NOLINT(cppcoreguidelines-macro-usage)
#else
#define CONTEXT_PC uc_->uc_mcontext.gregs[REG_RIP]  // NOLINT(cppcoreguidelines-macro-usage)
#define CONTEXT_SP uc_->uc_mcontext.gregs[REG_RSP]  // NOLINT(cppcoreguidelines-macro-usage)
#define CONTEXT_FP uc_->uc_mcontext.gregs[REG_RBP]  // NOLINT(cppcoreguidelines-macro-usage)
#endif
#elif defined(PANDA_TARGET_X86)
#define CONTEXT_PC uc_->uc_mcontext.gregs[REG_EIP]  // NOLINT(cppcoreguidelines-macro-usage)
#define CONTEXT_SP uc_->uc_mcontext.gregs[REG_ESP]  // NOLINT(cppcoreguidelines-macro-usage)
#define CONTEXT_FP uc_->uc_mcontext.gregs[REG_EBP]  // NOLINT(cppcoreguidelines-macro-usage)
#endif

class SignalContext {
public:
    explicit SignalContext(void *ucontextRaw)
    {
        uc_ = reinterpret_cast<ucontext_t *>(ucontextRaw);
    }
    uintptr_t GetPC()
    {
        return CONTEXT_PC;
    }
    void SetPC(uintptr_t pc)
    {
        CONTEXT_PC = pc;
    }
    uintptr_t GetSP()
    {
#if defined(PANDA_TARGET_ARM64)
#ifdef __APPLE__
        return uc_->uc_mcontext->__ss.__sp;
#else
        auto sc = reinterpret_cast<struct sigcontext *>(&uc_->uc_mcontext);
        return sc->sp;
#endif
#else
        return CONTEXT_SP;
#endif
    }
    uintptr_t *GetFP()
    {
        return reinterpret_cast<uintptr_t *>(CONTEXT_FP);
    }
#if (defined(PANDA_TARGET_ARM64) || defined(PANDA_TARGET_ARM32))
    uintptr_t GetLR()
    {
        return CONTEXT_LR;
    }
    void SetLR(uintptr_t lr)
    {
        CONTEXT_LR = lr;
    }
#elif defined(PANDA_TARGET_AMD64)
    void SetSP(uintptr_t sp)
    {
        CONTEXT_SP = sp;
    }

#endif

private:
    ucontext_t *uc_;
};

class SignalManager {
public:
    void InitSignals();

    bool IsInitialized()
    {
        return isInit_;
    }

    bool SignalActionHandler(int sig, siginfo_t *info, void *context);
    bool InCompiledCode(const siginfo_t *siginfo, const void *context, bool checkBytecodePc) const;
    void AddHandler(SignalHandler *handler, bool oatCode);

    void RemoveHandler(SignalHandler *handler);
    void GetMethodAndReturnPcAndSp(const siginfo_t *siginfo, const void *context, const Method **outMethod,
                                   const uintptr_t *outReturnPc, const uintptr_t *outSp);

    mem::InternalAllocatorPtr GetAllocator()
    {
        return allocator_;
    }

    void DeleteHandlersArray();

    explicit SignalManager(mem::InternalAllocatorPtr allocator) : allocator_(allocator) {}
    SignalManager(SignalManager &&) = delete;
    SignalManager &operator=(SignalManager &&) = delete;
    virtual ~SignalManager() = default;

private:
    bool isInit_ {false};
    mem::InternalAllocatorPtr allocator_;
    PandaVector<SignalHandler *> compiledCodeHandler_;
    PandaVector<SignalHandler *> otherHandlers_;
    NO_COPY_SEMANTIC(SignalManager);
};

class SignalHandler {
public:
    SignalHandler() = default;

    virtual bool Action(int sig, siginfo_t *siginfo, void *context) = 0;

    SignalHandler(SignalHandler &&) = delete;
    SignalHandler &operator=(SignalHandler &&) = delete;
    virtual ~SignalHandler() = default;

private:
    NO_COPY_SEMANTIC(SignalHandler);
};

class NullPointerHandler final : public SignalHandler {
public:
    NullPointerHandler() = default;

    bool Action(int sig, siginfo_t *siginfo, void *context) override;

    NullPointerHandler(NullPointerHandler &&) = delete;
    NullPointerHandler &operator=(NullPointerHandler &&) = delete;
    ~NullPointerHandler() override;

private:
    NO_COPY_SEMANTIC(NullPointerHandler);
};

class StackOverflowHandler final : public SignalHandler {
public:
    StackOverflowHandler() = default;
    ~StackOverflowHandler() override = default;

    NO_COPY_SEMANTIC(StackOverflowHandler);
    NO_MOVE_SEMANTIC(StackOverflowHandler);

    bool Action(int sig, siginfo_t *siginfo, void *context) override;
};

class SamplingProfilerHandler final : public SignalHandler {
public:
    SamplingProfilerHandler() = default;
    ~SamplingProfilerHandler() override = default;

    NO_COPY_SEMANTIC(SamplingProfilerHandler);
    NO_MOVE_SEMANTIC(SamplingProfilerHandler);

    bool Action(int sig, siginfo_t *siginfo, void *context) override;
};

class CrashFallbackDumpHandler final : public SignalHandler {
public:
    CrashFallbackDumpHandler() = default;
    ~CrashFallbackDumpHandler() override = default;

    NO_COPY_SEMANTIC(CrashFallbackDumpHandler);
    NO_MOVE_SEMANTIC(CrashFallbackDumpHandler);

    bool Action(int sig, siginfo_t *siginfo, void *context) override;
};

class AotEscapeSignalHandler final : public SignalHandler {
public:
    AotEscapeSignalHandler() = default;
    ~AotEscapeSignalHandler() override = default;

    NO_COPY_SEMANTIC(AotEscapeSignalHandler);
    NO_MOVE_SEMANTIC(AotEscapeSignalHandler);

    bool Action(int sig, siginfo_t *siginfo, void *context) override;

    static bool HandleAction(int sig, siginfo_t *siginfo, void *context);

    static bool IsEscapeSignalFlagExists();

    static void SetEscaped(bool isEscape);

    static void SetEscapedFlagFilePath(std::string &escapeFlagPath);

private:
    static bool NotTheTargetSignal(int sig);

    static std::string escapeSignalFlagFilePath_;
    static bool isEscaped_;
    static constexpr int CRASH_SIGNAL_THREASHOLD = 3;
};

}  // namespace ark

#endif  // PANDA_RUNTIME_SIGNAL_HANDLER_H
