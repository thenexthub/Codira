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


#ifndef MRT_SIGNAL_SIGNALUTILS_H
#define MRT_SIGNAL_SIGNALUTILS_H

#include <csignal>

#include "Base/CString.h"
#include "Base/Types.h"
#include "Base/Log.h"
#include "ucontext.h"

namespace MapleRuntime {
#if defined(MRT_DEBUG) && (MRT_DEBUG == 1)
const char* SignalCodeName(int sig, int code);
FixedCString PrintSignalInfo(const siginfo_t& info);
#endif
// Archtecture dependent
Uptr GetPCFromUContext(const ucontext_t& context);
Uptr GetFAFromUContext(const ucontext_t& context);
} // namespace MapleRuntime

#endif // MRT_SIGNAL_SIGNALUTILS_H
