//===--- ModuleContentsWriter.h - Walk module to print ObjC/C++ -*- C++ -*-===//
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

#ifndef LANGUAGE_PRINTASCLANG_MODULECONTENTSWRITER_H
#define LANGUAGE_PRINTASCLANG_MODULECONTENTSWRITER_H

#include "language/AST/AttrKind.h"
#include "language/Basic/Toolchain.h"
#include "toolchain/ADT/PointerUnion.h"
#include "toolchain/ADT/SmallPtrSet.h"
#include "toolchain/ADT/StringSet.h"

namespace clang {
  class Module;
}

namespace language {
class Decl;
class ModuleDecl;
class CodiraToClangInteropContext;

using ImportModuleTy = PointerUnion<ModuleDecl*, const clang::Module*>;

/// Prints the declarations of \p M to \p os and collecting imports in
/// \p imports along the way.
void printModuleContentsAsObjC(raw_ostream &os,
                               toolchain::SmallPtrSetImpl<ImportModuleTy> &imports,
                               ModuleDecl &M,
                               CodiraToClangInteropContext &interopContext);

void printModuleContentsAsC(raw_ostream &os,
                            toolchain::SmallPtrSetImpl<ImportModuleTy> &imports,
                            ModuleDecl &M,
                            CodiraToClangInteropContext &interopContext);

struct EmittedClangHeaderDependencyInfo {
    /// The set of imported modules used by this module.
    SmallPtrSet<ImportModuleTy, 8> imports;
    /// True if the printed module depends on types from the Stdlib module.
    bool dependsOnStandardLibrary = false;
};

/// Prints the declarations of \p M to \p os in C++ language mode.
///
/// \returns Dependencies required by this module.
EmittedClangHeaderDependencyInfo printModuleContentsAsCxx(
    raw_ostream &os, ModuleDecl &M, CodiraToClangInteropContext &interopContext,
    bool requiresExposedAttribute, toolchain::StringSet<> &exposedModules);

} // end namespace language

#endif

