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


#include "SignalManager.h"

#include <algorithm>

#include "Base/Log.h"
#include "Base/LogFile.h"
#include "Common/Runtime.h"
#include "Concurrency/ConcurrencyModel.h"
#include "Heap/Collector/TracingCollector.h"
#include "LoaderManager.h"
#include "Mutator/Mutator.h"
#include "Mutator/MutatorManager.h"
#include "Signal/SignalUtils.h"
#include "Inspector/CodeHeapData.h"
#include "Heap/Collector/TaskQueue.h"
#ifdef COV_SIGNALHANDLE
extern "C" void __gcov_flush(void);
#endif
namespace MapleRuntime {

void SignalManager::Init()
{
    PrepareSigStack();
    // Block some ignored signals
    BlockSignals();
#if !defined(__OHOS__) && !defined(__ANDROID__)
    // Install unexpected handler first
    InstallUnexpectedSignalHandlers();
    // Install sigsegv handler
    InstallSegvHandler();
    // Install sigusr1 handler
    InstallSIGUSR1Handlers();
#endif
}

void SignalManager::Fini()
{
    FreeSigStack();
}

void SignalManager::PrepareSigStack()
{
    constexpr int stackSizeMultiples = 4;
    signalStack.ss_sp = malloc(SIGSTKSZ * stackSizeMultiples);
    if (signalStack.ss_sp == nullptr) {
        LOG(RTLOG_FATAL, "Alloca signal stack failed.");
    }

    signalStack.ss_size = SIGSTKSZ * stackSizeMultiples;
    signalStack.ss_flags = 0;

    if (sigaltstack(&signalStack, nullptr) == -1) {
        LOG(RTLOG_FATAL, "sigaltstack failed.");
    }
}

void SignalManager::FreeSigStack()
{
    free(signalStack.ss_sp);
}

void SignalManager::BlockSignals()
{
    sigset_t set;
    CHECK_SIGNAL_CALL(sigemptyset, (&set), "sigemptyset failed in BlockSignals");
    CHECK_SIGNAL_CALL(sigaddset, (&set, SIGPIPE), "sigaddset failed in BlockSignals");
    CHECK_SIGNAL_CALL(pthread_sigmask, (SIG_BLOCK, &set, nullptr), "pthread_sigmask failed in BlockSignals");
}

static void CheckStackOverflow(const siginfo_t& info)
{
    if (Runtime::CurrentRef() != nullptr && !Runtime::Current().GetConcurrencyModel().GetStackGuardCheckFlag()) {
        return;
    }
    uintptr_t stackAddr = reinterpret_cast<uintptr_t>(CODEThreadStackAddrGet());
    uintptr_t topAddr = stackAddr - MapleRuntime::MRT_PAGE_SIZE;
    uintptr_t sigAddr = reinterpret_cast<uintptr_t>(info.si_addr);
    if (stackAddr != 0 && sigAddr >= topAddr && sigAddr < stackAddr) {
        FLOG(RTLOG_ERROR, "unhandled SIGSEGV from unmanaged stack overflow!");
    }
}

static void CheckSuspendState()
{
    ThreadLocalData* tlData = ThreadLocal::GetThreadLocalData();
    Mutator* mutator = tlData->mutator;
    if (mutator == nullptr) {
        return;
    }
    if (mutator->HasSuspensionRequest(Mutator::SuspensionType::SUSPENSION_FOR_EXIT)) {
        while (true) {
            sleep(INT_MAX);
        }
    }
}

void PrintSignalHandlerStack(int sig, const siginfo_t* info, void* context)
{
    DLOG(SIGNAL, "Unexpected signal:\n%s", PrintSignalInfo(*info).Str());
    MRT_FlushGCInfo();

    ucontext_t* ucontext = static_cast<ucontext_t*>(context);
    uintptr_t sigPc = GetPCFromUContext(*ucontext);
    uintptr_t sigFa = GetFAFromUContext(*ucontext);
    pthread_t thread = pthread_self();
    constexpr uint8_t threadNameLen = 16;
    constexpr uint32_t simpleSigStrSize = 256;
    char threadName[threadNameLen];
    pthread_getname_np(thread, threadName, threadNameLen);
    UnwindContext uwContext;
    const char* frameTypeStr;
    Mutator* mutator = Mutator::GetMutator();

    if (mutator != nullptr) {
        if (mutator->GetUnwindContext().GetUnwindContextStatus() == UnwindContextStatus::RISKY) {
            uwContext = Mutator::GetMutator()->GetUnwindContext();
            frameTypeStr = "native";
        } else if (StackManager::IsRuntimeFrame(sigPc)) {
            uwContext =
                UnwindContext(MachineFrame(reinterpret_cast<FrameAddress*>(sigFa), reinterpret_cast<uint32_t*>(sigPc)));
            frameTypeStr = "runtime";
        } else {
            if (sig == SIGABRT) {
                frameTypeStr = "runtime";
                char simpleSigStr[simpleSigStrSize];
                CHECK_IN_SIG(sprintf_s(simpleSigStr, simpleSigStrSize,
                            "Thread \"%s\" catched unhandled %s (%s) from %s frame. Please report to us.", threadName,
                            SignalManager::GetSignalName(sig), strsignal(sig), frameTypeStr) != -1);
                FLOG(RTLOG_ERROR, simpleSigStr);
                return;
            }
            uwContext =
                UnwindContext(MachineFrame(reinterpret_cast<FrameAddress*>(sigFa), reinterpret_cast<uint32_t*>(sigPc)));
            frameTypeStr = "managed";
        }
    } else {
        frameTypeStr = "native";
        char simpleSigStr[simpleSigStrSize];
        CHECK_IN_SIG(sprintf_s(simpleSigStr, simpleSigStrSize,
                     "Thread \"%s\" catched unhandled %s (%s) from %s frame. signal pc: 0x%lx", threadName,
                     SignalManager::GetSignalName(sig), strsignal(sig), frameTypeStr, sigPc) != -1);
        if (sig == SIGSEGV) {
            CHECK_IN_SIG(sprintf_s(simpleSigStr, simpleSigStrSize, "%s, addr: %p", simpleSigStr, info->si_addr) != -1);
        }
        FLOG(RTLOG_ERROR, simpleSigStr);
#if defined(ENABLE_BACKWARD_PTRAUTH_CFI)
        SigHandlerFrameinfo frameInfo(MachineFrame(reinterpret_cast<FrameAddress*>(sigFa),
            reinterpret_cast<uint32_t*>(sigPc), nullptr), FrameType::NATIVE);
#else
        SigHandlerFrameinfo frameInfo(MachineFrame(reinterpret_cast<FrameAddress*>(sigFa),
            reinterpret_cast<uint32_t*>(sigPc)), FrameType::NATIVE);
#endif
        frameInfo.PrintFrameInfo(0);
        return;
    }

    char simpleSigStr[simpleSigStrSize];
    CHECK_IN_SIG(sprintf_s(simpleSigStr, simpleSigStrSize,
                 "Thread \"%s\" catched unhandled %s (%s) from %s frame. signal pc: 0x%lx", threadName,
                 SignalManager::GetSignalName(sig), strsignal(sig), frameTypeStr, sigPc) != -1);
    if (sig == SIGSEGV) {
        CHECK_IN_SIG(sprintf_s(simpleSigStr, simpleSigStrSize, "%s, addr: %p", simpleSigStr, info->si_addr) != -1);
    }
    FLOG(RTLOG_ERROR, simpleSigStr);
    StackManager::PrintSignalStackTrace(&uwContext, sigPc, sigFa);
}

bool SignalManager::HandleUnexpectedSignal(int sig, siginfo_t* info, void* context)
{
    CheckSuspendState();
    PrintSignalHandlerStack(sig, info, context);
#ifdef COV_SIGNALHANDLE
    __gcov_flush();
#endif

    return false;
}

void SignalManager::InstallUnexpectedSignalHandlers()
{
    sigset_t mask;
    CHECK_SIGNAL_CALL(sigemptyset, (&mask), "sigemptyset failed");
    SignalAction sa;
    sa.saSignalAction= HandleUnexpectedSignal;
    sa.scMask = mask;
    sa.scFlags = SA_SIGINFO | SA_ONSTACK;

    AddHandlerToSignalStack(SIGABRT, &sa);
#ifdef __APPLE__
    AddHandlerToSignalStack(SIGSEGV, &sa);
#else
    AddHandlerToSignalStack(SIGBUS, &sa);
#endif
    AddHandlerToSignalStack(SIGILL, &sa);
    AddHandlerToSignalStack(SIGFPE, &sa);
}

void SignalManager::InstallSIGUSR1Handlers() const
{
    sigset_t mask;
    CHECK_SIGNAL_CALL(sigemptyset, (&mask), "sigemptyset failed");
    SignalAction sa;
    sa.saSignalAction= HandleUnexpectedSIGUSR1;
    sa.scMask = mask;
    sa.scFlags = SA_SIGINFO | SA_ONSTACK;
    AddHandlerToSignalStack(SIGUSR1, &sa);
}

bool SignalManager::HandleUnexpectedSIGUSR1(int sig, siginfo_t* info, void* context)
{
    Heap::GetHeap().GetCollectorResources().RequestHeapDump(GCTask::TaskType::TASK_TYPE_DUMP_HEAP);
    return true;
}

// Handle unexpected SIGSEGV
bool SignalManager::HandleUnexpectedSigsegv(int sig, siginfo_t* info, void* context)
{
    CheckSuspendState();
    // Do more functional things here.
    CheckStackOverflow(*info);

    PrintSignalHandlerStack(sig, info, context);
    return false;
}

void SignalManager::InstallSegvHandler()
{
    sigset_t mask;
    // Allow some signals to be triggered when handling SIGSEGV
    CHECK_SIGNAL_CALL(sigfillset, (&mask), "sigfillset failed in InstallSegvHandler");
    CHECK_SIGNAL_CALL(sigdelset, (&mask, SIGABRT), "sigdelset SIGABRT failed in InstallSegvHandler");
    CHECK_SIGNAL_CALL(sigdelset, (&mask, SIGBUS), "sigdelset SIGBUS failed in InstallSegvHandler");
    CHECK_SIGNAL_CALL(sigdelset, (&mask, SIGFPE), "sigdelset SIGFPE failed in InstallSegvHandler");
    CHECK_SIGNAL_CALL(sigdelset, (&mask, SIGILL), "sigdelset SIGILL failed in InstallSegvHandler");
    CHECK_SIGNAL_CALL(sigdelset, (&mask, SIGSEGV), "sigdelset SIGSEGV failed in InstallSegvHandler");

    if (Runtime::Current().GetConcurrencyModel().GetStackGuardCheckFlag()) {
        // Alloc extra one page memory to handle stack overflow
        constexpr uint8_t minPageCount = 16;
        extraStackSize = std::max(AlignUp<uint32_t>(MINSIGSTKSZ, MapleRuntime::MRT_PAGE_SIZE),
                                  static_cast<uint32_t>(minPageCount * MapleRuntime::MRT_PAGE_SIZE));
        extraStack = PagePool::Instance().GetPage(extraStackSize);
        stack_t ss{};
        ss.ss_sp = extraStack;
        ss.ss_size = extraStackSize;
        CHECK_SIGNAL_CALL(sigaltstack, (&ss, nullptr), "sigaltstack failed in InstallSegvHandler");
    }

    CHECK_SIGNAL_CALL(sigemptyset, (&mask), "sigemptyset failed");
    SignalAction unexcept;
    unexcept.saSignalAction= HandleUnexpectedSigsegv;
    unexcept.scMask = mask;
    unexcept.scFlags = SA_RESTART | SA_SIGINFO | SA_ONSTACK;
#ifdef __APPLE__
    AddHandlerToSignalStack(SIGBUS, &unexcept);
#else
    AddHandlerToSignalStack(SIGSEGV, &unexcept);
#endif
}

void SignalManager::AddHandlerToSignalStack(int signal, SignalAction* sa)
{
    SignalStack::InitializeSignalStack();

    if (signal <= 0 || signal >= _NSIG) {
        LOG(RTLOG_FATAL, "Invalid signal %d", signal);
    }

    SignalStack::GetStacks()[signal].AddHandler(sa);
    SignalStack::GetStacks()[signal].MarkSig(signal);
}

void SignalManager::RemoveHandlerFromSignalStack(int signal, bool (*fn)(int, siginfo_t*, void*))
{
    SignalStack::InitializeSignalStack();

    if (signal <= 0 || signal >= _NSIG) {
        LOG(RTLOG_FATAL, "Invalid signal %d", signal);
    }

    SignalStack::GetStacks()[signal].RemoveHandler(fn);
}

} // namespace MapleRuntime
