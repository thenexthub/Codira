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

#ifndef ECMASCRIPT_TOOLING_INIT_STATIC_H
#define ECMASCRIPT_TOOLING_INIT_STATIC_H

#include <memory>
#include <mutex>
#include "common/log_wrapper.h"

namespace OHOS::ArkCompiler::Toolchain {
bool InitializeArkFunctionsForStatic();

void OnMessageStatic(std::string &&message);

int StopDebuggerForStatic();

bool StartDebuggerForStatic(std::shared_ptr<void> endpoint, bool breakOnStart);

void WaitForDebuggerForStatic();

int StartDebuggerInitForStatic(uint32_t port, bool breakOnStart);

int StopDebuggerInitForStatic();
}

#endif //ECMASCRIPT_TOOLING_INIT_STATIC_H