//===-- DAPForward.h --------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_TOOLS_LLDB_DAP_DAPFORWARD_H
#define LLDB_TOOLS_LLDB_DAP_DAPFORWARD_H

// IWYU pragma: begin_exports

namespace lldb_dap {
class BaseRequestHandler;
class BreakpointBase;
class ExceptionBreakpoint;
class FunctionBreakpoint;
class InstructionBreakpoint;
class Log;
class ResponseHandler;
class SourceBreakpoint;
class Watchpoint;
struct DAP;
} // namespace lldb_dap

namespace lldb {
class SBAttachInfo;
class SBBreakpoint;
class SBBreakpointLocation;
class SBBroadcaster;
class SBCommandInterpreter;
class SBCommandReturnObject;
class SBCommunication;
class SBDebugger;
class SBEvent;
class SBFrame;
class SBHostOS;
class SBInstruction;
class SBInstructionList;
class SBLanguageRuntime;
class SBLaunchInfo;
class SBLineEntry;
class SBListener;
class SBModule;
class SBProcess;
class SBStream;
class SBStringList;
class SBTarget;
class SBThread;
class SBValue;
class SBWatchpoint;
} // namespace lldb

namespace llvm {
namespace json {
class Object;
} // namespace json
} // namespace llvm

// IWYU pragma: end_exports

#endif
