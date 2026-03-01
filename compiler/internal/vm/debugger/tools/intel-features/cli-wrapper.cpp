//===-- cli-wrapper.cpp -----------------------------------------*- C++ -*-===//
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
// CLI Wrapper for hardware features of Intel(R) architecture based processors
// to enable them to be used through LLDB's CLI. For details, please refer to
// cli wrappers of each individual feature, residing in their respective
// folders.
//
// Compile this into a shared lib and load by placing at appropriate locations
// on disk or by using "plugin load" command at the LLDB command line.
//
//===----------------------------------------------------------------------===//

#ifdef BUILD_INTEL_MPX
#include "intel-mpx/cli-wrapper-mpxtable.h"
#endif

#include "lldb/API/SBDebugger.h"

namespace lldb {
bool PluginInitialize(lldb::SBDebugger debugger);
}

bool lldb::PluginInitialize(lldb::SBDebugger debugger) {

#ifdef BUILD_INTEL_MPX
  MPXPluginInitialize(debugger);
#endif

  return true;
}
