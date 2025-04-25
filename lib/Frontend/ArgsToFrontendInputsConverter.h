//===--- ArgsToFrontendInputsConverter.h ------------------------*- C++ -*-===//
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

#ifndef SWIFT_FRONTEND_ARGSTOFRONTENDINPUTSCONVERTER_H
#define SWIFT_FRONTEND_ARGSTOFRONTENDINPUTSCONVERTER_H

#include "language/AST/DiagnosticConsumer.h"
#include "language/AST/DiagnosticEngine.h"
#include "language/Frontend/FrontendOptions.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Option/ArgList.h"
#include <set>

namespace language {

/// Implement argument semantics in a way that will make it easier to have
/// >1 primary file (or even a primary file list) in the future without
/// breaking anything today.
///
/// Semantics today:
/// If input files are on command line, primary files on command line are also
/// input files; they are not repeated without -primary-file. If input files are
/// in a file list, the primary files on the command line are repeated in the
/// file list. Thus, if there are any primary files, it is illegal to have both
/// (non-primary) input files and a file list. Finally, the order of input files
/// must match the order given on the command line or the file list.
///
/// Side note:
/// since each input file will cause a lot of work for the compiler, this code
/// is biased towards clarity and not optimized.
/// In the near future, it will be possible to put primary files in the
/// filelist, or to have a separate filelist for primaries. The organization
/// here anticipates that evolution.

class ArgsToFrontendInputsConverter {
  DiagnosticEngine &Diags;
  const llvm::opt::ArgList &Args;

  llvm::opt::Arg const *const FilelistPathArg;
  llvm::opt::Arg const *const PrimaryFilelistPathArg;
  llvm::opt::Arg const *const BadFileDescriptorRetryCountArg;

  /// A place to keep alive any buffers that are loaded as part of setting up
  /// the frontend inputs.
  SmallVector<std::unique_ptr<llvm::MemoryBuffer>, 4> ConfigurationFileBuffers;

  llvm::SetVector<StringRef> Files;

public:
  ArgsToFrontendInputsConverter(DiagnosticEngine &diags,
                                const llvm::opt::ArgList &args);

  /// Produces a FrontendInputsAndOutputs object with the inputs populated from
  /// the arguments the converter was initialized with.
  ///
  /// \param buffers If present, buffers read in the processing of the frontend
  /// inputs will be saved here. These should only be used for debugging
  /// purposes.
  std::optional<FrontendInputsAndOutputs>
  convert(SmallVectorImpl<std::unique_ptr<llvm::MemoryBuffer>> *buffers);

private:
  bool enforceFilelistExclusion();
  bool readInputFilesFromCommandLine();
  bool readInputFilesFromFilelist();
  bool forAllFilesInFilelist(llvm::opt::Arg const *const pathArg,
                             llvm::function_ref<void(StringRef)> fn);
  bool addFile(StringRef file);
  std::optional<std::set<StringRef>> readPrimaryFiles();

  /// Returns the newly set-up FrontendInputsAndOutputs, as well as a set of
  /// any unused primary files (those that do not correspond to an input).
  std::pair<FrontendInputsAndOutputs, std::set<StringRef>>
  createInputFilesConsumingPrimaries(std::set<StringRef> primaryFiles);

  /// Emits an error for each file in \p unusedPrimaryFiles.
  ///
  /// \returns true if \p unusedPrimaryFiles is non-empty.
  bool diagnoseUnusedPrimaryFiles(std::set<StringRef> unusedPrimaryFiles);

  bool
  isSingleThreadedWMO(const FrontendInputsAndOutputs &inputsAndOutputs) const;
};

} // namespace language

#endif /* SWIFT_FRONTEND_ARGSTOFRONTENDINPUTSCONVERTER_H */
