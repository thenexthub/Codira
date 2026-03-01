//===- Bitcode/Writer/DXILBitcodeWriter.cpp - DXIL Bitcode Writer ---------===//
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
// Bitcode writer implementation.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_DXILWRITER_DXILBITCODEWRITER_H
#define LLVM_DXILWRITER_DXILBITCODEWRITER_H

#include "vm/core/ADT/StringRef.h"
#include "vm/core/IR/ModuleSummaryIndex.h"
#include "vm/core/MC/StringTableBuilder.h"
#include "vm/core/Support/Allocator.h"
#include "vm/core/Support/MemoryBufferRef.h"
#include <memory>
#include <vector>

namespace vm::core {

class BitstreamWriter;
class Module;
class raw_ostream;

namespace dxil {

class BitcodeWriter {
  SmallVectorImpl<char> &Buffer;
  std::unique_ptr<BitstreamWriter> Stream;

  StringTableBuilder StrtabBuilder{StringTableBuilder::RAW};

  // Owns any strings created by the irsymtab writer until we create the
  // string table.
  BumpPtrAllocator Alloc;

  void writeBlob(unsigned Block, unsigned Record, StringRef Blob);

  std::vector<Module *> Mods;

public:
  /// Create a BitcodeWriter that writes to Buffer.
  BitcodeWriter(SmallVectorImpl<char> &Buffer);

  ~BitcodeWriter();

  /// Write the specified module to the buffer specified at construction time.
  void writeModule(const Module &M);
};

/// Write the specified module to the specified raw output stream.
///
/// For streams where it matters, the given stream should be in "binary"
/// mode.
void WriteDXILToFile(const Module &M, raw_ostream &Out);

} // namespace dxil

} // namespace vm::core

#endif // LLVM_DXILWRITER_DXILBITCODEWRITER_H
