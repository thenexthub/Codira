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


#include "SignalUtils.h"

#include <sys/ucontext.h>

#include "securec.h"

namespace MapleRuntime {
#if defined(MRT_DEBUG) && (MRT_DEBUG == 1)
const char* SignalCodeName(int sig, int code)
{
    if (sig != SIGSEGV) {
        return "UNKNOWN";
    } else {
        switch (code) {
            case SEGV_MAPERR:
                return "SEGV_MAPERR";
            case SEGV_ACCERR:
                return "SEGV_ACCERR";
            default:
                return "SEGV_UNKNOWN";
        }
    }
}

FixedCString PrintSignalInfo(const siginfo_t& info)
{
    constexpr size_t bufferSize = 256;
    char buf[bufferSize];
    int index = 0;
    index += sprintf_s(buf + index, sizeof(buf) - index, "  si_signo: %d (%s)\n  si_code: %d (%s)", info.si_signo,
                       strsignal(info.si_signo), info.si_code, SignalCodeName(info.si_signo, info.si_code));

    CHECK_IN_SIG(index != -1);
    if (info.si_signo == SIGSEGV) {
        CHECK_IN_SIG(sprintf_s(buf + index, sizeof(buf) - index, "\n  si_addr: %p", info.si_addr) != -1);
    }

    return FixedCString(buf);
}
#endif

Uptr GetPCFromUContext(const ucontext_t& ucontext)
{
#if defined(__APPLE__) && defined(__x86_64__)
    return static_cast<Uptr>(ucontext.uc_mcontext->__ss.__rip);
#elif defined(__APPLE__) && defined(__aarch64__)
    return static_cast<Uptr>(ucontext.uc_mcontext->__ss.__pc);
#elif defined(__aarch64__)
    return static_cast<Uptr>(ucontext.uc_mcontext.pc);
#elif defined(__arm__)
    return static_cast<Uptr>(ucontext.uc_mcontext.arm_pc);
#elif defined(__x86_64__)
    return static_cast<Uptr>(ucontext.uc_mcontext.gregs[REG_RIP]);
#else // __x86?
    return static_cast<Uptr>(ucontext.uc_mcontext.gregs[REG_EIP]);
#endif
}

Uptr GetFAFromUContext(const ucontext_t& ucontext)
{
#if defined(__APPLE__) && defined(__x86_64__)
    return static_cast<Uptr>(ucontext.uc_mcontext->__ss.__rbp);
#elif defined(__APPLE__) && defined(__aarch64__)
    return static_cast<Uptr>(ucontext.uc_mcontext->__ss.__fp);
#elif defined(__aarch64__)
    constexpr uint32_t fp = 29; // x29 is fp register.
    return static_cast<Uptr>(ucontext.uc_mcontext.regs[fp]);
#elif defined(__arm__)
    return static_cast<Uptr>(ucontext.uc_mcontext.arm_fp);
#elif defined(__x86_64__)
    return static_cast<Uptr>(ucontext.uc_mcontext.gregs[REG_RBP]);
#else // __x86?
    return static_cast<Uptr>(ucontext.uc_mcontext.gregs[REG_EBP]);
#endif
}
} // namespace MapleRuntime
