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
        return x29Fp;
    }
    unsigned long long GetPC()
    {
        return pc;
    }
    unsigned long long x18; /* 0x0 */
    unsigned long long x19; /* 0x8 */
    unsigned long long x20; /* 0x10 */
    unsigned long long x21; /* 0x18 */
    unsigned long long x22; /* 0x20 */
    unsigned long long x23; /* 0x28 */
    unsigned long long x24; /* 0x30 */
    unsigned long long x25; /* 0x38 */
    unsigned long long x26; /* 0x40 */
    unsigned long long x27; /* 0x48 */
    unsigned long long x28; /* 0x50 */
    unsigned long long x29Fp;  /* 0x58 */
    unsigned long long x30Lr;  /* 0x60 */

    /* Note: The pc does not actually save the pc register. PC is set to codethread_entry and lr
     * is initialized to 0 only during process initialization. In other cases, the values of pc
     * and lr are the same. The codethread address jumps to the pc, which is to avoid stack push
     * problems. If only lr is used instead of pc, lr is set to codethread_entry during codethread
     * initialization. In this case, when the lr register is used for stack push, the upper
     * stack of codethread_entry points to codethread_entry itself, and the stack push fails.
     * Similarly, the mcall push stack can be problematic.
     **/
    unsigned long long pc;  /* 0x68 */
    unsigned long long sp;  /* 0x70 */

    /* d8-d15 are the lower 64 bits of the 128-bit floating-point registers v8-v15 */
    unsigned long long d8;  /* 0x78 */
    unsigned long long d9;  /* 0x80 */
    unsigned long long d10; /* 0x88 */
    unsigned long long d11; /* 0x90 */
    unsigned long long d12; /* 0x98 */
    unsigned long long d13; /* 0xa0 */
    unsigned long long d14; /* 0xa8 */
    unsigned long long d15; /* 0xb0 */

    unsigned int fpcr;      /* 0xb8 */
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
