//===--- ArgsToFrontendOutputsConverter.cpp -------------------------------===//
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

#include "ArgsToFrontendOutputsConverter.h"

#include "ArgsToFrontendInputsConverter.h"
#include "ArgsToFrontendOptionsConverter.h"
#include "language/AST/DiagnosticsFrontend.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/OutputFileMap.h"
#include "language/Basic/Platform.h"
#include "language/Frontend/Frontend.h"
#include "language/Option/Options.h"
#include "language/Option/SanitizerOptions.h"
#include "language/Strings.h"
#include "toolchain/ADT/STLExtras.h"
#include "toolchain/TargetParser/Triple.h"
#include "toolchain/Option/Arg.h"
#include "toolchain/Option/ArgList.h"
#include "toolchain/Option/Option.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/LineIterator.h"
#include "toolchain/Support/Path.h"

using namespace language;
using namespace toolchain::opt;

bool ArgsToFrontendOutputsConverter::convert(
    std::vector<std::string> &mainOutputs,
    std::vector<std::string> &mainOutputsForIndexUnits,
    std::vector<SupplementaryOutputPaths> &supplementaryOutputs) {

  std::optional<OutputFilesComputer> ofc = OutputFilesComputer::create(
      Args, Diags, InputsAndOutputs,
      {"output", options::OPT_o, options::OPT_output_filelist, "-o"});
  if (!ofc)
    return true;
  std::optional<std::vector<std::string>> mains = ofc->computeOutputFiles();
  if (!mains)
    return true;

  std::optional<std::vector<std::string>> indexMains;
  if (Args.hasArg(options::OPT_index_unit_output_path,
                  options::OPT_index_unit_output_path_filelist)) {
    std::optional<OutputFilesComputer> iuofc = OutputFilesComputer::create(
        Args, Diags, InputsAndOutputs,
        {"index unit output path", options::OPT_index_unit_output_path,
         options::OPT_index_unit_output_path_filelist,
         "-index-unit-output-path"});
    if (!iuofc)
      return true;
    indexMains = iuofc->computeOutputFiles();
    if (!indexMains)
      return true;

    assert(mains->size() == indexMains->size() && "checks not equivalent?");
  }

  std::optional<std::vector<SupplementaryOutputPaths>> supplementaries =
      SupplementaryOutputPathsComputer(Args, Diags, InputsAndOutputs, *mains,
                                       ModuleName)
          .computeOutputPaths();
  if (!supplementaries)
    return true;

  mainOutputs = std::move(*mains);
  if (indexMains)
    mainOutputsForIndexUnits = std::move(*indexMains);
  supplementaryOutputs = std::move(*supplementaries);
  return false;
}

std::optional<std::vector<std::string>>
ArgsToFrontendOutputsConverter::readOutputFileList(const StringRef filelistPath,
                                                   DiagnosticEngine &diags) {
  toolchain::ErrorOr<std::unique_ptr<toolchain::MemoryBuffer>> buffer =
      toolchain::MemoryBuffer::getFile(filelistPath);
  if (!buffer) {
    diags.diagnose(SourceLoc(), diag::cannot_open_file, filelistPath,
                   buffer.getError().message());
    return std::nullopt;
  }
  std::vector<std::string> outputFiles;
  for (StringRef line : make_range(toolchain::line_iterator(*buffer.get()), {})) {
    outputFiles.push_back(line.str());
  }
  return outputFiles;
}

std::optional<std::vector<std::string>>
OutputFilesComputer::getOutputFilenamesFromCommandLineOrFilelist(
    const ArgList &args, DiagnosticEngine &diags, options::ID singleOpt,
    options::ID filelistOpt) {
  if (const Arg *A = args.getLastArg(filelistOpt)) {
    assert(!args.hasArg(singleOpt) &&
           "don't use -o with -output-filelist or -index-unit-output-path with "
           " -index-unit-output-filelist");
    return ArgsToFrontendOutputsConverter::readOutputFileList(A->getValue(),
                                                              diags);
  }
  return args.getAllArgValues(singleOpt);
}

