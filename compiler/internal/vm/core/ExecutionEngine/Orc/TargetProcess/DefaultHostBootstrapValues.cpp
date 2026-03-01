//===----- DefaultHostBootstrapValues.cpp - Defaults for host process -----===//
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

#include "vm/core/ExecutionEngine/Orc/TargetProcess/DefaultHostBootstrapValues.h"

#include "vm/core/ExecutionEngine/Orc/Shared/OrcRTBridge.h"
#include "vm/core/ExecutionEngine/Orc/TargetProcess/JITLoaderGDB.h"
#include "vm/core/ExecutionEngine/Orc/TargetProcess/RegisterEHFrames.h"

#ifdef __APPLE__
#include <dlfcn.h>
#endif // __APPLE__

namespace vm::core::orc {

void addDefaultBootstrapValuesForHostProcess(
    StringMap<std::vector<char>> &BootstrapMap,
    StringMap<ExecutorAddr> &BootstrapSymbols) {

  // FIXME: We probably shouldn't set these on Windows?
  BootstrapSymbols[rt::RegisterEHFrameSectionAllocActionName] =
      ExecutorAddr::fromPtr(&llvm_orc_registerEHFrameSectionAllocAction);
  BootstrapSymbols[rt::DeregisterEHFrameSectionAllocActionName] =
      ExecutorAddr::fromPtr(&llvm_orc_deregisterEHFrameSectionAllocAction);

  BootstrapSymbols[rt::RegisterJITLoaderGDBAllocActionName] =
      ExecutorAddr::fromPtr(&llvm_orc_registerJITLoaderGDBAllocAction);

#ifdef __APPLE__
  if (!dlsym(RTLD_DEFAULT, "__unw_add_find_dynamic_unwind_sections"))
    BootstrapMap["darwin-use-ehframes-only"].push_back(1);
#endif // __APPLE__
}

} // namespace vm::core::orc
