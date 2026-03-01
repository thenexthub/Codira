//===- OffloadEmitter.cpp -------------------------------------------------===//
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

#include "vm/core/Object/OffloadBinary.h"
#include "vm/core/ObjectYAML/OffloadYAML.h"
#include "vm/core/ObjectYAML/yaml2obj.h"
#include "vm/core/Support/raw_ostream.h"

using namespace vm::core;
using namespace OffloadYAML;

namespace vm::core {
namespace yaml {

bool yaml2offload(Binary &Doc, raw_ostream &Out, ErrorHandler EH) {
  for (const auto &Member : Doc.Members) {
    object::OffloadBinary::OffloadingImage Image{};
    if (Member.ImageKind)
      Image.TheImageKind = *Member.ImageKind;
    if (Member.OffloadKind)
      Image.TheOffloadKind = *Member.OffloadKind;
    if (Member.Flags)
      Image.Flags = *Member.Flags;

    if (Member.StringEntries)
      for (const auto &Entry : *Member.StringEntries)
        Image.StringData[Entry.Key] = Entry.Value;

    SmallVector<char, 1024> Data;
    raw_svector_ostream OS(Data);
    if (Member.Content)
      Member.Content->writeAsBinary(OS);
    Image.Image = MemoryBuffer::getMemBufferCopy(OS.str());

    // Copy the data to a new buffer so we can modify the bytes directly.
    auto Buffer = object::OffloadBinary::write(Image);
    auto *TheHeader =
        reinterpret_cast<object::OffloadBinary::Header *>(&Buffer[0]);
    if (Doc.Version)
      TheHeader->Version = *Doc.Version;
    if (Doc.Size)
      TheHeader->Size = *Doc.Size;
    if (Doc.EntryOffset)
      TheHeader->EntryOffset = *Doc.EntryOffset;
    if (Doc.EntrySize)
      TheHeader->EntrySize = *Doc.EntrySize;

    Out.write(Buffer.begin(), Buffer.size());
  }

  return true;
}

} // namespace yaml
} // namespace vm::core
