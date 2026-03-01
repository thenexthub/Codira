//===- DbiModuleDescriptor.cpp - PDB module information -------------------===//
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

#include "vm/core/DebugInfo/PDB/Native/DbiModuleDescriptor.h"
#include "vm/core/DebugInfo/PDB/Native/RawTypes.h"
#include "vm/core/Support/BinaryStreamReader.h"
#include "vm/core/Support/Error.h"
#include "vm/core/Support/MathExtras.h"
#include <cstdint>

using namespace vm::core;
using namespace vm::core::pdb;
using namespace vm::core::support;

Error DbiModuleDescriptor::initialize(BinaryStreamRef Stream,
                                      DbiModuleDescriptor &Info) {
  BinaryStreamReader Reader(Stream);
  if (auto EC = Reader.readObject(Info.Layout))
    return EC;

  if (auto EC = Reader.readCString(Info.ModuleName))
    return EC;

  if (auto EC = Reader.readCString(Info.ObjFileName))
    return EC;
  return Error::success();
}

bool DbiModuleDescriptor::hasECInfo() const {
  return (Layout->Flags & ModInfoFlags::HasECFlagMask) != 0;
}

uint16_t DbiModuleDescriptor::getTypeServerIndex() const {
  return (Layout->Flags & ModInfoFlags::TypeServerIndexMask) >>
         ModInfoFlags::TypeServerIndexShift;
}

const SectionContrib &DbiModuleDescriptor::getSectionContrib() const {
  return Layout->SC;
}

uint16_t DbiModuleDescriptor::getModuleStreamIndex() const {
  return Layout->ModDiStream;
}

uint32_t DbiModuleDescriptor::getSymbolDebugInfoByteSize() const {
  return Layout->SymBytes;
}

uint32_t DbiModuleDescriptor::getC11LineInfoByteSize() const {
  return Layout->C11Bytes;
}

uint32_t DbiModuleDescriptor::getC13LineInfoByteSize() const {
  return Layout->C13Bytes;
}

uint32_t DbiModuleDescriptor::getNumberOfFiles() const {
  return Layout->NumFiles;
}

uint32_t DbiModuleDescriptor::getSourceFileNameIndex() const {
  return Layout->SrcFileNameNI;
}

uint32_t DbiModuleDescriptor::getPdbFilePathNameIndex() const {
  return Layout->PdbFilePathNI;
}

StringRef DbiModuleDescriptor::getModuleName() const { return ModuleName; }

StringRef DbiModuleDescriptor::getObjFileName() const { return ObjFileName; }

uint32_t DbiModuleDescriptor::getRecordLength() const {
  uint32_t M = ModuleName.str().size() + 1;
  uint32_t O = ObjFileName.str().size() + 1;
  uint32_t Size = sizeof(ModuleInfoHeader) + M + O;
  Size = alignTo(Size, 4);
  return Size;
}