std::optional<OutputFilesComputer> OutputFilesComputer::create(
    const toolchain::opt::ArgList &args, DiagnosticEngine &diags,
    const FrontendInputsAndOutputs &inputsAndOutputs, OutputOptInfo optInfo) {
  std::optional<std::vector<std::string>> outputArguments =
      getOutputFilenamesFromCommandLineOrFilelist(args, diags, optInfo.SingleID,
                                                  optInfo.FilelistID);
  if (!outputArguments)
    return std::nullopt;
  const StringRef outputDirectoryArgument =
      outputArguments->size() == 1 &&
              toolchain::sys::fs::is_directory(outputArguments->front())
          ? StringRef(outputArguments->front())
          : StringRef();
  ArrayRef<std::string> outputFileArguments =
      outputDirectoryArgument.empty() ? ArrayRef<std::string>(*outputArguments)
                                      : ArrayRef<std::string>();
  const StringRef firstInput =
      inputsAndOutputs.hasSingleInput()
          ? StringRef(inputsAndOutputs.getFilenameOfFirstInput())
          : StringRef();
  const FrontendOptions::ActionType requestedAction =
      ArgsToFrontendOptionsConverter::determineRequestedAction(args);

  if (!outputFileArguments.empty() &&
      outputFileArguments.size() !=
          inputsAndOutputs.countOfInputsProducingMainOutputs()) {
    diags.diagnose(
        SourceLoc(),
        diag::error_if_any_output_files_are_specified_they_all_must_be,
        optInfo.PrettyName);
    return std::nullopt;
  }

  const file_types::ID outputType =
      FrontendOptions::formatForPrincipalOutputFileForAction(requestedAction);

  return OutputFilesComputer(
      diags, inputsAndOutputs, std::move(outputFileArguments),
      outputDirectoryArgument, firstInput, requestedAction,
      args.getLastArg(options::OPT_module_name),
      file_types::getExtension(outputType),
      FrontendOptions::doesActionProduceTextualOutput(requestedAction),
      optInfo);
}

OutputFilesComputer::OutputFilesComputer(
    DiagnosticEngine &diags,
    const FrontendInputsAndOutputs &inputsAndOutputs,
    std::vector<std::string> outputFileArguments,
    const StringRef outputDirectoryArgument, const StringRef firstInput,
    const FrontendOptions::ActionType requestedAction,
    const toolchain::opt::Arg *moduleNameArg, const StringRef suffix,
    const bool hasTextualOutput, OutputOptInfo optInfo)
    : Diags(diags), InputsAndOutputs(inputsAndOutputs),
      OutputFileArguments(outputFileArguments),
      OutputDirectoryArgument(outputDirectoryArgument), FirstInput(firstInput),
      RequestedAction(requestedAction), ModuleNameArg(moduleNameArg),
      Suffix(suffix), HasTextualOutput(hasTextualOutput),
      OutputInfo(optInfo) {}

std::optional<std::vector<std::string>>
OutputFilesComputer::computeOutputFiles() const {
  std::vector<std::string> outputFiles;
  unsigned i = 0;
  bool hadError = InputsAndOutputs.forEachInputProducingAMainOutputFile(
      [&](const InputFile &input) -> bool {

        StringRef outputArg = OutputFileArguments.empty()
                                  ? StringRef()
                                  : StringRef(OutputFileArguments[i++]);

        std::optional<std::string> outputFile =
            computeOutputFile(outputArg, input);
        if (!outputFile)
          return true;
        outputFiles.push_back(*outputFile);
        return false;
      });
  return hadError ? std::nullopt
                  : std::optional<std::vector<std::string>>(outputFiles);
}

std::optional<std::string>
OutputFilesComputer::computeOutputFile(StringRef outputArg,
                                       const InputFile &input) const {
  // Return an empty string to signify no output.
  // The frontend does not currently produce a diagnostic
  // if a -o argument is present for such an action
  // for instance languagec -frontend -o foo -interpret foo.code
  if (!FrontendOptions::doesActionProduceOutput(RequestedAction))
    return std::string();

  if (!OutputDirectoryArgument.empty())
    return deriveOutputFileForDirectory(input);

  if (!outputArg.empty())
    return outputArg.str();

  return deriveOutputFileFromInput(input);
}

std::optional<std::string>
OutputFilesComputer::deriveOutputFileFromInput(const InputFile &input) const {
  if (input.getFileName() == "-" || HasTextualOutput)
    return std::string("-");

  std::string baseName = determineBaseNameOfOutput(input);
  if (baseName.empty()) {
    // Assuming FrontendOptions::doesActionProduceOutput(RequestedAction)
    Diags.diagnose(SourceLoc(), diag::error_no_output_filename_specified,
                   OutputInfo.PrettyName);
    return std::nullopt;
  }
  return deriveOutputFileFromParts("", baseName);
}

std::optional<std::string> OutputFilesComputer::deriveOutputFileForDirectory(
    const InputFile &input) const {
  std::string baseName = determineBaseNameOfOutput(input);
  if (baseName.empty()) {
    Diags.diagnose(SourceLoc(), diag::error_implicit_output_file_is_directory,
                   OutputDirectoryArgument, OutputInfo.SingleOptSpelling);
    return std::nullopt;
  }
  return deriveOutputFileFromParts(OutputDirectoryArgument, baseName);
}

