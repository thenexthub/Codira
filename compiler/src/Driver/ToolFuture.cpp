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

/**
 * @file
 *
 * This file implements the ToolFuture class and its subclasses.
 */

#include "Codira/Driver/ToolFuture.h"

using namespace Codira;

ToolFuture::State ThreadFuture::GetState()
{
    if (!result.has_value()) {
        result = future.get();
    }
    return result.value() ? State::SUCCESS : State::FAILED;
}

#ifdef _WIN32
ToolFuture::State WindowsProcessFuture::GetState()
{
    DWORD state = WaitForSingleObject(pi.hProcess, 0);
    if (state == WAIT_FAILED || state == WAIT_ABANDONED) {
        return State::FAILED;
    } else if (state == WAIT_TIMEOUT) {
        return State::RUNNING;
    }
    DWORD exit_code;
    if (FALSE == GetExitCodeProcess(pi.hProcess, &exit_code)) {
        return State::FAILED;
    }
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return exit_code == 0 ? State::SUCCESS : State::FAILED;
}
#else
ToolFuture::State LinuxProcessFuture::GetState()
{
    int status = 0;
    int result = waitpid(pid, &status, WNOHANG);
    if (result < 0 || status != 0) {
        // If an error occurs because the file is deleted, the error information is not printed.
        return State::FAILED;
    }
    if (result > 0) {
        return State::SUCCESS;
    }
    return State::RUNNING;
}
#endif

