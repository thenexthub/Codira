//===--- CompilerInstance.cpp ---------------------------------------------===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//
//
// Coding style: https://mlir.toolchain.org/getting_started/DeveloperGuide/
//
//===----------------------------------------------------------------------===//

#include "language/Compability/Frontend/CompilerInstance.h"
#include "language/Compability/Frontend/CompilerInvocation.h"
#include "language/Compability/Frontend/TextDiagnosticPrinter.h"
#include "language/Compability/Parser/parsing.h"
#include "language/Compability/Parser/provenance.h"
#include "language/Compability/Semantics/semantics.h"
#include "language/Compability/Support/Fortran-features.h"
#include "language/Compability/Support/Timing.h"
#include "mlir/Support/RawOstreamExtras.h"
#include "language/Core/Basic/DiagnosticFrontend.h"
#include "toolchain/ADT/StringExtras.h"
#include "toolchain/MC/TargetRegistry.h"
#include "toolchain/Pass.h"
#include "toolchain/Support/Errc.h"
#include "toolchain/Support/Error.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/raw_ostream.h"
#include "toolchain/TargetParser/TargetParser.h"
#include "toolchain/TargetParser/Triple.h"

using namespace language::Compability::frontend;

CompilerInstance::CompilerInstance()
    : invocation(new CompilerInvocation()),
      allSources(new language::Compability::parser::AllSources()),
      allCookedSources(new language::Compability::parser::AllCookedSources(*allSources)),
      parsing(new language::Compability::parser::Parsing(*allCookedSources)) {
  // TODO: This is a good default during development, but ultimately we should
  // give the user the opportunity to specify this.
  allSources->set_encoding(language::Compability::parser::Encoding::UTF_8);
}

CompilerInstance::~CompilerInstance() {
  assert(outputFiles.empty() && "Still output files in flight?");
}

void CompilerInstance::setInvocation(
    std::shared_ptr<CompilerInvocation> value) {
  invocation = std::move(value);
}

void CompilerInstance::setSemaOutputStream(raw_ostream &value) {
  ownedSemaOutputStream.release();
  semaOutputStream = &value;
}

void CompilerInstance::setSemaOutputStream(std::unique_ptr<raw_ostream> value) {
  ownedSemaOutputStream.swap(value);
  semaOutputStream = ownedSemaOutputStream.get();
}

// Helper method to generate the path of the output file. The following logic
// applies:
// 1. If the user specifies the output file via `-o`, then use that (i.e.
//    the outputFilename parameter).
// 2. If the user does not specify the name of the output file, derive it from
//    the input file (i.e. inputFilename + extension)
// 3. If the output file is not specified and the input file is `-`, then set
//    the output file to `-` as well.
static std::string getOutputFilePath(toolchain::StringRef outputFilename,
                                     toolchain::StringRef inputFilename,
                                     toolchain::StringRef extension) {

  // Output filename _is_ specified. Just use that.
  if (!outputFilename.empty())
    return std::string(outputFilename);

  // Output filename _is not_ specified. Derive it from the input file name.
  std::string outFile = "-";
  if (!extension.empty() && (inputFilename != "-")) {
    toolchain::SmallString<128> path(inputFilename);
    toolchain::sys::path::replace_extension(path, extension);
    outFile = std::string(path);
  }

  return outFile;
}

std::unique_ptr<toolchain::raw_pwrite_stream>
CompilerInstance::createDefaultOutputFile(bool binary, toolchain::StringRef baseName,
                                          toolchain::StringRef extension) {

  // Get the path of the output file
  std::string outputFilePath =
      getOutputFilePath(getFrontendOpts().outputFile, baseName, extension);

  // Create the output file
  toolchain::Expected<std::unique_ptr<toolchain::raw_pwrite_stream>> os =
      createOutputFileImpl(outputFilePath, binary);

  // If successful, add the file to the list of tracked output files and
  // return.
  if (os) {
    outputFiles.emplace_back(OutputFile(outputFilePath));
    return std::move(*os);
  }

  // If unsuccessful, issue an error and return Null
  unsigned diagID = getDiagnostics().getCustomDiagID(
      language::Core::DiagnosticsEngine::Error, "unable to open output file '%0': '%1'");
  getDiagnostics().Report(diagID)
      << outputFilePath << toolchain::errorToErrorCode(os.takeError()).message();
  return nullptr;
}

