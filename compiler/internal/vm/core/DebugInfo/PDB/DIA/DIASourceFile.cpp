//===- DIASourceFile.cpp - DIA implementation of IPDBSourceFile -*- C++ -*-===//
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

#include "vm/core/DebugInfo/PDB/DIA/DIASourceFile.h"
#include "vm/core/DebugInfo/PDB/ConcreteSymbolEnumerator.h"
#include "vm/core/DebugInfo/PDB/DIA/DIAEnumSymbols.h"
#include "vm/core/DebugInfo/PDB/DIA/DIASession.h"
#include "vm/core/DebugInfo/PDB/DIA/DIAUtils.h"
#include "vm/core/DebugInfo/PDB/PDBSymbolCompiland.h"

using namespace vm::core;
using namespace vm::core::pdb;

DIASourceFile::DIASourceFile(const DIASession &PDBSession,
                             CComPtr<IDiaSourceFile> DiaSourceFile)
    : Session(PDBSession), SourceFile(DiaSourceFile) {}

std::string DIASourceFile::getFileName() const {
  return invokeBstrMethod(*SourceFile, &IDiaSourceFile::get_fileName);
}

uint32_t DIASourceFile::getUniqueId() const {
  DWORD Id;
  return (S_OK == SourceFile->get_uniqueId(&Id)) ? Id : 0;
}

std::string DIASourceFile::getChecksum() const {
  DWORD ByteSize = 0;
  HRESULT Result = SourceFile->get_checksum(0, &ByteSize, nullptr);
  if (ByteSize == 0)
    return std::string();
  std::vector<BYTE> ChecksumBytes(ByteSize);
  Result = SourceFile->get_checksum(ByteSize, &ByteSize, &ChecksumBytes[0]);
  if (S_OK != Result)
    return std::string();
  return std::string(ChecksumBytes.begin(), ChecksumBytes.end());
}

PDB_Checksum DIASourceFile::getChecksumType() const {
  DWORD Type;
  HRESULT Result = SourceFile->get_checksumType(&Type);
  if (S_OK != Result)
    return PDB_Checksum::None;
  return static_cast<PDB_Checksum>(Type);
}

std::unique_ptr<IPDBEnumChildren<PDBSymbolCompiland>>
DIASourceFile::getCompilands() const {
  CComPtr<IDiaEnumSymbols> DiaEnumerator;
  HRESULT Result = SourceFile->get_compilands(&DiaEnumerator);
  if (S_OK != Result)
    return nullptr;

  auto Enumerator = std::unique_ptr<IPDBEnumSymbols>(
      new DIAEnumSymbols(Session, DiaEnumerator));
  return std::unique_ptr<IPDBEnumChildren<PDBSymbolCompiland>>(
      new ConcreteSymbolEnumerator<PDBSymbolCompiland>(std::move(Enumerator)));
}
