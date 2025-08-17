//===--- FileIndexRecord.h - Index data per file ----------------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_LIB_INDEX_FILEINDEXRECORD_H
#define LANGUAGE_CORE_LIB_INDEX_FILEINDEXRECORD_H

#include "language/Core/Basic/SourceLocation.h"
#include "language/Core/Index/DeclOccurrence.h"
#include "language/Core/Index/IndexSymbol.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/SmallVector.h"
#include <vector>

namespace language::Core {
class IdentifierInfo;

namespace index {

/// Stores the declaration occurrences seen in a particular source or header
/// file of a translation unit
class FileIndexRecord {
private:
  FileID FID;
  bool IsSystem;
  mutable bool IsSorted = false;
  mutable std::vector<DeclOccurrence> Decls;

public:
  FileIndexRecord(FileID FID, bool IsSystem) : FID(FID), IsSystem(IsSystem) {}

  ArrayRef<DeclOccurrence> getDeclOccurrencesSortedByOffset() const;

  FileID getFileID() const { return FID; }
  bool isSystem() const { return IsSystem; }

  /// Adds an occurrence of the canonical declaration \c D at the supplied
  /// \c Offset
  ///
  /// \param Roles the roles the occurrence fulfills in this position.
  /// \param Offset the offset in the file of this occurrence.
  /// \param D the canonical declaration this is an occurrence of.
  /// \param Relations the set of symbols related to this occurrence.
  void addDeclOccurence(SymbolRoleSet Roles, unsigned Offset, const Decl *D,
                        ArrayRef<SymbolRelation> Relations);

  /// Adds an occurrence of the given macro at the supplied \c Offset.
  ///
  /// \param Roles the roles the occurrence fulfills in this position.
  /// \param Offset the offset in the file of this occurrence.
  /// \param Name the name of the macro.
  /// \param MI the canonical declaration this is an occurrence of.
  void addMacroOccurence(SymbolRoleSet Roles, unsigned Offset,
                         const IdentifierInfo *Name, const MacroInfo *MI);

  /// Remove any macro occurrences for header guards. When preprocessing, this
  /// will only be accurate after HandleEndOfFile.
  void removeHeaderGuardMacros();

  void print(toolchain::raw_ostream &OS, SourceManager &SM) const;
};

} // end namespace index
} // end namespace language::Core

#endif // LANGUAGE_CORE_LIB_INDEX_FILEINDEXRECORD_H
