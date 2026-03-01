//===-- BreakpointResolverScripted.cpp ------------------------------------===//
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

#include "lldb/Breakpoint/BreakpointResolverScripted.h"


#include "lldb/Breakpoint/BreakpointLocation.h"
#include "lldb/Core/Debugger.h"
#include "lldb/Core/Module.h"
#include "lldb/Core/Section.h"
#include "lldb/Core/StructuredDataImpl.h"
#include "lldb/Interpreter/CommandInterpreter.h"
#include "lldb/Interpreter/ScriptInterpreter.h"
#include "lldb/Target/Process.h"
#include "lldb/Target/Target.h"
#include "lldb/Utility/Log.h"
#include "lldb/Utility/StreamString.h"

using namespace lldb;
using namespace lldb_private;

// BreakpointResolverScripted:
BreakpointResolverScripted::BreakpointResolverScripted(
    const BreakpointSP &bkpt, const llvm::StringRef class_name,
    lldb::SearchDepth depth, const StructuredDataImpl &args_data)
    : BreakpointResolver(bkpt, BreakpointResolver::PythonResolver),
      m_class_name(std::string(class_name)), m_depth(depth), m_args(args_data) {
  CreateImplementationIfNeeded(bkpt);
}

void BreakpointResolverScripted::CreateImplementationIfNeeded(
    BreakpointSP breakpoint_sp) {
  if (m_interface_sp)
    return;

  if (m_class_name.empty())
    return;

  if (!breakpoint_sp)
    return;

  TargetSP target_sp = breakpoint_sp->GetTargetSP();
  ScriptInterpreter *script_interp = target_sp->GetDebugger()
                                              .GetScriptInterpreter();
  if (!script_interp)
    return;

  if (!m_interface_sp)
    m_interface_sp = script_interp->CreateScriptedBreakpointInterface();

  if (!m_interface_sp) {
    m_error = Status::FromErrorStringWithFormat(
        "BreakpointResolverScripted::%s () - ERROR: %s", __FUNCTION__,
        "Script interpreter couldn't create Scripted Breakpoint Interface");
    return;
  }

  auto obj_or_err =
      m_interface_sp->CreatePluginObject(m_class_name, breakpoint_sp, m_args);
  if (!obj_or_err) {
    m_interface_sp.reset();
    m_error = Status::FromError(obj_or_err.takeError());
    return;
  }

  StructuredData::ObjectSP object_sp = *obj_or_err;
  if (!object_sp || !object_sp->IsValid()) {
    m_error = Status::FromErrorStringWithFormat(
        "ScriptedBreakpoint::%s () - ERROR: %s", __FUNCTION__,
        "Failed to create valid script object");
  }
}

void BreakpointResolverScripted::NotifyBreakpointSet() {
  CreateImplementationIfNeeded(GetBreakpoint());
}

BreakpointResolverSP BreakpointResolverScripted::CreateFromStructuredData(
    const StructuredData::Dictionary &options_dict, Status &error) {
  llvm::StringRef class_name;
  bool success;

  success = options_dict.GetValueForKeyAsString(
      GetKey(OptionNames::PythonClassName), class_name);
  if (!success) {
    error =
        Status::FromErrorString("BRFL::CFSD: Couldn't find class name entry.");
    return nullptr;
  }
  // The Python function will actually provide the search depth, this is a
  // placeholder.
  lldb::SearchDepth depth = lldb::eSearchDepthTarget;

  StructuredDataImpl args_data_impl;
  StructuredData::Dictionary *args_dict = nullptr;
  if (options_dict.GetValueForKeyAsDictionary(GetKey(OptionNames::ScriptArgs),
                                              args_dict))
    args_data_impl.SetObjectSP(args_dict->shared_from_this());
  return std::make_shared<BreakpointResolverScripted>(nullptr, class_name,
                                                      depth, args_data_impl);
}

StructuredData::ObjectSP
BreakpointResolverScripted::SerializeToStructuredData() {
  StructuredData::DictionarySP options_dict_sp(
      new StructuredData::Dictionary());

  options_dict_sp->AddStringItem(GetKey(OptionNames::PythonClassName),
                                   m_class_name);
  if (m_args.IsValid())
    options_dict_sp->AddItem(GetKey(OptionNames::ScriptArgs),
                             m_args.GetObjectSP());

  return WrapOptionsDict(options_dict_sp);
}

ScriptInterpreter *BreakpointResolverScripted::GetScriptInterpreter() {
  return GetBreakpoint()->GetTarget().GetDebugger().GetScriptInterpreter();
}

Searcher::CallbackReturn BreakpointResolverScripted::SearchCallback(
    SearchFilter &filter, SymbolContext &context, Address *addr) {
  bool should_continue = true;
  if (!m_interface_sp)
    return Searcher::eCallbackReturnStop;

  should_continue = m_interface_sp->ResolverCallback(context);
  if (should_continue)
    return Searcher::eCallbackReturnContinue;

  return Searcher::eCallbackReturnStop;
}

lldb::SearchDepth
BreakpointResolverScripted::GetDepth() {
  lldb::SearchDepth depth = lldb::eSearchDepthModule;
  if (m_interface_sp)
    depth = m_interface_sp->GetDepth();

  return depth;
}

void BreakpointResolverScripted::GetDescription(Stream *s) {
  StructuredData::GenericSP generic_sp;
  std::optional<std::string> short_help;

  CreateImplementationIfNeeded(GetBreakpoint());

  if (m_interface_sp) {
    short_help = m_interface_sp->GetShortHelp();
  }
  if (short_help && !short_help->empty())
    s->PutCString(short_help->c_str());
  else
    s->Printf("python class = %s", m_class_name.c_str());
}

std::optional<std::string> BreakpointResolverScripted::GetLocationDescription(
    lldb::BreakpointLocationSP bp_loc_sp, lldb::DescriptionLevel level) {
  CreateImplementationIfNeeded(GetBreakpoint());
  if (m_interface_sp)
    return m_interface_sp->GetLocationDescription(bp_loc_sp, level);
  return {};
}

lldb::BreakpointLocationSP
BreakpointResolverScripted::WasHit(lldb::StackFrameSP frame_sp,
                                   lldb::BreakpointLocationSP bp_loc_sp) {
  if (m_interface_sp)
    return m_interface_sp->WasHit(frame_sp, bp_loc_sp);
  return {};
}

void BreakpointResolverScripted::Dump(Stream *s) const {}

lldb::BreakpointResolverSP
BreakpointResolverScripted::CopyForBreakpoint(BreakpointSP &breakpoint) {
  return std::make_shared<BreakpointResolverScripted>(breakpoint, m_class_name,
                                                      m_depth, m_args);
}
