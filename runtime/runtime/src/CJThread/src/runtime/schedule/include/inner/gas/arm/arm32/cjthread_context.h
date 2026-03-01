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


#ifndef RUNTIME_CODETHREAD_CONTEXT_H
#define RUNTIME_CODETHREAD_CONTEXT_H

struct CODEThreadContext {
    unsigned long long GetFrameAddress()
    {
        return r11fp;
    }
    unsigned long long GetPC()
    {
        return r15pc;
    }

    unsigned int r4;
    unsigned int r5;
    unsigned int r6;
    unsigned int r7;
    unsigned int r8;
    unsigned int r9;
    unsigned int r10;
    unsigned int r11fp;
    unsigned int r13sp;
    unsigned int r14lr;
    unsigned int r15pc;

    unsigned long long d8;
    unsigned long long d9;
    unsigned long long d10;
    unsigned long long d11;
    unsigned long long d12;
    unsigned long long d13;
    unsigned long long d14;
    unsigned long long d15;

    unsigned int fpscr;
};

extern "C" void CODEThreadMcall(void *func, void **gCODEThread);

extern "C" void CODEThreadMcallNosave(void *func, void **gCODEThread);

extern "C" void CODEThreadExecute(void *nextCODEThread, void **gCODEThread);

#ifdef __OHOS__
extern "C" void SingleCODEThreadStoreSP();
#endif

extern "C" void CODEThreadContextGet(struct CODEThreadContext *context);

extern "C" void CODEThreadContextSet(struct CODEThreadContext *context);

extern "C" void CODEThreadContextInit(struct CODEThreadContext *context, void *func, char *stackBaseAddr);

#endif // RUNTIME_CODETHREAD_CONTEXT_H
