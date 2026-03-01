//===- FileUtilities.cpp - utilities for working with files ---------------===//
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
// Definitions of common utilities for working with files.
//
//===----------------------------------------------------------------------===//

#include "mlir/Support/FileUtilities.h"
#include "mlir/Support/LLVM.h"
#include "vm/core/Support/Alignment.h"
#include "vm/core/Support/FileUtilities.h"
#include "vm/core/Support/MemoryBuffer.h"
#include "vm/core/Support/ToolOutputFile.h"

using namespace mlir;

static std::unique_ptr<toolchain::MemoryBuffer>
openInputFileImpl(StringRef inputFilename, std::string *errorMessage,
                  std::optional<toolchain::Align> alignment) {
  auto fileOrErr = toolchain::MemoryBuffer::getFileOrSTDIN(
      inputFilename, /*IsText=*/false, /*RequiresNullTerminator=*/true,
      alignment);
  if (std::error_code error = fileOrErr.getError()) {
    if (errorMessage)
      *errorMessage = "cannot open input file '" + inputFilename.str() +
                      "': " + error.message();
    return nullptr;
  }

  return std::move(*fileOrErr);
}
std::unique_ptr<toolchain::MemoryBuffer>
mlir::openInputFile(StringRef inputFilename, std::string *errorMessage) {
  return openInputFileImpl(inputFilename, errorMessage,
                           /*alignment=*/std::nullopt);
}
std::unique_ptr<toolchain::MemoryBuffer>
mlir::openInputFile(toolchain::StringRef inputFilename, toolchain::Align alignment,
                    std::string *errorMessage) {
  return openInputFileImpl(inputFilename, errorMessage, alignment);
}

std::unique_ptr<toolchain::ToolOutputFile>
mlir::openOutputFile(StringRef outputFilename, std::string *errorMessage) {
  std::error_code error;
  auto result = std::make_unique<toolchain::ToolOutputFile>(outputFilename, error,
                                                       toolchain::sys::fs::OF_None);
  if (error) {
    if (errorMessage)
      *errorMessage = "cannot open output file '" + outputFilename.str() +
                      "': " + error.message();
    return nullptr;
  }

  return result;
}
