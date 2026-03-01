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

#ifndef ECMASCRIPT_TOOLING_TEST_UTILS_TEST_EVENTS_H
#define ECMASCRIPT_TOOLING_TEST_UTILS_TEST_EVENTS_H

#include <utility>

#include "agent/runtime_impl.h"
#include "backend/js_pt_hooks.h"
#include "test/utils/test_channel.h"

namespace panda::ecmascript::tooling::test {
using DebuggerStmtCallback = std::function<bool(const JSPtLocation &)>;
using BreakpointCallback = std::function<bool(const JSPtLocation &)>;
using LoadModuleCallback = std::function<bool(std::string_view)>;
using ExceptionCallback = std::function<bool(const JSPtLocation &)>;
using SingleStepCallback = std::function<bool(const JSPtLocation &)>;
using NativeOutCallback = std::function<bool()>;
using VmStartCallback = std::function<bool()>;
using VmDeathCallback = std::function<bool()>;
using Scenario = std::function<bool()>;

enum class DebugEvent {
    BREAKPOINT,
    LOAD_MODULE,
    PAUSED,
    STEP_COMPLETE,
    EXCEPTION,
    METHOD_ENTRY,
    SINGLE_STEP,
    VM_START,
    VM_INITIALIZATION,
    VM_DEATH,
    UNINITIALIZED,
    DROPFRAME,
    CHECK_COMPLETE
};

std::ostream &operator<<(std::ostream &out, DebugEvent value);

struct TestEvents {
    DebuggerStmtCallback debuggerStmt;
    BreakpointCallback breakpoint;
    LoadModuleCallback loadModule;
    ExceptionCallback exception;
    SingleStepCallback singleStep;
    NativeOutCallback nativeOut;
    VmStartCallback vmStart;
    VmDeathCallback vmDeath;

    Scenario scenario;
    const EcmaVM *vm_ {nullptr};
    JSDebugger *debugInterface_ {nullptr};
    DebuggerImpl *debugger_ {nullptr};
    RuntimeImpl *runtime_ {nullptr};
    TestChannel *channel_ {nullptr};
    TestEvents();
    virtual ~TestEvents() = default;

    virtual std::pair<std::string, std::string> GetEntryPoint() = 0;
};
}  // namespace panda::ecmascript::tooling::test

#endif  // ECMASCRIPT_TOOLING_TEST_UTILS_TEST_EVENTS_H
