//===-- TraceIntelPTConstants.h ---------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_TRACE_INTEL_PT_CONSTANTS_H
#define LLDB_SOURCE_PLUGINS_TRACE_INTEL_PT_CONSTANTS_H

#include "lldb/lldb-types.h"
#include <cstddef>
#include <optional>

namespace lldb_private {
namespace trace_intel_pt {

const size_t kDefaultIptTraceSize = 4 * 1024;                  // 4KB
const size_t kDefaultProcessBufferSizeLimit = 5 * 1024 * 1024; // 500MB
const bool kDefaultEnableTscValue = false;
const std::optional<size_t> kDefaultPsbPeriod;
const bool kDefaultPerCpuTracing = false;
const bool kDefaultDisableCgroupFiltering = false;

// Physical address where the kernel is loaded in x86 architecture. Refer to
// https://github.com/torvalds/linux/blob/master/Documentation/x86/x86_64/mm.rst
// for the start address of kernel text section.
// The kernel entry point is 0x1000000 by default when KASLR is disabled.
const lldb::addr_t kDefaultKernelLoadAddress = 0xffffffff81000000;
const lldb::pid_t kDefaultKernelProcessID = 1;

} // namespace trace_intel_pt
} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_TRACE_INTEL_PT_CONSTANTS_H
