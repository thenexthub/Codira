//===-- DataCollection.cpp --------------------------------------*- C++ -*-===//
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

#include "language/Core/AST/DataCollection.h"
#include "language/Core/Basic/SourceManager.h"
#include "language/Core/Lex/Lexer.h"

namespace language::Core {
namespace data_collection {

/// Prints the macro name that contains the given SourceLocation into the given
/// raw_string_ostream.
static void printMacroName(toolchain::raw_string_ostream &MacroStack,
                           ASTContext &Context, SourceLocation Loc) {
  MacroStack << Lexer::getImmediateMacroName(Loc, Context.getSourceManager(),
                                             Context.getLangOpts());

  // Add an empty space at the end as a padding to prevent
  // that macro names concatenate to the names of other macros.
  MacroStack << " ";
}

/// Returns a string that represents all macro expansions that expanded into the
/// given SourceLocation.
///
/// If 'getMacroStack(A) == getMacroStack(B)' is true, then the SourceLocations
/// A and B are expanded from the same macros in the same order.
std::string getMacroStack(SourceLocation Loc, ASTContext &Context) {
  std::string MacroStack;
  toolchain::raw_string_ostream MacroStackStream(MacroStack);
  SourceManager &SM = Context.getSourceManager();

  // Iterate over all macros that expanded into the given SourceLocation.
  while (Loc.isMacroID()) {
    // Add the macro name to the stream.
    printMacroName(MacroStackStream, Context, Loc);
    Loc = SM.getImmediateMacroCallerLoc(Loc);
  }
  return MacroStack;
}

} // end namespace data_collection
} // end namespace language::Core
