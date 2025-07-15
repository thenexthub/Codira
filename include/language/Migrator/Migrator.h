//===--- Migrator.h - Migrator ----------------------------------*- C++ -*-===//
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
// The top-level Codira Migrator driver.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_MIGRATOR_MIGRATOR_H
#define LANGUAGE_MIGRATOR_MIGRATOR_H

#include "language/Migrator/MigrationState.h"
#include "toolchain/ADT/IntrusiveRefCntPtr.h"

namespace language {
class CompilerInstance;

namespace migrator {

/// If needed, run the migrator on the compiler invocation's input file and emit
/// a "replacement map" describing the requested changes to the source file.
/// \return true on error.
bool updateCodeAndEmitRemapIfNeeded(CompilerInstance *Instance);

/// Specify options when running syntactic migration pass.
struct SyntacticPassOptions {
  bool RunOptionalTryMigration = false;
};

struct Migrator {
  CompilerInstance *StartInstance;
  const CompilerInvocation &StartInvocation;
  SourceManager SrcMgr;
  std::vector<toolchain::IntrusiveRefCntPtr<MigrationState>> States;

  Migrator(CompilerInstance *StartInstance,
           const CompilerInvocation &StartInvocation);

  /// The maximum number of times to run the compiler over the input to get
  /// fix-its. Nullability changes may expose other fix-its in subsequent
  /// compilations.
  static constexpr unsigned MaxCompilerFixitPassIterations = 7;

  /// Repeatedly perform a number of compiler-fix-it migrations in a row, until
  /// there are no new suggestions from the compiler or some other error
  /// occurred.
  ///
  /// Returns the last CompilerInstance used in the iterations, provided
  /// that the CompilerInvocation used to set it up was successful. Otherwise,
  /// returns nullptr.
  std::unique_ptr<language::CompilerInstance>
  repeatFixitMigrations(const unsigned Iterations,
                        language::version::Version CodiraLanguageVersion);

  /// Perform a single compiler fix-it migration on the last state, and push
  /// the result onto the state history.
  ///
  /// Returns the CompilerInstance used for the fix-it run, provided its
  /// setup from a CompilerInvocation was successful.
  std::unique_ptr<language::CompilerInstance>
  performAFixItMigration(language::version::Version CodiraLanguageVersion);

  /// Starting with the last state, perform the following migration passes.
  ///
  /// Returns true if failed:
  ///   - Setting up the Codira CompilerInstance failed.
  ///   - performSema emitted fatal errors along the way.
  bool performSyntacticPasses(SyntacticPassOptions Opts);

  /// Emit a replacement map from the very start state's output text to the
  /// final state's output text to the StartInvocation's output file.
  bool emitRemap() const;

  /// Emit the output text of the final state in States to the path specified
  /// by -emit-migrated-file-path in StartInvocation.MigratorOptions.
  ///
  /// Returns true if an attempt was made and failed.
  bool emitMigratedFile() const;

  /// Dump all of the migration states encountered so far to
  /// StartInvocation.MigratorOptions.DumpMigrationStatesDir.
  ///
  /// Returns true if an attempt was made and failed.
  bool dumpStates() const;

  /// Get the options for the Migrator.
  const MigratorOptions &getMigratorOptions() const;

  /// Get the filename of the input given by this->StartInvocation.
  const StringRef getInputFilename() const;
};

} // end namespace migrator
} // end namespace language

#endif // LANGUAGE_MIGRATOR_MIGRATOR_H
