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


#ifndef CODIRARUNTIME_SANITIZERCOMPILERCALLS_H
#define CODIRARUNTIME_SANITIZERCOMPILERCALLS_H

#include <cstdint>

#include "Base/Macros.h"

extern "C" {
#ifdef GENERAL_ASAN_SUPPORT_INTERFACE
MRT_EXPORT void CODE_MCC_AsanRead(volatile const void* addr, uintptr_t size);
MRT_EXPORT void CODE_MCC_AsanWrite(volatile const void* addr, uintptr_t size);
MRT_EXPORT void CODE_MCC_AsanHandleNoReturn(const void* rsp);
#endif

#ifdef CODIRA_TSAN_SUPPORT
MRT_EXPORT void CODE_MCC_TsanWriteMemory(const void* addr, size_t size);
MRT_EXPORT void CODE_MCC_TsanReadMemory(const void* addr, size_t size);
MRT_EXPORT void CODE_MCC_TsanWriteMemoryRange(const void* addr, size_t size);
MRT_EXPORT void CODE_MCC_TsanReadMemoryRange(const void* addr, size_t size);
MRT_EXPORT void* CODE_MCC_TsanGetRaceProc(void);
MRT_EXPORT void* CODE_MCC_TsanGetThreadState(void);
#endif
}

#endif // CODIRARUNTIME_SANITIZERCOMPILERCALLS_H
