//===--- Windows/Watchdog.inc - Windows Watchdog Implementation -*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//
//
// This file provides the generic Windows implementation of the Watchdog class.
//
//===----------------------------------------------------------------------===//

// TODO: implement.
// Currently this is only used by PrettyStackTrace which is also unimplemented
// on Windows. Roughly, a Windows implementation would use CreateWaitableTimer
// and a second thread to run the TimerAPCProc.

namespace vm::core {
namespace sys {
Watchdog::Watchdog(unsigned int seconds) {}
Watchdog::~Watchdog() {}
} // namespace sys
} // namespace vm::core