toolchain::Expected<std::unique_ptr<toolchain::raw_pwrite_stream>>
CompilerInstance::createOutputFileImpl(toolchain::StringRef outputFilePath,
                                       bool binary) {

  // Creates the file descriptor for the output file
  std::unique_ptr<toolchain::raw_fd_ostream> os;

  std::error_code error;
  os.reset(new toolchain::raw_fd_ostream(
      outputFilePath, error,
      (binary ? toolchain::sys::fs::OF_None : toolchain::sys::fs::OF_TextWithCRLF)));
  if (error) {
    return toolchain::errorCodeToError(error);
  }

  // For seekable streams, just return the stream corresponding to the output
  // file.
  if (!binary || os->supportsSeeking())
    return std::move(os);

  // For non-seekable streams, we need to wrap the output stream into something
  // that supports 'pwrite' and takes care of the ownership for us.
  return std::make_unique<toolchain::buffer_unique_ostream>(std::move(os));
}

void CompilerInstance::clearOutputFiles(bool eraseFiles) {
  for (OutputFile &of : outputFiles)
    if (!of.filename.empty() && eraseFiles)
      toolchain::sys::fs::remove(of.filename);

  outputFiles.clear();
}

bool CompilerInstance::executeAction(FrontendAction &act) {
  CompilerInvocation &invoc = this->getInvocation();

  toolchain::Triple targetTriple{toolchain::Triple(invoc.getTargetOpts().triple)};

  // Set some sane defaults for the frontend.
  invoc.setDefaultFortranOpts();
  // Update the fortran options based on user-based input.
  invoc.setFortranOpts();
  // Set the encoding to read all input files in based on user input.
  allSources->set_encoding(invoc.getFortranOpts().encoding);
  if (!setUpTargetMachine())
    return false;
  // Set options controlling lowering to FIR.
  invoc.setLoweringOptions();

  if (invoc.getEnableTimers()) {
    toolchain::TimePassesIsEnabled = true;

    timingStreamMLIR = std::make_unique<language::Compability::support::string_ostream>();
    timingStreamLLVM = std::make_unique<language::Compability::support::string_ostream>();
    timingStreamCodeGen = std::make_unique<language::Compability::support::string_ostream>();

    timingMgr.setEnabled(true);
    timingMgr.setDisplayMode(mlir::DefaultTimingManager::DisplayMode::Tree);
    timingMgr.setOutput(
        language::Compability::support::createTimingFormatterText(*timingStreamMLIR));

    // Creating a new TimingScope will automatically start the timer. Since this
    // is the top-level timer, this is ok because it will end up capturing the
    // time for all the bookkeeping and other tasks that take place between
    // parsing, lowering etc. for which finer-grained timers will be created.
    timingScopeRoot = timingMgr.getRootScope();
  }

  // Run the frontend action `act` for every input file.
  for (const FrontendInputFile &fif : getFrontendOpts().inputs) {
    if (act.beginSourceFile(*this, fif)) {
      if (toolchain::Error err = act.execute()) {
        consumeError(std::move(err));
      }
      act.endSourceFile();
    }
  }

  if (timingMgr.isEnabled()) {
    timingScopeRoot.stop();

    // Write the timings to the associated output stream and clear all timers.
    // We need to provide another stream because the TimingManager will attempt
    // to print in its destructor even if it has been cleared. By the time that
    // destructor runs, the output streams will have been destroyed, so give it
    // a null stream.
    timingMgr.print();
    timingMgr.setOutput(
        language::Compability::support::createTimingFormatterText(mlir::thread_safe_nulls()));

    // This prints the timings in "reverse" order, starting from code
    // generation, followed by LLVM-IR optimizations, then MLIR optimizations
    // and transformations and the frontend. If any of the steps are disabled,
    // for instance because code generation was not performed, the strings
    // will be empty.
    if (!timingStreamCodeGen->str().empty())
      toolchain::errs() << timingStreamCodeGen->str() << "\n";

    if (!timingStreamLLVM->str().empty())
      toolchain::errs() << timingStreamLLVM->str() << "\n";

    if (!timingStreamMLIR->str().empty())
      toolchain::errs() << timingStreamMLIR->str() << "\n";
  }

  return !getDiagnostics().getClient()->getNumErrors();
}

