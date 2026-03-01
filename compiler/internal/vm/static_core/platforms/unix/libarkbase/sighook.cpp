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

#include "libarkbase/utils/logger.h"
#include <dlfcn.h>

#include <array>
#include <algorithm>
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <libarkbase/os/mutex.h>
#include <type_traits>
#include <utility>
#include <unistd.h>

#include "libarkbase/os/sighook.h"

#include <securec.h>
#include <ucontext.h>
#include "libarkbase/utils/utils.h"

namespace ark {

static decltype(&sigaction) g_realSigaction;
static decltype(&sigprocmask) g_realSigProcMask;

static os::memory::PandaThreadKey GetHandlingSignalKey()
{
    static os::memory::PandaThreadKey key;
    [[maybe_unused]] static bool init = []() {
        if (int rc = os::memory::PandaThreadKeyCreate(&key, nullptr); rc != 0) {
            LOG(FATAL, RUNTIME) << "failed to create sigchain thread key: " << os::Error(rc).ToString();
        }
        return true;
    }();

    return key;
}

static bool GetHandlingSignal()
{
    return os::memory::PandaGetspecific(GetHandlingSignalKey()) != nullptr;
}

static void SetHandlingSignal(bool value)
{
    os::memory::PandaSetspecific(GetHandlingSignalKey(), reinterpret_cast<void *>(static_cast<uintptr_t>(value)));
}

class SignalHook {
public:
    SignalHook() = default;

    ~SignalHook() = default;

    NO_COPY_SEMANTIC(SignalHook);
    NO_MOVE_SEMANTIC(SignalHook);

    bool IsHook()
    {
        return isHook_;
    }

    void HookSig(int signo)
    {
        if (!isHook_) {
            RegisterAction(signo);
            isHook_ = true;
        }
    }

    void RegisterAction(int signo)
    {
        struct sigaction handlerAction = {};
        sigfillset(&handlerAction.sa_mask);
        // SIGSEGV from signal handler must be handled as well
        sigdelset(&handlerAction.sa_mask, SIGSEGV);

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        handlerAction.sa_sigaction = SignalHook::Handler;
        // SA_NODEFER+: do not block signals from the signal handler
        // SA_ONSTACK-: call signal handler on the same stack
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        handlerAction.sa_flags = SA_RESTART | SA_SIGINFO | SA_NODEFER;
        g_realSigaction(signo, nullptr, &oldAction_);
        g_realSigaction(signo, &handlerAction, &userAction_);
    }

    void RegisterHookAction(const SighookAction *sa)
    {
        for (SighookAction &handler : hookActionHandlers_) {
            if (handler.scSigaction == nullptr) {
                handler = *sa;
                return;
            }
        }
        LOG(FATAL, RUNTIME) << "failed to Register Hook Action, too much handlers";
    }

    void RegisterUserAction(const struct sigaction *newAction)
    {
        userActionRegister_ = true;
        if constexpr (std::is_same_v<decltype(userAction_), struct sigaction>) {
            userAction_ = *newAction;
        } else {
            userAction_.sa_flags = newAction->sa_flags;      // NOLINT
            userAction_.sa_handler = newAction->sa_handler;  // NOLINT
#if defined(SA_RESTORER)
            userAction_.sa_restorer = newAction->sa_restorer;  // NOLINT
#endif
            sigemptyset(&userAction_.sa_mask);
            MemcpyUnsafe(&userAction_.sa_mask, &newAction->sa_mask,
                         std::min(sizeof(userAction_.sa_mask), sizeof(newAction->sa_mask)));
        }
    }

    struct sigaction GetUserAction()
    {
        if constexpr (std::is_same_v<decltype(userAction_), struct sigaction>) {
            return userAction_;
        } else {
            struct sigaction result {};
            result.sa_flags = userAction_.sa_flags;      // NOLINT
            result.sa_handler = userAction_.sa_handler;  // NOLINT
#if defined(SA_RESTORER)
            result.sa_restorer = userAction_.sa_restorer;
#endif
            MemcpyUnsafe(&result.sa_mask, &userAction_.sa_mask,
                         std::min(sizeof(userAction_.sa_mask), sizeof(result.sa_mask)));
            return result;
        }
    }

