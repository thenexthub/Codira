//===--- ArgsToFrontendOutputsConverter.h -----------------------*- C++ -*-===//
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

#ifndef LANGUAGE_FRONTEND_ARGSTOFRONTENDOUTPUTSCONVERTER_H
#define LANGUAGE_FRONTEND_ARGSTOFRONTENDOUTPUTSCONVERTER_H

#include "language/AST/DiagnosticConsumer.h"
#include "language/AST/DiagnosticEngine.h"
#include "language/Basic/Toolchain.h"
#include "language/Basic/SupplementaryOutputPaths.h"
#include "language/Frontend/FrontendOptions.h"
#include "language/Option/Options.h"
#include "toolchain/Option/ArgList.h"

#include <vector>

namespace language {
class OutputFileMap;

/// Given the command line arguments and information about the inputs,
/// Fill in all the information in FrontendInputsAndOutputs.

class ArgsToFrontendOutputsConverter {
  const toolchain::opt::ArgList &Args;
  StringRef ModuleName;
  FrontendInputsAndOutputs &InputsAndOutputs;
  DiagnosticEngine &Diags;

public:
  ArgsToFrontendOutputsConverter(const toolchain::opt::ArgList &args,
                                 StringRef moduleName,
                                 FrontendInputsAndOutputs &inputsAndOutputs,
                                 DiagnosticEngine &diags)
      : Args(args), ModuleName(moduleName), InputsAndOutputs(inputsAndOutputs),
        Diags(diags) {}

  bool convert(std::vector<std::string> &mainOutputs,
               std::vector<std::string> &mainOutputsForIndexUnits,
               std::vector<SupplementaryOutputPaths> &supplementaryOutputs);

  /// Try to read an output file list file.
  /// \returns `None` if it could not open the filelist.
  static std::optional<std::vector<std::string>>
  readOutputFileList(StringRef filelistPath, DiagnosticEngine &diags);
};

struct OutputOptInfo {
  StringRef PrettyName;
  options::ID SingleID;
  options::ID FilelistID;
  StringRef SingleOptSpelling;
};

class OutputFilesComputer {
  DiagnosticEngine &Diags;
  const FrontendInputsAndOutputs &InputsAndOutputs;
  const std::vector<std::string> OutputFileArguments;
  const std::string OutputDirectoryArgument;
  const std::string FirstInput;
  const FrontendOptions::ActionType RequestedAction;
  const toolchain::opt::Arg *const ModuleNameArg;
  const StringRef Suffix;
  const bool HasTextualOutput;
  const OutputOptInfo OutputInfo;

  OutputFilesComputer(DiagnosticEngine &diags,
                      const FrontendInputsAndOutputs &inputsAndOutputs,
                      std::vector<std::string> outputFileArguments,
                      StringRef outputDirectoryArgument, StringRef firstInput,
                      FrontendOptions::ActionType requestedAction,
                      const toolchain::opt::Arg *moduleNameArg, StringRef suffix,
                      bool hasTextualOutput,
                      OutputOptInfo optInfo);

public:
  static std::optional<OutputFilesComputer>
  create(const toolchain::opt::ArgList &args, DiagnosticEngine &diags,
         const FrontendInputsAndOutputs &inputsAndOutputs,
         OutputOptInfo optInfo);

  /// \return the output filenames on the command line or in the output
  /// filelist. If there
  /// were neither -o's nor an output filelist, returns an empty vector.
  static std::optional<std::vector<std::string>>
  getOutputFilenamesFromCommandLineOrFilelist(const toolchain::opt::ArgList &args,
                                              DiagnosticEngine &diags,
                                              options::ID singleOpt,
                                              options::ID filelistOpt);

  std::optional<std::vector<std::string>> computeOutputFiles() const;

private:
  std::optional<std::string> computeOutputFile(StringRef outputArg,
                                               const InputFile &input) const;

  /// \return the correct output filename when none was specified.
  ///
  /// Such an absence should only occur when invoking the frontend
  /// without the driver,
  /// because the driver will always pass -o with an appropriate filename
  /// if output is required for the requested action.
  std::optional<std::string>
  deriveOutputFileFromInput(const InputFile &input) const;

  /// \return the correct output filename when a directory was specified.
  ///
  /// Such a specification should only occur when invoking the frontend
  /// directly, because the driver will always pass -o with an appropriate
  /// filename if output is required for the requested action.
  std::optional<std::string>
  deriveOutputFileForDirectory(const InputFile &input) const;

  std::string determineBaseNameOfOutput(const InputFile &input) const;

  std::string deriveOutputFileFromParts(StringRef dir, StringRef base) const;
};

class SupplementaryOutputPathsComputer {
  const toolchain::opt::ArgList &Args;
  DiagnosticEngine &Diags;
  const FrontendInputsAndOutputs &InputsAndOutputs;
  ArrayRef<std::string> OutputFiles;
  StringRef ModuleName;

  const FrontendOptions::ActionType RequestedAction;

public:
  SupplementaryOutputPathsComputer(
      const toolchain::opt::ArgList &args, DiagnosticEngine &diags,
      const FrontendInputsAndOutputs &inputsAndOutputs,
      ArrayRef<std::string> outputFiles, StringRef moduleName);
  std::optional<std::vector<SupplementaryOutputPaths>>
  computeOutputPaths() const;

private:
  /// \Return a set of supplementary output paths for each input that might
  /// produce supplementary outputs, or None to signal an error.
  /// \note
  /// Batch-mode supports multiple primary inputs.
  /// \par
  /// The paths are derived from arguments
  /// such as -emit-module-path. These are not the final computed paths,
  /// merely the ones passed in via the command line.
  /// \note
  /// In the future, these will also include those passed in via whatever
  /// filelist scheme gets implemented to handle cases where the command line
  /// arguments become burdensome.
  std::optional<std::vector<SupplementaryOutputPaths>>
  getSupplementaryOutputPathsFromArguments() const;

  /// Read a supplementary output file map file.
  /// \returns `None` if it could not open the file map.
  std::optional<std::vector<SupplementaryOutputPaths>>
  readSupplementaryOutputFileMap() const;

  /// Given an ID corresponding to supplementary output argument
  /// (e.g. -emit-module-path), collect all such paths, and ensure
  /// there are the right number of them.
  std::optional<std::vector<std::string>>
  getSupplementaryFilenamesFromArguments(options::ID pathID) const;

  std::optional<SupplementaryOutputPaths> computeOutputPathsForOneInput(
      StringRef outputFilename,
      const SupplementaryOutputPaths &pathsFromFilelists,
      const InputFile &) const;

  StringRef deriveDefaultSupplementaryOutputPathExcludingExtension(
      StringRef outputFilename, const InputFile &) const;

  /// \return empty string if no output file.
  std::string determineSupplementaryOutputFilename(
      options::ID emitOpt, std::string pathFromArgumentsOrFilelists,
      file_types::ID type, StringRef mainOutputIfUsable,
      StringRef defaultSupplementaryOutputPathExcludingExtension,
      bool forceDefaultSupplementaryOutputPathExcludingExtension = false) const;

  void deriveModulePathParameters(StringRef mainOutputFile,
                                  options::ID &emitOption,
                                  std::string &extension,
                                  std::string &mainOutputIfUsable) const;
};

} // namespace language

#endif /* LANGUAGE_FRONTEND_ARGSTOFRONTENDOUTPUTSCONVERTER_H */
