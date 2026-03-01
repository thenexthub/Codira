//===-------- JITLink_DWARFRecordSectionSplitter.cpp - JITLink-------------===//
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

#include "vm/core/ExecutionEngine/JITLink/DWARFRecordSectionSplitter.h"
#include "vm/core/Support/BinaryStreamReader.h"

#define DEBUG_TYPE "jitlink"

namespace vm::core {
namespace jitlink {

DWARFRecordSectionSplitter::DWARFRecordSectionSplitter(StringRef SectionName)
    : SectionName(SectionName) {}

Error DWARFRecordSectionSplitter::operator()(LinkGraph &G) {
  auto *Section = G.findSectionByName(SectionName);

  if (!Section) {
    LLVM_DEBUG({
      dbgs() << "DWARFRecordSectionSplitter: No " << SectionName
             << " section. Nothing to do\n";
    });
    return Error::success();
  }

  LLVM_DEBUG({
    dbgs() << "DWARFRecordSectionSplitter: Processing " << SectionName
           << "...\n";
  });

  DenseMap<Block *, LinkGraph::SplitBlockCache> Caches;

  {
    // Pre-build the split caches.
    for (auto *B : Section->blocks())
      Caches[B] = LinkGraph::SplitBlockCache::value_type();
    for (auto *Sym : Section->symbols())
      Caches[&Sym->getBlock()]->push_back(Sym);
    for (auto *B : Section->blocks())
      toolchain::sort(*Caches[B], [](const Symbol *LHS, const Symbol *RHS) {
        return LHS->getOffset() > RHS->getOffset();
      });
  }

  // Iterate over blocks (we do this by iterating over Caches entries rather
  // than Section->blocks() as we will be inserting new blocks along the way,
  // which would invalidate iterators in the latter sequence.
  for (auto &KV : Caches) {
    auto &B = *KV.first;
    auto &BCache = KV.second;
    if (auto Err = processBlock(G, B, BCache))
      return Err;
  }

  return Error::success();
}

Error DWARFRecordSectionSplitter::processBlock(
    LinkGraph &G, Block &B, LinkGraph::SplitBlockCache &Cache) {
  LLVM_DEBUG(dbgs() << "  Processing block at " << B.getAddress() << "\n");

  // Section should not contain zero-fill blocks.
  if (B.isZeroFill())
    return make_error<JITLinkError>("Unexpected zero-fill block in " +
                                    SectionName + " section");

  if (B.getSize() == 0) {
    LLVM_DEBUG(dbgs() << "    Block is empty. Skipping.\n");
    return Error::success();
  }

  BinaryStreamReader BlockReader(
      StringRef(B.getContent().data(), B.getContent().size()),
      G.getEndianness());

  std::vector<Edge::OffsetT> SplitOffsets;
  while (true) {
    LLVM_DEBUG({
      dbgs() << "    Processing CFI record at "
             << (B.getAddress() + BlockReader.getOffset()) << "\n";
    });

    uint32_t Length;
    if (auto Err = BlockReader.readInteger(Length))
      return Err;
    if (Length != 0xffffffff) {
      if (auto Err = BlockReader.skip(Length))
        return Err;
    } else {
      uint64_t ExtendedLength;
      if (auto Err = BlockReader.readInteger(ExtendedLength))
        return Err;
      if (auto Err = BlockReader.skip(ExtendedLength))
        return Err;
    }

    // If this was the last block then there's nothing more to split
    if (BlockReader.empty())
      break;

    SplitOffsets.push_back(BlockReader.getOffset());
  }

  G.splitBlock(B, SplitOffsets);

  return Error::success();
}

} // namespace jitlink
} // namespace vm::core
