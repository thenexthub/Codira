//===--- PrintingDiagnosticConsumer.h - Print Text Diagnostics --*- C++ -*-===//
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
//  This file defines the PrintingDiagnosticConsumer class, which displays
//  diagnostics as text to a terminal.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_PRINTINGDIAGNOSTICCONSUMER_H
#define LANGUAGE_PRINTINGDIAGNOSTICCONSUMER_H

#include "language/AST/DiagnosticBridge.h"
#include "language/AST/DiagnosticConsumer.h"
#include "language/Basic/DiagnosticOptions.h"
#include "language/Basic/Toolchain.h"

#include "toolchain/Support/Process.h"
#include "toolchain/Support/raw_ostream.h"

namespace language {

/// Diagnostic consumer that displays diagnostics to standard error.
class PrintingDiagnosticConsumer : public DiagnosticConsumer {
  toolchain::raw_ostream &Stream;
  bool ForceColors = false;
  bool EmitMacroExpansionFiles = false;
  bool DidErrorOccur = false;
  DiagnosticOptions::FormattingStyle FormattingStyle =
      DiagnosticOptions::FormattingStyle::LLVM;
  bool SuppressOutput = false;

#if LANGUAGE_BUILD_LANGUAGE_SYNTAX
  /// language-syntax rendering
  DiagnosticBridge DiagBridge;
#endif
 
public:
  PrintingDiagnosticConsumer(toolchain::raw_ostream &stream = toolchain::errs());
  ~PrintingDiagnosticConsumer();

  virtual void handleDiagnostic(SourceManager &SM,
                                const DiagnosticInfo &Info) override;

  virtual bool finishProcessing() override;

  void flush(bool includeTrailingBreak);

  virtual void flush() override { flush(false); }

  void forceColors() {
    ForceColors = true;
    toolchain::sys::Process::UseANSIEscapeCodes(true);
  }

  void setFormattingStyle(DiagnosticOptions::FormattingStyle style) {
    FormattingStyle = style;
  }

  void setEmitMacroExpansionFiles(bool ShouldEmit) {
    EmitMacroExpansionFiles = ShouldEmit;
  }

  bool didErrorOccur() {
    return DidErrorOccur;
  }

  void setSuppressOutput(bool suppressOutput) {
    SuppressOutput = suppressOutput;
  }

private:
  /// Retrieve the SourceFileSyntax for the given buffer.
  void *getSourceFileSyntax(SourceManager &SM, unsigned bufferID,
                            StringRef displayName);

  void queueBuffer(SourceManager &sourceMgr, unsigned bufferID);
  void printDiagnostic(SourceManager &SM, const DiagnosticInfo &Info);
};
  
}

#endif
