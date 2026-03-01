//===-- ScriptInterpreterPythonInterfaces.cpp -----------------------------===//
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

#include "lldb/Core/PluginManager.h"
#include "lldb/Host/Config.h"
#include "lldb/lldb-enumerations.h"

#if LLDB_ENABLE_PYTHON

#include "ScriptInterpreterPythonInterfaces.h"

using namespace lldb;
using namespace lldb_private;

LLDB_PLUGIN_DEFINE(ScriptInterpreterPythonInterfaces)

llvm::StringRef
ScriptInterpreterPythonInterfaces::GetPluginDescriptionStatic() {
  return "Script Interpreter Python Interfaces";
}

void ScriptInterpreterPythonInterfaces::Initialize() {
  OperatingSystemPythonInterface::Initialize();
  ScriptedPlatformPythonInterface::Initialize();
  ScriptedProcessPythonInterface::Initialize();
  ScriptedStopHookPythonInterface::Initialize();
  ScriptedBreakpointPythonInterface::Initialize();
  ScriptedThreadPlanPythonInterface::Initialize();
  ScriptedFrameProviderPythonInterface::Initialize();
}

void ScriptInterpreterPythonInterfaces::Terminate() {
  OperatingSystemPythonInterface::Terminate();
  ScriptedPlatformPythonInterface::Terminate();
  ScriptedProcessPythonInterface::Terminate();
  ScriptedStopHookPythonInterface::Terminate();
  ScriptedBreakpointPythonInterface::Terminate();
  ScriptedThreadPlanPythonInterface::Terminate();
  ScriptedFrameProviderPythonInterface::Terminate();
}

#endif
