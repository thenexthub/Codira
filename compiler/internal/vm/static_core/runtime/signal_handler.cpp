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

#include "signal_handler.h"
#include "libarkbase/utils/logger.h"
#include <algorithm>
#include <cstdlib>
#include <sstream>
#include "include/method.h"
#include "include/runtime.h"
#include "include/panda_vm.h"
#include <sys/ucontext.h>
#include <sys/stat.h>
#include "compiler_options.h"
#include "code_info/code_info.h"
#include "include/stack_walker.h"
#include "tooling/pt_thread_info.h"
#include "tooling/sampler/sampling_profiler.h"
#include "runtime/runtime_helpers.h"

#ifdef PANDA_TARGET_AMD64
extern "C" void StackOverflowExceptionEntrypointTrampoline();
#endif

namespace ark {

static void UseDebuggerdSignalHandler(int sig)
{
    LOG(WARNING, RUNTIME) << "panda vm can not handle sig " << sig << ", call next handler";
}

static bool CallSignalActionHandler(int sig, siginfo_t *info, void *context)
{
    return Runtime::GetCurrent()->GetSignalManager()->SignalActionHandler(sig, info, context);
}

bool SignalManager::SignalActionHandler(int sig, siginfo_t *info, void *context)
{
    ark::Logger::Sync();
    LOG(DEBUG, RUNTIME) << "Handle signal " << sig << " pc: " << std::hex << SignalContext(info).GetPC();

    if (InCompiledCode(info, context, true)) {
        for (const auto &handler : compiledCodeHandler_) {
            if (handler->Action(sig, info, context)) {
                return true;
            }
        }
    }
    for (const auto &handler : otherHandlers_) {
        if (handler->Action(sig, info, context)) {
            return true;
        }
    }

    // Use the default exception handler function.
    UseDebuggerdSignalHandler(sig);
    return false;
}

bool SignalManager::InCompiledCode([[maybe_unused]] const siginfo_t *siginfo, [[maybe_unused]] const void *context,
                                   [[maybe_unused]] bool checkBytecodePc) const
{
    // NOTE(00510180) leak judge GetMethodAndReturnPcAndSp
    return true;
}

void SignalManager::AddHandler(SignalHandler *handler, bool oatCode)
{
    if (oatCode) {
        compiledCodeHandler_.push_back(handler);
    } else {
        otherHandlers_.push_back(handler);
    }
}

void SignalManager::RemoveHandler(SignalHandler *handler)
{
    auto itOat = std::find(compiledCodeHandler_.begin(), compiledCodeHandler_.end(), handler);
    if (itOat != compiledCodeHandler_.end()) {
        compiledCodeHandler_.erase(itOat);
        return;
    }
    auto itOther = std::find(otherHandlers_.begin(), otherHandlers_.end(), handler);
    if (itOther != otherHandlers_.end()) {
        otherHandlers_.erase(itOther);
        return;
    }
    LOG(FATAL, RUNTIME) << "handler doesn't exist: " << handler;
}

void SignalManager::InitSignals()
{
    if (isInit_) {
        return;
    }

    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, SIGABRT);
    sigdelset(&mask, SIGBUS);
    sigdelset(&mask, SIGFPE);
    sigdelset(&mask, SIGILL);
    sigdelset(&mask, SIGSEGV);

    ClearSignalHooksHandlersArray();

    // if running in phone,Sigchain will work,AddSpecialSignalHandlerFn in sighook will not be used
    SigchainAction sigchainAction = {
        CallSignalActionHandler,
        mask,
        SA_SIGINFO,
    };
    AddSpecialSignalHandlerFn(SIGSEGV, &sigchainAction);
#if defined(PANDA_TARGET_OHOS)
    AddSpecialSignalHandlerFn(SIGABRT, &sigchainAction);
    AddSpecialSignalHandlerFn(SIGBUS, &sigchainAction);
    AddSpecialSignalHandlerFn(SIGFPE, &sigchainAction);
    AddSpecialSignalHandlerFn(SIGILL, &sigchainAction);
#endif
    isInit_ = true;

    for (auto tmp : compiledCodeHandler_) {
        allocator_->Delete(tmp);
    }
    compiledCodeHandler_.clear();
    for (auto tmp : otherHandlers_) {
        allocator_->Delete(tmp);
    }
    otherHandlers_.clear();
}

