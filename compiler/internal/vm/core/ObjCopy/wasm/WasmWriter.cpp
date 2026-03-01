//===- WasmWriter.cpp -----------------------------------------------------===//
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

#include "WasmWriter.h"
#include "vm/core/BinaryFormat/Wasm.h"
#include "vm/core/Support/Endian.h"
#include "vm/core/Support/LEB128.h"
#include "vm/core/Support/raw_ostream.h"

namespace vm::core {
namespace objcopy {
namespace wasm {

using namespace object;
using namespace vm::core::wasm;

Writer::SectionHeader Writer::createSectionHeader(const Section &S,
                                                  size_t &SectionSize) {
  SectionHeader Header;
  raw_svector_ostream OS(Header);
  OS << S.SectionType;
  bool HasName = S.SectionType == WASM_SEC_CUSTOM;
  SectionSize = S.Contents.size();
  if (HasName)
    SectionSize += getULEB128Size(S.Name.size()) + S.Name.size();
  // If we read this section from an object file, use its original size for the
  // padding of the LEB value to avoid changing the file size. Otherwise, pad
  // out to 5 bytes to make it predictable, and match the behavior of clang.
  unsigned HeaderSecSizeEncodingLen = S.HeaderSecSizeEncodingLen.value_or(5);
  encodeULEB128(SectionSize, OS, HeaderSecSizeEncodingLen);
  if (HasName) {
    encodeULEB128(S.Name.size(), OS);
    OS << S.Name;
  }
  // Total section size is the content size plus 1 for the section type and
  // the LEB-encoded size.
  SectionSize = SectionSize + 1 + HeaderSecSizeEncodingLen;
  return Header;
}

size_t Writer::finalize() {
  size_t ObjectSize = sizeof(WasmMagic) + sizeof(WasmVersion);
  SectionHeaders.reserve(Obj.Sections.size());
  // Finalize the headers of each section so we know the total size.
  for (const Section &S : Obj.Sections) {
    size_t SectionSize;
    SectionHeaders.push_back(createSectionHeader(S, SectionSize));
    ObjectSize += SectionSize;
  }
  return ObjectSize;
}

Error Writer::write() {
  size_t TotalSize = finalize();
  Out.reserveExtraSpace(TotalSize);

  // Write the header.
  Out.write(Obj.Header.Magic.data(), Obj.Header.Magic.size());
  uint32_t Version;
  support::endian::write32le(&Version, Obj.Header.Version);
  Out.write(reinterpret_cast<const char *>(&Version), sizeof(Version));

  // Write each section.
  for (size_t I = 0, S = SectionHeaders.size(); I < S; ++I) {
    Out.write(SectionHeaders[I].data(), SectionHeaders[I].size());
    Out.write(reinterpret_cast<const char *>(Obj.Sections[I].Contents.data()),
              Obj.Sections[I].Contents.size());
  }

  return Error::success();
}

} // end namespace wasm
} // end namespace objcopy
} // end namespace vm::core
