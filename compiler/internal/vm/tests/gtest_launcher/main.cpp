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

#include <unistd.h>
#include <csignal>
#include <sys/prctl.h>
#include <gtest/gtest.h>

#if !defined(TIMEOUT_SIGNAL)
#error "TIMEOUT_SIGNAL is not defined"
#endif
#if !defined(GDB_PATH)
#error "GDB_PATH is not defined"
#else
#define XSTR(x) STR(x)
#define STR(x) #x
#define GDB_PATH_STR XSTR(GDB_PATH)
#endif

namespace panda {
void HandleTimeout(int sig)
{
    if (sig != TIMEOUT_SIGNAL) {
        return;
    }

    // Allow to attach gdb.
    // Do it before fork because the child can attach gdb before
    // the parent allows it.
    prctl(PR_SET_PTRACER, PR_SET_PTRACER_ANY, 0, 0, 0);
    pid_t pid = fork();
    if (pid != 0) {
        // Parent
        raise(SIGSTOP);  // wait for child's gdb invocation to pick us up
    } else {
        // Child
        std::stringstream out;
        out << getppid();
        std::string ppid_str = out.str();
        execlp(GDB_PATH_STR, "-q", "--batch",
               "-p", ppid_str.c_str(),
               "-ex", "info threads",
               "-ex", "thread apply all bt",
               "-ex", "kill", nullptr);
    }
}

int main(int argc, char *argv[])
{
    signal(TIMEOUT_SIGNAL, HandleTimeout);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
}  // namespace panda
