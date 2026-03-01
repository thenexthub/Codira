//===-- State.cpp ---------------------------------------------------------===//
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

#include "lldb/Utility/State.h"

using namespace lldb;
using namespace lldb_private;

const char *lldb_private::StateAsCString(StateType state) {
  switch (state) {
  case eStateInvalid:
    return "invalid";
  case eStateUnloaded:
    return "unloaded";
  case eStateConnected:
    return "connected";
  case eStateAttaching:
    return "attaching";
  case eStateLaunching:
    return "launching";
  case eStateStopped:
    return "stopped";
  case eStateRunning:
    return "running";
  case eStateStepping:
    return "stepping";
  case eStateCrashed:
    return "crashed";
  case eStateDetached:
    return "detached";
  case eStateExited:
    return "exited";
  case eStateSuspended:
    return "suspended";
  }
  return "unknown";
}

const char *lldb_private::GetPermissionsAsCString(uint32_t permissions) {
  switch (permissions) {
  case 0:
    return "---";
  case ePermissionsWritable:
    return "-w-";
  case ePermissionsReadable:
    return "r--";
  case ePermissionsExecutable:
    return "--x";
  case ePermissionsReadable | ePermissionsWritable:
    return "rw-";
  case ePermissionsReadable | ePermissionsExecutable:
    return "r-x";
  case ePermissionsWritable | ePermissionsExecutable:
    return "-wx";
  case ePermissionsReadable | ePermissionsWritable | ePermissionsExecutable:
    return "rwx";
  default:
    break;
  }
  return "???";
}

bool lldb_private::StateIsRunningState(StateType state) {
  switch (state) {
  case eStateAttaching:
  case eStateLaunching:
  case eStateRunning:
  case eStateStepping:
    return true;

  case eStateConnected:
  case eStateDetached:
  case eStateInvalid:
  case eStateUnloaded:
  case eStateStopped:
  case eStateCrashed:
  case eStateExited:
  case eStateSuspended:
    break;
  }
  return false;
}

bool lldb_private::StateIsStoppedState(StateType state, bool must_exist) {
  switch (state) {
  case eStateInvalid:
  case eStateConnected:
  case eStateAttaching:
  case eStateLaunching:
  case eStateRunning:
  case eStateStepping:
  case eStateDetached:
    break;

  case eStateUnloaded:
  case eStateExited:
    return !must_exist;

  case eStateStopped:
  case eStateCrashed:
  case eStateSuspended:
    return true;
  }
  return false;
}
