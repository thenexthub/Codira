//===-- Utility.h -----------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_INSTRUMENTATIONRUNTIME_UTILITY_UTILITY_H
#define LLDB_SOURCE_PLUGINS_INSTRUMENTATIONRUNTIME_UTILITY_UTILITY_H

#include <tuple>

#include "lldb/lldb-forward.h"
#include "lldb/lldb-private-enumerations.h"

namespace lldb_private {

/// On Darwin, if LLDB loaded libclang_rt, it's coming from a locally built
/// compiler-rt, and we should prefer it in favour of the system sanitizers
/// when running InstrumentationRuntime utility expressions that use symbols
/// from the sanitizer libraries. This helper searches the target for such a
/// dylib. Returns nullptr if no such dylib was found.
std::tuple<lldb::ModuleSP, HistoryPCType>
GetPreferredAsanModule(const Target &target);

} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_INSTRUMENTATIONRUNTIME_UTILITY_UTILITY_H
