//===- Writer.h -------------------------------------------------*- C++ -*-===//
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

#ifndef LLD_COFF_WRITER_H
#define LLD_COFF_WRITER_H

#include "Chunks.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Object/COFF.h"
#include <chrono>
#include <cstdint>
#include <vector>

namespace lld::coff {
static const int pageSize = 4096;
class COFFLinkerContext;

void writeResult(COFFLinkerContext &ctx);

class PartialSection {
public:
  PartialSection(StringRef n, uint32_t chars)
      : name(n), characteristics(chars) {}
  StringRef name;
  unsigned characteristics;
  std::vector<Chunk *> chunks;
};

// OutputSection represents a section in an output file. It's a
// container of chunks. OutputSection and Chunk are 1:N relationship.
// Chunks cannot belong to more than one OutputSections. The writer
// creates multiple OutputSections and assign them unique,
// non-overlapping file offsets and RVAs.
class OutputSection {
public:
  OutputSection(llvm::StringRef n, uint32_t chars) : name(n) {
    header.Characteristics = chars;
  }
  void addChunk(Chunk *c);
  void insertChunkAtStart(Chunk *c);
  void merge(OutputSection *other);
  void setPermissions(uint32_t c);
  uint64_t getRVA() const { return header.VirtualAddress; }
  uint64_t getFileOff() const { return header.PointerToRawData; }
  void writeHeaderTo(uint8_t *buf, bool isDebug);
  void addContributingPartialSection(PartialSection *sec);

  // Sort chunks to split native and EC sections on hybrid targets.
  void splitECChunks();

  // Returns the size of this section in an executable memory image.
  // This may be smaller than the raw size (the raw size is multiple
  // of disk sector size, so there may be padding at end), or may be
  // larger (if that's the case, the loader reserves spaces after end
  // of raw data).
  uint64_t getVirtualSize() { return header.VirtualSize; }

  // Returns the size of the section in the output file.
  uint64_t getRawSize() { return header.SizeOfRawData; }

  // Set offset into the string table storing this section name.
  // Used only when the name is longer than 8 bytes.
  void setStringTableOff(uint32_t v) { stringTableOff = v; }

  bool isCodeSection() const {
    return (header.Characteristics & llvm::COFF::IMAGE_SCN_CNT_CODE) &&
           (header.Characteristics & llvm::COFF::IMAGE_SCN_MEM_READ) &&
           (header.Characteristics & llvm::COFF::IMAGE_SCN_MEM_EXECUTE);
  }

  // N.B. The section index is one based.
  uint32_t sectionIndex = 0;

  llvm::StringRef name;
  llvm::object::coff_section header = {};

  std::vector<Chunk *> chunks;
  std::vector<Chunk *> origChunks;

  std::vector<PartialSection *> contribSections;

private:
  uint32_t stringTableOff = 0;
};

} // namespace lld::coff

#endif