std::string
OutputFilesComputer::determineBaseNameOfOutput(const InputFile &input) const {
  std::string nameToStem =
      input.isPrimary()
          ? input.getFileName()
          : ModuleNameArg ? ModuleNameArg->getValue() : FirstInput;
  return toolchain::sys::path::stem(nameToStem).str();
}

std::string
OutputFilesComputer::deriveOutputFileFromParts(StringRef dir,
                                               StringRef base) const {
  assert(!base.empty());
  toolchain::SmallString<128> path(dir);
  toolchain::sys::path::append(path, base);
  toolchain::sys::path::replace_extension(path, Suffix);
  return std::string(path.str());
}

SupplementaryOutputPathsComputer::SupplementaryOutputPathsComputer(
    const ArgList &args, DiagnosticEngine &diags,
    const FrontendInputsAndOutputs &inputsAndOutputs,
    ArrayRef<std::string> outputFiles, StringRef moduleName)
    : Args(args), Diags(diags), InputsAndOutputs(inputsAndOutputs),
      OutputFiles(outputFiles), ModuleName(moduleName),
      RequestedAction(
          ArgsToFrontendOptionsConverter::determineRequestedAction(Args)) {}

std::optional<std::vector<SupplementaryOutputPaths>>
SupplementaryOutputPathsComputer::computeOutputPaths() const {
  std::optional<std::vector<SupplementaryOutputPaths>> pathsFromUser =
      Args.hasArg(options::OPT_supplementary_output_file_map)
          ? readSupplementaryOutputFileMap()
          : getSupplementaryOutputPathsFromArguments();
  if (!pathsFromUser)
    return std::nullopt;

  if (InputsAndOutputs.hasPrimaryInputs())
    assert(OutputFiles.size() == pathsFromUser->size());
  else if (InputsAndOutputs.isSingleThreadedWMO())
    assert(OutputFiles.size() == pathsFromUser->size() &&
           pathsFromUser->size() == 1);
  else {
    // Multi-threaded WMO is the exception
    assert(OutputFiles.size() == InputsAndOutputs.inputCount() &&
           pathsFromUser->size() == (InputsAndOutputs.hasInputs() ? 1 : 0));
  }

  std::vector<SupplementaryOutputPaths> outputPaths;
  unsigned i = 0;
  bool hadError = InputsAndOutputs.forEachInputProducingSupplementaryOutput(
      [&](const InputFile &input) -> bool {
        if (auto suppPaths = computeOutputPathsForOneInput(
                OutputFiles[i], (*pathsFromUser)[i], input)) {
          ++i;
          outputPaths.push_back(*suppPaths);
          return false;
        }
        return true;
      });
  if (hadError)
    return std::nullopt;

  // In WMO mode compute supplementary output paths for optimization record
  // data (opt-remarks). We need one path per LLVMModule that will be created as
  // part of wmo.
  if (!InputsAndOutputs.hasPrimaryInputs()) {
    unsigned i = 0;
    InputsAndOutputs.forEachInput([&](const InputFile &input) -> bool {
      // First input is already computed.
      if (InputsAndOutputs.firstInput().getFileName() == input.getFileName()) {
        ++i;
        return false;
      }
      SupplementaryOutputPaths outputs;

      // Compute auxiliar opt record paths.
      if(OutputFiles.size() > 1) {
        StringRef defaultSupplementaryOutputPathExcludingExtension =
            deriveDefaultSupplementaryOutputPathExcludingExtension(
              OutputFiles[i], input);

        auto YAMLOptRecordPath = determineSupplementaryOutputFilename(
          options::OPT_save_optimization_record,
          "",
          file_types::TY_YAMLOptRecord, "",
          defaultSupplementaryOutputPathExcludingExtension, true);
        outputs.YAMLOptRecordPath = YAMLOptRecordPath;

        auto bitstreamOptRecordPath = determineSupplementaryOutputFilename(
          options::OPT_save_optimization_record,
          "",
          file_types::TY_BitstreamOptRecord, "",
          defaultSupplementaryOutputPathExcludingExtension, true);

        outputs.BitstreamOptRecordPath = bitstreamOptRecordPath;
      }

      outputPaths.emplace_back(std::move(outputs));
      ++i;
      return false;
    });
  }
  return outputPaths;
}

