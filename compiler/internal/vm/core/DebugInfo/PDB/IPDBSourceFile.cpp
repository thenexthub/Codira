//===- IPDBSourceFile.cpp - base interface for a PDB source file ----------===//
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

#include "vm/core/DebugInfo/PDB/IPDBSourceFile.h"
#include "vm/core/DebugInfo/PDB/PDBExtras.h"
#include "vm/core/DebugInfo/PDB/PDBTypes.h"
#include "vm/core/Support/Format.h"
#include "vm/core/Support/raw_ostream.h"
#include <cstdint>
#include <string>

using namespace vm::core;
using namespace vm::core::pdb;

IPDBSourceFile::~IPDBSourceFile() = default;

void IPDBSourceFile::dump(raw_ostream &OS, int Indent) const {
  OS.indent(Indent);
  PDB_Checksum ChecksumType = getChecksumType();
  OS << "[";
  if (ChecksumType != PDB_Checksum::None) {
    OS << ChecksumType << ": ";
    std::string Checksum = getChecksum();
    for (uint8_t c : Checksum)
      OS << format_hex_no_prefix(c, 2, true);
  } else
    OS << "No checksum";
  OS << "] " << getFileName() << "\n";
}
