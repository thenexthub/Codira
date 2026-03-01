//===-- lldb-target-fuzzer.cpp - Fuzz target creation ---------------------===//
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

#include "utils/TempFile.h"

#include "lldb/API/SBDebugger.h"
#include "lldb/API/SBTarget.h"

using namespace lldb;
using namespace lldb_fuzzer;
using namespace llvm;

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv) {
  SBDebugger::Initialize();
  return 0;
}

extern "C" int LLVMFuzzerTestOneInput(uint8_t *data, size_t size) {
  std::unique_ptr<TempFile> file = TempFile::Create(data, size);
  if (!file)
    return 1;

  SBDebugger debugger = SBDebugger::Create(false);
  SBTarget target = debugger.CreateTarget(file->GetPath().data());
  debugger.DeleteTarget(target);
  SBDebugger::Destroy(debugger);
  SBModule::GarbageCollectAllocatedModules();

  return 0;
}