std::optional<std::vector<SupplementaryOutputPaths>>
SupplementaryOutputPathsComputer::getSupplementaryOutputPathsFromArguments()
    const {

  auto clangHeaderOutput = getSupplementaryFilenamesFromArguments(
      options::OPT_emit_objc_header_path);
  auto moduleOutput =
      getSupplementaryFilenamesFromArguments(options::OPT_emit_module_path);
  auto moduleDocOutput =
      getSupplementaryFilenamesFromArguments(options::OPT_emit_module_doc_path);
  auto dependenciesFile = getSupplementaryFilenamesFromArguments(
      options::OPT_emit_dependencies_path);
  auto referenceDependenciesFile = getSupplementaryFilenamesFromArguments(
      options::OPT_emit_reference_dependencies_path);
  auto serializedDiagnostics = getSupplementaryFilenamesFromArguments(
      options::OPT_serialize_diagnostics_path);
  auto loadedModuleTrace = getSupplementaryFilenamesFromArguments(
      options::OPT_emit_loaded_module_trace_path);
  auto TBD = getSupplementaryFilenamesFromArguments(options::OPT_emit_tbd_path);
  auto moduleInterfaceOutput = getSupplementaryFilenamesFromArguments(
      options::OPT_emit_module_interface_path);
  auto privateModuleInterfaceOutput = getSupplementaryFilenamesFromArguments(
      options::OPT_emit_private_module_interface_path);
  auto packageModuleInterfaceOutput = getSupplementaryFilenamesFromArguments(options::OPT_emit_package_module_interface_path);
  auto moduleSourceInfoOutput = getSupplementaryFilenamesFromArguments(
      options::OPT_emit_module_source_info_path);
  auto moduleSummaryOutput = getSupplementaryFilenamesFromArguments(
      options::OPT_emit_module_summary_path);
  auto abiDescriptorOutput = getSupplementaryFilenamesFromArguments(
      options::OPT_emit_abi_descriptor_path);
  auto apiDescriptorOutput = getSupplementaryFilenamesFromArguments(
      options::OPT_emit_api_descriptor_path);
  auto constValuesOutput = getSupplementaryFilenamesFromArguments(
      options::OPT_emit_const_values_path);
  auto moduleSemanticInfoOutput = getSupplementaryFilenamesFromArguments(
      options::OPT_emit_module_semantic_info_path);
  auto optRecordOutput = getSupplementaryFilenamesFromArguments(
      options::OPT_save_optimization_record_path);
  if (!clangHeaderOutput || !moduleOutput || !moduleDocOutput ||
      !dependenciesFile || !referenceDependenciesFile ||
      !serializedDiagnostics || !loadedModuleTrace || !TBD ||
      !moduleInterfaceOutput || !privateModuleInterfaceOutput || !packageModuleInterfaceOutput ||
      !moduleSourceInfoOutput || !moduleSummaryOutput || !abiDescriptorOutput ||
      !moduleSemanticInfoOutput || !optRecordOutput) {
    return std::nullopt;
  }
  std::vector<SupplementaryOutputPaths> result;

  const unsigned N =
      InputsAndOutputs.countOfFilesProducingSupplementaryOutput();
  for (unsigned i = 0; i < N; ++i) {
    SupplementaryOutputPaths sop;
    sop.ClangHeaderOutputPath = (*clangHeaderOutput)[i];
    sop.ModuleOutputPath = (*moduleOutput)[i];
    sop.ModuleDocOutputPath = (*moduleDocOutput)[i];
    sop.DependenciesFilePath = (*dependenciesFile)[i];
    sop.ReferenceDependenciesFilePath = (*referenceDependenciesFile)[i];
    sop.SerializedDiagnosticsPath = (*serializedDiagnostics)[i];
    sop.LoadedModuleTracePath = (*loadedModuleTrace)[i];
    sop.TBDPath = (*TBD)[i];
    sop.ModuleInterfaceOutputPath = (*moduleInterfaceOutput)[i];
    sop.PrivateModuleInterfaceOutputPath = (*privateModuleInterfaceOutput)[i];
    sop.PackageModuleInterfaceOutputPath = (*packageModuleInterfaceOutput)[i];
    sop.ModuleSourceInfoOutputPath = (*moduleSourceInfoOutput)[i];
    sop.ModuleSummaryOutputPath = (*moduleSummaryOutput)[i];
    sop.ABIDescriptorOutputPath = (*abiDescriptorOutput)[i];
    sop.APIDescriptorOutputPath = (*apiDescriptorOutput)[i];
    sop.ConstValuesOutputPath = (*constValuesOutput)[i];
    sop.ModuleSemanticInfoOutputPath = (*moduleSemanticInfoOutput)[i];
    sop.YAMLOptRecordPath = (*optRecordOutput)[i];
    sop.BitstreamOptRecordPath = (*optRecordOutput)[i];
    result.push_back(sop);
  }
  return result;
}

// Extend this routine for filelists if/when we have them.

