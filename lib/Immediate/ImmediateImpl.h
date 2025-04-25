//===--- ImmediateImpl.h - Support functions for immediate mode -*- C++ -*-===//
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

#ifndef SWIFT_IMMEDIATEIMPL_H
#define SWIFT_IMMEDIATEIMPL_H

#include "language/AST/LinkLibrary.h"
#include "language/AST/SearchPathOptions.h"
#include "language/Basic/LLVM.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"

namespace llvm {
  class Function;
  class Module;
}

namespace language {
  class CompilerInstance;
  class DiagnosticEngine;
  class GeneratedModule;
  class IRGenOptions;
  class ModuleDecl;
  class SILOptions;

namespace immediate {

/// Returns a handle to the runtime suitable for other \c dlsym or \c dlclose
/// calls or \c null if an error occurred.
///
/// \param runtimeLibPaths Paths to search for stdlib dylibs.
void *loadSwiftRuntime(ArrayRef<std::string> runtimeLibPaths);
bool tryLoadLibraries(ArrayRef<LinkLibrary> LinkLibraries,
                      SearchPathOptions SearchPathOpts,
                      DiagnosticEngine &Diags);
bool linkLLVMModules(llvm::Module *Module,
                     std::unique_ptr<llvm::Module> &&SubModule);
bool autolinkImportedModules(ModuleDecl *M, const IRGenOptions &IRGenOpts);

} // end namespace immediate
} // end namespace language

#endif

