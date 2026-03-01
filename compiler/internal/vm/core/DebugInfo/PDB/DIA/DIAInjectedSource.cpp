//===- DIAInjectedSource.cpp - DIA impl for IPDBInjectedSource --*- C++ -*-===//
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

#include "vm/core/DebugInfo/PDB/DIA/DIAInjectedSource.h"
#include "vm/core/ADT/ArrayRef.h"
#include "vm/core/DebugInfo/PDB/ConcreteSymbolEnumerator.h"
#include "vm/core/DebugInfo/PDB/DIA/DIASession.h"
#include "vm/core/DebugInfo/PDB/DIA/DIAUtils.h"

using namespace vm::core;
using namespace vm::core::pdb;

DIAInjectedSource::DIAInjectedSource(CComPtr<IDiaInjectedSource> DiaSourceFile)
    : SourceFile(DiaSourceFile) {}

uint32_t DIAInjectedSource::getCrc32() const {
  DWORD Crc;
  return (S_OK == SourceFile->get_crc(&Crc)) ? Crc : 0;
}

uint64_t DIAInjectedSource::getCodeByteSize() const {
  ULONGLONG Size;
  return (S_OK == SourceFile->get_length(&Size)) ? Size : 0;
}

std::string DIAInjectedSource::getFileName() const {
  return invokeBstrMethod(*SourceFile, &IDiaInjectedSource::get_filename);
}

std::string DIAInjectedSource::getObjectFileName() const {
  return invokeBstrMethod(*SourceFile, &IDiaInjectedSource::get_objectFilename);
}

std::string DIAInjectedSource::getVirtualFileName() const {
  return invokeBstrMethod(*SourceFile,
                          &IDiaInjectedSource::get_virtualFilename);
}

uint32_t DIAInjectedSource::getCompression() const {
  DWORD Compression = 0;
  if (S_OK != SourceFile->get_sourceCompression(&Compression))
    return PDB_SourceCompression::None;
  return static_cast<uint32_t>(Compression);
}

std::string DIAInjectedSource::getCode() const {
  DWORD DataSize;
  if (S_OK != SourceFile->get_source(0, &DataSize, nullptr))
    return "";

  std::vector<uint8_t> Buffer(DataSize);
  if (S_OK != SourceFile->get_source(DataSize, &DataSize, Buffer.data()))
    return "";
  assert(Buffer.size() == DataSize);
  return std::string(reinterpret_cast<const char *>(Buffer.data()),
                     Buffer.size());
}