void CompilerInstance::createDiagnostics(language::Core::DiagnosticConsumer *client,
                                         bool shouldOwnClient) {
  diagnostics = createDiagnostics(getDiagnosticOpts(), client, shouldOwnClient);
}

language::Core::IntrusiveRefCntPtr<language::Core::DiagnosticsEngine>
CompilerInstance::createDiagnostics(language::Core::DiagnosticOptions &opts,
                                    language::Core::DiagnosticConsumer *client,
                                    bool shouldOwnClient) {
  auto diags = toolchain::makeIntrusiveRefCnt<language::Core::DiagnosticsEngine>(
      language::Core::DiagnosticIDs::create(), opts);

  // Create the diagnostic client for reporting errors or for
  // implementing -verify.
  if (client) {
    diags->setClient(client, shouldOwnClient);
  } else {
    diags->setClient(new TextDiagnosticPrinter(toolchain::errs(), opts));
  }
  return diags;
}

// Get feature string which represents combined explicit target features
// for AMD GPU and the target features specified by the user
static std::string
getExplicitAndImplicitAMDGPUTargetFeatures(language::Core::DiagnosticsEngine &diags,
                                           const TargetOptions &targetOpts,
                                           const toolchain::Triple triple) {
  toolchain::StringRef cpu = targetOpts.cpu;
  toolchain::StringMap<bool> implicitFeaturesMap;
  // Get the set of implicit target features
  toolchain::AMDGPU::fillAMDGPUFeatureMap(cpu, triple, implicitFeaturesMap);

  // Add target features specified by the user
  for (auto &userFeature : targetOpts.featuresAsWritten) {
    std::string userKeyString = userFeature.substr(1);
    implicitFeaturesMap[userKeyString] = (userFeature[0] == '+');
  }

  auto HasError =
      toolchain::AMDGPU::insertWaveSizeFeature(cpu, triple, implicitFeaturesMap);
  if (HasError.first) {
    unsigned diagID = diags.getCustomDiagID(language::Core::DiagnosticsEngine::Error,
                                            "Unsupported feature ID: %0");
    diags.Report(diagID) << HasError.second;
    return std::string();
  }

  toolchain::SmallVector<std::string> featuresVec;
  for (auto &implicitFeatureItem : implicitFeaturesMap) {
    featuresVec.push_back((toolchain::Twine(implicitFeatureItem.second ? "+" : "-") +
                           implicitFeatureItem.first().str())
                              .str());
  }
  toolchain::sort(featuresVec);
  return toolchain::join(featuresVec, ",");
}

