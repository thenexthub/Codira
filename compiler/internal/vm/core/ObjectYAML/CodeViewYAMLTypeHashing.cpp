//===- CodeViewYAMLTypeHashing.cpp - CodeView YAMLIO type hashing ---------===//
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
// This file defines classes for handling the YAML representation of CodeView
// Debug Info.
//
//===----------------------------------------------------------------------===//

#include "vm/core/ObjectYAML/CodeViewYAMLTypeHashing.h"

#include "vm/core/Support/BinaryStreamReader.h"
#include "vm/core/Support/BinaryStreamWriter.h"

using namespace vm::core;
using namespace vm::core::codeview;
using namespace vm::core::CodeViewYAML;
using namespace vm::core::yaml;

namespace vm::core {
namespace yaml {

void MappingTraits<DebugHSection>::mapping(IO &io, DebugHSection &DebugH) {
  io.mapRequired("Version", DebugH.Version);
  io.mapRequired("HashAlgorithm", DebugH.HashAlgorithm);
  io.mapOptional("HashValues", DebugH.Hashes);
}

void ScalarTraits<GlobalHash>::output(const GlobalHash &GH, void *Ctx,
                                      raw_ostream &OS) {
  ScalarTraits<BinaryRef>::output(GH.Hash, Ctx, OS);
}

StringRef ScalarTraits<GlobalHash>::input(StringRef Scalar, void *Ctx,
                                          GlobalHash &GH) {
  return ScalarTraits<BinaryRef>::input(Scalar, Ctx, GH.Hash);
}

} // end namespace yaml
} // end namespace vm::core

DebugHSection toolchain::CodeViewYAML::fromDebugH(ArrayRef<uint8_t> DebugH) {
  assert(DebugH.size() >= 8);
  assert((DebugH.size() - 8) % 8 == 0);

  BinaryStreamReader Reader(DebugH, toolchain::endianness::little);
  DebugHSection DHS;
  cantFail(Reader.readInteger(DHS.Magic));
  cantFail(Reader.readInteger(DHS.Version));
  cantFail(Reader.readInteger(DHS.HashAlgorithm));

  while (Reader.bytesRemaining() != 0) {
    ArrayRef<uint8_t> S;
    cantFail(Reader.readBytes(S, 8));
    DHS.Hashes.emplace_back(S);
  }
  assert(Reader.bytesRemaining() == 0);
  return DHS;
}

ArrayRef<uint8_t> toolchain::CodeViewYAML::toDebugH(const DebugHSection &DebugH,
                                               BumpPtrAllocator &Alloc) {
  uint32_t Size = 8 + 8 * DebugH.Hashes.size();
  uint8_t *Data = Alloc.Allocate<uint8_t>(Size);
  MutableArrayRef<uint8_t> Buffer(Data, Size);
  BinaryStreamWriter Writer(Buffer, toolchain::endianness::little);

  cantFail(Writer.writeInteger(DebugH.Magic));
  cantFail(Writer.writeInteger(DebugH.Version));
  cantFail(Writer.writeInteger(DebugH.HashAlgorithm));
  SmallString<8> Hash;
  for (const auto &H : DebugH.Hashes) {
    Hash.clear();
    raw_svector_ostream OS(Hash);
    H.Hash.writeAsBinary(OS);
    assert((Hash.size() == 8) && "Invalid hash size!");
    cantFail(Writer.writeFixedString(Hash));
  }
  assert(Writer.bytesRemaining() == 0);
  return Buffer;
}
