//===--- DiagnosticBridge.h - Diagnostic Bridge to CodiraSyntax --*- C++ -*-===//
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
//  This file declares the DiagnosticBridge class, which bridges to language-syntax
//  for diagnostics printing.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_BASIC_DIAGNOSTICBRIDGE_H
#define LANGUAGE_BASIC_DIAGNOSTICBRIDGE_H

#include "language/AST/DiagnosticConsumer.h"
#include "language/Basic/SourceManager.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/Support/raw_ostream.h"

namespace language {
/// Declare the bridge between language-syntax and language-frontend for diagnostics
/// handling. The methods in this class should only be called when
/// `LANGUAGE_BUILD_LANGUAGE_SYNTAX` is defined as they are otherwise undefined.
class DiagnosticBridge {
  /// A queued up source file known to the queued diagnostics.
  using QueuedBuffer = void *;

  /// Per-frontend state maintained on the Codira side.
  void *perFrontendState = nullptr;

  /// The queued diagnostics structure.
  void *queuedDiagnostics = nullptr;
  toolchain::DenseMap<unsigned, QueuedBuffer> queuedBuffers;

  /// Source file syntax nodes cached by { source manager, buffer ID }.
  toolchain::DenseMap<std::pair<SourceManager *, unsigned>, void *> sourceFileSyntax;

public:
  /// Enqueue diagnostics.
  void enqueueDiagnostic(SourceManager &SM, const DiagnosticInfo &Info,
                         unsigned innermostBufferID);

  /// Emit a single diagnostic without location information.
  void emitDiagnosticWithoutLocation(
      const DiagnosticInfo &Info, toolchain::raw_ostream &out, bool forceColors);

  /// Flush all enqueued diagnostics.
  void flush(toolchain::raw_ostream &OS, bool includeTrailingBreak,
             bool forceColors);

  /// Retrieve the stack of source buffers from the provided location out to
  /// a physical source file, with source buffer IDs for each step along the way
  /// due to (e.g.) macro expansions or generated code.
  ///
  /// The resulting vector will always contain valid source locations. If the
  /// initial location is invalid, the result will be empty.
  static SmallVector<unsigned, 1> getSourceBufferStack(SourceManager &sourceMgr,
                                                       SourceLoc loc);

  /// Print the category footnotes as part of teardown.
  void printCategoryFootnotes(toolchain::raw_ostream &os, bool forceColors);

  DiagnosticBridge() = default;
  ~DiagnosticBridge();

private:
  /// Retrieve the SourceFileSyntax for the given buffer.
  void *getSourceFileSyntax(SourceManager &SM, unsigned bufferID,
                            StringRef displayName);

  void queueBuffer(SourceManager &sourceMgr, unsigned bufferID);
};
} // namespace language

#endif
