//===- MachOReader.h --------------------------------------------*- C++ -*-===//
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

#ifndef LLVM_LIB_OBJCOPY_MACHO_MACHOREADER_H
#define LLVM_LIB_OBJCOPY_MACHO_MACHOREADER_H

#include "MachOObject.h"
#include "vm/core/BinaryFormat/MachO.h"
#include "vm/core/ObjCopy/MachO/MachOObjcopy.h"
#include "vm/core/Object/MachO.h"
#include <memory>

namespace vm::core {
namespace objcopy {
namespace macho {

// The hierarchy of readers is responsible for parsing different inputs:
// raw binaries and regular MachO object files.
class Reader {
public:
  virtual ~Reader() = default;
  virtual Expected<std::unique_ptr<Object>> create() const = 0;
};

class MachOReader : public Reader {
  const object::MachOObjectFile &MachOObj;

  void readHeader(Object &O) const;
  Error readLoadCommands(Object &O) const;
  void readSymbolTable(Object &O) const;
  void setSymbolInRelocationInfo(Object &O) const;
  void readRebaseInfo(Object &O) const;
  void readBindInfo(Object &O) const;
  void readWeakBindInfo(Object &O) const;
  void readLazyBindInfo(Object &O) const;
  void readExportInfo(Object &O) const;
  void readLinkData(Object &O, std::optional<size_t> LCIndex,
                    LinkData &LD) const;
  void readCodeSignature(Object &O) const;
  void readDataInCodeData(Object &O) const;
  void readLinkerOptimizationHint(Object &O) const;
  void readFunctionStartsData(Object &O) const;
  void readDylibCodeSignDRs(Object &O) const;
  void readExportsTrie(Object &O) const;
  void readChainedFixups(Object &O) const;
  void readIndirectSymbolTable(Object &O) const;
  void readSwiftVersion(Object &O) const;

public:
  explicit MachOReader(const object::MachOObjectFile &Obj) : MachOObj(Obj) {}

  Expected<std::unique_ptr<Object>> create() const override;
};

} // end namespace macho
} // end namespace objcopy
} // end namespace vm::core

#endif // LLVM_LIB_OBJCOPY_MACHO_MACHOREADER_H
