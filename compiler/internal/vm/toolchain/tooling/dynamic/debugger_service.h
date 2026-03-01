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

#ifndef ECMASCRIPT_TOOLING_DEBUGGER_SERVICE_H
#define ECMASCRIPT_TOOLING_DEBUGGER_SERVICE_H

#include <functional>
#include <memory>
#include <string>

#include "common/macros.h"

namespace panda::ecmascript {
class EcmaVM;
}  // namespace panda::ecmascript

struct DebugResponse {
    size_t size;
    char* response;
};

namespace panda::ecmascript::tooling {
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

TOOLCHAIN_EXPORT void InitializeDebugger(::panda::ecmascript::EcmaVM *vm,
    const std::function<void(const void *, const std::string &)> &onResponse);

TOOLCHAIN_EXPORT void UninitializeDebugger(::panda::ecmascript::EcmaVM *vm);

TOOLCHAIN_EXPORT void OnMessage(const ::panda::ecmascript::EcmaVM *vm, std::string &&message);

TOOLCHAIN_EXPORT void WaitForDebugger(const ::panda::ecmascript::EcmaVM *vm);

TOOLCHAIN_EXPORT void ProcessMessage(const ::panda::ecmascript::EcmaVM *vm);

TOOLCHAIN_EXPORT int32_t GetDispatchStatus(const ::panda::ecmascript::EcmaVM *vm);

// Return the dynamically allocated string (must be freed by the caller)
TOOLCHAIN_EXPORT DebugResponse GetCallFrames(const ::panda::ecmascript::EcmaVM *vm);

// Return the dynamically allocated string (must be freed by the caller)
TOOLCHAIN_EXPORT DebugResponse OperateDebugMessage(const ::panda::ecmascript::EcmaVM *vm, const char* message);

TOOLCHAIN_EXPORT void SetDebugApp(::panda::ecmascript::EcmaVM *vm);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
}  // panda::ecmascript::tooling

#endif