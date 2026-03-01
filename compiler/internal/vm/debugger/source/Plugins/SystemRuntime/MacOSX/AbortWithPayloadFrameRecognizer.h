//===-- AbortWithPayloadFrameRecognizer.h -----------------------*- C++ -*-===//
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
#ifndef LLDB_MACOSX_ABORTWITHPAYLOADFRAMERECOGNIZER_H
#define LLDB_MACOSX_ABORTWITHPAYLOADFRAMERECOGNIZER_H

#include "lldb/Target/Process.h"
#include "lldb/Target/StackFrameRecognizer.h"
#include "lldb/Utility/ConstString.h"
#include "lldb/Utility/FileSpec.h"

#include <tuple>

namespace lldb_private {

void RegisterAbortWithPayloadFrameRecognizer(Process *process);

class AbortWithPayloadRecognizedStackFrame : public RecognizedStackFrame {
public:
  AbortWithPayloadRecognizedStackFrame(lldb::StackFrameSP &frame_sp,
                                       lldb::ValueObjectListSP &args_sp);
};

class AbortWithPayloadFrameRecognizer : public StackFrameRecognizer {
public:
  std::string GetName() override {
    return "abort_with_payload StackFrame Recognizer";
  }
  lldb::RecognizedStackFrameSP
  RecognizeFrame(lldb::StackFrameSP frame_sp) override;
};
} // namespace lldb_private

#endif // LLDB_MACOSX_ABORTWITHPAYLOADFRAMERECOGNIZER_H
