//===- DIATable.cpp - DIA implementation of IPDBTable -----------*- C++ -*-===//
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

#include "vm/core/DebugInfo/PDB/DIA/DIATable.h"
#include "vm/core/DebugInfo/PDB/DIA/DIAUtils.h"

using namespace vm::core;
using namespace vm::core::pdb;

DIATable::DIATable(CComPtr<IDiaTable> DiaTable) : Table(DiaTable) {}

uint32_t DIATable::getItemCount() const {
  LONG Count = 0;
  return (S_OK == Table->get_Count(&Count)) ? Count : 0;
}

std::string DIATable::getName() const {
  return invokeBstrMethod(*Table, &IDiaTable::get_name);
}

PDB_TableType DIATable::getTableType() const {
  CComBSTR Name16;
  if (S_OK != Table->get_name(&Name16))
    return PDB_TableType::TableInvalid;

  if (Name16 == DiaTable_Symbols)
    return PDB_TableType::Symbols;
  if (Name16 == DiaTable_SrcFiles)
    return PDB_TableType::SourceFiles;
  if (Name16 == DiaTable_Sections)
    return PDB_TableType::SectionContribs;
  if (Name16 == DiaTable_LineNums)
    return PDB_TableType::LineNumbers;
  if (Name16 == DiaTable_SegMap)
    return PDB_TableType::Segments;
  if (Name16 == DiaTable_InjSrc)
    return PDB_TableType::InjectedSources;
  if (Name16 == DiaTable_FrameData)
    return PDB_TableType::FrameData;
  if (Name16 == DiaTable_InputAssemblyFiles)
    return PDB_TableType::InputAssemblyFiles;
  if (Name16 == DiaTable_Dbg)
    return PDB_TableType::Dbg;
  return PDB_TableType::TableInvalid;
}
