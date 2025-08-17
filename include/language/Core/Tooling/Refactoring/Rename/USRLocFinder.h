//===--- USRLocFinder.h - Clang refactoring library -----------------------===//
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
///
/// \file
/// Provides functionality for finding all instances of a USR in a given
/// AST.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_TOOLING_REFACTORING_RENAME_USRLOCFINDER_H
#define LANGUAGE_CORE_TOOLING_REFACTORING_RENAME_USRLOCFINDER_H

#include "language/Core/AST/AST.h"
#include "language/Core/Tooling/Core/Replacement.h"
#include "language/Core/Tooling/Refactoring/AtomicChange.h"
#include "language/Core/Tooling/Refactoring/Rename/SymbolOccurrences.h"
#include "toolchain/ADT/StringRef.h"
#include <string>
#include <vector>

namespace language::Core {
namespace tooling {

/// Create atomic changes for renaming all symbol references which are
/// identified by the USRs set to a given new name.
///
/// \param USRs The set containing USRs of a particular old symbol.
/// \param NewName The new name to replace old symbol name.
/// \param TranslationUnitDecl The translation unit declaration.
///
/// \return Atomic changes for renaming.
std::vector<tooling::AtomicChange>
createRenameAtomicChanges(toolchain::ArrayRef<std::string> USRs,
                          toolchain::StringRef NewName, Decl *TranslationUnitDecl);

/// Finds the symbol occurrences for the symbol that's identified by the given
/// USR set.
///
/// \return SymbolOccurrences that can be converted to AtomicChanges when
/// renaming.
SymbolOccurrences getOccurrencesOfUSRs(ArrayRef<std::string> USRs,
                                       StringRef PrevName, Decl *Decl);

} // end namespace tooling
} // end namespace language::Core

#endif // LANGUAGE_CORE_TOOLING_REFACTORING_RENAME_USRLOCFINDER_H
