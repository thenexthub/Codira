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
#include "libarkbase/os/exec.h"
#include "libarkbase/os/failure_retry.h"
#include "sys/wait.h"

namespace ark::os::exec {

Expected<int, Error> ExecNoWait(Span<const char *> args)
{
    ASSERT(!args.Empty());
    ASSERT_PRINT(args[args.Size() - 1] == nullptr, "Last argument must be a nullptr");

    if (pid_t pid = fork(); pid == 0) {
        setpgid(0, 0);
        execv(args[0], const_cast<char **>(args.Data()));
        _exit(1);
    } else if (pid < 0) {
        return Unexpected(Error(errno));
    } else {
        return pid;
    }
}

Expected<int, Error> Wait(int64_t process, bool testStatus)
{
    auto pid = static_cast<pid_t>(process);
    ASSERT(pid == process);

    int status = -1;
    pid_t resPid = PANDA_FAILURE_RETRY(waitpid(pid, &status, 0));
    if (resPid != pid) {
        return Unexpected(Error(errno));
    }

    if (!testStatus) {
        return status;
    }

    if (WIFEXITED(status)) {         // NOLINT(hicpp-signed-bitwise)
        return WEXITSTATUS(status);  // NOLINT(hicpp-signed-bitwise)
    }
    return Unexpected(Error("Process finished improperly"));
}

Expected<int, Error> Exec(Span<const char *> args)
{
    auto res = ExecNoWait(args);
    if (!res.HasValue()) {
        return res;
    }
    pid_t pid = res.Value();
    return Wait(pid, true);
}

}  // namespace ark::os::exec
