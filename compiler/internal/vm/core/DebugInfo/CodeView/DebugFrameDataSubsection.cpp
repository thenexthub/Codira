//===- DebugFrameDataSubsection.cpp -----------------------------*- C++ -*-===//
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

#include "vm/core/DebugInfo/CodeView/DebugFrameDataSubsection.h"
#include "vm/core/DebugInfo/CodeView/CodeViewError.h"
#include "vm/core/Support/BinaryStreamReader.h"
#include "vm/core/Support/BinaryStreamWriter.h"

using namespace vm::core;
using namespace vm::core::codeview;

Error DebugFrameDataSubsectionRef::initialize(BinaryStreamReader Reader) {
  if (Reader.bytesRemaining() % sizeof(FrameData) != 0) {
    if (auto EC = Reader.readObject(RelocPtr))
      return EC;
  }

  if (Reader.bytesRemaining() % sizeof(FrameData) != 0)
    return make_error<CodeViewError>(cv_error_code::corrupt_record,
                                     "Invalid frame data record format!");

  uint32_t Count = Reader.bytesRemaining() / sizeof(FrameData);
  if (auto EC = Reader.readArray(Frames, Count))
    return EC;
  return Error::success();
}

Error DebugFrameDataSubsectionRef::initialize(BinaryStreamRef Section) {
  BinaryStreamReader Reader(Section);
  return initialize(Reader);
}

uint32_t DebugFrameDataSubsection::calculateSerializedSize() const {
  uint32_t Size = sizeof(FrameData) * Frames.size();
  if (IncludeRelocPtr)
    Size += sizeof(uint32_t);
  return Size;
}

Error DebugFrameDataSubsection::commit(BinaryStreamWriter &Writer) const {
  if (IncludeRelocPtr) {
    if (auto EC = Writer.writeInteger<uint32_t>(0))
      return EC;
  }

  std::vector<FrameData> SortedFrames(Frames.begin(), Frames.end());
  toolchain::sort(SortedFrames, [](const FrameData &LHS, const FrameData &RHS) {
    return LHS.RvaStart < RHS.RvaStart;
  });
  if (auto EC = Writer.writeArray(ArrayRef(SortedFrames)))
    return EC;
  return Error::success();
}

void DebugFrameDataSubsection::addFrameData(const FrameData &Frame) {
  Frames.push_back(Frame);
}
