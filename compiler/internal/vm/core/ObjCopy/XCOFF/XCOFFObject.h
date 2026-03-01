//===- XCOFFObject.h --------------------------------------------*- C++ -*-===//
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

#ifndef LLVM_LIB_OBJCOPY_XCOFF_XCOFFOBJECT_H
#define LLVM_LIB_OBJCOPY_XCOFF_XCOFFOBJECT_H

#include "vm/core/ADT/ArrayRef.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/Object/XCOFFObjectFile.h"
#include <vector>

namespace vm::core {
namespace objcopy {
namespace xcoff {

using namespace object;

struct Section {
  XCOFFSectionHeader32 SectionHeader;
  ArrayRef<uint8_t> Contents;
  std::vector<XCOFFRelocation32> Relocations;
};

struct Symbol {
  XCOFFSymbolEntry32 Sym;
  // For now, each auxiliary symbol is only an opaque binary blob with no
  // distinction.
  StringRef AuxSymbolEntries;
};

struct Object {
  XCOFFFileHeader32 FileHeader;
  XCOFFAuxiliaryHeader32 OptionalFileHeader;
  std::vector<Section> Sections;
  std::vector<Symbol> Symbols;
  StringRef StringTable;
};

} // end namespace xcoff
} // end namespace objcopy
} // end namespace vm::core

#endif // LLVM_LIB_OBJCOPY_XCOFF_XCOFFOBJECT_H
