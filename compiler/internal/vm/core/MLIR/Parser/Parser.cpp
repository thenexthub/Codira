//===- Parser.cpp - MLIR Unified Parser Interface -------------------------===//
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
// This file implements the parser for the MLIR textual form.
//
//===----------------------------------------------------------------------===//

#include "mlir/Parser/Parser.h"
#include "mlir/AsmParser/AsmParser.h"
#include "mlir/Bytecode/BytecodeReader.h"
#include "vm/core/Support/SourceMgr.h"

using namespace mlir;

static std::pair<int64_t, int64_t>
getLineAndColStart(const toolchain::SourceMgr &sourceMgr) {
  unsigned lastFileID = sourceMgr.getNumBuffers();
  if (lastFileID == 1)
    return {0, 0};

  auto bufferID = sourceMgr.getMainFileID();
  const toolchain::MemoryBuffer *main = sourceMgr.getMemoryBuffer(bufferID);
  const toolchain::MemoryBuffer *last = sourceMgr.getMemoryBuffer(lastFileID);
  // Exclude same start.
  if (main->getBufferStart() < last->getBufferStart() &&
      main->getBufferEnd() >= last->getBufferEnd()) {
    return sourceMgr.getLineAndColumn(
        toolchain::SMLoc::getFromPointer(last->getBufferStart()), bufferID);
  }
  return {0, 0};
}

LogicalResult mlir::parseSourceFile(const toolchain::SourceMgr &sourceMgr,
                                    Block *block, const ParserConfig &config,
                                    LocationAttr *sourceFileLoc) {
  const auto *sourceBuf = sourceMgr.getMemoryBuffer(sourceMgr.getMainFileID());
  if (sourceFileLoc) {
    auto [line, column] = getLineAndColStart(sourceMgr);
    *sourceFileLoc = FileLineColLoc::get(
        config.getContext(), sourceBuf->getBufferIdentifier(), line, column);
  }
  if (isBytecode(*sourceBuf))
    return readBytecodeFile(*sourceBuf, block, config);
  return parseAsmSourceFile(sourceMgr, block, config);
}
LogicalResult
mlir::parseSourceFile(const std::shared_ptr<toolchain::SourceMgr> &sourceMgr,
                      Block *block, const ParserConfig &config,
                      LocationAttr *sourceFileLoc) {
  const auto *sourceBuf =
      sourceMgr->getMemoryBuffer(sourceMgr->getMainFileID());
  if (sourceFileLoc) {
    auto [line, column] = getLineAndColStart(*sourceMgr);
    *sourceFileLoc = FileLineColLoc::get(
        config.getContext(), sourceBuf->getBufferIdentifier(), line, column);
  }
  if (isBytecode(*sourceBuf))
    return readBytecodeFile(sourceMgr, block, config);
  return parseAsmSourceFile(*sourceMgr, block, config);
}

LogicalResult mlir::parseSourceFile(toolchain::StringRef filename, Block *block,
                                    const ParserConfig &config,
                                    LocationAttr *sourceFileLoc) {
  auto sourceMgr = std::make_shared<toolchain::SourceMgr>();
  return parseSourceFile(filename, sourceMgr, block, config, sourceFileLoc);
}

static LogicalResult loadSourceFileBuffer(toolchain::StringRef filename,
                                          toolchain::SourceMgr &sourceMgr,
                                          MLIRContext *ctx) {
  if (sourceMgr.getNumBuffers() != 0) {
    // TODO: Extend to support multiple buffers.
    return emitError(mlir::UnknownLoc::get(ctx),
                     "only main buffer parsed at the moment");
  }
  auto fileOrErr = toolchain::MemoryBuffer::getFileOrSTDIN(filename);
  if (fileOrErr.getError())
    return emitError(mlir::UnknownLoc::get(ctx),
                     "could not open input file " + filename);

  // Load the MLIR source file.
  sourceMgr.AddNewSourceBuffer(std::move(*fileOrErr), SMLoc());
  return success();
}

LogicalResult mlir::parseSourceFile(toolchain::StringRef filename,
                                    toolchain::SourceMgr &sourceMgr, Block *block,
                                    const ParserConfig &config,
                                    LocationAttr *sourceFileLoc) {
  if (failed(loadSourceFileBuffer(filename, sourceMgr, config.getContext())))
    return failure();
  return parseSourceFile(sourceMgr, block, config, sourceFileLoc);
}
LogicalResult mlir::parseSourceFile(
    toolchain::StringRef filename, const std::shared_ptr<toolchain::SourceMgr> &sourceMgr,
    Block *block, const ParserConfig &config, LocationAttr *sourceFileLoc) {
  if (failed(loadSourceFileBuffer(filename, *sourceMgr, config.getContext())))
    return failure();
  return parseSourceFile(sourceMgr, block, config, sourceFileLoc);
}

LogicalResult mlir::parseSourceString(toolchain::StringRef sourceStr, Block *block,
                                      const ParserConfig &config,
                                      StringRef sourceName,
                                      LocationAttr *sourceFileLoc) {
  auto memBuffer =
      toolchain::MemoryBuffer::getMemBuffer(sourceStr, sourceName,
                                       /*RequiresNullTerminator=*/false);
  if (!memBuffer)
    return failure();

  toolchain::SourceMgr sourceMgr;
  sourceMgr.AddNewSourceBuffer(std::move(memBuffer), SMLoc());
  return parseSourceFile(sourceMgr, block, config, sourceFileLoc);
}
