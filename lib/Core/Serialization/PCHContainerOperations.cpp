//=== Serialization/PCHContainerOperations.cpp - PCH Containers -*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
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
//  This file defines PCHContainerOperations and RawPCHContainerOperation.
//
//===----------------------------------------------------------------------===//

#include "language/Core/Serialization/PCHContainerOperations.h"
#include "language/Core/AST/ASTConsumer.h"
#include "toolchain/Support/raw_ostream.h"
#include <utility>

using namespace language::Core;

PCHContainerWriter::~PCHContainerWriter() {}
PCHContainerReader::~PCHContainerReader() {}

namespace {

/// A PCHContainerGenerator that writes out the PCH to a flat file.
class RawPCHContainerGenerator : public ASTConsumer {
  std::shared_ptr<PCHBuffer> Buffer;
  std::unique_ptr<raw_pwrite_stream> OS;

public:
  RawPCHContainerGenerator(std::unique_ptr<toolchain::raw_pwrite_stream> OS,
                           std::shared_ptr<PCHBuffer> Buffer)
      : Buffer(std::move(Buffer)), OS(std::move(OS)) {}

  ~RawPCHContainerGenerator() override = default;

  void HandleTranslationUnit(ASTContext &Ctx) override {
    if (Buffer->IsComplete) {
      // Make sure it hits disk now.
      *OS << Buffer->Data;
      OS->flush();
    }
    // Free the space of the temporary buffer.
    toolchain::SmallVector<char, 0> Empty;
    Buffer->Data = std::move(Empty);
  }
};

} // anonymous namespace

std::unique_ptr<ASTConsumer> RawPCHContainerWriter::CreatePCHContainerGenerator(
    CompilerInstance &CI, const std::string &MainFileName,
    const std::string &OutputFileName, std::unique_ptr<toolchain::raw_pwrite_stream> OS,
    std::shared_ptr<PCHBuffer> Buffer) const {
  return std::make_unique<RawPCHContainerGenerator>(std::move(OS), Buffer);
}

ArrayRef<toolchain::StringRef> RawPCHContainerReader::getFormats() const {
  static StringRef Raw("raw");
  return ArrayRef(Raw);
}

StringRef
RawPCHContainerReader::ExtractPCH(toolchain::MemoryBufferRef Buffer) const {
  return Buffer.getBuffer();
}

PCHContainerOperations::PCHContainerOperations() {
  registerWriter(std::make_unique<RawPCHContainerWriter>());
  registerReader(std::make_unique<RawPCHContainerReader>());
}
