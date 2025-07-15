//===--- SymbolGraphGen.h - Codira SymbolGraph Generator -------------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SYMBOLGRAPHGEN_SYMBOLGRAPHGEN_H
#define LANGUAGE_SYMBOLGRAPHGEN_SYMBOLGRAPHGEN_H

#include "language/AST/Module.h"
#include "language/AST/Type.h"
#include "SymbolGraphOptions.h"
#include "PathComponent.h"
#include "FragmentInfo.h"

namespace language {
class ValueDecl;

namespace symbolgraphgen {

/// Emit a Symbol Graph JSON file for a module.
int emitSymbolGraphForModule(ModuleDecl *M, const SymbolGraphOptions &Options);

/// Print a Symbol Graph containing a single node for the given decl to \p OS.
/// The \p ParentContexts out parameter will also be populated with information
/// about each parent context of the given decl, from outermost to innermost.
///
/// \returns \c EXIT_SUCCESS if the kind of the provided node is supported or
/// \c EXIT_FAILURE otherwise.
int printSymbolGraphForDecl(const ValueDecl *D, Type BaseTy,
                            bool InSynthesizedExtension,
                            const SymbolGraphOptions &Options,
                            toolchain::raw_ostream &OS,
                            SmallVectorImpl<PathComponent> &ParentContexts,
                            SmallVectorImpl<FragmentInfo> &FragmentInfo);

} // end namespace symbolgraphgen
} // end namespace language

#endif // LANGUAGE_SYMBOLGRAPHGEN_SYMBOLGRAPHGEN_H
