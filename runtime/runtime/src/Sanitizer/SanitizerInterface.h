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


#ifndef CODIRARUNTIME_SANITIZERINTERFACE_H
#define CODIRARUNTIME_SANITIZERINTERFACE_H

#include <cstdint>

#include "Base/Macros.h"
#include "Common/TypeDef.h"
#include "SanitizerMacros.h"

#ifdef CODIRA_ASAN_SUPPORT
#include "Sanitizer/AddressSanitizer/AsanInterface.h"
#endif // CODIRA_ASAN_SUPPORT

#ifdef CODIRA_TSAN_SUPPORT
#include "Sanitizer/ThreadSanitizer/TsanInterface.h"
#endif // CODIRA_TSAN_SUPPORT

#ifdef CODIRA_HWASAN_SUPPORT
#include "Sanitizer/HwAddressSanitizer/HwasanInterface.h"
#endif // CODIRA_HWASAN_SUPPORT

#ifdef CODIRA_GWPASAN_SUPPORT
#include "Sanitizer/GwpAddressSanitizer/GwpAsanInterface.h"
#endif

#ifndef SANITIZER_NAME
#error "sanitizer name not defined, please check corresponding sanitizer interface header."
#endif

#ifndef SANITIZER_SHORTEN_NAME
#error "sanitizer shorten name not defined, please check corresponding sanitizer interface header."
#endif

namespace MapleRuntime {
namespace Sanitizer {
// general callbacks
void OnHeapAllocated(void* addr, size_t size);
void OnHeapDeallocated(void* addr, size_t size);

#if defined(GENERAL_ASAN_SUPPORT_INTERFACE) || defined(CODIRA_GWPASAN_SUPPORT)
void* ArrayAcquireMemoryRegion(ArrayRef array, void* addr, size_t size);
void* ArrayReleaseMemoryRegion(ArrayRef array, void* alias, size_t size);
#endif

#ifdef GENERAL_ASAN_SUPPORT_INTERFACE
void AsanRead(volatile const void* addr, uintptr_t size);
void AsanWrite(volatile const void* addr, uintptr_t size);

void HandleNoReturn(uint64_t from, uint64_t to);
#endif
} // namespace Sanitizer
} // namespace MapleRuntime

#endif // CODIRARUNTIME_SANITIZERINTERFACE_H
