//===--- FileIndexRecord.cpp - Index data per file --------------*- C++ -*-===//
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

#include "FileIndexRecord.h"
#include "language/Core/AST/DeclTemplate.h"
#include "language/Core/Basic/SourceManager.h"
#include "toolchain/Support/Path.h"

using namespace language::Core;
using namespace language::Core::index;

ArrayRef<DeclOccurrence>
FileIndexRecord::getDeclOccurrencesSortedByOffset() const {
  if (!IsSorted) {
    toolchain::stable_sort(Decls,
                      [](const DeclOccurrence &A, const DeclOccurrence &B) {
                        return A.Offset < B.Offset;
                      });
    IsSorted = true;
  }
  return Decls;
}

void FileIndexRecord::addDeclOccurence(SymbolRoleSet Roles, unsigned Offset,
                                       const Decl *D,
                                       ArrayRef<SymbolRelation> Relations) {
  assert(D->isCanonicalDecl() &&
         "Occurrences should be associated with their canonical decl");
  IsSorted = false;
  Decls.emplace_back(Roles, Offset, D, Relations);
}

void FileIndexRecord::addMacroOccurence(SymbolRoleSet Roles, unsigned Offset,
                                        const IdentifierInfo *Name,
                                        const MacroInfo *MI) {
  IsSorted = false;
  Decls.emplace_back(Roles, Offset, Name, MI);
}

void FileIndexRecord::removeHeaderGuardMacros() {
  toolchain::erase_if(Decls, [](const DeclOccurrence &D) {
    if (const auto *MI = D.DeclOrMacro.dyn_cast<const MacroInfo *>())
      return MI->isUsedForHeaderGuard();
    return false;
  });
}

void FileIndexRecord::print(toolchain::raw_ostream &OS, SourceManager &SM) const {
  OS << "DECLS BEGIN ---\n";
  for (auto &DclInfo : Decls) {
    if (const auto *D = dyn_cast<const Decl *>(DclInfo.DeclOrMacro)) {
      SourceLocation Loc = SM.getFileLoc(D->getLocation());
      PresumedLoc PLoc = SM.getPresumedLoc(Loc);
      OS << toolchain::sys::path::filename(PLoc.getFilename()) << ':'
         << PLoc.getLine() << ':' << PLoc.getColumn();

      if (const auto *ND = dyn_cast<NamedDecl>(D)) {
        OS << ' ' << ND->getDeclName();
      }
    } else {
      const auto *MI = cast<const MacroInfo *>(DclInfo.DeclOrMacro);
      SourceLocation Loc = SM.getFileLoc(MI->getDefinitionLoc());
      PresumedLoc PLoc = SM.getPresumedLoc(Loc);
      OS << toolchain::sys::path::filename(PLoc.getFilename()) << ':'
         << PLoc.getLine() << ':' << PLoc.getColumn();
      OS << ' ' << DclInfo.MacroName->getName();
    }

    OS << '\n';
  }
  OS << "DECLS END ---\n";
}
