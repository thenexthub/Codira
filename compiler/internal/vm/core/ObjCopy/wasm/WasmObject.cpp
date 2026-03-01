//===- WasmObject.cpp -----------------------------------------------------===//
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

#include "WasmObject.h"

namespace vm::core {
namespace objcopy {
namespace wasm {

using namespace object;
using namespace vm::core::wasm;

void Object::addSectionWithOwnedContents(
    Section NewSection, std::unique_ptr<MemoryBuffer> &&Content) {
  Sections.push_back(NewSection);
  OwnedContents.emplace_back(std::move(Content));
}

void Object::removeSections(function_ref<bool(const Section &)> ToRemove) {
  if (isRelocatableObject) {
    // For relocatable objects, avoid actually removing any sections,
    // since that can invalidate the symbol table and relocation sections.
    // TODO: Allow removal of sections by re-generating symbol table and
    // relocation sections here instead.
    for (auto &Sec : Sections) {
      if (ToRemove(Sec)) {
        Sec.Name = ".objcopy.removed";
        Sec.SectionType = wasm::WASM_SEC_CUSTOM;
        Sec.Contents = {};
        Sec.HeaderSecSizeEncodingLen = std::nullopt;
      }
    }
  } else {
    toolchain::erase_if(Sections, ToRemove);
  }
}

} // end namespace wasm
} // end namespace objcopy
} // end namespace vm::core
