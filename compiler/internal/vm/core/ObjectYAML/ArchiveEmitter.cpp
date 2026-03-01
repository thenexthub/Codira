//===- ArchiveEmitter.cpp ---------------------------- --------------------===//
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

#include "vm/core/ObjectYAML/ArchiveYAML.h"
#include "vm/core/ObjectYAML/yaml2obj.h"
#include "vm/core/Support/raw_ostream.h"

using namespace vm::core;
using namespace ArchYAML;

namespace vm::core {
namespace yaml {

bool yaml2archive(ArchYAML::Archive &Doc, raw_ostream &Out, ErrorHandler EH) {
  Out.write(Doc.Magic.data(), Doc.Magic.size());

  if (Doc.Content) {
    Doc.Content->writeAsBinary(Out);
    return true;
  }

  if (!Doc.Members)
    return true;

  auto WriteField = [&](StringRef Field, uint8_t Size) {
    Out.write(Field.data(), Field.size());
    for (size_t I = Field.size(); I != Size; ++I)
      Out.write(' ');
  };

  for (const Archive::Child &C : *Doc.Members) {
    for (auto &P : C.Fields)
      WriteField(P.second.Value, P.second.MaxLength);

    if (C.Content)
      C.Content->writeAsBinary(Out);
    if (C.PaddingByte)
      Out.write(*C.PaddingByte);
  }

  return true;
}

} // namespace yaml
} // namespace vm::core