// Get feature string which represents combined explicit target features
// for NVPTX and the target features specified by the user/
// TODO: Have a more robust target conf like `clang/lib/Basic/Targets/NVPTX.cpp`
static std::string
getExplicitAndImplicitNVPTXTargetFeatures(language::Core::DiagnosticsEngine &diags,
                                          const TargetOptions &targetOpts,
                                          const toolchain::Triple triple) {
  toolchain::StringRef cpu = targetOpts.cpu;
  toolchain::StringMap<bool> implicitFeaturesMap;
  std::string errorMsg;
  bool ptxVer = false;

  // Add target features specified by the user
  for (auto &userFeature : targetOpts.featuresAsWritten) {
    toolchain::StringRef userKeyString(toolchain::StringRef(userFeature).drop_front(1));
    implicitFeaturesMap[userKeyString.str()] = (userFeature[0] == '+');
    // Check if the user provided a PTX version
    if (userKeyString.starts_with("ptx"))
      ptxVer = true;
  }

  // Set the default PTX version to `ptx61` if none was provided.
  // TODO: set the default PTX version based on the chip.
  if (!ptxVer)
    implicitFeaturesMap["ptx61"] = true;

  // Set the compute capability.
  implicitFeaturesMap[cpu.str()] = true;

  toolchain::SmallVector<std::string> featuresVec;
  for (auto &implicitFeatureItem : implicitFeaturesMap) {
    featuresVec.push_back((toolchain::Twine(implicitFeatureItem.second ? "+" : "-") +
                           implicitFeatureItem.first().str())
                              .str());
  }
  toolchain::sort(featuresVec);
  return toolchain::join(featuresVec, ",");
}

std::string CompilerInstance::getTargetFeatures() {
  const TargetOptions &targetOpts = getInvocation().getTargetOpts();
  const toolchain::Triple triple(targetOpts.triple);

  // Clang does not append all target features to the clang -cc1 invocation.
  // Some target features are parsed implicitly by language::Core::TargetInfo child
  // class. Clang::TargetInfo classes are the basic clang classes and
  // they cannot be reused by Flang.
  // That's why we need to extract implicit target features and add
  // them to the target features specified by the user
  if (triple.isAMDGPU()) {
    return getExplicitAndImplicitAMDGPUTargetFeatures(getDiagnostics(),
                                                      targetOpts, triple);
  } else if (triple.isNVPTX()) {
    return getExplicitAndImplicitNVPTXTargetFeatures(getDiagnostics(),
                                                     targetOpts, triple);
  }
  return toolchain::join(targetOpts.featuresAsWritten.begin(),
                    targetOpts.featuresAsWritten.end(), ",");
}

bool CompilerInstance::setUpTargetMachine() {
  const TargetOptions &targetOpts = getInvocation().getTargetOpts();
  const std::string &theTriple = targetOpts.triple;

  // Create `Target`
  std::string error;
  const toolchain::Target *theTarget =
      toolchain::TargetRegistry::lookupTarget(theTriple, error);
  if (!theTarget) {
    getDiagnostics().Report(language::Core::diag::err_fe_unable_to_create_target)
        << error;
    return false;
  }
  // Create `TargetMachine`
  const auto &CGOpts = getInvocation().getCodeGenOpts();
  std::optional<toolchain::CodeGenOptLevel> OptLevelOrNone =
      toolchain::CodeGenOpt::getLevel(CGOpts.OptimizationLevel);
  assert(OptLevelOrNone && "Invalid optimization level!");
  toolchain::CodeGenOptLevel OptLevel = *OptLevelOrNone;
  std::string featuresStr = getTargetFeatures();
  std::optional<toolchain::CodeModel::Model> cm = getCodeModel(CGOpts.CodeModel);

  toolchain::TargetOptions tOpts = toolchain::TargetOptions();
  tOpts.EnableAIXExtendedAltivecABI = targetOpts.EnableAIXExtendedAltivecABI;

  targetMachine.reset(theTarget->createTargetMachine(
      toolchain::Triple(theTriple), /*CPU=*/targetOpts.cpu,
      /*Features=*/featuresStr, /*Options=*/tOpts,
      /*Reloc::Model=*/CGOpts.getRelocationModel(),
      /*CodeModel::Model=*/cm, OptLevel));
  assert(targetMachine && "Failed to create TargetMachine");
  if (cm.has_value()) {
    const toolchain::Triple triple(theTriple);
    if ((cm == toolchain::CodeModel::Medium || cm == toolchain::CodeModel::Large) &&
        triple.getArch() == toolchain::Triple::x86_64) {
      targetMachine->setLargeDataThreshold(CGOpts.LargeDataThreshold);
    }
  }
  return true;
}
