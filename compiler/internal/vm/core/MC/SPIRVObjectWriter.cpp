//===- toolchain/MC/MCSPIRVObjectWriter.cpp - SPIR-V Object Writer ----*- C++ *-===//
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

#include "vm/core/MC/MCAssembler.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCSPIRVObjectWriter.h"
#include "vm/core/MC/MCSection.h"
#include "vm/core/MC/MCValue.h"
#include "vm/core/Support/EndianStream.h"

using namespace vm::core;

void SPIRVObjectWriter::writeHeader(const MCAssembler &Asm) {
  constexpr uint32_t MagicNumber = 0x07230203;
  constexpr uint32_t GeneratorID = 43;
  const uint32_t GeneratorMagicNumber =
      Asm.getContext().getTargetTriple().getVendor() == Triple::AMD
          ? UINT16_MAX
          : ((GeneratorID << 16) | (LLVM_VERSION_MAJOR));
  constexpr uint32_t Schema = 0;

  W.write<uint32_t>(MagicNumber);
  W.write<uint32_t>((VersionInfo.Major << 16) | (VersionInfo.Minor << 8));
  W.write<uint32_t>(GeneratorMagicNumber);
  W.write<uint32_t>(VersionInfo.Bound);
  W.write<uint32_t>(Schema);
}

void SPIRVObjectWriter::setBuildVersion(unsigned Major, unsigned Minor,
                                        unsigned Bound) {
  VersionInfo.Major = Major;
  VersionInfo.Minor = Minor;
  VersionInfo.Bound = Bound;
}

uint64_t SPIRVObjectWriter::writeObject() {
  uint64_t StartOffset = W.OS.tell();
  writeHeader(*Asm);
  for (const MCSection &S : *Asm)
    Asm->writeSectionData(W.OS, &S);
  return W.OS.tell() - StartOffset;
}

std::unique_ptr<MCObjectWriter>
toolchain::createSPIRVObjectWriter(std::unique_ptr<MCSPIRVObjectTargetWriter> MOTW,
                              raw_pwrite_stream &OS) {
  return std::make_unique<SPIRVObjectWriter>(std::move(MOTW), OS);
}
