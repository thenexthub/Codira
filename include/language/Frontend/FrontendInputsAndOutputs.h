//===--- FrontendInputs.h ---------------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_FRONTEND_FRONTENDINPUTS_H
#define LANGUAGE_FRONTEND_FRONTENDINPUTS_H

#include "language/Basic/PrimarySpecificPaths.h"
#include "language/Basic/SupplementaryOutputPaths.h"
#include "language/Frontend/InputFile.h"
#include "toolchain/ADT/Hashing.h"
#include "toolchain/ADT/StringMap.h"

#include <string>
#include <vector>

namespace toolchain {
class MemoryBuffer;
}

namespace language {

class DiagnosticEngine;

/// Information about all the inputs and outputs to the frontend.

class FrontendInputsAndOutputs {
  friend class ArgsToFrontendInputsConverter;

  std::vector<InputFile> AllInputs;
  toolchain::StringMap<unsigned> PrimaryInputsByName;
  std::vector<unsigned> PrimaryInputsInOrder;

  /// The file type for main output files. Assuming all inputs produce the
  /// same kind of output.
  file_types::ID PrincipalOutputType = file_types::ID::TY_INVALID;

  /// In Single-threaded WMO mode, all inputs are used
  /// both for importing and compiling.
  bool IsSingleThreadedWMO = false;

  /// Punt where needed to enable batch mode experiments.
  bool AreBatchModeChecksBypassed = false;

  /// Recover missing inputs. Note that recovery itself is users responsibility.
  bool ShouldRecoverMissingInputs = false;

public:
  bool areBatchModeChecksBypassed() const { return AreBatchModeChecksBypassed; }
  void setBypassBatchModeChecks(bool bbc) { AreBatchModeChecksBypassed = bbc; }

  FrontendInputsAndOutputs() = default;
  FrontendInputsAndOutputs(const FrontendInputsAndOutputs &other);
  FrontendInputsAndOutputs &operator=(const FrontendInputsAndOutputs &other);

  // Whole-module-optimization (WMO) routines:

  // SingleThreadedWMO produces only main output file. In contrast,
  // multi-threaded WMO produces one main output per input, as single-file and
  // batch-mode do for each primary. Both WMO modes produce only one set of
  // supplementary outputs.

  bool isSingleThreadedWMO() const { return IsSingleThreadedWMO; }
  void setIsSingleThreadedWMO(bool istw) { IsSingleThreadedWMO = istw; }

  bool isWholeModule() const { return !hasPrimaryInputs(); }

  bool shouldRecoverMissingInputs() { return ShouldRecoverMissingInputs; }
  void setShouldRecoverMissingInputs() { ShouldRecoverMissingInputs = true; }

  // Readers:

  // All inputs:

  ArrayRef<InputFile> getAllInputs() const { return AllInputs; }

  std::vector<std::string> getInputFilenames() const;

  /// \return nullptr if not a primary input file.
  const InputFile *primaryInputNamed(StringRef name) const;

  unsigned inputCount() const { return AllInputs.size(); }

  bool hasInputs() const { return !AllInputs.empty(); }

  bool hasSingleInput() const { return inputCount() == 1; }

  const InputFile &firstInput() const { return AllInputs[0]; }
  InputFile &firstInput() { return AllInputs[0]; }

  const InputFile &lastInput() const { return AllInputs.back(); }

  const std::string &getFilenameOfFirstInput() const;

  bool isReadingFromStdin() const;

  /// If \p fn returns true, exits early and returns true.
  bool forEachInput(toolchain::function_ref<bool(const InputFile &)> fn) const;

  // Primaries:

  const InputFile &firstPrimaryInput() const;
  const InputFile &lastPrimaryInput() const;

  /// If \p fn returns true, exit early and return true.
  bool
  forEachPrimaryInput(toolchain::function_ref<bool(const InputFile &)> fn) const;

  /// Iterates over primary inputs, exposing their unique ordered index
  /// If \p fn returns true, exit early and return true.
  bool forEachPrimaryInputWithIndex(
      toolchain::function_ref<bool(const InputFile &, unsigned index)> fn) const;

  /// If \p fn returns true, exit early and return true.
  bool
  forEachNonPrimaryInput(toolchain::function_ref<bool(const InputFile &)> fn) const;

  unsigned primaryInputCount() const { return PrimaryInputsInOrder.size(); }

  // Primary count readers:

  bool hasUniquePrimaryInput() const { return primaryInputCount() == 1; }

  bool hasPrimaryInputs() const { return primaryInputCount() > 0; }

  bool hasMultiplePrimaryInputs() const { return primaryInputCount() > 1; }

  /// Fails an assertion if there is more than one primary input.
  /// Used in situations where only one primary input can be handled
  /// and where batch mode has not been implemented yet.
  void assertMustNotBeMoreThanOnePrimaryInput() const;

  /// Fails an assertion when there is more than one primary input unless
  /// the experimental -bypass-batch-mode-checks argument was passed to
  /// the front end.
  /// FIXME: When batch mode is complete, this function should be obsolete.
  void
  assertMustNotBeMoreThanOnePrimaryInputUnlessBatchModeChecksHaveBeenBypassed()
      const;

