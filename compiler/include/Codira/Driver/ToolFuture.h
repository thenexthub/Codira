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
 * This file declares ToolFuture class and its subclasses.
 */

#ifndef CODIRA_DRIVER_TOOLFUTURE_H
#define CODIRA_DRIVER_TOOLFUTURE_H

#include <future>
#include <optional>
#ifdef _WIN32
#include <windows.h>
#include <iomanip>
#undef CONST
#undef interface
#else
#include <sys/wait.h>
#endif

#include "Codira/Utils/Utils.h"

using namespace Codira;

class ToolFuture {
public:
    enum class State {
        SUCCESS,
        RUNNING,
        FAILED
    };

    /**
     * @brief Get status of the asynchronous operation indicated by 'ToolFuture'.
     *
     * @return State The thread state.
     */
    virtual State GetState() = 0;

    /**
     * @brief The destructor of class ToolFuture.
     */
    virtual ~ToolFuture() {};
};

class ThreadFuture : public ToolFuture {
public:
    /**
     * @brief The constructor of class ThreadFuture.
     *
     * @param input The result of asynchronous operation.
     * @return ThreadFuture The thread future.
     */
    explicit ThreadFuture(std::future<bool>&& input) : future(std::move(input)) {}

    /**
     * @brief Get status of the asynchronous operation indicated by 'ThreadFuture'.
     *
     * @return State The thread state.
     */
    State GetState() override;
private:
    std::optional<bool> result = std::nullopt;
    std::future<bool> future;
};

#ifdef _WIN32
class WindowsProcessFuture : public ToolFuture {
public:
    /**
     * @brief The constructor of class WindowsProcessFuture.
     *
     * @param pi The process information.
     * @return WindowsProcessFuture The windows process future.
     */
    explicit WindowsProcessFuture(PROCESS_INFORMATION pi): pi(pi) {}

    /**
     * @brief Get status of the asynchronous operation indicated by 'WindowsProcessFuture'.
     *
     * @return State The thread state.
     */
    State GetState() override;
private:
    PROCESS_INFORMATION pi;
};
#else
class LinuxProcessFuture : public ToolFuture {
public:
    /**
     * @brief The constructor of class LinuxProcessFuture.
     *
     * @param pi The process id.
     * @return LinuxProcessFuture The linux process future.
     */
    explicit LinuxProcessFuture(pid_t pid) : pid(pid) {}

    /**
     * @brief Get status of the asynchronous operation indicated by 'LinuxProcessFuture'.
     *
     * @return State The status of asynchronous operation.
     */
    State GetState() override;
private:
    pid_t pid;
};
#endif
#endif // CODIRA_DRIVER_TOOLFUTURE_H