    static void Handler(int signo, siginfo_t *siginfo, void *ucontextRaw);
    static void CallOldAction(int signo, siginfo_t *siginfo, void *ucontextRaw);

    void RemoveHookAction(bool (*action)(int, siginfo_t *, void *))
    {
        // no thread safe
        for (size_t i = 0; i < HOOK_LENGTH; ++i) {
            if (hookActionHandlers_[i].scSigaction == action) {
                for (size_t j = i; j < HOOK_LENGTH - 1; ++j) {
                    hookActionHandlers_[j] = hookActionHandlers_[j + 1];
                }
                hookActionHandlers_[HOOK_LENGTH - 1].scSigaction = nullptr;
                return;
            }
        }
        LOG(FATAL, RUNTIME) << "failed to find remove hook handler";
    }

    bool IsUserActionRegister()
    {
        return userActionRegister_;
    }

    void ClearHookActionHandlers()
    {
        for (SighookAction &handler : hookActionHandlers_) {
            handler.scSigaction = nullptr;
        }
    }

private:
    static bool SetHandlingSignal(int signo, siginfo_t *siginfo, void *ucontextRaw);

    constexpr static const int HOOK_LENGTH = 2;
    bool isHook_ {false};
    std::array<SighookAction, HOOK_LENGTH> hookActionHandlers_ {};
    struct sigaction userAction_ {};
    struct sigaction oldAction_ = {};
    bool userActionRegister_ {false};
};

static std::array<SignalHook, _NSIG + 1> g_signalHooks;

void SignalHook::CallOldAction(int signo, siginfo_t *siginfo, void *ucontextRaw)
{
    auto handlerFlags = static_cast<size_t>(g_signalHooks[signo].oldAction_.sa_flags);
    sigset_t mask = g_signalHooks[signo].oldAction_.sa_mask;
    g_realSigProcMask(SIG_SETMASK, &mask, nullptr);

    if ((handlerFlags & SA_SIGINFO)) {                                              // NOLINT
        g_signalHooks[signo].oldAction_.sa_sigaction(signo, siginfo, ucontextRaw);  // NOLINT
    } else {
        if (g_signalHooks[signo].oldAction_.sa_handler == nullptr) {  // NOLINT
            g_realSigaction(signo, &g_signalHooks[signo].oldAction_, nullptr);
            kill(getpid(), signo);  // send signal again
            return;
        }
        g_signalHooks[signo].oldAction_.sa_handler(signo);  // NOLINT
    }
}

bool SignalHook::SetHandlingSignal(int signo, siginfo_t *siginfo, void *ucontextRaw)
{
    for (const auto &handler : g_signalHooks[signo].hookActionHandlers_) {
        if (handler.scSigaction == nullptr) {
            break;
        }

        bool handlerNoreturn = ((handler.scFlags & SIGHOOK_ALLOW_NORETURN) != 0);
        sigset_t previousMask;
        g_realSigProcMask(SIG_SETMASK, &handler.scMask, &previousMask);

        bool oldHandleKey = GetHandlingSignal();
        if (!handlerNoreturn) {
            ::ark::SetHandlingSignal(true);
        }
        if (handler.scSigaction(signo, siginfo, ucontextRaw)) {
            ::ark::SetHandlingSignal(oldHandleKey);
            return false;
        }

        g_realSigProcMask(SIG_SETMASK, &previousMask, nullptr);
        ::ark::SetHandlingSignal(oldHandleKey);
    }

    return true;
}

void SignalHook::Handler(int signo, siginfo_t *siginfo, void *ucontextRaw)
{
    if (!GetHandlingSignal()) {
        if (!SetHandlingSignal(signo, siginfo, ucontextRaw)) {
            return;
        }
    }

    // if not set user handler,call linker handler
    if (!g_signalHooks[signo].IsUserActionRegister()) {
        CallOldAction(signo, siginfo, ucontextRaw);
        return;
    }

    // call user handler
    auto handlerFlags = static_cast<size_t>(g_signalHooks[signo].userAction_.sa_flags);
    auto *ucontext = static_cast<ucontext_t *>(ucontextRaw);
    sigset_t mask;
    sigemptyset(&mask);
    constexpr int N = sizeof(sigset_t) * 2;
    for (int i = 0; i < N; ++i) {
        if (sigismember(&ucontext->uc_sigmask, i) == 1 ||
            sigismember(&g_signalHooks[signo].userAction_.sa_mask, i) == 1) {
            sigaddset(&mask, i);
        }
    }

    if ((handlerFlags & SA_NODEFER) == 0) {  // NOLINT
        sigaddset(&mask, signo);
    }
    g_realSigProcMask(SIG_SETMASK, &mask, nullptr);

    if ((handlerFlags & SA_SIGINFO)) {                                               // NOLINT
        g_signalHooks[signo].userAction_.sa_sigaction(signo, siginfo, ucontextRaw);  // NOLINT
    } else {
        auto handler = g_signalHooks[signo].userAction_.sa_handler;  // NOLINT
        if (handler == SIG_IGN) {                                    // NOLINT
            return;
        }
        if (handler == SIG_DFL) {  // NOLINT
            LOG(FATAL, RUNTIME) << "Actually signal:" << signo << " | register sigaction's handler == SIG_DFL";
        }
        ASSERT(handler != nullptr);
        handler(signo);
    }

    // if user handler not exit, continue call Old Action
    CallOldAction(signo, siginfo, ucontextRaw);
}

template <typename Sigaction>
static bool FindRealSigfn(Sigaction *realFun, Sigaction hookFun, const char *name)
{
    auto found = reinterpret_cast<Sigaction>(dlsym(RTLD_NEXT, name));
    if (found == nullptr) {
        found = reinterpret_cast<Sigaction>(dlsym(RTLD_DEFAULT, name));
        if (found == nullptr || found == hookFun) {
            LOG(ERROR, RUNTIME) << "dlsym(RTLD_DEFAULT, " << name << ") can not find really " << name;
            return false;
        }
    }
    *realFun = found;
    LOG(INFO, RUNTIME) << "find " << name << " success";
    return true;
}

#if PANDA_TARGET_MACOS
__attribute__((constructor(102))) static bool InitRealSignalFun()
#else
__attribute__((constructor)) static bool InitRealSignalFun()
#endif
{
    static bool initialized = ([]() {
        bool success = true;
        success &= FindRealSigfn(&g_realSigaction, sigaction, "sigaction");
        success &= FindRealSigfn(&g_realSigProcMask, sigprocmask, "sigprocmask");
        return success;
    })();
    return initialized;
}

// NOLINTNEXTLINE(readability-identifier-naming)
static int RegisterUserHandler(int signal, const struct sigaction *newAction, struct sigaction *oldAction,
                               int (*really)(int, const struct sigaction *, struct sigaction *))
{
    // just hook signal in range, other use libc sigaction
    if (signal <= 0 || signal >= _NSIG) {
        LOG(ERROR, RUNTIME) << "illegal signal " << signal;
        return -1;
    }

    if (g_signalHooks[signal].IsHook()) {
        auto userAction = g_signalHooks[signal].SignalHook::GetUserAction();
        if (newAction != nullptr) {
            g_signalHooks[signal].RegisterUserAction(newAction);
        }
        if (oldAction != nullptr) {
            *oldAction = userAction;
        }
        return 0;
    }

    return really(signal, newAction, oldAction);
}

static int RegisterUserMask(int how, const sigset_t *newSet, sigset_t *oldSet,
                            int (*really)(int, const sigset_t *, sigset_t *))
{
    if (GetHandlingSignal()) {
        return really(how, newSet, oldSet);
    }

    if (newSet == nullptr) {
        return really(how, newSet, oldSet);
    }

    sigset_t buildSigset = *newSet;
    if (how == SIG_BLOCK || how == SIG_SETMASK) {
        for (int i = 1; i < _NSIG; ++i) {
            if (g_signalHooks[i].IsHook() && (sigismember(&buildSigset, i) != 0)) {
                sigdelset(&buildSigset, i);
            }
        }
    }
    const sigset_t *buildSigsetConst = &buildSigset;
    return really(how, buildSigsetConst, oldSet);
}

// CC-OFFNXT(G.NAM.03) libc hook
// NOLINT(readability-identifier-naming)
extern "C" int sigaction(int sig, const struct sigaction *__restrict act, struct sigaction *oact)
{
    if (!InitRealSignalFun()) {
        return -1;
    }
    return RegisterUserHandler(sig, act, oact, g_realSigaction);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" int sigprocmask(int how, const sigset_t *newSet, sigset_t *oldSet)  // NOLINT
{
    if (!InitRealSignalFun()) {
        return -1;
    }
    return RegisterUserMask(how, newSet, oldSet, g_realSigProcMask);
}

extern "C" void RegisterHookHandler(int signal, const SighookAction *sa)
{
    if (!InitRealSignalFun()) {
        return;
    }

    if (signal <= 0 || signal >= _NSIG) {
        LOG(FATAL, RUNTIME) << "illegal signal " << signal;
    }

    g_signalHooks[signal].RegisterHookAction(sa);
    g_signalHooks[signal].HookSig(signal);
}

extern "C" void RemoveHookHandler(int signal, bool (*action)(int, siginfo_t *, void *))
{
    if (!InitRealSignalFun()) {
        return;
    }

    if (signal <= 0 || signal >= _NSIG) {
        LOG(FATAL, RUNTIME) << "illegal signal " << signal;
    }

    g_signalHooks[signal].RemoveHookAction(action);
}

extern "C" void CheckOldHookHandler(int signal)
{
    if (!InitRealSignalFun()) {
        return;
    }

    if (signal <= 0 || signal >= _NSIG) {
        LOG(FATAL, RUNTIME) << "illegal signal " << signal;
    }

    // get old action
    struct sigaction oldAction {};
    g_realSigaction(signal, nullptr, &oldAction);

    if (oldAction.sa_sigaction != SignalHook::Handler) {  // NOLINT
        LOG(ERROR, RUNTIME) << "error: Check old hook handler found unexpected action "
                            << (oldAction.sa_sigaction != nullptr);  // NOLINT
        g_signalHooks[signal].RegisterAction(signal);
    }
}

extern "C" void AddSpecialSignalHandlerFn(int signal, SigchainAction *sa)
{
    LOG(DEBUG, RUNTIME) << "panda sighook RegisterHookHandler is used, signal:" << signal << " action:" << sa;
    RegisterHookHandler(signal, reinterpret_cast<SighookAction *>(sa));
}
extern "C" void RemoveSpecialSignalHandlerFn(int signal, bool (*fn)(int, siginfo_t *, void *))
{
    LOG(DEBUG, RUNTIME) << "panda sighook RemoveHookHandler is used, signal:"
                        << "sigaction";
    RemoveHookHandler(signal, fn);
}

extern "C" void EnsureFrontOfChain(int signal)
{
    LOG(DEBUG, RUNTIME) << "panda sighook CheckOldHookHandler is used, signal:" << signal;
    CheckOldHookHandler(signal);
}

void ClearSignalHooksHandlersArray()
{
    for (int i = 1; i < _NSIG; i++) {
        g_signalHooks[i].ClearHookActionHandlers();
    }
}

}  // namespace ark
