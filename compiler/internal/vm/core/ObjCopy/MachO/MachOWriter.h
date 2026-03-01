//===- MachOWriter.h --------------------------------------------*- C++ -*-===//
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

#ifndef LLVM_LIB_OBJCOPY_MACHO_MACHOWRITER_H
#define LLVM_LIB_OBJCOPY_MACHO_MACHOWRITER_H

#include "MachOLayoutBuilder.h"
#include "MachOObject.h"
#include "vm/core/BinaryFormat/MachO.h"
#include "vm/core/ObjCopy/MachO/MachOObjcopy.h"
#include "vm/core/Object/MachO.h"

namespace vm::core {
class Error;

namespace objcopy {
namespace macho {

class MachOWriter {
  Object &O;
  bool Is64Bit;
  bool IsLittleEndian;
  uint64_t PageSize;
  std::unique_ptr<WritableMemoryBuffer> Buf;
  raw_ostream &Out;
  MachOLayoutBuilder LayoutBuilder;

  size_t headerSize() const;
  size_t loadCommandsSize() const;
  size_t symTableSize() const;
  size_t strTableSize() const;

  void writeHeader();
  void writeLoadCommands();
  template <typename StructType>
  void writeSectionInLoadCommand(const Section &Sec, uint8_t *&Out);
  void writeSections();
  void writeSymbolTable();
  void writeStringTable();
  void writeRebaseInfo();
  void writeBindInfo();
  void writeWeakBindInfo();
  void writeLazyBindInfo();
  void writeExportInfo();
  void writeIndirectSymbolTable();
  void writeLinkData(std::optional<size_t> LCIndex, const LinkData &LD);
  void writeCodeSignatureData();
  void writeDataInCodeData();
  void writeLinkerOptimizationHint();
  void writeFunctionStartsData();
  void writeDylibCodeSignDRsData();
  void writeChainedFixupsData();
  void writeExportsTrieData();
  void writeTail();

public:
  MachOWriter(Object &O, bool Is64Bit, bool IsLittleEndian,
              StringRef OutputFileName, uint64_t PageSize, raw_ostream &Out)
      : O(O), Is64Bit(Is64Bit), IsLittleEndian(IsLittleEndian),
        PageSize(PageSize), Out(Out),
        LayoutBuilder(O, Is64Bit, OutputFileName, PageSize) {}

  size_t totalSize() const;
  Error finalize();
  Error write();
};

} // end namespace macho
} // end namespace objcopy
} // end namespace vm::core

#endif // LLVM_LIB_OBJCOPY_MACHO_MACHOWRITER_H
