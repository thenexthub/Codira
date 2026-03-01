//===- PDB.h ----------------------------------------------------*- C++ -*-===//
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

#ifndef LLD_COFF_PDB_H
#define LLD_COFF_PDB_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringRef.h"
#include <optional>

namespace llvm::codeview {
union DebugInfo;
}

namespace lld {
class Timer;

namespace coff {
class SectionChunk;
class COFFLinkerContext;

void createPDB(COFFLinkerContext &ctx, llvm::ArrayRef<uint8_t> sectionTable,
               llvm::codeview::DebugInfo *buildId);

std::optional<std::pair<llvm::StringRef, uint32_t>>
getFileLineCodeView(const SectionChunk *c, uint32_t addr);

// For statistics
struct PDBStats {
  uint64_t globalSymbols = 0;
  uint64_t moduleSymbols = 0;
  uint64_t publicSymbols = 0;
  uint64_t nbTypeRecords = 0;
  uint64_t nbTypeRecordsBytes = 0;
  uint64_t nbTPIrecords = 0;
  uint64_t nbIPIrecords = 0;
  uint64_t strTabSize = 0;
  std::string largeInputTypeRecs;
};

} // namespace coff
} // namespace lld

#endif
