//===- DebugCrossExSubsection.cpp -----------------------------------------===//
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

#include "vm/core/DebugInfo/CodeView/DebugCrossExSubsection.h"
#include "vm/core/DebugInfo/CodeView/CodeViewError.h"
#include "vm/core/Support/BinaryStreamReader.h"
#include "vm/core/Support/BinaryStreamWriter.h"
#include "vm/core/Support/Error.h"
#include <cstdint>

using namespace vm::core;
using namespace vm::core::codeview;

Error DebugCrossModuleExportsSubsectionRef::initialize(
    BinaryStreamReader Reader) {
  if (Reader.bytesRemaining() % sizeof(CrossModuleExport) != 0)
    return make_error<CodeViewError>(
        cv_error_code::corrupt_record,
        "Cross Scope Exports section is an invalid size!");

  uint32_t Size = Reader.bytesRemaining() / sizeof(CrossModuleExport);
  return Reader.readArray(References, Size);
}

Error DebugCrossModuleExportsSubsectionRef::initialize(BinaryStreamRef Stream) {
  BinaryStreamReader Reader(Stream);
  return initialize(Reader);
}

void DebugCrossModuleExportsSubsection::addMapping(uint32_t Local,
                                                   uint32_t Global) {
  Mappings[Local] = Global;
}

uint32_t DebugCrossModuleExportsSubsection::calculateSerializedSize() const {
  return Mappings.size() * sizeof(CrossModuleExport);
}

Error DebugCrossModuleExportsSubsection::commit(
    BinaryStreamWriter &Writer) const {
  for (const auto &M : Mappings) {
    if (auto EC = Writer.writeInteger(M.first))
      return EC;
    if (auto EC = Writer.writeInteger(M.second))
      return EC;
  }
  return Error::success();
}
