//===-- InstrumentationRuntimeASan.cpp ------------------------------------===//
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

#include "InstrumentationRuntimeASan.h"

#include "lldb/Breakpoint/StoppointCallbackContext.h"
#include "lldb/Core/Module.h"
#include "lldb/Core/PluginInterface.h"
#include "lldb/Core/PluginManager.h"
#include "lldb/Symbol/Symbol.h"
#include "lldb/Target/Process.h"
#include "lldb/Utility/RegularExpression.h"

#include "Plugins/InstrumentationRuntime/Utility/ReportRetriever.h"

using namespace lldb;
using namespace lldb_private;

LLDB_PLUGIN_DEFINE(InstrumentationRuntimeASan)

lldb::InstrumentationRuntimeSP
InstrumentationRuntimeASan::CreateInstance(const lldb::ProcessSP &process_sp) {
  return InstrumentationRuntimeSP(new InstrumentationRuntimeASan(process_sp));
}

void InstrumentationRuntimeASan::Initialize() {
  PluginManager::RegisterPlugin(
      GetPluginNameStatic(), "AddressSanitizer instrumentation runtime plugin.",
      CreateInstance, GetTypeStatic);
}

void InstrumentationRuntimeASan::Terminate() {
  PluginManager::UnregisterPlugin(CreateInstance);
}

lldb::InstrumentationRuntimeType InstrumentationRuntimeASan::GetTypeStatic() {
  return eInstrumentationRuntimeTypeAddressSanitizer;
}

InstrumentationRuntimeASan::~InstrumentationRuntimeASan() { Deactivate(); }

const RegularExpression &
InstrumentationRuntimeASan::GetPatternForRuntimeLibrary() {
  // FIXME: This shouldn't include the "dylib" suffix.
  static RegularExpression regex(
      llvm::StringRef("libclang_rt.asan_(.*)_dynamic\\.dylib"));
  return regex;
}

bool InstrumentationRuntimeASan::CheckIfRuntimeIsValid(
    const lldb::ModuleSP module_sp) {
  const Symbol *symbol = module_sp->FindFirstSymbolWithNameAndType(
      ConstString("__asan_get_alloc_stack"), lldb::eSymbolTypeAny);

  return symbol != nullptr;
}

bool InstrumentationRuntimeASan::NotifyBreakpointHit(
    void *baton, StoppointCallbackContext *context, user_id_t break_id,
    user_id_t break_loc_id) {
  assert(baton && "null baton");
  if (!baton)
    return false;

  InstrumentationRuntimeASan *const instance =
      static_cast<InstrumentationRuntimeASan *>(baton);

  ProcessSP process_sp = instance->GetProcessSP();

  return ReportRetriever::NotifyBreakpointHit(process_sp, context, break_id,
                                              break_loc_id);
}

void InstrumentationRuntimeASan::Activate() {
  if (IsActive())
    return;

  ProcessSP process_sp = GetProcessSP();
  if (!process_sp)
    return;

  Breakpoint *breakpoint = ReportRetriever::SetupBreakpoint(
      GetRuntimeModuleSP(), process_sp, ConstString("_ZN6__asanL7AsanDieEv"));

  if (!breakpoint)
    return;

  const bool sync = false;

  breakpoint->SetCallback(InstrumentationRuntimeASan::NotifyBreakpointHit, this,
                          sync);
  breakpoint->SetBreakpointKind("address-sanitizer-report");
  SetBreakpointID(breakpoint->GetID());

  SetActive(true);
}

void InstrumentationRuntimeASan::Deactivate() {
  SetActive(false);

  if (GetBreakpointID() == LLDB_INVALID_BREAK_ID)
    return;

  if (ProcessSP process_sp = GetProcessSP()) {
    process_sp->GetTarget().RemoveBreakpointByID(GetBreakpointID());
    SetBreakpointID(LLDB_INVALID_BREAK_ID);
  }
}
