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


#ifndef MRT_SIGNAL_STACK_H
#define MRT_SIGNAL_STACK_H

#include <csignal>

#include "Base/Log.h"
#include "Base/LogFile.h"

#ifdef __APPLE__
#define _NSIG NSIG
using sighandler_t = sig_t;
#endif

namespace MapleRuntime {

constexpr uint64_t SIGNAL_STACK_ALLOW_NORETURN = 0x1UL;

class SignalStack {
public:
    SignalStack() noexcept : isMark(false), isUserSigHandler(false) {}

    bool IsMarked() { return isMark; }

    void MarkSig(int signal)
    {
        if (!isMark) {
            Register(signal);
            isMark = true;
        }
    }

    bool IsUserSigHandler() { return isUserSigHandler; }

    void setUserSigHandler(bool flag)
    {
        isUserSigHandler = flag;
    }

    void Register(int signal);

    struct sigaction GetAction();
    void SetAction(const struct sigaction* newAction);

    void AddHandler(SignalAction* sa);
    void RemoveHandler(bool (*fn)(int, siginfo_t*, void*));

    static void Handler(int signal, siginfo_t* siginfo, void* ucontextRaw);
    static void HandlerImpl(void* args);
    static void InitializeSignalStack();
    static SignalStack* GetStacks() { return stacks; }
    struct sigaction sigAction;
private:
    bool isMark;

    bool isUserSigHandler;
    
    std::vector<SignalAction> handlerStack;
#ifdef __APPLE__
    static SignalStack stacks[NSIG];
#else
    static SignalStack stacks[_NSIG];
#endif
};
} // namespace MapleRuntime
#endif // MRT_SIGNAL_STACK_H
