//===--- FixitApplyDiagnosticConsumer.h -------------------------*- C++ -*-===//
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
// This class records compiler interesting fix-its as textual edits.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_MIGRATOR_FIXITAPPLYDIAGNOSTICCONSUMER_H
#define LANGUAGE_MIGRATOR_FIXITAPPLYDIAGNOSTICCONSUMER_H

#include "language/AST/DiagnosticConsumer.h"
#include "language/Migrator/FixitFilter.h"
#include "language/Migrator/Migrator.h"
#include "language/Migrator/Replacement.h"
#include "clang/Rewrite/Core/RewriteBuffer.h"
#include "toolchain/ADT/SmallSet.h"

namespace language {

class CompilerInvocation;
struct DiagnosticInfo;
struct MigratorOptions;
class SourceManager;

namespace migrator {

struct Replacement;

class FixitApplyDiagnosticConsumer final
  : public DiagnosticConsumer, public FixitFilter {
  clang::RewriteBuffer RewriteBuf;

  /// The entire text of the input file.
  const StringRef Text;

  /// The name of the buffer, which should be the absolute path of the input
  /// filename.
  const StringRef BufferName;

  /// The number of fix-its pushed into the rewrite buffer. Use this to
  /// determine whether to call `printResult`.
  unsigned NumFixitsApplied;

  /// Tracks previous replacements so we don't pump the rewrite buffer with
  /// multiple equivalent replacements, which can result in weird behavior.
  toolchain::SmallSet<Replacement, 32> Replacements;

public:
  FixitApplyDiagnosticConsumer(const StringRef Text,
                               const StringRef BufferName);

  /// Print the resulting text, applying the caught fix-its to the given
  /// output stream.
  void printResult(toolchain::raw_ostream &OS) const;

  void handleDiagnostic(SourceManager &SM, const DiagnosticInfo &Info) override;

  unsigned getNumFixitsApplied() const {
    return NumFixitsApplied;
  }
};
} // end namespace migrator
} // end namespace language

#endif // LANGUAGE_MIGRATOR_FIXITAPPLYDIAGNOSTICCONSUMER_H