std::optional<std::vector<std::string>>
SupplementaryOutputPathsComputer::getSupplementaryFilenamesFromArguments(
    options::ID pathID) const {
  std::vector<std::string> paths = Args.getAllArgValues(pathID);

  const unsigned N =
      InputsAndOutputs.countOfFilesProducingSupplementaryOutput();

  if (paths.size() == N)
    return paths;
  else if (pathID == options::OPT_emit_loaded_module_trace_path &&
           paths.size() < N) {
    // We only need one file to output the module trace file because they
    // are all equivalent. Add additional empty output paths for module trace to
    // make sure the compiler won't panic for diag::error_wrong_number_of_arguments.
    for(unsigned I = paths.size(); I != N; I ++)
      paths.emplace_back();
    return paths;
  }

  if (paths.empty())
    return std::vector<std::string>(N, std::string());

  Diags.diagnose(SourceLoc(), diag::error_wrong_number_of_arguments,
                 Args.getLastArg(pathID)->getOption().getPrefixedName(), N,
                 paths.size());
  return std::nullopt;
}

static bool shouldEmitFineModuleTrace(FrontendOptions::ActionType action) {
  // Only full compilation jobs should emit fine module tracing file.
  // Other partial compilation jobs, such as emitting modules, only typecheck partially
  // so walking into every function bodies may be risky.
  switch(action) {
  case language::FrontendOptions::ActionType::Typecheck:
  case language::FrontendOptions::ActionType::EmitSILGen:
  case language::FrontendOptions::ActionType::EmitSIL:
  case language::FrontendOptions::ActionType::EmitAssembly:
  case language::FrontendOptions::ActionType::EmitLoweredSIL:
  case language::FrontendOptions::ActionType::EmitIRGen:
  case language::FrontendOptions::ActionType::EmitIR:
  case language::FrontendOptions::ActionType::EmitBC:
  case language::FrontendOptions::ActionType::EmitObject:
    return true;
  case language::FrontendOptions::ActionType::NoneAction:
  case language::FrontendOptions::ActionType::Parse:
  case language::FrontendOptions::ActionType::ResolveImports:
  case language::FrontendOptions::ActionType::DumpParse:
  case language::FrontendOptions::ActionType::DumpInterfaceHash:
  case language::FrontendOptions::ActionType::DumpAST:
  case language::FrontendOptions::ActionType::PrintAST:
  case language::FrontendOptions::ActionType::PrintASTDecl:
  case language::FrontendOptions::ActionType::DumpScopeMaps:
  case language::FrontendOptions::ActionType::DumpAvailabilityScopes:
  case language::FrontendOptions::ActionType::EmitImportedModules:
  case language::FrontendOptions::ActionType::EmitPCH:
  case language::FrontendOptions::ActionType::EmitModuleOnly:
  case language::FrontendOptions::ActionType::MergeModules:
  case language::FrontendOptions::ActionType::CompileModuleFromInterface:
  case language::FrontendOptions::ActionType::TypecheckModuleFromInterface:
  case language::FrontendOptions::ActionType::EmitSIBGen:
  case language::FrontendOptions::ActionType::EmitSIB:
  case language::FrontendOptions::ActionType::Immediate:
  case language::FrontendOptions::ActionType::REPL:
  case language::FrontendOptions::ActionType::DumpTypeInfo:
  case language::FrontendOptions::ActionType::EmitPCM:
  case language::FrontendOptions::ActionType::DumpPCM:
  case language::FrontendOptions::ActionType::ScanDependencies:
  case language::FrontendOptions::ActionType::PrintVersion:
  case language::FrontendOptions::ActionType::PrintArguments:
    return false;
  }
}

