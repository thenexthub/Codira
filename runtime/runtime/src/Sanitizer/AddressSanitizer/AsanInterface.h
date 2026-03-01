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


#ifndef CODIRARUNTIME_ASANINTERFACE_H
#define CODIRARUNTIME_ASANINTERFACE_H

#include <cstdint>
#include <cstddef>

#define SANITIZER_NAME "AddressSanitizer"
#define SANITIZER_SHORTEN_NAME "asan"

namespace MapleRuntime {
namespace Sanitizer {
void AsanStartSwitchThreadContext(void* oldThread, void* newThread);
void AsanEndSwitchThreadContext(void* newThread);

// this should be inserted before ProcessorSchedule and after CODEThreadContextGet
void AsanEnterCODEThread(void* thread);
// this should be inserted before CODEThreadContextSet
void AsanExitCODEThread(void* thread);

void OnHeapMadvise(void* addr, size_t size);
} // namespace Sanitizer
} // namespace MapleRuntime
#endif // CODIRARUNTIME_ASANINTERFACE_H
