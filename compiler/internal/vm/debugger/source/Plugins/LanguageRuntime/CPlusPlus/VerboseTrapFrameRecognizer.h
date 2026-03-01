//===-- VerboseTrapFrameRecognizer.h --------------------------------------===//
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

#ifndef LLDB_SOURCE_PLUGINS_LANGUAGERUNTIME_C_PLUS_PLUS_VERBOSETRAPFRAMERECOGNIZER_H
#define LLDB_SOURCE_PLUGINS_LANGUAGERUNTIME_C_PLUS_PLUS_VERBOSETRAPFRAMERECOGNIZER_H

#include "lldb/Target/StackFrameRecognizer.h"

namespace lldb_private {

void RegisterVerboseTrapFrameRecognizer(Process &process);

/// Holds the stack frame that caused the Verbose trap and the inlined stop
/// reason message.
class VerboseTrapRecognizedStackFrame : public RecognizedStackFrame {
public:
  VerboseTrapRecognizedStackFrame(lldb::StackFrameSP most_relevant_frame_sp,
                                  std::string stop_desc);

  lldb::StackFrameSP GetMostRelevantFrame() override;

private:
  lldb::StackFrameSP m_most_relevant_frame;
};

/// When a thread stops, it checks the current frame contains a
/// Verbose Trap diagnostic. If so, it returns a \a
/// VerboseTrapRecognizedStackFrame holding the diagnostic a stop reason
/// description with and the parent frame as the most relavant frame.
class VerboseTrapFrameRecognizer : public StackFrameRecognizer {
public:
  std::string GetName() override {
    return "Verbose Trap StackFrame Recognizer";
  }

  lldb::RecognizedStackFrameSP
  RecognizeFrame(lldb::StackFrameSP frame) override;
};

} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_LANGUAGERUNTIME_C_PLUS_PLUS_VERBOSETRAPFRAMERECOGNIZER_H
