//===--- Dependencies.h -- Unified header for dependency tracing utilities --===//
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

#ifndef SWIFT_FRONTENDTOOL_DEPENDENCIES_H
#define SWIFT_FRONTENDTOOL_DEPENDENCIES_H

namespace llvm::vfs {
class OutputBackend;
} // namespace llvm::vfs

namespace language {

class DependencyTracker;
class FrontendOptions;
class InputFile;
class ModuleDecl;
class CompilerInstance;

/// Emit the names of the modules imported by \c mainModule.
bool emitImportedModules(ModuleDecl *mainModule, const FrontendOptions &opts,
                         llvm::vfs::OutputBackend &backend);
bool emitLoadedModuleTraceIfNeeded(ModuleDecl *mainModule,
                                   DependencyTracker *depTracker,
                                   const FrontendOptions &opts,
                                   const InputFile &input);

bool emitFineModuleTraceIfNeeded(CompilerInstance &Instance,
                                 const FrontendOptions &opts);

} // end namespace language

#endif
