//===-- SBBreakpointLocation.h ----------------------------------*- C++ -*-===//
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

#ifndef LLDB_API_SBBREAKPOINTLOCATION_H
#define LLDB_API_SBBREAKPOINTLOCATION_H

#include "lldb/API/SBBreakpoint.h"
#include "lldb/API/SBDefines.h"

namespace lldb_private {
namespace python {
class SWIGBridge;
}
namespace lua {
class SWIGBridge;
}
} // namespace lldb_private

namespace lldb {

class LLDB_API SBBreakpointLocation {
  friend class lldb_private::ScriptInterpreter;

public:
  SBBreakpointLocation();

  SBBreakpointLocation(const lldb::SBBreakpointLocation &rhs);

  ~SBBreakpointLocation();

  const lldb::SBBreakpointLocation &
  operator=(const lldb::SBBreakpointLocation &rhs);

  break_id_t GetID();

  explicit operator bool() const;

  bool IsValid() const;

  lldb::SBAddress GetAddress();

  lldb::addr_t GetLoadAddress();

  void SetEnabled(bool enabled);

  bool IsEnabled();

  uint32_t GetHitCount();

  uint32_t GetIgnoreCount();

  void SetIgnoreCount(uint32_t n);

  void SetCondition(const char *condition);

  const char *GetCondition();

  void SetAutoContinue(bool auto_continue);

  bool GetAutoContinue();

#ifndef SWIG
  void SetCallback(SBBreakpointHitCallback callback, void *baton);
#endif

  void SetScriptCallbackFunction(const char *callback_function_name);

  SBError SetScriptCallbackFunction(const char *callback_function_name,
                                    lldb::SBStructuredData &extra_args);

  SBError SetScriptCallbackBody(const char *script_body_text);
  
  void SetCommandLineCommands(lldb::SBStringList &commands);

  bool GetCommandLineCommands(lldb::SBStringList &commands);
 
  void SetThreadID(lldb::tid_t sb_thread_id);

  lldb::tid_t GetThreadID();

  void SetThreadIndex(uint32_t index);

  uint32_t GetThreadIndex() const;

  void SetThreadName(const char *thread_name);

  const char *GetThreadName() const;

  void SetQueueName(const char *queue_name);

  const char *GetQueueName() const;

  bool IsResolved();

  bool GetDescription(lldb::SBStream &description, DescriptionLevel level);

  SBBreakpoint GetBreakpoint();

protected:
  friend class lldb_private::python::SWIGBridge;
  friend class lldb_private::lua::SWIGBridge;
  SBBreakpointLocation(const lldb::BreakpointLocationSP &break_loc_sp);

private:
  friend class SBBreakpoint;
  friend class SBBreakpointCallbackBaton;

  void SetLocation(const lldb::BreakpointLocationSP &break_loc_sp);
  BreakpointLocationSP GetSP() const;

  lldb::BreakpointLocationWP m_opaque_wp;
};

} // namespace lldb

#endif // LLDB_API_SBBREAKPOINTLOCATION_H
