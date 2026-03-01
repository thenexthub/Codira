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

#ifndef PANDA_RUNTIME_BRIDGE_BRIDGE_H_
#define PANDA_RUNTIME_BRIDGE_BRIDGE_H_

#include <cstdint>
#include "libarkbase/macros.h"

namespace ark {

class Method;
class Frame;
class ManagedThread;

extern "C" void InterpreterToCompiledCodeBridge(const uint8_t *, const Frame *, const Method *, ManagedThread *);
extern "C" void InterpreterToCompiledCodeBridgeDyn(const uint8_t *, const Frame *, const Method *, ManagedThread *);
extern "C" uint64_t InvokeCompiledCodeWithArgArray(const int64_t *, const Frame *, const Method *, ManagedThread *);
extern "C" uint64_t InvokeCompiledCodeWithArgArrayDyn(const uint64_t *, uint32_t, const Frame *, const Method *,
                                                      ManagedThread *);

extern "C" int64_t InvokeInterpreter(ManagedThread *thread, const uint8_t *pc, Frame *frame, Frame *lastFrame);

extern "C" void CompiledCodeToInterpreterBridge();
extern "C" void CompiledCodeToInterpreterBridgeDyn();
extern "C" void AbstractMethodStub();
extern "C" void DefaultConflictMethodStub();

PANDA_PUBLIC_API const void *GetCompiledCodeToInterpreterBridge(const Method *method);

PANDA_PUBLIC_API const void *GetCompiledCodeToInterpreterBridge();

PANDA_PUBLIC_API const void *GetCompiledCodeToInterpreterBridgeDyn();

PANDA_PUBLIC_API const void *GetAbstractMethodStub();

PANDA_PUBLIC_API const void *GetDefaultConflictMethodStub();
}  // namespace ark

#endif  // PANDA_RUNTIME_BRIDGE_BRIDGE_H_
