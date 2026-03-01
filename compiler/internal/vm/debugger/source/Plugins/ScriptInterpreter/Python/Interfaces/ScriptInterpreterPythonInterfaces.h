//===-- ScriptInterpreterPythonInterfaces.h ---------------------*- C++ -*-===//
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

#ifndef LLDB_PLUGINS_SCRIPTINTERPRETER_PYTHON_SCRIPTINTERPRETERPYTHONINTERFACES_H
#define LLDB_PLUGINS_SCRIPTINTERPRETER_PYTHON_SCRIPTINTERPRETERPYTHONINTERFACES_H

#include "lldb/Core/PluginInterface.h"
#include "lldb/Host/Config.h"
#include "lldb/lldb-private.h"

#if LLDB_ENABLE_PYTHON

#include "OperatingSystemPythonInterface.h"
#include "ScriptedBreakpointPythonInterface.h"
#include "ScriptedFrameProviderPythonInterface.h"
#include "ScriptedFramePythonInterface.h"
#include "ScriptedPlatformPythonInterface.h"
#include "ScriptedProcessPythonInterface.h"
#include "ScriptedStopHookPythonInterface.h"
#include "ScriptedThreadPlanPythonInterface.h"

namespace lldb_private {
class ScriptInterpreterPythonInterfaces : public PluginInterface {
public:
  static void Initialize();
  static void Terminate();
  static llvm::StringRef GetPluginNameStatic() {
    return "script-interpreter-python-interfaces";
  }
  static llvm::StringRef GetPluginDescriptionStatic();
};
} // namespace lldb_private

#endif // LLDB_ENABLE_PYTHON
#endif // LLDB_PLUGINS_SCRIPTINTERPRETER_PYTHON_SCRIPTINTERPRETERPYTHONINTERFACES_H
