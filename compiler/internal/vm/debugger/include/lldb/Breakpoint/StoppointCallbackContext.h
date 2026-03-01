//===-- StoppointCallbackContext.h ------------------------------*- C++ -*-===//
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

#ifndef LLDB_BREAKPOINT_STOPPOINTCALLBACKCONTEXT_H
#define LLDB_BREAKPOINT_STOPPOINTCALLBACKCONTEXT_H

#include "lldb/Target/ExecutionContext.h"
#include "lldb/lldb-private.h"

namespace lldb_private {

/// \class StoppointCallbackContext StoppointCallbackContext.h
/// "lldb/Breakpoint/StoppointCallbackContext.h" Class holds the information
/// that a breakpoint callback needs to evaluate this stop.

/// General Outline:
/// When we hit a breakpoint we need to package up whatever information is
/// needed to evaluate breakpoint commands and conditions.  This class is the
/// container of that information.

class StoppointCallbackContext {
public:
  StoppointCallbackContext();

  StoppointCallbackContext(Event *event, const ExecutionContext &exe_ctx,
                           bool synchronously = false);

  /// Clear the object's state.
  ///
  /// Sets the event, process and thread to NULL, and the frame index to an
  /// invalid value.
  void Clear();

  // Member variables
  Event *event = nullptr; // This is the event, the callback can modify this to
                          // indicate the meaning of the breakpoint hit
  ExecutionContextRef
      exe_ctx_ref;     // This tells us where we have stopped, what thread.
  bool is_synchronous =
      false; // Is the callback being executed synchronously with the
             // breakpoint,
             // or asynchronously as the event is retrieved?
};

} // namespace lldb_private

#endif // LLDB_BREAKPOINT_STOPPOINTCALLBACKCONTEXT_H
