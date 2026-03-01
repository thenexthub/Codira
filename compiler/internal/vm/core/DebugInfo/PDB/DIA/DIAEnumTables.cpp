//===- DIAEnumTables.cpp - DIA Table Enumerator Impl ------------*- C++ -*-===//
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

#include "vm/core/DebugInfo/PDB/DIA/DIAEnumTables.h"
#include "vm/core/DebugInfo/PDB/DIA/DIATable.h"

using namespace vm::core;
using namespace vm::core::pdb;

DIAEnumTables::DIAEnumTables(CComPtr<IDiaEnumTables> DiaEnumerator)
    : Enumerator(DiaEnumerator) {}

uint32_t DIAEnumTables::getChildCount() const {
  LONG Count = 0;
  return (S_OK == Enumerator->get_Count(&Count)) ? Count : 0;
}

std::unique_ptr<IPDBTable>
DIAEnumTables::getChildAtIndex(uint32_t Index) const {
  CComPtr<IDiaTable> Item;
  VARIANT Var;
  Var.vt = VT_UINT;
  Var.uintVal = Index;
  if (S_OK != Enumerator->Item(Var, &Item))
    return nullptr;

  return std::unique_ptr<IPDBTable>(new DIATable(Item));
}

std::unique_ptr<IPDBTable> DIAEnumTables::getNext() {
  CComPtr<IDiaTable> Item;
  ULONG CeltFetched = 0;
  if (S_OK != Enumerator->Next(1, &Item, &CeltFetched))
    return nullptr;

  return std::unique_ptr<IPDBTable>(new DIATable(Item));
}

void DIAEnumTables::reset() { Enumerator->Reset(); }
