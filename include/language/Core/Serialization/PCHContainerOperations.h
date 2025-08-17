//===-- PCHContainerOperations.h - PCH Containers ---------------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_SERIALIZATION_PCHCONTAINEROPERATIONS_H
#define LANGUAGE_CORE_SERIALIZATION_PCHCONTAINEROPERATIONS_H

#include "language/Core/Basic/Module.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/ADT/StringMap.h"
#include "toolchain/Support/MemoryBufferRef.h"
#include <memory>

namespace toolchain {
class raw_pwrite_stream;
}

namespace language::Core {

class ASTConsumer;
class CompilerInstance;

struct PCHBuffer {
  ASTFileSignature Signature;
  toolchain::SmallVector<char, 0> Data;
  bool IsComplete;
};

/// This abstract interface provides operations for creating
/// containers for serialized ASTs (precompiled headers and clang
/// modules).
class PCHContainerWriter {
public:
  virtual ~PCHContainerWriter() = 0;
  virtual toolchain::StringRef getFormat() const = 0;

  /// Return an ASTConsumer that can be chained with a
  /// PCHGenerator that produces a wrapper file format containing a
  /// serialized AST bitstream.
  virtual std::unique_ptr<ASTConsumer>
  CreatePCHContainerGenerator(CompilerInstance &CI,
                              const std::string &MainFileName,
                              const std::string &OutputFileName,
                              std::unique_ptr<toolchain::raw_pwrite_stream> OS,
                              std::shared_ptr<PCHBuffer> Buffer) const = 0;
};

/// This abstract interface provides operations for unwrapping
/// containers for serialized ASTs (precompiled headers and clang
/// modules).
class PCHContainerReader {
public:
  virtual ~PCHContainerReader() = 0;
  /// Equivalent to the format passed to -fmodule-format=
  virtual toolchain::ArrayRef<toolchain::StringRef> getFormats() const = 0;

  /// Returns the serialized AST inside the PCH container Buffer.
  virtual toolchain::StringRef ExtractPCH(toolchain::MemoryBufferRef Buffer) const = 0;
};

/// Implements write operations for a raw pass-through PCH container.
class RawPCHContainerWriter : public PCHContainerWriter {
  toolchain::StringRef getFormat() const override { return "raw"; }

  /// Return an ASTConsumer that can be chained with a
  /// PCHGenerator that writes the module to a flat file.
  std::unique_ptr<ASTConsumer>
  CreatePCHContainerGenerator(CompilerInstance &CI,
                              const std::string &MainFileName,
                              const std::string &OutputFileName,
                              std::unique_ptr<toolchain::raw_pwrite_stream> OS,
                              std::shared_ptr<PCHBuffer> Buffer) const override;
};

/// Implements read operations for a raw pass-through PCH container.
class RawPCHContainerReader : public PCHContainerReader {
  toolchain::ArrayRef<toolchain::StringRef> getFormats() const override;
  /// Simply returns the buffer contained in Buffer.
  toolchain::StringRef ExtractPCH(toolchain::MemoryBufferRef Buffer) const override;
};

/// A registry of PCHContainerWriter and -Reader objects for different formats.
class PCHContainerOperations {
  toolchain::StringMap<std::unique_ptr<PCHContainerWriter>> Writers;
  toolchain::StringMap<PCHContainerReader *> Readers;
  toolchain::SmallVector<std::unique_ptr<PCHContainerReader>> OwnedReaders;

public:
  /// Automatically registers a RawPCHContainerWriter and
  /// RawPCHContainerReader.
  PCHContainerOperations();
  void registerWriter(std::unique_ptr<PCHContainerWriter> Writer) {
    Writers[Writer->getFormat()] = std::move(Writer);
  }
  void registerReader(std::unique_ptr<PCHContainerReader> Reader) {
    assert(!Reader->getFormats().empty() &&
           "PCHContainerReader must handle >=1 format");
    for (toolchain::StringRef Fmt : Reader->getFormats())
      Readers[Fmt] = Reader.get();
    OwnedReaders.push_back(std::move(Reader));
  }
  const PCHContainerWriter *getWriterOrNull(toolchain::StringRef Format) {
    return Writers[Format].get();
  }
  const PCHContainerReader *getReaderOrNull(toolchain::StringRef Format) {
    return Readers[Format];
  }
  const PCHContainerReader &getRawReader() {
    return *getReaderOrNull("raw");
  }
};

}

#endif