  // Count-dependent readers:

  /// \return the unique primary input, if one exists.
  const InputFile *getUniquePrimaryInput() const;

  const InputFile &getRequiredUniquePrimaryInput() const;

  /// FIXME: Should combine all primaries for the result
  /// instead of just answering "batch" if there is more than one primary.
  std::string getStatsFileMangledInputName() const;

  const InputFile &getFirstOutputProducingInput() const;

  unsigned getIndexOfFirstOutputProducingInput() const;

  bool isInputPrimary(StringRef file) const;

  unsigned numberOfPrimaryInputsEndingWith(StringRef extension) const;

  // Multi-facet readers

  // If we have exactly one input filename, and its extension is "bc" or "ll",
  // treat the input as TOOLCHAIN_IR.
  bool shouldTreatAsLLVM() const;
  bool shouldTreatAsSIL() const;
  bool shouldTreatAsModuleInterface() const;
  bool shouldTreatAsNonPackageModuleInterface() const;
  bool shouldTreatAsObjCHeader() const;

  bool areAllNonPrimariesSIB() const;

  /// \return true for error
  bool verifyInputs(DiagnosticEngine &diags, bool treatAsSIL,
                    bool isREPLRequested, bool isNoneRequested) const;

  // Changing inputs

public:
  void clearInputs();
  void addInput(const InputFile &input);
  void addInputFile(StringRef file, toolchain::MemoryBuffer *buffer = nullptr);
  void addPrimaryInputFile(StringRef file,
                           toolchain::MemoryBuffer *buffer = nullptr);

  // Outputs

private:
  friend class ArgsToFrontendOptionsConverter;
  friend struct InterfaceSubContextDelegateImpl;
  void setMainAndSupplementaryOutputs(
      ArrayRef<std::string> outputFiles,
      ArrayRef<SupplementaryOutputPaths> supplementaryOutputs,
      ArrayRef<std::string> outputFilesForIndexUnits = {});
  void setPrincipalOutputType(file_types::ID type) {
    PrincipalOutputType = type;
  }

public:
  unsigned countOfInputsProducingMainOutputs() const;

  bool hasInputsProducingMainOutputs() const {
    return countOfInputsProducingMainOutputs() != 0;
  }
  file_types::ID getPrincipalOutputType() const { return PrincipalOutputType; }

  const InputFile &firstInputProducingOutput() const;
  const InputFile &lastInputProducingOutput() const;

  /// Under single-threaded WMO, we pretend that the first input
  /// generates the main output, even though it will include code
  /// generated from all of them.
  ///
  /// If \p fn returns true, return early and return true.
  bool forEachInputProducingAMainOutputFile(
      toolchain::function_ref<bool(const InputFile &)> fn) const;

  std::vector<std::string> copyOutputFilenames() const;
  std::vector<std::string> copyIndexUnitOutputFilenames() const;

  void forEachOutputFilename(toolchain::function_ref<void(StringRef)> fn) const;

  /// Gets the name of the specified output filename.
  /// If multiple files are specified, the last one is returned.
  std::string getSingleOutputFilename() const;

  /// Gets the name of the specified output filename to record in the index unit
  /// output files. If multiple are specified, the last one is returned.
  std::string getSingleIndexUnitOutputFilename() const;

  bool isOutputFilenameStdout() const;
  bool isOutputFileDirectory() const;
  bool hasNamedOutputFile() const;

  // Supplementary outputs

  unsigned countOfFilesProducingSupplementaryOutput() const;

  /// If \p fn returns true, exit early and return true.
  bool forEachInputProducingSupplementaryOutput(
      toolchain::function_ref<bool(const InputFile &)> fn) const;

  /// Assumes there is not more than one primary input file, if any.
  /// Otherwise, you would need to call getPrimarySpecificPathsForPrimary
  /// to tell it which primary input you wanted the outputs for.
  const PrimarySpecificPaths &
  getPrimarySpecificPathsForAtMostOnePrimary() const;

  const PrimarySpecificPaths &
  getPrimarySpecificPathsForRemaining(unsigned i) const;

  const PrimarySpecificPaths &
      getPrimarySpecificPathsForPrimary(StringRef) const;

  bool hasSupplementaryOutputPath(
      toolchain::function_ref<const std::string &(const SupplementaryOutputPaths &)>
          extractorFn) const;

#define OUTPUT(NAME, TYPE)                                                     \
  bool has##NAME() const {                                                     \
    return hasSupplementaryOutputPath(                                         \
        [](const SupplementaryOutputPaths &outs) -> const std::string & {      \
          return outs.NAME;                                                    \
        });                                                                    \
  }
#include "language/Basic/SupplementaryOutputPaths.def"
#undef OUTPUT

  bool hasDependencyTrackerPath() const;
};

} // namespace language

#endif // LANGUAGE_FRONTEND_FRONTENDINPUTS_H