void SignalManager::GetMethodAndReturnPcAndSp([[maybe_unused]] const siginfo_t *siginfo,
                                              [[maybe_unused]] const void *context,
                                              [[maybe_unused]] const Method **outMethod,
                                              [[maybe_unused]] const uintptr_t *outReturnPc,
                                              [[maybe_unused]] const uintptr_t *outSp)
{
    // just stub now
}

void SignalManager::DeleteHandlersArray()
{
    if (isInit_) {
        for (auto tmp : compiledCodeHandler_) {
            allocator_->Delete(tmp);
        }
        compiledCodeHandler_.clear();
        for (auto tmp : otherHandlers_) {
            allocator_->Delete(tmp);
        }
        otherHandlers_.clear();
        RemoveSpecialSignalHandlerFn(SIGSEGV, CallSignalActionHandler);
#if defined(PANDA_TARGET_OHOS)
        RemoveSpecialSignalHandlerFn(SIGABRT, CallSignalActionHandler);
        RemoveSpecialSignalHandlerFn(SIGBUS, CallSignalActionHandler);
        RemoveSpecialSignalHandlerFn(SIGFPE, CallSignalActionHandler);
        RemoveSpecialSignalHandlerFn(SIGILL, CallSignalActionHandler);
#endif
        isInit_ = false;
    }
}

bool InAllocatedCodeRange(uintptr_t pc)
{
    Thread *thread = Thread::GetCurrent();
    if (thread == nullptr) {
        // Current thread is not attatched to any of the VMs
        return false;
    }

    if (Runtime::GetCurrent()->GetClassLinker()->GetAotManager()->InAotFileRange(pc)) {
        return true;
    }

    auto heapManager = thread->GetVM()->GetHeapManager();
    if (heapManager == nullptr) {
        return false;
    }
    auto codeAllocator = heapManager->GetCodeAllocator();
    if (codeAllocator == nullptr) {
        return false;
    }
    return codeAllocator->InAllocatedCodeRange(ToVoidPtr(pc));
}

bool IsInvalidPointer(uintptr_t addr)
{
    if (addr == 0) {
        return true;
    }

    return !IsAligned(addr, alignof(uintptr_t));
}

