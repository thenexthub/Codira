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


#ifndef MRT_CODE_SCHEDULER_H
#define MRT_CODE_SCHEDULER_H

#include "Base/Globals.h"
#include "sema.h"
#include "waitqueue.h"

namespace MapleRuntime {
#ifdef __cplusplus
extern "C" {
#endif
struct UnitType {
    uint8_t* placeholder;
    // Declare 32-byte align to ensure c++ generate same function definition as cangjie IR in both x86 and arm64.
} __attribute__((aligned(32)));

void* MCC_NewCODEThread(void* execute, void* future, void* scheduler);
void* MCC_NewCODEThreadNoReturn(void* executeClosure, void* closurePtr, void* scheduler, void* futureTi);
void MRT_CodeRuntimeInit();
void MRT_SetCommandLineArgs(int argc, const char* argv[]);
const char** MRT_GetCommandLineArgs();
void MRT_CodeRuntimeStart(void* execute);
int MRT_NewWaitQueue(void* waitQueuePtr);
bool MRT_SuspendWithTimeout(void* wq, const WqCallbackFunc callBack, void* callBackObj, int64_t timeOut);
void MRT_ResumeOne(void* wq, const WqCallbackFunc callBack, void* callBackObj);
void MRT_ResumeAll(void* wq, const WqCallbackFunc callBack, void* callBackObj);

int MRT_NewSem(void* semPtr);
void MRT_SemAcquire(void* sem, bool isPushToHead);
void MRT_SemRelease(void* sem);
int64_t MRT_GetCurrentThreadID();

#ifdef __cplusplus
};
#endif
} // namespace MapleRuntime

#endif // MRT_CODE_SCHEDULER_H
