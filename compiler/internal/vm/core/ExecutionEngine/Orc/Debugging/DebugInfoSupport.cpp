//===--- DebugInfoSupport.cpp -- Utils for debug info support ---*- C++ -*-===//
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
// Utilities to preserve and parse debug info from LinkGraphs.
//
//===----------------------------------------------------------------------===//

#include "vm/core/ExecutionEngine/Orc/Debugging/DebugInfoSupport.h"

#include "vm/core/Support/SmallVectorMemoryBuffer.h"

#define DEBUG_TYPE "orc"

using namespace vm::core;
using namespace vm::core::orc;
using namespace vm::core::jitlink;

namespace {
static DenseSet<StringRef> DWARFSectionNames = {
#define HANDLE_DWARF_SECTION(ENUM_NAME, ELF_NAME, CMDLINE_NAME, OPTION)        \
  StringRef(ELF_NAME),
#include "vm/core/BinaryFormat/Dwarf.def"
#undef HANDLE_DWARF_SECTION
};

// We might be able to drop relocations to symbols that do end up
// being pruned by the linker, but for now we just preserve all
static void preserveDWARFSection(LinkGraph &G, Section &Sec) {
  DenseMap<Block *, Symbol *> Preserved;
  for (auto Sym : Sec.symbols()) {
    auto [It, Inserted] = Preserved.try_emplace(&Sym->getBlock());
    if (Inserted || Sym->isLive())
      It->second = Sym;
  }
  for (auto Block : Sec.blocks()) {
    auto &PSym = Preserved[Block];
    if (!PSym)
      PSym = &G.addAnonymousSymbol(*Block, 0, 0, false, true);
    else if (!PSym->isLive())
      PSym->setLive(true);
  }
}

static SmallVector<char, 0> getSectionData(Section &Sec) {
  SmallVector<char, 0> SecData;
  SmallVector<Block *, 8> SecBlocks(Sec.blocks().begin(), Sec.blocks().end());
  std::sort(SecBlocks.begin(), SecBlocks.end(), [](Block *LHS, Block *RHS) {
    return LHS->getAddress() < RHS->getAddress();
  });
  // Convert back to what object file would have, one blob of section content
  // Assumes all zerofill
  // TODO handle alignment?
  // TODO handle alignment offset?
  for (auto *Block : SecBlocks) {
    if (Block->isZeroFill())
      SecData.resize(SecData.size() + Block->getSize(), 0);
    else
      SecData.append(Block->getContent().begin(), Block->getContent().end());
  }
  return SecData;
}

static void dumpDWARFContext(DWARFContext &DC) {
  auto options = toolchain::DIDumpOptions();
  options.DumpType &= ~DIDT_UUID;
  options.DumpType &= ~(1 << DIDT_ID_DebugFrame);
  LLVM_DEBUG(DC.dump(dbgs(), options));
}

} // namespace

Error toolchain::orc::preserveDebugSections(LinkGraph &G) {
  if (!G.getTargetTriple().isOSBinFormatELF()) {
    return make_error<StringError>(
        "preserveDebugSections only supports ELF LinkGraphs!",
        inconvertibleErrorCode());
  }
  for (auto &Sec : G.sections()) {
    if (DWARFSectionNames.count(Sec.getName())) {
      LLVM_DEBUG(dbgs() << "Preserving DWARF section " << Sec.getName()
                        << "\n");
      preserveDWARFSection(G, Sec);
    }
  }
  return Error::success();
}

Expected<std::pair<std::unique_ptr<DWARFContext>,
                   StringMap<std::unique_ptr<MemoryBuffer>>>>
toolchain::orc::createDWARFContext(LinkGraph &G) {
  if (!G.getTargetTriple().isOSBinFormatELF()) {
    return make_error<StringError>(
        "createDWARFContext only supports ELF LinkGraphs!",
        inconvertibleErrorCode());
  }
  StringMap<std::unique_ptr<MemoryBuffer>> DWARFSectionData;
  for (auto &Sec : G.sections()) {
    if (DWARFSectionNames.count(Sec.getName())) {
      auto SecData = getSectionData(Sec);
      auto Name = Sec.getName();
      // DWARFContext expects the section name to not start with a dot
      Name.consume_front(".");
      LLVM_DEBUG(dbgs() << "Creating DWARFContext section " << Name
                        << " with size " << SecData.size() << "\n");
      DWARFSectionData[Name] =
          std::make_unique<SmallVectorMemoryBuffer>(std::move(SecData));
    }
  }
  auto Ctx =
      DWARFContext::create(DWARFSectionData, G.getPointerSize(),
                           G.getEndianness() == toolchain::endianness::little);
  dumpDWARFContext(*Ctx);
  return std::make_pair(std::move(Ctx), std::move(DWARFSectionData));
}
