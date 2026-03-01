//===- DIALineNumber.cpp - DIA implementation of IPDBLineNumber -*- C++ -*-===//
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

#include "vm/core/DebugInfo/PDB/DIA/DIALineNumber.h"

using namespace vm::core;
using namespace vm::core::pdb;

DIALineNumber::DIALineNumber(CComPtr<IDiaLineNumber> DiaLineNumber)
    : LineNumber(DiaLineNumber) {}

uint32_t DIALineNumber::getLineNumber() const {
  DWORD Line = 0;
  return (S_OK == LineNumber->get_lineNumber(&Line)) ? Line : 0;
}

uint32_t DIALineNumber::getLineNumberEnd() const {
  DWORD LineEnd = 0;
  return (S_OK == LineNumber->get_lineNumberEnd(&LineEnd)) ? LineEnd : 0;
}

uint32_t DIALineNumber::getColumnNumber() const {
  DWORD Column = 0;
  return (S_OK == LineNumber->get_columnNumber(&Column)) ? Column : 0;
}

uint32_t DIALineNumber::getColumnNumberEnd() const {
  DWORD ColumnEnd = 0;
  return (S_OK == LineNumber->get_columnNumberEnd(&ColumnEnd)) ? ColumnEnd : 0;
}

uint32_t DIALineNumber::getAddressSection() const {
  DWORD Section = 0;
  return (S_OK == LineNumber->get_addressSection(&Section)) ? Section : 0;
}

uint32_t DIALineNumber::getAddressOffset() const {
  DWORD Offset = 0;
  return (S_OK == LineNumber->get_addressOffset(&Offset)) ? Offset : 0;
}

uint32_t DIALineNumber::getRelativeVirtualAddress() const {
  DWORD RVA = 0;
  return (S_OK == LineNumber->get_relativeVirtualAddress(&RVA)) ? RVA : 0;
}

uint64_t DIALineNumber::getVirtualAddress() const {
  ULONGLONG Addr = 0;
  return (S_OK == LineNumber->get_virtualAddress(&Addr)) ? Addr : 0;
}

uint32_t DIALineNumber::getLength() const {
  DWORD Length = 0;
  return (S_OK == LineNumber->get_length(&Length)) ? Length : 0;
}

uint32_t DIALineNumber::getSourceFileId() const {
  DWORD Id = 0;
  return (S_OK == LineNumber->get_sourceFileId(&Id)) ? Id : 0;
}

uint32_t DIALineNumber::getCompilandId() const {
  DWORD Id = 0;
  return (S_OK == LineNumber->get_compilandId(&Id)) ? Id : 0;
}

bool DIALineNumber::isStatement() const {
  BOOL Statement = 0;
  return (S_OK == LineNumber->get_statement(&Statement)) ? Statement : false;
}
