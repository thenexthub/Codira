//===--- MakeStyleDependencies.h -- header for emitting dependency files --===//
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
//
//  This file defines interface for emitting Make Style Dependency Files.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_FRONTEND_MAKESTYLEDEPENDENCIES_H
#define SWIFT_FRONTEND_MAKESTYLEDEPENDENCIES_H

#include "llvm/ADT/StringRef.h"

namespace llvm {
class raw_ostream;
} // namespace llvm

namespace language {

class CompilerInstance;
class DiagnosticEngine;
class FrontendOptions;
class InputFile;

/// Emit make style dependency file from compiler instance if needed.
bool emitMakeDependenciesIfNeeded(CompilerInstance &instance,
                                  const InputFile &input);

/// Emit make style dependency file from a serialized buffer.
bool emitMakeDependenciesFromSerializedBuffer(llvm::StringRef buffer,
                                              llvm::raw_ostream &os,
                                              const FrontendOptions &opts,
                                              const InputFile &input,
                                              DiagnosticEngine &diags);

} // end namespace language

#endif
