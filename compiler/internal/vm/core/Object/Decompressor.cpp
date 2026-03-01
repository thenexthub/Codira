//===-- Decompressor.cpp --------------------------------------------------===//
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

#include "vm/core/Object/Decompressor.h"
#include "vm/core/ADT/StringExtras.h"
#include "vm/core/BinaryFormat/ELF.h"
#include "vm/core/Object/ObjectFile.h"
#include "vm/core/Support/Compression.h"
#include "vm/core/Support/DataExtractor.h"
#include "vm/core/Support/Endian.h"

using namespace vm::core;
using namespace vm::core::support::endian;
using namespace object;

Expected<Decompressor> Decompressor::create(StringRef Name, StringRef Data,
                                            bool IsLE, bool Is64Bit) {
  Decompressor D(Data);
  if (Error Err = D.consumeCompressedHeader(Is64Bit, IsLE))
    return std::move(Err);
  return D;
}

Decompressor::Decompressor(StringRef Data)
    : SectionData(Data), DecompressedSize(0) {}

Error Decompressor::consumeCompressedHeader(bool Is64Bit, bool IsLittleEndian) {
  using namespace ELF;
  uint64_t HdrSize = Is64Bit ? sizeof(Elf64_Chdr) : sizeof(Elf32_Chdr);
  if (SectionData.size() < HdrSize)
    return createError("corrupted compressed section header");

  DataExtractor Extractor(SectionData, IsLittleEndian, 0);
  uint64_t Offset = 0;
  auto ChType = Extractor.getUnsigned(&Offset, Is64Bit ? sizeof(Elf64_Word)
                                                       : sizeof(Elf32_Word));
  switch (ChType) {
  case ELFCOMPRESS_ZLIB:
    CompressionType = DebugCompressionType::Zlib;
    break;
  case ELFCOMPRESS_ZSTD:
    CompressionType = DebugCompressionType::Zstd;
    break;
  default:
    return createError("unsupported compression type (" + Twine(ChType) + ")");
  }
  if (const char *Reason = toolchain::compression::getReasonIfUnsupported(
          compression::formatFor(CompressionType)))
    return createError(Reason);

  // Skip Elf64_Chdr::ch_reserved field.
  if (Is64Bit)
    Offset += sizeof(Elf64_Word);

  DecompressedSize = Extractor.getUnsigned(
      &Offset, Is64Bit ? sizeof(Elf64_Xword) : sizeof(Elf32_Word));
  SectionData = SectionData.substr(HdrSize);
  return Error::success();
}

Error Decompressor::decompress(MutableArrayRef<uint8_t> Output) {
  return compression::decompress(CompressionType,
                                 arrayRefFromStringRef(SectionData),
                                 Output.data(), Output.size());
}
