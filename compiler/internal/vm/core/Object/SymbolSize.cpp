//===- SymbolSize.cpp -----------------------------------------------------===//
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

#include "vm/core/Object/SymbolSize.h"
#include "vm/core/ADT/STLExtras.h"
#include "vm/core/Object/COFF.h"
#include "vm/core/Object/ELFObjectFile.h"
#include "vm/core/Object/MachO.h"
#include "vm/core/Object/Wasm.h"
#include "vm/core/Object/XCOFFObjectFile.h"

using namespace vm::core;
using namespace object;

// Orders increasingly by (SectionID, Address).
int toolchain::object::compareAddress(const SymEntry *A, const SymEntry *B) {
  if (A->SectionID != B->SectionID)
    return A->SectionID < B->SectionID ? -1 : 1;
  if (A->Address != B->Address)
    return A->Address < B->Address ? -1 : 1;
  return 0;
}

static unsigned getSectionID(const ObjectFile &O, SectionRef Sec) {
  if (auto *M = dyn_cast<MachOObjectFile>(&O))
    return M->getSectionID(Sec);
  if (isa<WasmObjectFile>(&O))
    return Sec.getIndex();
  if (isa<XCOFFObjectFile>(&O))
    return Sec.getIndex();
  return cast<COFFObjectFile>(O).getSectionID(Sec);
}

static unsigned getSymbolSectionID(const ObjectFile &O, SymbolRef Sym) {
  if (auto *M = dyn_cast<MachOObjectFile>(&O))
    return M->getSymbolSectionID(Sym);
  if (const auto *M = dyn_cast<WasmObjectFile>(&O))
    return M->getSymbolSectionId(Sym);
  if (const auto *M = dyn_cast<XCOFFObjectFile>(&O))
    return M->getSymbolSectionID(Sym);
  return cast<COFFObjectFile>(O).getSymbolSectionID(Sym);
}

std::vector<std::pair<SymbolRef, uint64_t>>
toolchain::object::computeSymbolSizes(const ObjectFile &O) {
  std::vector<std::pair<SymbolRef, uint64_t>> Ret;

  if (const auto *E = dyn_cast<ELFObjectFileBase>(&O)) {
    auto Syms = E->symbols();
    if (Syms.empty())
      Syms = E->getDynamicSymbolIterators();
    for (ELFSymbolRef Sym : Syms)
      Ret.push_back({Sym, Sym.getSize()});
    return Ret;
  }

  if (const auto *E = dyn_cast<XCOFFObjectFile>(&O)) {
    for (XCOFFSymbolRef Sym : E->symbols())
      Ret.push_back({Sym, Sym.getSize()});
    return Ret;
  }

  if (const auto *E = dyn_cast<WasmObjectFile>(&O)) {
    for (SymbolRef Sym : E->symbols()) {
      Ret.push_back({Sym, E->getSymbolSize(Sym)});
    }
    return Ret;
  }

  // Collect sorted symbol addresses. Include dummy addresses for the end
  // of each section.
  std::vector<SymEntry> Addresses;
  unsigned SymNum = 0;
  for (symbol_iterator I = O.symbol_begin(), E = O.symbol_end(); I != E; ++I) {
    SymbolRef Sym = *I;
    Expected<uint64_t> ValueOrErr = Sym.getValue();
    if (!ValueOrErr)
      // TODO: Actually report errors helpfully.
      report_fatal_error(ValueOrErr.takeError());
    Addresses.push_back({I, *ValueOrErr, SymNum, getSymbolSectionID(O, Sym)});
    ++SymNum;
  }
  for (SectionRef Sec : O.sections()) {
    uint64_t Address = Sec.getAddress();
    uint64_t Size = Sec.getSize();
    Addresses.push_back(
        {O.symbol_end(), Address + Size, 0, getSectionID(O, Sec)});
  }

  if (Addresses.empty())
    return Ret;

  array_pod_sort(Addresses.begin(), Addresses.end(), compareAddress);

  // Compute the size as the gap to the next symbol. If multiple symbols have
  // the same address, give both the same size. Because Addresses is sorted,
  // use two pointers to keep track of the current symbol vs. the next symbol
  // that doesn't have the same address for size computation.
  for (unsigned I = 0, NextI = 0, N = Addresses.size() - 1; I < N; ++I) {
    auto &P = Addresses[I];
    if (P.I == O.symbol_end())
      continue;

    // If the next pointer is behind, update it to the next symbol.
    if (NextI <= I) {
      NextI = I + 1;
      while (NextI < N && Addresses[NextI].Address == P.Address)
        ++NextI;
    }

    uint64_t Size = Addresses[NextI].Address - P.Address;
    P.Address = Size;
  }

  // Assign the sorted symbols in the original order.
  Ret.resize(SymNum);
  for (SymEntry &P : Addresses) {
    if (P.I == O.symbol_end())
      continue;
    Ret[P.Number] = {*P.I, P.Address};
  }
  return Ret;
}
