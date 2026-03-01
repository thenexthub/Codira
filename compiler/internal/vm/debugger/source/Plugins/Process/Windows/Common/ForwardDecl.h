//===-- ForwardDecl.h -------------------------------------------*- C++ -*-===//
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

#ifndef liblldb_Plugins_Process_Windows_ForwardDecl_H_
#define liblldb_Plugins_Process_Windows_ForwardDecl_H_

#include <memory>

// ExceptionResult is returned by the debug delegate to specify how it processed
// the exception.
enum class ExceptionResult {
  BreakInDebugger,  // Break in the debugger and give the user a chance to
                    // interact with
                    // the program before continuing.
  MaskException,    // Eat the exception and don't let the application know it
                    // occurred.
  SendToApplication // Send the exception to the application to be handled as if
                    // there were
                    // no debugger attached.
};

namespace lldb_private {

class ProcessWindows;

class IDebugDelegate;
class DebuggerThread;
class ExceptionRecord;

typedef std::shared_ptr<IDebugDelegate> DebugDelegateSP;
typedef std::shared_ptr<DebuggerThread> DebuggerThreadSP;
typedef std::shared_ptr<ExceptionRecord> ExceptionRecordSP;
typedef std::unique_ptr<ExceptionRecord> ExceptionRecordUP;
}

#endif
