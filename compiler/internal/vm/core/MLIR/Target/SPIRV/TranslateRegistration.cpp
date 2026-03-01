//===- TranslateRegistration.cpp - hooks to mlir-translate ----------------===//
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
// This file implements a translation from SPIR-V binary module to MLIR SPIR-V
// ModuleOp.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/SPIRV/IR/SPIRVDialect.h"
#include "mlir/Dialect/SPIRV/IR/SPIRVOps.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/Verifier.h"
#include "mlir/Parser/Parser.h"
#include "mlir/Support/FileUtilities.h"
#include "mlir/Target/SPIRV/Deserialization.h"
#include "mlir/Target/SPIRV/Serialization.h"
#include "mlir/Tools/mlir-translate/Translation.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/Support/FileSystem.h"
#include "vm/core/Support/MemoryBuffer.h"
#include "vm/core/Support/Path.h"
#include "vm/core/Support/SourceMgr.h"
#include "vm/core/Support/ToolOutputFile.h"

using namespace mlir;

//===----------------------------------------------------------------------===//
// Deserialization registration
//===----------------------------------------------------------------------===//

// Deserializes the SPIR-V binary module stored in the file named as
// `inputFilename` and returns a module containing the SPIR-V module.
static OwningOpRef<Operation *>
deserializeModule(const toolchain::MemoryBuffer *input, MLIRContext *context,
                  const spirv::DeserializationOptions &options) {
  context->loadDialect<spirv::SPIRVDialect>();

  // Make sure the input stream can be treated as a stream of SPIR-V words
  auto *start = input->getBufferStart();
  auto size = input->getBufferSize();
  if (size % sizeof(uint32_t) != 0) {
    emitError(UnknownLoc::get(context))
        << "SPIR-V binary module must contain integral number of 32-bit words";
    return {};
  }

  auto binary = toolchain::ArrayRef(reinterpret_cast<const uint32_t *>(start),
                               size / sizeof(uint32_t));
  return spirv::deserialize(binary, context, options);
}

namespace mlir {
void registerFromSPIRVTranslation() {
  static toolchain::cl::opt<bool> enableControlFlowStructurization(
      "spirv-structurize-control-flow",
      toolchain::cl::desc(
          "Enable control flow structurization into `spirv.mlir.selection` and "
          "`spirv.mlir.loop`. This may need to be disabled to support "
          "deserialization of early exits (see #138688)"),
      toolchain::cl::init(true));

  TranslateToMLIRRegistration fromBinary(
      "deserialize-spirv", "deserializes the SPIR-V module",
      [](toolchain::SourceMgr &sourceMgr, MLIRContext *context) {
        assert(sourceMgr.getNumBuffers() == 1 && "expected one buffer");
        return deserializeModule(
            sourceMgr.getMemoryBuffer(sourceMgr.getMainFileID()), context,
            {enableControlFlowStructurization});
      });
}
} // namespace mlir

//===----------------------------------------------------------------------===//
// Serialization registration
//===----------------------------------------------------------------------===//

static LogicalResult
serializeModule(spirv::ModuleOp moduleOp, raw_ostream &output,
                const spirv::SerializationOptions &options) {
  SmallVector<uint32_t, 0> binary;
  if (failed(spirv::serialize(moduleOp, binary)))
    return failure();

  size_t sizeInBytes = binary.size() * sizeof(uint32_t);

  output.write(reinterpret_cast<char *>(binary.data()), sizeInBytes);

  if (options.saveModuleForValidation) {
    size_t dirSeparator =
        options.validationFilePrefix.find(toolchain::sys::path::get_separator());
    // If file prefix includes directory check if that directory exists.
    if (dirSeparator != std::string::npos) {
      toolchain::StringRef parentDir =
          toolchain::sys::path::parent_path(options.validationFilePrefix);
      if (!toolchain::sys::fs::is_directory(parentDir))
        return moduleOp.emitError(
            "validation prefix directory does not exist\n");
    }

    SmallString<128> filename;
    int fd = 0;

    std::error_code errorCode = toolchain::sys::fs::createUniqueFile(
        options.validationFilePrefix + "%%%%%%.spv", fd, filename);
    if (errorCode)
      return moduleOp.emitError("error creating validation output file: ")
             << errorCode.message() << "\n";

    toolchain::raw_fd_ostream validationOutput(fd, /*shouldClose=*/true);
    validationOutput.write(reinterpret_cast<char *>(binary.data()),
                           sizeInBytes);
    validationOutput.flush();
  }

  return mlir::success();
}

namespace mlir {
void registerToSPIRVTranslation() {
  static toolchain::cl::opt<std::string> validationFilesPrefix(
      "spirv-save-validation-files-with-prefix",
      toolchain::cl::desc(
          "When non-empty string is passed each serialized SPIR-V module is "
          "saved to an additional file that starts with the given prefix. This "
          "is used to generate separate binaries for validation, where "
          "`--split-input-file` normally combines all outputs into one. The "
          "one combined output (`-o`) is still written. Created files need to "
          "be removed manually once processed."),
      toolchain::cl::init(""));

  TranslateFromMLIRRegistration toBinary(
      "serialize-spirv", "serialize SPIR-V dialect",
      [](spirv::ModuleOp moduleOp, raw_ostream &output) {
        return serializeModule(moduleOp, output,
                               {true, false, !validationFilesPrefix.empty(),
                                validationFilesPrefix});
      },
      [](DialectRegistry &registry) {
        registry.insert<spirv::SPIRVDialect>();
      });
}
} // namespace mlir

//===----------------------------------------------------------------------===//
// Round-trip registration
//===----------------------------------------------------------------------===//

static LogicalResult roundTripModule(spirv::ModuleOp module, bool emitDebugInfo,
                                     raw_ostream &output) {
  SmallVector<uint32_t, 0> binary;
  MLIRContext *context = module->getContext();

  spirv::SerializationOptions options;
  options.emitDebugInfo = emitDebugInfo;
  if (failed(spirv::serialize(module, binary, options)))
    return failure();

  MLIRContext deserializationContext(context->getDialectRegistry());
  // TODO: we should only load the required dialects instead of all dialects.
  deserializationContext.loadAllAvailableDialects();
  // Then deserialize to get back a SPIR-V module.
  OwningOpRef<spirv::ModuleOp> spirvModule =
      spirv::deserialize(binary, &deserializationContext);
  if (!spirvModule)
    return failure();
  spirvModule->print(output);

  return mlir::success();
}

namespace mlir {
void registerTestRoundtripSPIRV() {
  TranslateFromMLIRRegistration roundtrip(
      "test-spirv-roundtrip", "test roundtrip in SPIR-V dialect",
      [](spirv::ModuleOp module, raw_ostream &output) {
        return roundTripModule(module, /*emitDebugInfo=*/false, output);
      },
      [](DialectRegistry &registry) {
        registry.insert<spirv::SPIRVDialect>();
      });
}

void registerTestRoundtripDebugSPIRV() {
  TranslateFromMLIRRegistration roundtrip(
      "test-spirv-roundtrip-debug", "test roundtrip debug in SPIR-V",
      [](spirv::ModuleOp module, raw_ostream &output) {
        return roundTripModule(module, /*emitDebugInfo=*/true, output);
      },
      [](DialectRegistry &registry) {
        registry.insert<spirv::SPIRVDialect>();
      });
}
} // namespace mlir
