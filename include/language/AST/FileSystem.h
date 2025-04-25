//===--- FileSystem.h - File helpers that interact with Diags ---*- C++ -*-===//
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

#ifndef SWIFT_AST_FILESYSTEM_H
#define SWIFT_AST_FILESYSTEM_H

#include "language/Basic/FileSystem.h"
#include "language/AST/DiagnosticEngine.h"
#include "language/AST/DiagnosticsCommon.h"
#include "llvm/Support/VirtualOutputBackend.h"
#include "llvm/Support/VirtualOutputConfig.h"

namespace language {
/// A wrapper around llvm::vfs::OutputBackend to handle diagnosing any file
/// system errors during output creation.
///
/// \returns true if there were any errors, either from the filesystem
/// operations or from \p action returning true.
inline bool
withOutputPath(DiagnosticEngine &diags, llvm::vfs::OutputBackend &Backend,
               StringRef outputPath,
               llvm::function_ref<bool(llvm::raw_pwrite_stream &)> action) {
  assert(!outputPath.empty());
  llvm::vfs::OutputConfig config;
  config.setAtomicWrite().setOnlyIfDifferent();

  auto outputFile = Backend.createFile(outputPath, config);
  if (!outputFile) {
    diags.diagnose(SourceLoc(), diag::error_opening_output, outputPath,
                   toString(outputFile.takeError()));
    return true;
  }

  bool failed = action(*outputFile);
  // If there is an error, discard output. Otherwise keep the output file.
  if (auto error = failed ? outputFile->discard() : outputFile->keep()) {
    // Don't diagnose discard error.
    if (failed)
      consumeError(std::move(error));
    else
      diags.diagnose(SourceLoc(), diag::error_closing_output, outputPath,
                     toString(std::move(error)));
    return true;
  }
  return failed;
}
} // end namespace language

#endif // SWIFT_AST_FILESYSTEM_H
