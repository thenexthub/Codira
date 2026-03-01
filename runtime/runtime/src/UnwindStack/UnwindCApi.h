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


#ifndef MRT_UNWIND_API_H
#define MRT_UNWIND_API_H

#include "Common/StackType.h"

namespace MapleRuntime {
struct ThreadLocalData;
extern "C" MRT_EXPORT void MRT_RestoreTopManagedContextFromN2CStub(FrameAddress* fa);
extern "C" MRT_EXPORT void MRT_SaveTopManagedContextToN2CStub(FrameAddress* fa);
extern "C" MRT_EXPORT void MRT_SaveC2NContext(const uint32_t* pc, void* fa, ThreadLocalData* tlData);
extern "C" MRT_EXPORT void MRT_DeleteC2NContext(ThreadLocalData* tlData);
extern "C" MRT_EXPORT void MRT_UpdateUwContext(const uint32_t* pc, void* fa, UnwindContextStatus status,
                                               ThreadLocalData* tlData);
} // namespace MapleRuntime
#endif // MRT_UNWIND_API_H
