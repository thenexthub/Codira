//===- WasmObject.h ---------------------------------------------*- C++ -*-===//
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

#ifndef LLVM_LIB_OBJCOPY_WASM_WASMOBJECT_H
#define LLVM_LIB_OBJCOPY_WASM_WASMOBJECT_H

#include "vm/core/ADT/ArrayRef.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/Object/Wasm.h"
#include "vm/core/Support/MemoryBuffer.h"
#include <vector>

namespace vm::core {
namespace objcopy {
namespace wasm {

struct Section {
  // For now, each section is only an opaque binary blob with no distinction
  // between custom and known sections.
  uint8_t SectionType;
  std::optional<uint8_t> HeaderSecSizeEncodingLen;
  StringRef Name;
  ArrayRef<uint8_t> Contents;
};

struct Object {
  toolchain::wasm::WasmObjectHeader Header;
  // For now don't discriminate between kinds of sections.
  std::vector<Section> Sections;
  bool isRelocatableObject = false;

  void addSectionWithOwnedContents(Section NewSection,
                                   std::unique_ptr<MemoryBuffer> &&Content);
  void removeSections(function_ref<bool(const Section &)> ToRemove);

private:
  std::vector<std::unique_ptr<MemoryBuffer>> OwnedContents;
};

} // end namespace wasm
} // end namespace objcopy
} // end namespace vm::core

#endif // LLVM_LIB_OBJCOPY_WASM_WASMOBJECT_H
