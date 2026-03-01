/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LIBPANDABASE_OS_UNIX_EXEC_H
#define LIBPANDABASE_OS_UNIX_EXEC_H

#include <array>
#include "sys/wait.h"
#include <unistd.h>

#include "libarkbase/macros.h"
#include "libarkbase/os/error.h"
#include "libarkbase/os/failure_retry.h"
#include "libarkbase/utils/expected.h"
#include "libarkbase/utils/span.h"

namespace ark::os::exec {

template <typename Callback>
Expected<int, Error> ExecWithCallbackNoWait(Callback callback, Span<const char *> args)
{
    ASSERT(!args.Empty());
    ASSERT_PRINT(args[args.Size() - 1] == nullptr, "Last argument must be a nullptr");

    if (pid_t pid = fork(); pid == 0) {
        callback();
        setpgid(0, 0);
        execv(args[0], const_cast<char **>(args.Data()));
        _exit(1);
    } else if (pid < 0) {
        return Unexpected(Error(errno));
    } else {
        return pid;
    }
}

template <typename Callback>
Expected<int, Error> ExecWithCallback(Callback callback, Span<const char *> args)
{
    auto res = ExecWithCallbackNoWait(callback, args);
    if (!res.HasValue()) {
        return res;
    }
    pid_t pid = res.Value();

    int status = -1;
    pid_t resPid = PANDA_FAILURE_RETRY(waitpid(pid, &status, 0));
    if (resPid != pid) {
        return Unexpected(Error(errno));
    }
    auto statusCasted = static_cast<unsigned int>(status);
    if (WIFEXITED(statusCasted)) {
        return static_cast<int>(WEXITSTATUS(statusCasted));
    }
    return Unexpected(Error("Process finished improperly"));
}

}  // namespace ark::os::exec

#endif  // LIBPANDABASE_OS_EXEC_H
