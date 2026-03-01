//===-------------- MachO.cpp - JIT linker function for MachO -------------===//
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
// MachO jit-link function.
//
//===----------------------------------------------------------------------===//

#include "vm/core/ExecutionEngine/JITLink/MachO.h"

#include "vm/core/BinaryFormat/MachO.h"
#include "vm/core/ExecutionEngine/JITLink/MachO_arm64.h"
#include "vm/core/ExecutionEngine/JITLink/MachO_x86_64.h"
#include "vm/core/Support/Format.h"

using namespace vm::core;

#define DEBUG_TYPE "jitlink"

namespace vm::core {
namespace jitlink {

Expected<std::unique_ptr<LinkGraph>>
createLinkGraphFromMachOObject(MemoryBufferRef ObjectBuffer,
                               std::shared_ptr<orc::SymbolStringPool> SSP) {
  StringRef Data = ObjectBuffer.getBuffer();
  if (Data.size() < 4)
    return make_error<JITLinkError>("Truncated MachO buffer \"" +
                                    ObjectBuffer.getBufferIdentifier() + "\"");

  uint32_t Magic;
  memcpy(&Magic, Data.data(), sizeof(uint32_t));
  LLVM_DEBUG({
    dbgs() << "jitLink_MachO: magic = " << format("0x%08" PRIx32, Magic)
           << ", identifier = \"" << ObjectBuffer.getBufferIdentifier()
           << "\"\n";
  });

  if (Magic == MachO::MH_MAGIC || Magic == MachO::MH_CIGAM)
    return make_error<JITLinkError>("MachO 32-bit platforms not supported");
  else if (Magic == MachO::MH_MAGIC_64 || Magic == MachO::MH_CIGAM_64) {

    if (Data.size() < sizeof(MachO::mach_header_64))
      return make_error<JITLinkError>("Truncated MachO buffer \"" +
                                      ObjectBuffer.getBufferIdentifier() +
                                      "\"");

    // Read the CPU type from the header.
    uint32_t CPUType;
    memcpy(&CPUType, Data.data() + 4, sizeof(uint32_t));
    if (Magic == MachO::MH_CIGAM_64)
      CPUType = toolchain::byteswap<uint32_t>(CPUType);

    LLVM_DEBUG({
      dbgs() << "jitLink_MachO: cputype = " << format("0x%08" PRIx32, CPUType)
             << "\n";
    });

    switch (CPUType) {
    case MachO::CPU_TYPE_ARM64:
      return createLinkGraphFromMachOObject_arm64(ObjectBuffer, std::move(SSP));
    case MachO::CPU_TYPE_X86_64:
      return createLinkGraphFromMachOObject_x86_64(ObjectBuffer,
                                                   std::move(SSP));
    }
    return make_error<JITLinkError>("MachO-64 CPU type not valid");
  } else
    return make_error<JITLinkError>("Unrecognized MachO magic value");
}

void link_MachO(std::unique_ptr<LinkGraph> G,
                std::unique_ptr<JITLinkContext> Ctx) {

  switch (G->getTargetTriple().getArch()) {
  case Triple::aarch64:
    return link_MachO_arm64(std::move(G), std::move(Ctx));
  case Triple::x86_64:
    return link_MachO_x86_64(std::move(G), std::move(Ctx));
  default:
    Ctx->notifyFailed(make_error<JITLinkError>("MachO-64 CPU type not valid"));
    return;
  }
}

template <typename MachOHeaderType>
static Expected<Block &> createLocalHeaderBlock(LinkGraph &G, Section &Sec) {
  auto &B = G.createMutableContentBlock(Sec, sizeof(MachOHeaderType),
                                        orc::ExecutorAddr(), 8, 0, true);
  MachOHeaderType Hdr;
  Hdr.magic = G.getPointerSize() == 4 ? MachO::MH_MAGIC : MachO::MH_MAGIC_64;
  if (auto CPUType = MachO::getCPUType(G.getTargetTriple()))
    Hdr.cputype = *CPUType;
  else
    return CPUType.takeError();
  if (auto CPUSubType = MachO::getCPUSubType(G.getTargetTriple()))
    Hdr.cpusubtype = *CPUSubType;
  else
    return CPUSubType.takeError();
  Hdr.filetype = MachO::MH_OBJECT;

  if (G.getEndianness() != endianness::native)
    MachO::swapStruct(Hdr);

  memcpy(B.getAlreadyMutableContent().data(), &Hdr, sizeof(Hdr));

  return B;
}

Expected<Symbol &> getOrCreateLocalMachOHeader(LinkGraph &G) {
  StringRef LocalHeaderSectionName("__TEXT,__lcl_macho_hdr");
  Section *Sec = G.findSectionByName(LocalHeaderSectionName);
  if (Sec) {
    assert(Sec->blocks_size() == 1 && "Unexpected number of blocks");
    assert(Sec->symbols_size() == 1 && "Unexpected number of symbols");
    auto &Sym = **Sec->symbols().begin();
    assert(Sym.getOffset() == 0 && "Symbol not at start of header block");
    return Sym;
  }

  // Create the local header section, move all other sections up in the
  // section ordering to ensure that it's laid out first.
  for (auto &Sec : G.sections())
    Sec.setOrdinal(Sec.getOrdinal() + 1);

  Sec = &G.createSection(LocalHeaderSectionName, orc::MemProt::Read);

  Sec->setOrdinal(0);

  Block *B = nullptr;
  switch (G.getTargetTriple().getArch()) {
  case Triple::aarch64:
  case Triple::x86_64:
    if (auto BOrErr = createLocalHeaderBlock<MachO::mach_header_64>(G, *Sec))
      B = &*BOrErr;
    else
      return BOrErr.takeError();
    break;
  default:
    return make_error<JITLinkError>("Cannot create local Mach-O header for " +
                                    G.getName() + ": unsupported triple " +
                                    G.getTargetTriple().str());
  }

  return G.addAnonymousSymbol(*B, 0, B->getSize(), false, false);
}

} // end namespace jitlink
} // end namespace vm::core
