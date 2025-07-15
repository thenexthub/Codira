//===--- CodiraToClangInteropContext.h - Interop context ---------*- C++ -*-===//
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

#ifndef LANGUAGE_PRINTASCLANG_LANGUAGETOCLANGINTEROPCONTEXT_H
#define LANGUAGE_PRINTASCLANG_LANGUAGETOCLANGINTEROPCONTEXT_H

#include "language/AST/ASTAllocated.h"
#include "language/AST/Module.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/STLExtras.h"
#include "toolchain/ADT/StringSet.h"
#include <memory>
#include <optional>

namespace language {

class Decl;
class IRABIDetailsProvider;
class IRGenOptions;
class ModuleDecl;
class ExtensionDecl;
class NominalTypeDecl;

/// The \c CodiraToClangInteropContext class is responsible for providing
/// access to the other required subsystems of the compiler during the emission
/// of a clang header. It provides access to the other subsystems lazily to
/// ensure avoid any additional setup cost that's not required.
class CodiraToClangInteropContext {
public:
  CodiraToClangInteropContext(ModuleDecl &mod, const IRGenOptions &irGenOpts);
  ~CodiraToClangInteropContext();

  IRABIDetailsProvider &getIrABIDetails();
  const ASTContext &getASTContext() { return mod.getASTContext(); }

  // Runs the given function if we haven't emitted some context-specific stub
  // for the given concrete stub name.
  void runIfStubForDeclNotEmitted(toolchain::StringRef stubName,
                                  toolchain::function_ref<void(void)> function);

  void recordExtensions(const NominalTypeDecl *typeDecl,
                        const ExtensionDecl *ext);

  toolchain::ArrayRef<const ExtensionDecl *>
  getExtensionsForNominalType(const NominalTypeDecl *typeDecl) const;

private:
  ModuleDecl &mod;
  const IRGenOptions &irGenOpts;
  std::unique_ptr<IRABIDetailsProvider> irABIDetails;
  toolchain::StringSet<> emittedStubs;
  toolchain::DenseMap<const NominalTypeDecl *, std::vector<const ExtensionDecl *>>
      extensions;
};

} // end namespace language

#endif
