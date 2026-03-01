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
#ifndef PANDA_RUNTIME_ENTRYPOINTS_ENTRYPOINTS_H_
#define PANDA_RUNTIME_ENTRYPOINTS_ENTRYPOINTS_H_

#include "entrypoints_gen.h"
#include "runtime/include/thread.h"

namespace ark {

class Frame;

extern "C" Frame *CreateFrameWithSize(uint32_t size, uint32_t nregs, Method *method, Frame *prev);

extern "C" Frame *CreateFrameWithActualArgsAndSize(uint32_t size, uint32_t nregs, uint32_t numActualArgs,
                                                   Method *method, Frame *prev);

extern "C" PANDA_PUBLIC_API Frame *CreateNativeFrameWithActualArgsAndSize(uint32_t size, uint32_t nregs,
                                                                          uint32_t numActualArgs, Method *method,
                                                                          Frame *prev);

extern "C" Frame *CreateFrameForMethod(Method *method, Frame *prev);

extern "C" Frame *CreateFrameForMethodDyn(Method *method, Frame *prev);

extern "C" Frame *CreateFrameForMethodWithActualArgs(uint32_t numActualArgs, Method *method, Frame *prev);

extern "C" Frame *CreateFrameForMethodWithActualArgsDyn(uint32_t numActualArgs, Method *method, Frame *prev);

extern "C" PANDA_PUBLIC_API void FreeFrame(Frame *frame);

extern "C" void FreeFrameInterp(Frame *frame, ManagedThread *current);

extern "C" void ThrowInstantiationErrorEntrypoint(Class *klass);

}  // namespace ark

#endif  // PANDA_RUNTIME_ENTRYPOINTS_ENTRYPOINTS_H_
