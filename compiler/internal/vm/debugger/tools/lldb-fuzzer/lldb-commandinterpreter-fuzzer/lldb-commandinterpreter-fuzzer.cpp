//===-- lldb-commandinterpreter-fuzzer.cpp -------------------------------===//
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
//===---------------------------------------------------------------------===//

#include <string>

#include "lldb/API/SBCommandInterpreter.h"
#include "lldb/API/SBCommandInterpreterRunOptions.h"
#include "lldb/API/SBCommandReturnObject.h"
#include "lldb/API/SBDebugger.h"
#include "lldb/API/SBTarget.h"

using namespace lldb;

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv) {
  SBDebugger::Initialize();
  return 0;
}

extern "C" int LLVMFuzzerTestOneInput(uint8_t *data, size_t size) {
  // Convert the data into a null-terminated string
  std::string str((char *)data, size);

  // Create a debugger and a dummy target
  SBDebugger debugger = SBDebugger::Create(false);
  SBTarget target = debugger.GetDummyTarget();

  // Create a command interpreter for the current debugger
  // A return object is needed to run the command interpreter
  SBCommandReturnObject ro = SBCommandReturnObject();
  SBCommandInterpreter ci = debugger.GetCommandInterpreter();

  // Use the fuzzer generated input as input for the command interpreter
  if (ci.IsValid()) {
    ci.HandleCommand(str.c_str(), ro, false);
  }

  debugger.DeleteTarget(target);
  SBDebugger::Destroy(debugger);
  SBModule::GarbageCollectAllocatedModules();

  return 0;
}
