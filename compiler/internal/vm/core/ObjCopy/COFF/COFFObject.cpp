//===- COFFObject.cpp -----------------------------------------------------===//
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

#include "COFFObject.h"
#include "vm/core/ADT/DenseSet.h"

namespace vm::core {
namespace objcopy {
namespace coff {

using namespace object;

void Object::addSymbols(ArrayRef<Symbol> NewSymbols) {
  for (Symbol S : NewSymbols) {
    S.UniqueId = NextSymbolUniqueId++;
    S.OriginalRawIndex = NextSymbolOriginalIndex;
    NextSymbolOriginalIndex += 1 + S.Sym.NumberOfAuxSymbols;
    Symbols.emplace_back(S);
  }
  updateSymbols();
}

void Object::updateSymbols() {
  SymbolMap = DenseMap<size_t, Symbol *>(Symbols.size());
  for (Symbol &Sym : Symbols)
    SymbolMap[Sym.UniqueId] = &Sym;
}

const Symbol *Object::findSymbol(size_t UniqueId) const {
  return SymbolMap.lookup(UniqueId);
}

Error Object::removeSymbols(
    function_ref<Expected<bool>(const Symbol &)> ToRemove) {
  Error Errs = Error::success();
  toolchain::erase_if(Symbols, [ToRemove, &Errs](const Symbol &Sym) {
    Expected<bool> ShouldRemove = ToRemove(Sym);
    if (!ShouldRemove) {
      Errs = joinErrors(std::move(Errs), ShouldRemove.takeError());
      return false;
    }
    return *ShouldRemove;
  });

  updateSymbols();
  return Errs;
}

Error Object::markSymbols() {
  for (Symbol &Sym : Symbols)
    Sym.Referenced = false;
  for (const Section &Sec : Sections) {
    for (const Relocation &R : Sec.Relocs) {
      auto It = SymbolMap.find(R.Target);
      if (It == SymbolMap.end())
        return createStringError(object_error::invalid_symbol_index,
                                 "relocation target %zu not found", R.Target);
      It->second->Referenced = true;
    }
  }
  return Error::success();
}

void Object::addSections(ArrayRef<Section> NewSections) {
  for (Section S : NewSections) {
    S.UniqueId = NextSectionUniqueId++;
    Sections.emplace_back(S);
  }
  updateSections();
}

void Object::updateSections() {
  SectionMap = DenseMap<ssize_t, Section *>(Sections.size());
  size_t Index = 1;
  for (Section &S : Sections) {
    SectionMap[S.UniqueId] = &S;
    S.Index = Index++;
  }
}

const Section *Object::findSection(ssize_t UniqueId) const {
  return SectionMap.lookup(UniqueId);
}

void Object::removeSections(function_ref<bool(const Section &)> ToRemove) {
  DenseSet<ssize_t> AssociatedSections;
  auto RemoveAssociated = [&AssociatedSections](const Section &Sec) {
    return AssociatedSections.contains(Sec.UniqueId);
  };
  do {
    DenseSet<ssize_t> RemovedSections;
    toolchain::erase_if(Sections, [ToRemove, &RemovedSections](const Section &Sec) {
      bool Remove = ToRemove(Sec);
      if (Remove)
        RemovedSections.insert(Sec.UniqueId);
      return Remove;
    });
    // Remove all symbols referring to the removed sections.
    AssociatedSections.clear();
    toolchain::erase_if(
        Symbols, [&RemovedSections, &AssociatedSections](const Symbol &Sym) {
          // If there are sections that are associative to a removed
          // section,
          // remove those as well as nothing will include them (and we can't
          // leave them dangling).
          if (RemovedSections.contains(Sym.AssociativeComdatTargetSectionId))
            AssociatedSections.insert(Sym.TargetSectionId);
          return RemovedSections.contains(Sym.TargetSectionId);
        });
    ToRemove = RemoveAssociated;
  } while (!AssociatedSections.empty());
  updateSections();
  updateSymbols();
}

void Object::truncateSections(function_ref<bool(const Section &)> ToTruncate) {
  for (Section &Sec : Sections) {
    if (ToTruncate(Sec)) {
      Sec.clearContents();
      Sec.Relocs.clear();
      Sec.Header.SizeOfRawData = 0;
    }
  }
}

} // end namespace coff
} // end namespace objcopy
} // end namespace vm::core
