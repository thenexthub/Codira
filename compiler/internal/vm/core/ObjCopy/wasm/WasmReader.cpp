//===- WasmReader.cpp -----------------------------------------------------===//
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

#include "WasmReader.h"

namespace vm::core {
namespace objcopy {
namespace wasm {

using namespace object;
using namespace vm::core::wasm;

Expected<std::unique_ptr<Object>> Reader::create() const {
  auto Obj = std::make_unique<Object>();
  Obj->Header = WasmObj.getHeader();
  Obj->isRelocatableObject = WasmObj.isRelocatableObject();
  Obj->Sections.reserve(WasmObj.getNumSections());
  for (const SectionRef &Sec : WasmObj.sections()) {
    const WasmSection &WS = WasmObj.getWasmSection(Sec);
    Obj->Sections.push_back({static_cast<uint8_t>(WS.Type),
                             WS.HeaderSecSizeEncodingLen, WS.Name, WS.Content});
    // Give known sections standard names to allow them to be selected. (Custom
    // sections already have their names filled in by the parser).
    Section &ReaderSec = Obj->Sections.back();
    if (ReaderSec.SectionType > WASM_SEC_CUSTOM &&
        ReaderSec.SectionType <= WASM_SEC_LAST_KNOWN)
      ReaderSec.Name = sectionTypeToString(ReaderSec.SectionType);
  }
  return std::move(Obj);
}

} // end namespace wasm
} // end namespace objcopy
} // end namespace vm::core
