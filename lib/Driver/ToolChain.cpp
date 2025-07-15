//===--- ToolChain.cpp - Collections of tools for one platform ------------===//
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
/// \file This file defines the base implementation of the ToolChain class.
/// The platform-specific subclasses are implemented in ToolChains.cpp.
/// For organizational purposes, the platform-independent logic for
/// constructing job invocations is also located in ToolChains.cpp.
//
//===----------------------------------------------------------------------===//

#include "language/Basic/Assertions.h"
#include "language/Driver/ToolChain.h"
#include "language/Driver/Compilation.h"
#include "language/Driver/Driver.h"
#include "language/Driver/Job.h"
#include "toolchain/ADT/STLExtras.h"
#include "toolchain/ADT/SetVector.h"
#include "toolchain/Option/ArgList.h"
#include "toolchain/Remarks/RemarkFormat.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/Program.h"

using namespace language;
using namespace language::driver;
using namespace toolchain::opt;

ToolChain::JobContext::JobContext(Compilation &C, ArrayRef<const Job *> Inputs,
                                  ArrayRef<const Action *> InputActions,
                                  const CommandOutput &Output,
                                  const OutputInfo &OI)
    : C(C), Inputs(Inputs), InputActions(InputActions), Output(Output), OI(OI),
      Args(C.getArgs()) {}

ArrayRef<InputPair> ToolChain::JobContext::getTopLevelInputFiles() const {
  return C.getInputFiles();
}
const char *ToolChain::JobContext::getAllSourcesPath() const {
  return C.getAllSourcesPath();
}

const char *
ToolChain::JobContext::getTemporaryFilePath(const toolchain::Twine &name,
                                            StringRef suffix) const {
  SmallString<128> buffer;
  std::error_code EC = toolchain::sys::fs::createTemporaryFile(name, suffix, buffer);
  if (EC) {
    // Use the constructor that prints both the error code and the description.
    // FIXME: This should not take down the entire process.
    auto error = toolchain::make_error<toolchain::StringError>(
        EC,
        "- unable to create temporary file for " + name + "." + suffix);
    toolchain::report_fatal_error(std::move(error));
  }

  C.addTemporaryFile(buffer.str(), PreserveOnSignal::Yes);
  // We can't just reference the data in the TemporaryFiles vector because
  // that could theoretically get copied to a new address.
  return C.getArgs().MakeArgString(buffer.str());
}

std::optional<Job::ResponseFileInfo>
ToolChain::getResponseFileInfo(const Compilation &C, const char *executablePath,
                               const ToolChain::InvocationInfo &invocationInfo,
                               const ToolChain::JobContext &context) const {
  // Never use a response file if this is a dummy driver for SourceKit, we
  // just want the frontend arguments.
  if (getDriver().isDummyDriverForFrontendInvocation())
    return std::nullopt;

  const bool forceResponseFiles =
      C.getArgs().hasArg(options::OPT_driver_force_response_files);
  assert((invocationInfo.allowsResponseFiles || !forceResponseFiles) &&
         "Cannot force response file if platform does not allow it");

  if (forceResponseFiles || (invocationInfo.allowsResponseFiles &&
                             !toolchain::sys::commandLineFitsWithinSystemLimits(
                                 executablePath, invocationInfo.Arguments))) {
    const char *responseFilePath =
        context.getTemporaryFilePath("arguments", "resp");
    const char *responseFileArg =
        C.getArgs().MakeArgString(Twine("@") + responseFilePath);
    return {{responseFilePath, responseFileArg}};
  }
  return std::nullopt;
}

