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


#include "SignalStack.h"
#include "Codira.h"

#include <dlfcn.h>
#include <pthread.h>
#include <csignal>
#include <cstdlib>
#include <climits>

#include <algorithm>
#include <atomic>
#include <initializer_list>
#include <mutex>
#include <type_traits>
#include <utility>
#include "Codira.h"
#include "securec.h"
#ifdef COV_SIGNALHANDLE
extern "C" void __gcov_flush(void);
#endif
namespace MapleRuntime {
SignalStack SignalStack::stacks[_NSIG];

static decltype(&sigaction) g_linkedSignalAction;
static decltype(&sigprocmask) g_linkedSignalProcmask;
static pthread_key_t g_sigchainKey;
static constexpr size_t SIGSET_SIZE = _NSIG / 8 / sizeof(long);

void SigOrSet(sigset_t* dest, const sigset_t* left, const sigset_t* right)
{
    if (dest == nullptr || left == nullptr || right == nullptr) {
        return;
    }

    unsigned long* destination = reinterpret_cast<unsigned long*>(dest);
    const unsigned long* leftSet = reinterpret_cast<const unsigned long*>(left);
    const unsigned long* rightSet = reinterpret_cast<const unsigned long*>(right);

    for (size_t i = 0; i < SIGSET_SIZE; ++i) {
        destination[i] = leftSet[i] | rightSet[i];
    }
}

static void CreatePthreadKey()
{
    static std::once_flag once;
    std::call_once(once, []() {
        int rc = pthread_key_create(&g_sigchainKey, nullptr);
        if (rc != 0) {
            FLOG(RTLOG_ERROR, "failed to create signalStack pthread key: rc:%d error:%d", rc, errno);
        }
    });
}

static pthread_key_t GetHandlingSignalKey()
{
    return g_sigchainKey;
}

static bool GetHandlingSignal()
{
    void* res = pthread_getspecific(GetHandlingSignalKey());
    return res == nullptr ? false : true;
}

static void SetHandlingSignal(bool value)
{
    pthread_setspecific(GetHandlingSignalKey(), reinterpret_cast<void *>(value));
}

void SignalStack::AddHandler(SignalAction* sa)
{
    handlerStack.push_back(*sa);
}

void SignalStack::RemoveHandler(bool (*fn)(int, siginfo_t*, void*))
{
    for (std::vector<SignalAction>::iterator it = handlerStack.begin(); it != handlerStack.end(); it++) {
        if ((*it).saSignalAction == fn) {
            handlerStack.erase(it);
            break;
        }
    }
}

struct SignalArgs {
    int signal;
    siginfo_t* siginfo;
    void* ucontextRaw;
    bool isAsync;
};

void SignalStack::Handler(int signal, siginfo_t* siginfo, void* ucontextRaw)
{
    FLOG(RTLOG_ERROR, "CODENatvie Handle signal: %d.", signal);
    SignalArgs* args = new SignalArgs{signal, siginfo, ucontextRaw, false};
    if (args == nullptr) {
        FLOG(RTLOG_ERROR, "Signal Handler fail: failed to new SignalArgs");
        return;
    }
    switch (signal) {
        case SIGSEGV:
        case SIGBUS:
        case SIGFPE:
            HandlerImpl(args);
            break;
        case SIGABRT:
        case SIGILL:
            // By default, handle SIGABRT and SIGILL signals synchronously;
            // when the user registers a handler, handle SIGABRT and SIGILL signals asynchronously.
            if (!SignalStack::stacks[signal].IsUserSigHandler()) {
                HandlerImpl(args);
            } else {
                args->isAsync = true;
                if (RunCODETaskSignal(reinterpret_cast<CODETaskFunc>(
                                    MapleRuntime::SignalStack::HandlerImpl),
                                    args) == NULL) {
                    LOG(RTLOG_ERROR, "Signal Handler fail. as RunCODETask return null\n");
                    delete args;
                }
            }
            break;
        default:
            args->isAsync = true;
            if (RunCODETaskSignal(reinterpret_cast<CODETaskFunc>(MapleRuntime::SignalStack::HandlerImpl), args) == NULL) {
                LOG(RTLOG_ERROR, "Signal Handler fail. as RunCODETask return null\n");
                delete args;
            }
    }
}

void SignalStack::HandlerImpl(void* args)
{
    // Extract signal arguments
    SignalArgs* signalArgs = reinterpret_cast<SignalArgs*>(args);
    int signal = signalArgs->signal;
    siginfo_t* siginfo = signalArgs->siginfo;
    void* ucontextRaw = signalArgs->ucontextRaw;
    ScopedEntryTrace trace("CODERT_SIGNAL_HANDLER");
    // Check if we are already handling a signal
    if (!GetHandlingSignal()) {
        std::vector<SignalAction>& handlerStack = SignalStack::stacks[signal].handlerStack;
        for (auto it = handlerStack.rbegin(); it != handlerStack.rend(); ++it) {
            const SignalAction& handler = *it;
            if (handler.saSignalAction == nullptr) {
                break;
            }
            // Save the previous signal mask
            sigset_t previous_mask;
            g_linkedSignalProcmask(SIG_SETMASK, &handler.scMask, &previous_mask);
            bool previous_value = GetHandlingSignal();
            // If the signal is handled asynchronously, signal reentry is allowed
            // and reentered signals are handled by the registered Codira handler since it has no return value.
            // Otherwise, it is handled by the OS default handler.
            if (!signalArgs->isAsync) {
                // marke thread is handling a signal
                SetHandlingSignal(true);
            }
            // Execute the signal handler
            if (handler.saSignalAction(signal, siginfo, ucontextRaw)) {
                SetHandlingSignal(previous_value);
                delete signalArgs;
                return;
            }
            g_linkedSignalProcmask(SIG_SETMASK, &previous_mask, nullptr);
            SetHandlingSignal(previous_value);
        }
    }
    int handlerFlags = SignalStack::stacks[signal].sigAction.sa_flags;
    ucontext_t* ucontext = static_cast<ucontext_t*>(ucontextRaw);
    // Combine the signal masks
    sigset_t mask;
    SigOrSet(&mask, &ucontext->uc_sigmask, &SignalStack::stacks[signal].sigAction.sa_mask);
    // Add the current signal to the mask if SA_NODEFER is not set
    if (!(handlerFlags & SA_NODEFER)) {
        sigaddset(&mask, signal);
    }

    // Handle the signal based on the signal action
    if (handlerFlags & SA_SIGINFO) {
        g_linkedSignalProcmask(SIG_SETMASK, &mask, nullptr);
#ifdef __APPLE__
        if (SignalStack::stacks[signal].sigAction.sa_sigaction == nullptr) {
            LOG(RTLOG_ERROR, "Handle unexpected signal action.");
            int retNum = signal + 128; // Signal base return number
            exit(retNum);
        }
#endif
        // Call the signal action with siginfo and ucontext
        SignalStack::stacks[signal].sigAction.sa_sigaction(signal, siginfo, ucontextRaw);
    } else {
        // Get the signal handler
        auto handler = SignalStack::stacks[signal].sigAction.sa_handler;
        if (handler == SIG_IGN) {
            delete signalArgs;
            return;
        } else if (handler == SIG_DFL) {
            // Restore default signal handler and re-raise the signal
            struct sigaction dfl = {};
            dfl.sa_handler = SIG_DFL;
            g_linkedSignalAction(signal, &dfl, nullptr);
            raise(signal);
            delete signalArgs;
            return;
        } else {
            g_linkedSignalProcmask(SIG_SETMASK, &mask, nullptr);
#ifdef COV_SIGNALHANDLE
            __gcov_flush();
#endif
            handler(signal);
        }
    }
    delete signalArgs;
}

template <typename T>
static void FindSymbolInLibc(T* result, const char* name)
{
#if defined(__OHOS__) || defined(__ANDROID__)
    constexpr const char* libName = "libc.so";
#elif defined(__APPLE__)
    constexpr const char* libName = "libc.dylib";
#else
    constexpr const char* libName = "libc.so.6";
#endif
    static void* libc = []() {
        void* res = dlopen(libName, RTLD_LOCAL | RTLD_LAZY);
        if (!res) {
            LOG(RTLOG_FATAL, "failed to dlopen %s: %s", libName, dlerror());
        }
        return res;
    }();

    void* sym = dlsym(libc, name);
    if (sym == nullptr) {
        LOG(RTLOG_FATAL, "failed to find %s in %s", name, libName);
    }
    *result = reinterpret_cast<T>(sym);
}

__attribute__((constructor)) void SignalStack::InitializeSignalStack()
{
    static std::once_flag once;
    std::call_once(once, []() {
        FindSymbolInLibc(&g_linkedSignalAction, "sigaction");
        FindSymbolInLibc(&g_linkedSignalProcmask, "sigprocmask");
    });
}

void SignalStack::Register(int signal)
{
    struct sigaction handlerAction = {};
    sigfillset(&handlerAction.sa_mask);
    CreatePthreadKey();

    handlerAction.sa_sigaction = SignalStack::Handler;
    handlerAction.sa_flags = SA_RESTART | SA_SIGINFO | SA_ONSTACK;

    // Change the current signal behavior and store the old behavior into `sigAction`.
    g_linkedSignalAction(signal, &handlerAction, &sigAction);
    // Do not modify the current behavior; only query and confirm the current behavior.
    g_linkedSignalAction(signal, nullptr, &handlerAction);
}

struct sigaction SignalStack::GetAction()
{
    return sigAction;
}

void SignalStack::SetAction(const struct sigaction* newAction)
{
    sigAction = *newAction;
}

static int SigactionImpl(int signal, const struct sigaction* newAction, struct sigaction* oldAction,
                         int (*linked)(int, const struct sigaction*, struct sigaction*))
{
    if (signal <= 0 || signal >= _NSIG) {
        errno = EINVAL;
        return -1;
    }

    if (SignalStack::GetStacks()[signal].IsMarked()) {
        struct sigaction tmpAction = SignalStack::GetStacks()[signal].GetAction();
        if (newAction != nullptr) {
            SignalStack::GetStacks()[signal].SetAction(newAction);
        }
        if (oldAction != nullptr) {
            *oldAction = tmpAction;
        }
        return 0;
    }

    return linked(signal, newAction, oldAction);
}

// since the -fvisibility=hidden property causes this symbol defaults to GLOBAL+HIDDEN
// this will cause the symbol to become LOCAL+HIDDEN when linked to libcangjie-runtime.so
// specify visibility for symbols that need to be exported
extern "C" MRT_EXPORT int sigaction(int signal, const struct sigaction* newAction, struct sigaction* oldAction)
{
    SignalStack::InitializeSignalStack();
    return SigactionImpl(signal, newAction, oldAction, g_linkedSignalAction);
}

extern "C" MRT_EXPORT sighandler_t signal(int signal, sighandler_t handler)
{
    SignalStack::InitializeSignalStack();

    if (signal <= 0 || signal >= _NSIG) {
        errno = EINVAL;
        return SIG_ERR;
    }

    struct sigaction sa = {};
    sa.sa_handler = handler;
    sa.sa_flags = SA_RESTART | SA_ONSTACK;
    sigemptyset(&sa.sa_mask);

    if (SignalStack::GetStacks()[signal].IsMarked()) {
        sighandler_t oldhandler =
            reinterpret_cast<sighandler_t>(SignalStack::GetStacks()[signal].GetAction().sa_handler);
        SignalStack::GetStacks()[signal].SetAction(&sa);
        return oldhandler;
    }

    if (g_linkedSignalAction(signal, &sa, &sa) == -1) {
        return SIG_ERR;
    }

    return reinterpret_cast<sighandler_t>(sa.sa_handler);
}

void AdjustSignalMask(sigset_t* set)
{
    for (int i = 1; i < _NSIG; ++i) {
        if (SignalStack::GetStacks()[i].IsMarked() && sigismember(set, i)) {
            sigdelset(set, i);
        }
    }
}

int SigProcMask(int how, const sigset_t* newSet, sigset_t* oldSet,
                int (*linked)(int, const sigset_t*, sigset_t*))
{
    if (GetHandlingSignal()) {
        return linked(how, newSet, oldSet);
    }

    if (newSet == nullptr) {
        return linked(how, newSet, oldSet);
    }

    sigset_t* newSetPtr = const_cast<sigset_t*>(newSet);
    sigset_t tmpset = *newSet;

    if (how == SIG_BLOCK || how == SIG_SETMASK) {
        AdjustSignalMask(&tmpset);
    }

    *newSetPtr = tmpset;

    return linked(how, newSetPtr, oldSet);
}

extern "C" MRT_EXPORT int sigprocmask(int how, const sigset_t* newSet, sigset_t* oldSet)
{
    SignalStack::InitializeSignalStack();
    return SigProcMask(how, newSet, oldSet, g_linkedSignalProcmask);
}

} // namespace MapleRuntime
