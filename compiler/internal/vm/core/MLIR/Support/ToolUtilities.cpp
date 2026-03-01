//===- ToolUtilities.cpp - MLIR Tool Utilities ----------------------------===//
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
//
// This file defines common utilities for implementing MLIR tools.
//
//===----------------------------------------------------------------------===//

#include "mlir/Support/ToolUtilities.h"
#include "mlir/Support/LLVM.h"
#include "vm/core/Support/SourceMgr.h"
#include "vm/core/Support/raw_ostream.h"
#include <string>
#include <utility>

using namespace mlir;

LogicalResult
mlir::splitAndProcessBuffer(std::unique_ptr<toolchain::MemoryBuffer> originalBuffer,
                            ChunkBufferHandler processChunkBuffer,
                            raw_ostream &os, toolchain::StringRef inputSplitMarker,
                            toolchain::StringRef outputSplitMarker) {
  toolchain::MemoryBufferRef originalBufferRef = originalBuffer->getMemBufferRef();
  // If splitting is disabled, we process the full input buffer.
  if (inputSplitMarker.empty())
    return processChunkBuffer(std::move(originalBuffer), originalBufferRef, os);

  const int inputSplitMarkerLen = inputSplitMarker.size();

  SmallVector<StringRef, 8> rawSourceBuffers;
  const int checkLen = 2;
  // Split dropping the last checkLen chars to enable flagging near misses.
  originalBufferRef.getBuffer().split(rawSourceBuffers,
                                      inputSplitMarker.drop_back(checkLen));
  if (rawSourceBuffers.empty())
    return success();

  // Add the original buffer to the source manager.
  toolchain::SourceMgr fileSourceMgr;
  fileSourceMgr.AddNewSourceBuffer(std::move(originalBuffer), SMLoc());

  // Flag near misses by iterating over all the sub-buffers found when splitting
  // with the prefix of the splitMarker. Use a sliding window where we only add
  // a buffer as a sourceBuffer if terminated by a full match of the
  // splitMarker, else flag a warning (if near miss) and extend the size of the
  // buffer under consideration.
  SmallVector<StringRef, 8> sourceBuffers;
  StringRef prev;
  for (auto buffer : rawSourceBuffers) {
    if (prev.empty()) {
      prev = buffer;
      continue;
    }

    // Check that suffix is as expected and doesn't have any dash post.
    bool expectedSuffix =
        buffer.starts_with(inputSplitMarker.take_back(checkLen)) &&
        buffer.size() > checkLen && buffer[checkLen] != '0';
    if (expectedSuffix) {
      sourceBuffers.push_back(prev);
      prev = buffer.drop_front(checkLen);
    } else {
      // TODO: Consider making this a failure.
      auto splitLoc = SMLoc::getFromPointer(buffer.data());
      fileSourceMgr.PrintMessage(toolchain::errs(), splitLoc,
                                 toolchain::SourceMgr::DK_Warning,
                                 "near miss with file split marker");
      prev = StringRef(prev.data(), prev.size() + inputSplitMarkerLen -
                                        checkLen + buffer.size());
    }
  }
  if (!prev.empty())
    sourceBuffers.push_back(prev);

  // Process each chunk in turn.
  bool hadFailure = false;
  auto interleaveFn = [&](StringRef subBuffer) {
    auto splitLoc = SMLoc::getFromPointer(subBuffer.data());
    unsigned splitLine = fileSourceMgr.getLineAndColumn(splitLoc).first;
    std::string name((Twine("within split at ") +
                      originalBufferRef.getBufferIdentifier() + ":" +
                      Twine(splitLine) + " offset ")
                         .str());
    // Use MemoryBufferRef to avoid copying the buffer & keep at same location
    // relative to the original buffer.
    auto subMemBuffer =
        toolchain::MemoryBuffer::getMemBuffer(toolchain::MemoryBufferRef(subBuffer, name),
                                         /*RequiresNullTerminator=*/false);
    if (failed(
            processChunkBuffer(std::move(subMemBuffer), originalBufferRef, os)))
      hadFailure = true;
  };
  toolchain::interleave(sourceBuffers, os, interleaveFn,
                   (toolchain::Twine(outputSplitMarker) + "\n").str());

  // If any fails, then return a failure of the tool.
  return failure(hadFailure);
}

LogicalResult
mlir::splitAndProcessBuffer(std::unique_ptr<toolchain::MemoryBuffer> originalBuffer,
                            NoSourceChunkBufferHandler processChunkBuffer,
                            raw_ostream &os, toolchain::StringRef inputSplitMarker,
                            toolchain::StringRef outputSplitMarker) {
  auto process = [&](std::unique_ptr<toolchain::MemoryBuffer> chunkBuffer,
                     const toolchain::MemoryBufferRef &, raw_ostream &os) {
    return processChunkBuffer(std::move(chunkBuffer), os);
  };
  return splitAndProcessBuffer(std::move(originalBuffer), process, os,
                               inputSplitMarker, outputSplitMarker);
}