std::optional<SupplementaryOutputPaths>
SupplementaryOutputPathsComputer::computeOutputPathsForOneInput(
    StringRef outputFile, const SupplementaryOutputPaths &pathsFromArguments,
    const InputFile &input) const {
  StringRef defaultSupplementaryOutputPathExcludingExtension =
      deriveDefaultSupplementaryOutputPathExcludingExtension(outputFile, input);

  using namespace options;

  auto dependenciesFilePath = determineSupplementaryOutputFilename(
      OPT_emit_dependencies, pathsFromArguments.DependenciesFilePath,
      file_types::TY_Dependencies, "",
      defaultSupplementaryOutputPathExcludingExtension);

  auto referenceDependenciesFilePath = determineSupplementaryOutputFilename(
      OPT_emit_reference_dependencies,
      pathsFromArguments.ReferenceDependenciesFilePath,
      file_types::TY_CodiraDeps, "",
      defaultSupplementaryOutputPathExcludingExtension);

  auto constValuesOutputPath = determineSupplementaryOutputFilename(
      OPT_emit_const_values,
      pathsFromArguments.ConstValuesOutputPath,
      file_types::TY_ConstValues, "",
      defaultSupplementaryOutputPathExcludingExtension);

  auto serializedDiagnosticsPath = determineSupplementaryOutputFilename(
      OPT_serialize_diagnostics, pathsFromArguments.SerializedDiagnosticsPath,
      file_types::TY_SerializedDiagnostics, "",
      defaultSupplementaryOutputPathExcludingExtension);

  // There is no non-path form of -emit-fixits-path
  auto fixItsOutputPath = pathsFromArguments.FixItsOutputPath;

  auto clangHeaderOutputPath = determineSupplementaryOutputFilename(
      OPT_emit_objc_header, pathsFromArguments.ClangHeaderOutputPath,
      file_types::TY_ClangHeader, "",
      defaultSupplementaryOutputPathExcludingExtension);

  auto loadedModuleTracePath = determineSupplementaryOutputFilename(
      OPT_emit_loaded_module_trace, pathsFromArguments.LoadedModuleTracePath,
      file_types::TY_ModuleTrace, "",
      defaultSupplementaryOutputPathExcludingExtension);

  // We piggy-back on the loadedModuleTracePath to decide (1) whether
  // to emit the fine module Trace file, and (2) where to emit the fine module
  // trace file if the path isn't explicitly given by
  // LANGUAGE_COMPILER_FINE_GRAINED_TRACE_PATH.
  // FIXME: we probably need to move this to a frontend argument.
  toolchain::SmallString<128> FineModuleTracePath;
  if (!loadedModuleTracePath.empty() &&
      shouldEmitFineModuleTrace(RequestedAction) &&
      !Args.hasArg(OPT_disable_fine_module_tracing)) {
    if (const char *P = ::getenv("LANGUAGE_COMPILER_FINE_GRAINED_TRACE_PATH")) {
      StringRef FilePath = P;
      toolchain::sys::path::append(FineModuleTracePath, FilePath);
    } else {
      toolchain::sys::path::append(FineModuleTracePath, loadedModuleTracePath);
      toolchain::sys::path::remove_filename(FineModuleTracePath);
      toolchain::sys::path::append(FineModuleTracePath,
                              ".code_FINE_DEPENDENCY_TRACE.json");
    }
  }

  auto tbdPath = determineSupplementaryOutputFilename(
      OPT_emit_tbd, pathsFromArguments.TBDPath, file_types::TY_TBD, "",
      defaultSupplementaryOutputPathExcludingExtension);

  auto moduleDocOutputPath = determineSupplementaryOutputFilename(
      OPT_emit_module_doc, pathsFromArguments.ModuleDocOutputPath,
      file_types::TY_CodiraModuleDocFile, "",
      defaultSupplementaryOutputPathExcludingExtension);

  auto moduleSourceInfoOutputPath = determineSupplementaryOutputFilename(
      OPT_emit_module_source_info, pathsFromArguments.ModuleSourceInfoOutputPath,
      file_types::TY_CodiraSourceInfoFile, "",
      defaultSupplementaryOutputPathExcludingExtension);
  auto moduleSummaryOutputPath = determineSupplementaryOutputFilename(
      OPT_emit_module_summary, pathsFromArguments.ModuleSummaryOutputPath,
      file_types::TY_CodiraModuleSummaryFile, "",
      defaultSupplementaryOutputPathExcludingExtension);

  // There is no non-path form of -emit-interface-path
  auto ModuleInterfaceOutputPath =
      pathsFromArguments.ModuleInterfaceOutputPath;
  auto PrivateModuleInterfaceOutputPath =
      pathsFromArguments.PrivateModuleInterfaceOutputPath;
  auto PackageModuleInterfaceOutputPath =
      pathsFromArguments.PackageModuleInterfaceOutputPath;

  // There is no non-path form of -emit-abi-descriptor-path
  auto ABIDescriptorOutputPath = pathsFromArguments.ABIDescriptorOutputPath;

  // There is no non-path form of -emit-api-descriptor-path
  auto APIDescriptorOutputPath = pathsFromArguments.APIDescriptorOutputPath;

  // There is no non-path form of -emit-module-semantic-info-path
  auto ModuleSemanticInfoOutputPath =
      pathsFromArguments.ModuleSemanticInfoOutputPath;

  ID emitModuleOption;
  std::string moduleExtension;
  std::string mainOutputIfUsableForModule;
  deriveModulePathParameters(outputFile, emitModuleOption, moduleExtension,
                             mainOutputIfUsableForModule);

  auto moduleOutputPath = determineSupplementaryOutputFilename(
      emitModuleOption, pathsFromArguments.ModuleOutputPath,
      file_types::TY_CodiraModuleFile, mainOutputIfUsableForModule,
      defaultSupplementaryOutputPathExcludingExtension);

  auto YAMLOptRecordPath = determineSupplementaryOutputFilename(
      OPT_save_optimization_record, pathsFromArguments.YAMLOptRecordPath,
      file_types::TY_YAMLOptRecord, "",
      defaultSupplementaryOutputPathExcludingExtension);
  auto bitstreamOptRecordPath = determineSupplementaryOutputFilename(
      OPT_save_optimization_record, pathsFromArguments.BitstreamOptRecordPath,
      file_types::TY_BitstreamOptRecord, "",
      defaultSupplementaryOutputPathExcludingExtension);

  SupplementaryOutputPaths sop;
  sop.ClangHeaderOutputPath = clangHeaderOutputPath;
  sop.ModuleOutputPath = moduleOutputPath;
  sop.ModuleDocOutputPath = moduleDocOutputPath;
  sop.DependenciesFilePath = dependenciesFilePath;
  sop.ReferenceDependenciesFilePath = referenceDependenciesFilePath;
  sop.SerializedDiagnosticsPath = serializedDiagnosticsPath;
  sop.FixItsOutputPath = fixItsOutputPath;
  sop.LoadedModuleTracePath = loadedModuleTracePath;
  sop.FineModuleTracePath = FineModuleTracePath.str().str();
  sop.TBDPath = tbdPath;
  sop.ModuleInterfaceOutputPath = ModuleInterfaceOutputPath;
  sop.PrivateModuleInterfaceOutputPath = PrivateModuleInterfaceOutputPath;
  sop.PackageModuleInterfaceOutputPath = PackageModuleInterfaceOutputPath;
  sop.ModuleSourceInfoOutputPath = moduleSourceInfoOutputPath;
  sop.ModuleSummaryOutputPath = moduleSummaryOutputPath;
  sop.ABIDescriptorOutputPath = ABIDescriptorOutputPath;
  sop.APIDescriptorOutputPath = APIDescriptorOutputPath;
  sop.ConstValuesOutputPath = constValuesOutputPath;
  sop.ModuleSemanticInfoOutputPath = ModuleSemanticInfoOutputPath;
  sop.YAMLOptRecordPath = YAMLOptRecordPath;
  sop.BitstreamOptRecordPath = bitstreamOptRecordPath;
  return sop;
}

