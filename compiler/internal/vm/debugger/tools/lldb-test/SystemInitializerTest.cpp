//===-- SystemInitializerTest.cpp -------------------------------*- C++ -*-===//
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

#include "SystemInitializerTest.h"
#include "lldb/Core/Debugger.h"
#include "lldb/Core/PluginManager.h"
#include "lldb/Host/Host.h"
#include "lldb/Initialization/SystemInitializerCommon.h"
#include "lldb/Interpreter/CommandInterpreter.h"
#include "lldb/Utility/Timer.h"
#include "llvm/Support/TargetSelect.h"

#include <string>

#define LLDB_PLUGIN(p) LLDB_PLUGIN_DECLARE(p)
#include "Plugins/Plugins.def"

using namespace lldb_private;

SystemInitializerTest::SystemInitializerTest()
    : SystemInitializerCommon(nullptr) {}
SystemInitializerTest::~SystemInitializerTest() = default;

llvm::Error SystemInitializerTest::Initialize() {
  if (auto e = SystemInitializerCommon::Initialize())
    return e;

  // Initialize LLVM and Clang
  llvm::InitializeAllTargets();
  llvm::InitializeAllAsmPrinters();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllDisassemblers();

#define LLDB_SCRIPT_PLUGIN(p)
#define LLDB_PLUGIN(p) LLDB_PLUGIN_INITIALIZE(p);
#include "Plugins/Plugins.def"

  // We ignored all the script interpreter earlier, so initialize
  // ScriptInterpreterNone explicitly.
  LLDB_PLUGIN_INITIALIZE(ScriptInterpreterNone);

  // Scan for any system or user LLDB plug-ins
  PluginManager::Initialize();

  // The process settings need to know about installed plug-ins, so the
  // Settings must be initialized AFTER PluginManager::Initialize is called.
  Debugger::SettingsInitialize();

  Debugger::Initialize(nullptr);

  return llvm::Error::success();
}

void SystemInitializerTest::Terminate() {
  Debugger::Terminate();

  Debugger::SettingsTerminate();

  // Terminate and unload and loaded system or user LLDB plug-ins
  PluginManager::Terminate();

#define LLDB_SCRIPT_PLUGIN(p)
#define LLDB_PLUGIN(p) LLDB_PLUGIN_TERMINATE(p);
#include "Plugins/Plugins.def"

  // We ignored all the script interpreter earlier, so terminate
  // ScriptInterpreterNone explicitly.
  LLDB_PLUGIN_INITIALIZE(ScriptInterpreterNone);

  // Now shutdown the common parts, in reverse order.
  SystemInitializerCommon::Terminate();
}
