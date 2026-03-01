//===-- AssertFrameRecognizer.cpp -------------------------------*- C++ -*-===//
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

#ifndef LLDB_TARGET_ASSERTFRAMERECOGNIZER_H
#define LLDB_TARGET_ASSERTFRAMERECOGNIZER_H

#include "lldb/Target/Process.h"
#include "lldb/Target/StackFrameRecognizer.h"
#include "lldb/Utility/ConstString.h"
#include "lldb/Utility/FileSpec.h"

#include <tuple>

namespace lldb_private {

/// Registers the assert stack frame recognizer.
///
/// \param[in] process
///    The process that is currently asserting. This will give us information on
///    the target and the platform.
void RegisterAssertFrameRecognizer(Process *process);

/// \class AssertRecognizedStackFrame
///
/// Holds the stack frame where the assert is called from.
class AssertRecognizedStackFrame : public RecognizedStackFrame {
public:
  AssertRecognizedStackFrame(lldb::StackFrameSP most_relevant_frame_sp);
  lldb::StackFrameSP GetMostRelevantFrame() override;

private:
  lldb::StackFrameSP m_most_relevant_frame;
};

/// \class AssertFrameRecognizer
///
/// When a thread stops, it checks depending on the platform if the top frame is
/// an abort stack frame. If so, it looks for an assert stack frame in the upper
/// frames and set it as the most relavant frame when found.
class AssertFrameRecognizer : public StackFrameRecognizer {
public:
  std::string GetName() override { return "Assert StackFrame Recognizer"; }
  lldb::RecognizedStackFrameSP
  RecognizeFrame(lldb::StackFrameSP frame_sp) override;
};

} // namespace lldb_private

#endif // LLDB_TARGET_ASSERTFRAMERECOGNIZER_H
