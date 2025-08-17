//===- ModuleFile.cpp - Module description --------------------------------===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
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
//  This file implements the ModuleFile class, which describes a module that
//  has been loaded from an AST file.
//
//===----------------------------------------------------------------------===//

#include "language/Core/Serialization/ModuleFile.h"
#include "ASTReaderInternals.h"
#include "language/Core/Serialization/ContinuousRangeMap.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/Compiler.h"
#include "toolchain/Support/raw_ostream.h"

using namespace language::Core;
using namespace serialization;
using namespace reader;

ModuleFile::~ModuleFile() {
  delete static_cast<ASTIdentifierLookupTable *>(IdentifierLookupTable);
  delete static_cast<HeaderFileInfoLookupTable *>(HeaderFileInfoTable);
  delete static_cast<ASTSelectorLookupTable *>(SelectorLookupTable);
}

template<typename Key, typename Offset, unsigned InitialCapacity>
static void
dumpLocalRemap(StringRef Name,
               const ContinuousRangeMap<Key, Offset, InitialCapacity> &Map) {
  if (Map.begin() == Map.end())
    return;

  using MapType = ContinuousRangeMap<Key, Offset, InitialCapacity>;

  toolchain::errs() << "  " << Name << ":\n";
  for (typename MapType::const_iterator I = Map.begin(), IEnd = Map.end();
       I != IEnd; ++I) {
    toolchain::errs() << "    " << I->first << " -> " << I->second << "\n";
  }
}

LLVM_DUMP_METHOD void ModuleFile::dump() {
  toolchain::errs() << "\nModule: " << FileName << "\n";
  if (!Imports.empty()) {
    toolchain::errs() << "  Imports: ";
    for (unsigned I = 0, N = Imports.size(); I != N; ++I) {
      if (I)
        toolchain::errs() << ", ";
      toolchain::errs() << Imports[I]->FileName;
    }
    toolchain::errs() << "\n";
  }

  // Remapping tables.
  toolchain::errs() << "  Base source location offset: " << SLocEntryBaseOffset
               << '\n';

  toolchain::errs() << "  Base identifier ID: " << BaseIdentifierID << '\n'
               << "  Number of identifiers: " << LocalNumIdentifiers << '\n';

  toolchain::errs() << "  Base macro ID: " << BaseMacroID << '\n'
               << "  Number of macros: " << LocalNumMacros << '\n';
  dumpLocalRemap("Macro ID local -> global map", MacroRemap);

  toolchain::errs() << "  Base submodule ID: " << BaseSubmoduleID << '\n'
               << "  Number of submodules: " << LocalNumSubmodules << '\n';
  dumpLocalRemap("Submodule ID local -> global map", SubmoduleRemap);

  toolchain::errs() << "  Base selector ID: " << BaseSelectorID << '\n'
               << "  Number of selectors: " << LocalNumSelectors << '\n';
  dumpLocalRemap("Selector ID local -> global map", SelectorRemap);

  toolchain::errs() << "  Base preprocessed entity ID: " << BasePreprocessedEntityID
               << '\n'
               << "  Number of preprocessed entities: "
               << NumPreprocessedEntities << '\n';
  dumpLocalRemap("Preprocessed entity ID local -> global map",
                 PreprocessedEntityRemap);

  toolchain::errs() << "  Base type index: " << BaseTypeIndex << '\n'
               << "  Number of types: " << LocalNumTypes << '\n';

  toolchain::errs() << "  Base decl index: " << BaseDeclIndex << '\n'
               << "  Number of decls: " << LocalNumDecls << '\n';
}