StringRef SupplementaryOutputPathsComputer::
    deriveDefaultSupplementaryOutputPathExcludingExtension(
        StringRef outputFilename, const InputFile &input) const {
  // Put the supplementary output file next to the output file if possible.
  if (!outputFilename.empty() && outputFilename != "-")
    return outputFilename;

  if (input.isPrimary() && input.getFileName() != "-")
    return toolchain::sys::path::filename(input.getFileName());

  return ModuleName;
}

std::string
SupplementaryOutputPathsComputer::determineSupplementaryOutputFilename(
    options::ID emitOpt, std::string pathFromArguments, file_types::ID type,
    StringRef mainOutputIfUsable,
    StringRef defaultSupplementaryOutputPathExcludingExtension,
    bool forceDefaultSupplementaryOutputPathExcludingExtension) const {

  auto hasEmitOptArg = [&] () -> bool {
    if (Args.hasArg(emitOpt))
      return true;
    if (emitOpt == options::OPT_save_optimization_record &&
        Args.hasArg(options::OPT_save_optimization_record_EQ))
      return true;
    return false;
  };

  auto computeDefaultSupplementaryOutputPathExcludingExtension =
    [&] () -> std::string {
      toolchain::SmallString<128> path(
        defaultSupplementaryOutputPathExcludingExtension);

      toolchain::sys::path::replace_extension(path, file_types::getExtension(type));
      return path.str().str();
    };

  if (forceDefaultSupplementaryOutputPathExcludingExtension) {
    if (!hasEmitOptArg()) {
      return std::string();
    }
    return computeDefaultSupplementaryOutputPathExcludingExtension();
  }

  if (!pathFromArguments.empty())
    return pathFromArguments;

  if (!hasEmitOptArg())
    return std::string();

  if (!mainOutputIfUsable.empty()) {
    return mainOutputIfUsable.str();
  }

  return computeDefaultSupplementaryOutputPathExcludingExtension();
}

