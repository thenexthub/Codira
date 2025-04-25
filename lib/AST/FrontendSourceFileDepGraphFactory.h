//===----- FrontendSourceFileDepGraphFactory.h ------------------*- C++ -*-===//
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

#ifndef FrontendSourceFileDepGraphFactory_h
#define FrontendSourceFileDepGraphFactory_h

#include "language/AST/AbstractSourceFileDepGraphFactory.h"
#include "llvm/Support/VirtualOutputBackend.h"
namespace language {
namespace fine_grained_dependencies {

/// Constructs a SourceFileDepGraph from a *real* \c SourceFile
/// Reads the information provided by the frontend and builds the
/// SourceFileDepGraph

class FrontendSourceFileDepGraphFactory
    : public AbstractSourceFileDepGraphFactory {
  const SourceFile *SF;
  const DependencyTracker &depTracker;

public:
  FrontendSourceFileDepGraphFactory(const SourceFile *SF,
                                    llvm::vfs::OutputBackend &backend,
                                    StringRef outputPath,
                                    const DependencyTracker &depTracker,
                                    bool alsoEmitDotFile);

  ~FrontendSourceFileDepGraphFactory() override = default;

private:
  void addAllDefinedDecls() override;
  void addAllUsedDecls() override;
};

class ModuleDepGraphFactory : public AbstractSourceFileDepGraphFactory {
  const ModuleDecl *Mod;

public:
  ModuleDepGraphFactory(llvm::vfs::OutputBackend &backend,
                        const ModuleDecl *Mod, bool emitDot);

  ~ModuleDepGraphFactory() override = default;

private:
  void addAllDefinedDecls() override;
  void addAllUsedDecls() override {}
};

} // namespace fine_grained_dependencies
} // namespace language

#endif /* FrontendSourceFileDepGraphFactory_h */
