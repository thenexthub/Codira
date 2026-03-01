//===- NativeSourceFile.cpp - Native line number implementation -*- C++ -*-===//
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

#include "vm/core/DebugInfo/PDB/Native/NativeSourceFile.h"
#include "vm/core/ADT/StringExtras.h"
#include "vm/core/DebugInfo/PDB/Native/NativeSession.h"
#include "vm/core/DebugInfo/PDB/Native/PDBFile.h"
#include "vm/core/DebugInfo/PDB/Native/PDBStringTable.h"

using namespace vm::core;
using namespace vm::core::pdb;

NativeSourceFile::NativeSourceFile(NativeSession &Session, uint32_t FileId,
                                   const codeview::FileChecksumEntry &Checksum)
    : Session(Session), FileId(FileId), Checksum(Checksum) {}

std::string NativeSourceFile::getFileName() const {
  auto ST = Session.getPDBFile().getStringTable();
  if (!ST) {
    consumeError(ST.takeError());
    return "";
  }
  auto FileName = ST->getStringTable().getString(Checksum.FileNameOffset);
  if (!FileName) {
    consumeError(FileName.takeError());
    return "";
  }

  return std::string(FileName.get());
}

uint32_t NativeSourceFile::getUniqueId() const { return FileId; }

std::string NativeSourceFile::getChecksum() const {
  return toStringRef(Checksum.Checksum).str();
}

PDB_Checksum NativeSourceFile::getChecksumType() const {
  return static_cast<PDB_Checksum>(Checksum.Kind);
}

std::unique_ptr<IPDBEnumChildren<PDBSymbolCompiland>>
NativeSourceFile::getCompilands() const {
  return nullptr;
}