void SupplementaryOutputPathsComputer::deriveModulePathParameters(
    StringRef mainOutputFile, options::ID &emitOption, std::string &extension,
    std::string &mainOutputIfUsable) const {

  bool isSIB = RequestedAction == FrontendOptions::ActionType::EmitSIB ||
               RequestedAction == FrontendOptions::ActionType::EmitSIBGen;

  emitOption = !isSIB ? options::OPT_emit_module
                      : RequestedAction == FrontendOptions::ActionType::EmitSIB
                            ? options::OPT_emit_sib
                            : options::OPT_emit_sibgen;

  bool canUseMainOutputForModule =
      RequestedAction == FrontendOptions::ActionType::MergeModules ||
      RequestedAction == FrontendOptions::ActionType::EmitModuleOnly || isSIB;

  extension = file_types::getExtension(isSIB ? file_types::TY_SIB
                                             : file_types::TY_CodiraModuleFile)
                  .str();

  mainOutputIfUsable = canUseMainOutputForModule && !OutputFiles.empty()
                           ? mainOutputFile.str()
                           : "";
}

static SupplementaryOutputPaths
createFromTypeToPathMap(const TypeToPathMap *map) {
  SupplementaryOutputPaths paths;
  if (!map)
    return paths;
  const std::pair<file_types::ID, std::string &> typesAndStrings[] = {
#define OUTPUT(NAME, TYPE) {file_types::TYPE, paths.NAME},
#include "language/Basic/SupplementaryOutputPaths.def"
#undef OUTPUT
  };
  for (const std::pair<file_types::ID, std::string &> &typeAndString :
       typesAndStrings) {
    auto const out = map->find(typeAndString.first);
    typeAndString.second = out == map->end() ? "" : out->second;
  }
  return paths;
}

std::optional<std::vector<SupplementaryOutputPaths>>
SupplementaryOutputPathsComputer::readSupplementaryOutputFileMap() const {
  if (Arg *A = Args.getLastArg(options::OPT_emit_objc_header_path,
                               options::OPT_emit_module_path,
                               options::OPT_emit_module_doc_path,
                               options::OPT_emit_dependencies_path,
                               options::OPT_emit_reference_dependencies_path,
                               options::OPT_serialize_diagnostics_path,
                               options::OPT_emit_loaded_module_trace_path,
                               options::OPT_emit_module_interface_path,
                               options::OPT_emit_private_module_interface_path,
                               options::OPT_emit_package_module_interface_path,
                               options::OPT_emit_module_source_info_path,
                               options::OPT_emit_tbd_path)) {
    Diags.diagnose(SourceLoc(),
                   diag::error_cannot_have_supplementary_outputs,
                   A->getSpelling(), "-supplementary-output-file-map");
    return std::nullopt;
  }
  const StringRef supplementaryFileMapPath =
      Args.getLastArgValue(options::OPT_supplementary_output_file_map);

  unsigned BadFileDescriptorRetryCount = 0;
  if (const Arg *A = Args.getLastArg(options::OPT_bad_file_descriptor_retry_count)) {
    if (StringRef(A->getValue()).getAsInteger(10, BadFileDescriptorRetryCount)) {
      Diags.diagnose(SourceLoc(), diag::error_invalid_arg_value,
                     A->getAsString(Args), A->getValue());
      return std::nullopt;
    }
  }

  toolchain::ErrorOr<std::unique_ptr<toolchain::MemoryBuffer>> buffer = nullptr;
  for (unsigned I = 0; I < BadFileDescriptorRetryCount + 1; ++I) {
    buffer = toolchain::MemoryBuffer::getFile(supplementaryFileMapPath);
    if (buffer)
      break;
    if (buffer.getError().value() != EBADF)
      break;
  }
  if (!buffer) {
    Diags.diagnose(SourceLoc(), diag::cannot_open_file,
                   supplementaryFileMapPath, buffer.getError().message());
    return std::nullopt;
  }
  toolchain::Expected<OutputFileMap> OFM =
      OutputFileMap::loadFromBuffer(std::move(buffer.get()), "");
  if (auto Err = OFM.takeError()) {
    Diags.diagnose(SourceLoc(),
                   diag::error_unable_to_load_supplementary_output_file_map,
                   supplementaryFileMapPath, toolchain::toString(std::move(Err)));
    return std::nullopt;
  }

  std::vector<SupplementaryOutputPaths> outputPaths;
  bool hadError = false;
  InputsAndOutputs.forEachInputProducingSupplementaryOutput(
      [&](const InputFile &input) -> bool {
        const TypeToPathMap *mapForInput =
            OFM->getOutputMapForInput(input.getFileName());
        if (!mapForInput) {
          Diags.diagnose(
              SourceLoc(),
              diag::error_missing_entry_in_supplementary_output_file_map,
              supplementaryFileMapPath, input.getFileName());
          hadError = true;
        }
        outputPaths.push_back(createFromTypeToPathMap(mapForInput));
        return false;
      });
  if (hadError)
    return std::nullopt;

  return outputPaths;
}
