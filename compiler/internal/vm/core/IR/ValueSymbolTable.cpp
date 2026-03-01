//===- ValueSymbolTable.cpp - Implement the ValueSymbolTable class --------===//
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
//
// This file implements the ValueSymbolTable class for the IR library.
//
//===----------------------------------------------------------------------===//

#include "vm/core/IR/ValueSymbolTable.h"
#include "vm/core/ADT/SmallString.h"
#include "vm/core/Config/toolchain-config.h"
#include "vm/core/IR/GlobalValue.h"
#include "vm/core/IR/Module.h"
#include "vm/core/IR/Type.h"
#include "vm/core/IR/Value.h"
#include "vm/core/Support/Casting.h"
#include "vm/core/Support/Compiler.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/raw_ostream.h"
#include "vm/core/TargetParser/Triple.h"
#include <cassert>

using namespace vm::core;

#define DEBUG_TYPE "valuesymtab"

// Class destructor
ValueSymbolTable::~ValueSymbolTable() {
#ifndef NDEBUG // Only do this in -g mode...
  for (const auto &VI : vmap)
    dbgs() << "Value still in symbol table! Type = '"
           << *VI.getValue()->getType() << "' Name = '" << VI.getKeyData()
           << "'\n";
  assert(vmap.empty() && "Values remain in symbol table!");
#endif
}

ValueName *ValueSymbolTable::makeUniqueName(Value *V,
                                            SmallString<256> &UniqueName) {
  unsigned BaseSize = UniqueName.size();
  bool AppenDot = false;
  if (auto *GV = dyn_cast<GlobalValue>(V)) {
    // A dot is appended to mark it as clone during ABI demangling so that
    // for example "_Z1fv" and "_Z1fv.1" both demangle to "f()", the second
    // one being a clone.
    // On NVPTX we cannot use a dot because PTX only allows [A-Za-z0-9_$] for
    // identifiers. This breaks ABI demangling but at least ptxas accepts and
    // compiles the program.
    const Module *M = GV->getParent();
    if (!(M && M->getTargetTriple().isNVPTX()))
      AppenDot = true;
  }

  while (true) {
    // Trim any suffix off and append the next number.
    UniqueName.resize(BaseSize);
    raw_svector_ostream S(UniqueName);
    if (AppenDot)
      S << ".";
    S << ++LastUnique;

    // Retry if MaxNameSize has been exceeded.
    if (MaxNameSize > -1 && UniqueName.size() > (size_t)MaxNameSize) {
      assert(BaseSize >= UniqueName.size() - (size_t)MaxNameSize &&
             "Can't generate unique name: MaxNameSize is too small.");
      BaseSize -= UniqueName.size() - (size_t)MaxNameSize;
      continue;
    }
    // Try insert the vmap entry with this suffix.
    auto IterBool = vmap.insert(std::make_pair(UniqueName.str(), V));
    if (IterBool.second)
      return &*IterBool.first;
  }
}

// Insert a value into the symbol table with the specified name...
//
void ValueSymbolTable::reinsertValue(Value *V) {
  assert(V->hasName() && "Can't insert nameless Value into symbol table");

  // Try inserting the name, assuming it won't conflict.
  if (vmap.insert(V->getValueName())) {
    // LLVM_DEBUG(dbgs() << " Inserted value: " << V->getValueName() << ": " <<
    // *V << "\n");
    return;
  }

  // Otherwise, there is a naming conflict.  Rename this value.
  SmallString<256> UniqueName(V->getName().begin(), V->getName().end());

  // The name is too already used, just free it so we can allocate a new name.
  MallocAllocator Allocator;
  V->getValueName()->Destroy(Allocator);

  ValueName *VN = makeUniqueName(V, UniqueName);
  V->setValueName(VN);
}

void ValueSymbolTable::removeValueName(ValueName *V) {
  // LLVM_DEBUG(dbgs() << " Removing Value: " << V->getKeyData() << "\n");
  // Remove the value from the symbol table.
  vmap.remove(V);
}

/// createValueName - This method attempts to create a value name and insert
/// it into the symbol table with the specified name.  If it conflicts, it
/// auto-renames the name and returns that instead.
ValueName *ValueSymbolTable::createValueName(StringRef Name, Value *V) {
  if (MaxNameSize > -1 && Name.size() > (unsigned)MaxNameSize)
    Name = Name.substr(0, std::max(1u, (unsigned)MaxNameSize));

  // In the common case, the name is not already in the symbol table.
  auto IterBool = vmap.insert(std::make_pair(Name, V));
  if (IterBool.second) {
    // LLVM_DEBUG(dbgs() << " Inserted value: " << Entry.getKeyData() << ": "
    //           << *V << "\n");
    return &*IterBool.first;
  }

  // Otherwise, there is a naming conflict.  Rename this value.
  SmallString<256> UniqueName(Name.begin(), Name.end());
  return makeUniqueName(V, UniqueName);
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// dump - print out the symbol table
//
LLVM_DUMP_METHOD void ValueSymbolTable::dump() const {
  // dbgs() << "ValueSymbolTable:\n";
  for (const auto &I : *this) {
    // dbgs() << "  '" << I->getKeyData() << "' = ";
    I.getValue()->dump();
    // dbgs() << "\n";
  }
}
#endif