// This is the way to get compiled method entry point
// FP regsiter -> stack -> method -> compilerEntryPoint
static uintptr_t FindCompilerEntrypoint(const uintptr_t *fp)
{
    // Compiled code stack frame:
    // +----------------+
    // | Return address |
    // +----------------+ <- Frame pointer
    // | Frame pointer  |
    // +----------------+
    // | ark::Method* |
    // +----------------+
    const int compiledFrameMethodOffset = BoundaryFrame<FrameKind::COMPILER>::METHOD_OFFSET;

    if (IsInvalidPointer(reinterpret_cast<uintptr_t>(fp))) {
        return 0;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    uintptr_t pmethod = fp[compiledFrameMethodOffset];
    if (IsInvalidPointer(pmethod)) {
        return 0;
    }

    // it is important for GetCompiledEntryPoint method to have a lock-free implementation
    auto entrypoint = reinterpret_cast<uintptr_t>((reinterpret_cast<Method *>(pmethod))->GetCompiledEntryPoint());
    if (IsInvalidPointer(entrypoint)) {
        return 0;
    }

    if (!InAllocatedCodeRange(entrypoint)) {
        LOG(INFO, RUNTIME) << "Runtime SEGV handler: the entrypoint is not from JIT code";
        return 0;
    }

    if (!compiler::CodeInfo::VerifyCompiledEntry(entrypoint)) {
        // what we have found is not a compiled method
        return 0;
    }

    return entrypoint;
}

// Estimate if the PC belongs to FindCompilerEntrypoint
static bool PCInFindCompilerEntrypoint(uintptr_t pc)
{
    auto func = ToUintPtr(&FindCompilerEntrypoint);
    const unsigned thisMethodSizeEstimation = 0x1000;  // there is no way to find exact compiled method size
    return func < pc && pc < func + thisMethodSizeEstimation;
}

static void UpdateReturnAddress(SignalContext &signalContext, uintptr_t newAddress)
{
#if (defined(PANDA_TARGET_ARM64) || defined(PANDA_TARGET_ARM32))
    signalContext.SetLR(newAddress);
#elif defined(PANDA_TARGET_AMD64)
    auto *sp = reinterpret_cast<uintptr_t *>(signalContext.GetSP() - sizeof(uintptr_t));
    *sp = newAddress;
    signalContext.SetSP(reinterpret_cast<uintptr_t>(sp));
#endif
}

struct NullCheckEPInfo {
    uintptr_t handlerPc;
    uintptr_t newPc;
    bool isLLVM;
};

static std::optional<NullCheckEPInfo> LookupNullCheckEntrypoint(uintptr_t pc, uintptr_t entrypoint)
{
    compiler::CodeInfo codeinfo(compiler::CodeInfo::GetCodeOriginFromEntryPoint(ToVoidPtr(entrypoint)));
    if ((pc < entrypoint) || (pc > entrypoint + codeinfo.GetCodeSize())) {
        // we are not in a compiled method
        return std::nullopt;
    }

    for (auto const &icheck : codeinfo.GetImplicitNullChecksTable()) {
        uintptr_t nullCheckAddr = entrypoint + icheck.GetInstNativePc();
        auto offset = icheck.GetOffset();
        static constexpr uint32_t LLVM_HANDLER_TAG = 1U << 31U;
        if (pc == nullCheckAddr && (LLVM_HANDLER_TAG & offset) != 0) {
            // Code was compiled by LLVM.
            // We jump to the handler pc, which will call NullPointerException on its own.
            // We do not jump to NullPointerExceptionBridge directly here because LLVM code does not have stackmap on
            // memory instructions
            uintptr_t handlerPc = (~LLVM_HANDLER_TAG & offset) + entrypoint;
            return NullCheckEPInfo {handlerPc, nullCheckAddr, true};
        }
        // We insert information about implicit nullcheck after mem instruction,
        // because encoder can insert memory calculation before the instruction, and we don't know real address:
        //   addr |               |
        //    |   +---------------+  <--- nullCheckAddr - offset
        //    |   | address calc  |
        //    |   | memory inst   |  <--- pc
        //    V   +---------------+  <--- nullCheckAddr
        //        |               |
        if (pc < nullCheckAddr && pc + offset >= nullCheckAddr) {
            uintptr_t handlerPc = ToUintPtr(NullPointerExceptionBridge);
            return NullCheckEPInfo {handlerPc, nullCheckAddr, false};
        }
    }

    LOG(INFO, RUNTIME) << "SEGV can't be handled. No matching entry found in the NullCheck table.\n"
                       << "PC: " << std::hex << pc;
    for (auto const &icheck : codeinfo.GetImplicitNullChecksTable()) {
        LOG(INFO, RUNTIME) << "nullcheck: " << std::hex << (entrypoint + icheck.GetInstNativePc());
    }
    return std::nullopt;
}

static void SamplerSigSegvHandler([[maybe_unused]] int sig, [[maybe_unused]] siginfo_t *siginfo,
                                  [[maybe_unused]] void *context)
{
    auto mthread = ManagedThread::GetCurrent();
    ASSERT(mthread != nullptr);

    int numToReturn = 1;
    // NOLINTNEXTLINE(cert-err52-cpp)
    longjmp(mthread->GetPtThreadInfo()->GetSamplingInfo()->GetSigSegvJmpEnv(), numToReturn);
}

bool SamplingProfilerHandler::Action(int sig, [[maybe_unused]] siginfo_t *siginfo, void *context)
{
    if (sig != SIGSEGV) {
        return false;
    }
    auto *thread = ManagedThread::GetCurrent();
    if (thread == nullptr) {
        return false;
    }

    auto *sampler = Runtime::GetCurrent()->GetTools().GetSamplingProfiler();
    if (sampler == nullptr || !sampler->IsSegvHandlerEnable()) {
        return false;
    }

    auto *samplingInfo = thread->GetPtThreadInfo()->GetSamplingInfo();
    if (samplingInfo == nullptr || !samplingInfo->IsThreadSampling()) {
        return false;
    }

    SignalContext signalContext(context);
    signalContext.SetPC(reinterpret_cast<uintptr_t>(&SamplerSigSegvHandler));
    return true;
}

static constexpr uintptr_t MAX_MANAGED_OBJECT_SIZE = 1U << 30U;

bool NullPointerHandler::Action(int sig, siginfo_t *siginfo, void *context)
{
    if (sig != SIGSEGV) {
        return false;
    }
    SignalContext signalContext(context);

    uintptr_t const pc = signalContext.GetPC();
    if (!InAllocatedCodeRange(pc) || PCInFindCompilerEntrypoint(pc)) {
        return false;
    }
    if (ToUintPtr(siginfo->si_addr) > MAX_MANAGED_OBJECT_SIZE) {  // NOLINT(cppcoreguidelines-pro-type-union-access)
        return false;
    }
    uintptr_t const entrypoint = FindCompilerEntrypoint(signalContext.GetFP());
    if (entrypoint == 0) {
        return false;
    }

    if (std::optional<NullCheckEPInfo> info = LookupNullCheckEntrypoint(pc, entrypoint); info.has_value()) {
        LOG(DEBUG, RUNTIME) << "Transition to NullCheck entrypoint, signal: " << sig << " new pc:" << std::hex
                            << info->newPc;
        if (!info->isLLVM) {
            UpdateReturnAddress(signalContext, info->newPc);
        }
        signalContext.SetPC(info->handlerPc);
        EVENT_IMPLICIT_NULLCHECK(info->newPc);
        /* NullPointer has been check in aot or here now,then return to interpreter, so exception not build here
         * issue #1437
         * ark::ThrowNullPointerException();
         */
        return true;
    }
    return false;
}

NullPointerHandler::~NullPointerHandler() = default;

bool StackOverflowHandler::Action(int sig, [[maybe_unused]] siginfo_t *siginfo, [[maybe_unused]] void *context)
{
    if (sig != SIGSEGV) {
        return false;
    }
    auto *thread = ManagedThread::GetCurrent();
    if (thread == nullptr) {
        return false;
    }

    SignalContext signalContext(context);
    auto memCheckLocation = signalContext.GetSP() - ManagedThread::GetStackOverflowCheckOffset();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
    auto memFaultLocation = ToUintPtr(siginfo->si_addr);
    if (memCheckLocation != memFaultLocation) {
        return false;
    }

    LOG(DEBUG, RUNTIME) << "Stack overflow occurred";

    // StackOverflow stackmap has zero address
    thread->SetNativePc(0);
    // Set compiler Frame in Thread
    thread->SetCurrentFrame(reinterpret_cast<Frame *>(signalContext.GetFP()));
#ifdef PANDA_TARGET_AMD64
    signalContext.SetPC(reinterpret_cast<uintptr_t>(StackOverflowExceptionEntrypointTrampoline));
#else
    /* To save/restore callee-saved regs we get into StackOverflowExceptionEntrypoint
     * by means of StackOverflowExceptionBridge.
     * The bridge stores LR to ManagedThread.npc, which is used by StackWalker::CreateCFrame,
     * and it must be 0 in case of StackOverflow.
     */
    signalContext.SetLR(0);
    signalContext.SetPC(reinterpret_cast<uintptr_t>(StackOverflowExceptionBridge));
#endif

    return true;
}

bool CrashFallbackDumpHandler::Action(int sig, [[maybe_unused]] siginfo_t *siginfo, void *context)
{
    if (sig != SIGSEGV) {
        return false;
    }
    SignalContext signalContext(context);

    auto thread = ManagedThread::GetCurrent();
    if (thread == nullptr) {
        auto vmThread = Thread::GetCurrent();
        if (vmThread == nullptr) {
            LOG(ERROR, RUNTIME) << "SIGSEGV in unknown thread";
            return false;
        }
        LOG(ERROR, RUNTIME) << "SIGSEGV in runtime thread: threadType="
                            << helpers::ToUnderlying(vmThread->GetThreadType());
        PrintStack(Logger::Message(Logger::Level::ERROR, Logger::Component::RUNTIME, false).GetStream());
        return false;
    }
    if (thread->IsInNativeCode()) {
        LOG(ERROR, RUNTIME) << "SIGSEGV in managed thread (native code)";
        return false;
    }
    if (InAllocatedCodeRange(signalContext.GetPC())) {
        LOG(ERROR, RUNTIME) << "SIGSEGV in managed thread (managed compiled code)";
    } else {
        LOG(ERROR, RUNTIME) << "SIGSEGV in managed thread (managed code)";
    }
    ark::PrintStackTrace();
    PrintStack(Logger::Message(Logger::Level::ERROR, Logger::Component::RUNTIME, false).GetStream());
    return false;
}

// NOLINTNEXTLINE
std::string AotEscapeSignalHandler::escapeSignalFlagFilePath_ = "/data/storage/ark-profile/aot_escape.txt";
bool AotEscapeSignalHandler::isEscaped_ = false;

void AotEscapeSignalHandler::SetEscaped(bool isEscape)
{
    AotEscapeSignalHandler::isEscaped_ = isEscape;
}

void AotEscapeSignalHandler::SetEscapedFlagFilePath(std::string &escapeFlagPath)
{
    escapeSignalFlagFilePath_ = escapeFlagPath;
}

bool AotEscapeSignalHandler::NotTheTargetSignal(int sig)
{
    // following are the target signal, will cause to aot escape. If the target, return false
    auto result = true;
    if (sig == SIGSEGV || sig == SIGABRT || sig == SIGBUS || sig == SIGFPE || sig == SIGILL) {
        result = false;
    }
    return result;
}

bool AotEscapeSignalHandler::Action(int sig, [[maybe_unused]] siginfo_t *siginfo, void *context)
{
    return HandleAction(sig, siginfo, context);
}

#if defined(PANDA_TARGET_OHOS) || (defined(PANDA_TARGET_AMD64))
bool AotEscapeSignalHandler::HandleAction(int sig, [[maybe_unused]] siginfo_t *siginfo, void *context)
{
    if (NotTheTargetSignal(sig)) {
        return false;
    }
    if (IsEscapeSignalFlagExists()) {
        return false;
    }
    if (!Runtime::GetCurrent()->GetClassLinker()->GetAotManager()->HasAotFiles()) {
        return false;
    }
    LOG(ERROR, RUNTIME) << "AotEscapeSignalHandler handle AOT escape type, signal = " << sig;

    std::ifstream file(escapeSignalFlagFilePath_);
    std::stringstream buffer;
    if (file.is_open()) {
        buffer << file.rdbuf();
        file.close();
    }

    uintptr_t const pc = SignalContext(context).GetPC();
    buffer << "Signal: " << sig << ", PC: " << pc << ", isInAot: ";
    auto isInAot = Runtime::GetCurrent()->GetClassLinker()->GetAotManager()->InAotFileRange(pc);
    buffer << (isInAot ? "true\n" : "false\n");

    auto content = buffer.str();
    int failedNum = -1;
    const auto filePerm = 0666;
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    const auto perm = O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    int fd = open(escapeSignalFlagFilePath_.c_str(), perm, filePerm);
    if (fd == failedNum) {
        LOG(ERROR, RUNTIME) << "AotEscapeSignalHandler handle AOT escape. Failed to open: " << strerror(errno);
        return false;
    }
    if (write(fd, content.c_str(), content.length()) == failedNum) {
        LOG(ERROR, RUNTIME) << "AotEscapeSignalHandler handle AOT escape. Failed to write: " << strerror(errno);
    }
    close(fd);
    return false;
}

bool AotEscapeSignalHandler::IsEscapeSignalFlagExists()
{
    if (isEscaped_) {
        return true;
    }
    std::ifstream file(escapeSignalFlagFilePath_);
    if (!file.is_open()) {
        return false;
    }
    int lineCount = 0;
    std::string line;
    while (std::getline(file, line)) {
        lineCount++;
        if (lineCount >= CRASH_SIGNAL_THREASHOLD) {
            isEscaped_ = true;
            break;
        }
    }
    file.close();
    return isEscaped_;
}
#else
bool AotEscapeSignalHandler::HandleAction([[maybe_unused]] int sig, [[maybe_unused]] siginfo_t *siginfo,
                                          [[maybe_unused]] void *context)
{
    return isEscaped_;
}

bool AotEscapeSignalHandler::IsEscapeSignalFlagExists()
{
    return isEscaped_;
}
#endif

}  // namespace ark