std::unique_ptr<Job> ToolChain::constructJob(
    const JobAction &JA, Compilation &C, SmallVectorImpl<const Job *> &&inputs,
    ArrayRef<const Action *> inputActions,
    std::unique_ptr<CommandOutput> output, const OutputInfo &OI) const {
  JobContext context{C, inputs, inputActions, *output, OI};

  auto invocationInfo = [&]() -> InvocationInfo {
    switch (JA.getKind()) {
#define CASE(K)                                                                \
  case Action::Kind::K:                                                        \
    return constructInvocation(cast<K##Action>(JA), context);
      CASE(CompileJob)
      CASE(InterpretJob)
      CASE(BackendJob)
      CASE(MergeModuleJob)
      CASE(ModuleWrapJob)
      CASE(DynamicLinkJob)
      CASE(StaticLinkJob)
      CASE(GenerateDSYMJob)
      CASE(VerifyDebugInfoJob)
      CASE(GeneratePCHJob)
      CASE(AutolinkExtractJob)
      CASE(REPLJob)
      CASE(VerifyModuleInterfaceJob)
#undef CASE
    case Action::Kind::Input:
      toolchain_unreachable("not a JobAction");
    }

    // Work around MSVC warning: not all control paths return a value
    toolchain_unreachable("All switch cases are covered");
  }();

  // Special-case the Codira frontend.
  const char *executablePath = nullptr;
  if (StringRef(LANGUAGE_EXECUTABLE_NAME) == invocationInfo.ExecutableName) {
    executablePath = getDriver().getCodiraProgramPath().c_str();
  } else {
    std::string relativePath =
        findProgramRelativeToCodira(invocationInfo.ExecutableName);
    if (!relativePath.empty()) {
      executablePath = C.getArgs().MakeArgString(relativePath);
    } else {
      auto systemPath =
          toolchain::sys::findProgramByName(invocationInfo.ExecutableName);
      if (systemPath) {
        executablePath = C.getArgs().MakeArgString(systemPath.get());
      } else {
        // For debugging purposes.
        executablePath = invocationInfo.ExecutableName;
      }
    }
  }

  // Determine if the argument list is so long that it needs to be written into
  // a response file.
  auto responseFileInfo =
      getResponseFileInfo(C, executablePath, invocationInfo, context);

  return std::make_unique<Job>(
      JA, std::move(inputs), std::move(output), executablePath,
      std::move(invocationInfo.Arguments),
      std::move(invocationInfo.ExtraEnvironment),
      std::move(invocationInfo.FilelistInfos), responseFileInfo);
}

std::string
ToolChain::findProgramRelativeToCodira(StringRef executableName) const {
  auto insertionResult =
      ProgramLookupCache.insert(std::make_pair(executableName, ""));
  if (insertionResult.second) {
    std::string path = findProgramRelativeToCodiraImpl(executableName);
    insertionResult.first->setValue(std::move(path));
  }
  return insertionResult.first->getValue();
}

std::string
ToolChain::findProgramRelativeToCodiraImpl(StringRef executableName) const {
  StringRef languagePath = getDriver().getCodiraProgramPath();
  StringRef languageBinDir = toolchain::sys::path::parent_path(languagePath);

  auto result = toolchain::sys::findProgramByName(executableName, {languageBinDir});
  if (result)
    return result.get();
  return {};
}

file_types::ID ToolChain::lookupTypeForExtension(StringRef Ext) const {
  return file_types::lookupTypeForExtension(Ext);
}

void ToolChain::addLinkedLibArgs(const toolchain::opt::ArgList &Args,
                                 toolchain::opt::ArgStringList &FrontendArgs) {
  Args.getLastArg(options::OPT_l);
  for (auto Arg : Args.getAllArgValues(options::OPT_l)) {
    const std::string lArg("-l" + Arg);
    FrontendArgs.push_back(Args.MakeArgString(Twine(lArg)));
  }
}

toolchain::Expected<file_types::ID>
ToolChain::remarkFileTypeFromArgs(const toolchain::opt::ArgList &Args) const {
  const Arg *A = Args.getLastArg(options::OPT_save_optimization_record_EQ);
  if (!A)
    return file_types::TY_YAMLOptRecord;

  toolchain::Expected<toolchain::remarks::Format> FormatOrErr =
      toolchain::remarks::parseFormat(A->getValue());
  if (toolchain::Error E = FormatOrErr.takeError())
    return std::move(E);

  switch (*FormatOrErr) {
  case toolchain::remarks::Format::YAML:
    return file_types::TY_YAMLOptRecord;
  case toolchain::remarks::Format::Bitstream:
    return file_types::TY_BitstreamOptRecord;
  default:
    return toolchain::createStringError(std::errc::invalid_argument,
                                   "Unknown remark format.");
  }
}
