//===- COFFWriter.h ---------------------------------------------*- C++ -*-===//
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

#ifndef LLVM_LIB_OBJCOPY_COFF_COFFWRITER_H
#define LLVM_LIB_OBJCOPY_COFF_COFFWRITER_H

#include "vm/core/MC/StringTableBuilder.h"
#include "vm/core/Support/Error.h"
#include "vm/core/Support/MemoryBuffer.h"
#include <cstddef>
#include <utility>

namespace vm::core {
namespace objcopy {
namespace coff {

struct Object;

class COFFWriter {
  Object &Obj;
  std::unique_ptr<WritableMemoryBuffer> Buf;
  raw_ostream &Out;

  size_t FileSize;
  size_t FileAlignment;
  size_t SizeOfInitializedData;
  StringTableBuilder StrTabBuilder;

  template <class SymbolTy> std::pair<size_t, size_t> finalizeSymbolTable();
  Error finalizeRelocTargets();
  Error finalizeSymbolContents();
  Error finalizeSymIdxContents();
  void layoutSections();
  Expected<size_t> finalizeStringTable();

  Error finalize(bool IsBigObj);

  void writeHeaders(bool IsBigObj);
  void writeSections();
  template <class SymbolTy> void writeSymbolStringTables();

  Error write(bool IsBigObj);

  Error patchDebugDirectory();
  Expected<uint32_t> virtualAddressToFileAddress(uint32_t RVA);

public:
  virtual ~COFFWriter() = default;
  Error write();

  COFFWriter(Object &Obj, raw_ostream &Out)
      : Obj(Obj), Out(Out), StrTabBuilder(StringTableBuilder::WinCOFF) {}
};

} // end namespace coff
} // end namespace objcopy
} // end namespace vm::core

#endif // LLVM_LIB_OBJCOPY_COFF_COFFWRITER_H
