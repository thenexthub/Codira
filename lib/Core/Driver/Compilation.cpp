//===- Compilation.cpp - Compilation Task Implementation ------------------===//
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

#include "language/Core/Driver/Compilation.h"
#include "language/Core/Basic/LLVM.h"
#include "language/Core/Driver/Action.h"
#include "language/Core/Driver/Driver.h"
#include "language/Core/Driver/Job.h"
#include "language/Core/Driver/Options.h"
#include "language/Core/Driver/ToolChain.h"
#include "language/Core/Driver/Util.h"
#include "toolchain/Option/ArgList.h"
#include "toolchain/Option/OptSpecifier.h"
#include "toolchain/Option/Option.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/raw_ostream.h"
#include "toolchain/TargetParser/Triple.h"
#include <cassert>
#include <string>
#include <system_error>
#include <utility>

using namespace language::Core;
using namespace driver;
using namespace toolchain::opt;

Compilation::Compilation(const Driver &D, const ToolChain &_DefaultToolChain,
                         InputArgList *_Args, DerivedArgList *_TranslatedArgs,
                         bool ContainsError)
    : TheDriver(D), DefaultToolChain(_DefaultToolChain), Args(_Args),
      TranslatedArgs(_TranslatedArgs), ContainsError(ContainsError) {
  // The offloading host toolchain is the default toolchain.
  OrderedOffloadingToolchains.insert(
      std::make_pair(Action::OFK_Host, &DefaultToolChain));
}

Compilation::~Compilation() {
  // Remove temporary files. This must be done before arguments are freed, as
  // the file names might be derived from the input arguments.
  if (!TheDriver.isSaveTempsEnabled() && !ForceKeepTempFiles)
    CleanupFileList(TempFiles);

  delete TranslatedArgs;
  delete Args;

  // Free any derived arg lists.
  for (auto Arg : TCArgs)
    if (Arg.second != TranslatedArgs)
      delete Arg.second;
}

const DerivedArgList &
Compilation::getArgsForToolChain(const ToolChain *TC, StringRef BoundArch,
                                 Action::OffloadKind DeviceOffloadKind) {
  if (!TC)
    TC = &DefaultToolChain;

  DerivedArgList *&Entry = TCArgs[{TC, BoundArch, DeviceOffloadKind}];
  if (!Entry) {
    SmallVector<Arg *, 4> AllocatedArgs;
    DerivedArgList *OpenMPArgs = nullptr;
    // Translate OpenMP toolchain arguments provided via the -Xopenmp-target flags.
    if (DeviceOffloadKind == Action::OFK_OpenMP) {
      const ToolChain *HostTC = getSingleOffloadToolChain<Action::OFK_Host>();
      bool SameTripleAsHost = (TC->getTriple() == HostTC->getTriple());
      OpenMPArgs = TC->TranslateOpenMPTargetArgs(
          *TranslatedArgs, SameTripleAsHost, AllocatedArgs);
    }

    DerivedArgList *NewDAL = nullptr;
    if (!OpenMPArgs) {
      NewDAL = TC->TranslateXarchArgs(*TranslatedArgs, BoundArch,
                                      DeviceOffloadKind, &AllocatedArgs);
    } else {
      NewDAL = TC->TranslateXarchArgs(*OpenMPArgs, BoundArch, DeviceOffloadKind,
                                      &AllocatedArgs);
      if (!NewDAL)
        NewDAL = OpenMPArgs;
      else
        delete OpenMPArgs;
    }

    if (!NewDAL) {
      Entry = TC->TranslateArgs(*TranslatedArgs, BoundArch, DeviceOffloadKind);
      if (!Entry)
        Entry = TranslatedArgs;
    } else {
      Entry = TC->TranslateArgs(*NewDAL, BoundArch, DeviceOffloadKind);
      if (!Entry)
        Entry = NewDAL;
      else
        delete NewDAL;
    }

    // Add allocated arguments to the final DAL.
    for (auto *ArgPtr : AllocatedArgs)
      Entry->AddSynthesizedArg(ArgPtr);
  }

  return *Entry;
}

bool Compilation::CleanupFile(const char *File, bool IssueErrors) const {
  // FIXME: Why are we trying to remove files that we have not created? For
  // example we should only try to remove a temporary assembly file if
  // "clang -cc1" succeed in writing it. Was this a workaround for when
  // clang was writing directly to a .s file and sometimes leaving it behind
  // during a failure?

  // FIXME: If this is necessary, we can still try to split
  // toolchain::sys::fs::remove into a removeFile and a removeDir and avoid the
  // duplicated stat from is_regular_file.

  // Don't try to remove files which we don't have write access to (but may be
  // able to remove), or non-regular files. Underlying tools may have
  // intentionally not overwritten them.
  if (!toolchain::sys::fs::can_write(File) || !toolchain::sys::fs::is_regular_file(File))
    return true;

  if (std::error_code EC = toolchain::sys::fs::remove(File)) {
    // Failure is only failure if the file exists and is "regular". We checked
    // for it being regular before, and toolchain::sys::fs::remove ignores ENOENT,
    // so we don't need to check again.

    if (IssueErrors)
      getDriver().Diag(diag::err_drv_unable_to_remove_file)
        << EC.message();
    return false;
  }
  return true;
}

bool Compilation::CleanupFileList(const toolchain::opt::ArgStringList &Files,
                                  bool IssueErrors) const {
  bool Success = true;
  for (const auto &File: Files)
    Success &= CleanupFile(File, IssueErrors);
  return Success;
}

bool Compilation::CleanupFileMap(const ArgStringMap &Files,
                                 const JobAction *JA,
                                 bool IssueErrors) const {
  bool Success = true;
  for (const auto &File : Files) {
    // If specified, only delete the files associated with the JobAction.
    // Otherwise, delete all files in the map.
    if (JA && File.first != JA)
      continue;
    Success &= CleanupFile(File.second, IssueErrors);
  }
  return Success;
}

int Compilation::ExecuteCommand(const Command &C,
                                const Command *&FailingCommand,
                                bool LogOnly) const {
  if ((getDriver().CCPrintOptions ||
       getArgs().hasArg(options::OPT_v)) && !getDriver().CCGenDiagnostics) {
    raw_ostream *OS = &toolchain::errs();
    std::unique_ptr<toolchain::raw_fd_ostream> OwnedStream;

    // Follow gcc implementation of CC_PRINT_OPTIONS; we could also cache the
    // output stream.
    if (getDriver().CCPrintOptions &&
        !getDriver().CCPrintOptionsFilename.empty()) {
      std::error_code EC;
      OwnedStream.reset(new toolchain::raw_fd_ostream(
          getDriver().CCPrintOptionsFilename, EC,
          toolchain::sys::fs::OF_Append | toolchain::sys::fs::OF_TextWithCRLF));
      if (EC) {
        getDriver().Diag(diag::err_drv_cc_print_options_failure)
            << EC.message();
        FailingCommand = &C;
        return 1;
      }
      OS = OwnedStream.get();
    }

    if (getDriver().CCPrintOptions)
      *OS << "[Logging clang options]\n";

    C.Print(*OS, "\n", /*Quote=*/getDriver().CCPrintOptions);
  }

  if (LogOnly)
    return 0;

  std::string Error;
  bool ExecutionFailed;
  int Res = C.Execute(Redirects, &Error, &ExecutionFailed);
  if (PostCallback)
    PostCallback(C, Res);
  if (!Error.empty()) {
    assert(Res && "Error string set with 0 result code!");
    getDriver().Diag(diag::err_drv_command_failure) << Error;
  }

  if (Res)
    FailingCommand = &C;

  return ExecutionFailed ? 1 : Res;
}

using FailingCommandList = SmallVectorImpl<std::pair<int, const Command *>>;

static bool ActionFailed(const Action *A,
                         const FailingCommandList &FailingCommands) {
  if (FailingCommands.empty())
    return false;

  // CUDA/HIP/SYCL can have the same input source code compiled multiple times
  // so do not compile again if there are already failures. It is OK to abort
  // the CUDA/HIP/SYCL pipeline on errors.
  if (A->isOffloading(Action::OFK_Cuda) || A->isOffloading(Action::OFK_HIP) ||
      A->isOffloading(Action::OFK_SYCL))
    return true;

  for (const auto &CI : FailingCommands)
    if (A == &(CI.second->getSource()))
      return true;

  for (const auto *AI : A->inputs())
    if (ActionFailed(AI, FailingCommands))
      return true;

  return false;
}

void Compilation::ExecuteJobs(const JobList &Jobs,
                              FailingCommandList &FailingCommands,
                              bool LogOnly) const {
  // According to UNIX standard, driver need to continue compiling all the
  // inputs on the command line even one of them failed.
  // In all but CLMode, execute all the jobs unless the necessary inputs for the
  // job is missing due to previous failures.
  for (const auto &Job : Jobs) {
    if (ActionFailed(&Job.getSource(), FailingCommands))
      continue;
    const Command *FailingCommand = nullptr;
    if (int Res = ExecuteCommand(Job, FailingCommand, LogOnly)) {
      FailingCommands.push_back(std::make_pair(Res, FailingCommand));
      // Bail as soon as one command fails in cl driver mode.
      if (TheDriver.IsCLMode())
        return;
    }
  }
}

void Compilation::initCompilationForDiagnostics() {
  ForDiagnostics = true;

  // Free actions and jobs.
  Actions.clear();
  AllActions.clear();
  Jobs.clear();

  // Remove temporary files.
  if (!TheDriver.isSaveTempsEnabled() && !ForceKeepTempFiles)
    CleanupFileList(TempFiles);

  // Clear temporary/results file lists.
  TempFiles.clear();
  ResultFiles.clear();
  FailureResultFiles.clear();

  // Remove any user specified output.  Claim any unclaimed arguments, so as
  // to avoid emitting warnings about unused args.
  OptSpecifier OutputOpts[] = {
      options::OPT_o,  options::OPT_MD, options::OPT_MMD, options::OPT_M,
      options::OPT_MM, options::OPT_MF, options::OPT_MG,  options::OPT_MJ,
      options::OPT_MQ, options::OPT_MT, options::OPT_MV};
  for (const auto &Opt : OutputOpts) {
    if (TranslatedArgs->hasArg(Opt))
      TranslatedArgs->eraseArg(Opt);
  }
  TranslatedArgs->ClaimAllArgs();

  // Force re-creation of the toolchain Args, otherwise our modifications just
  // above will have no effect.
  for (auto Arg : TCArgs)
    if (Arg.second != TranslatedArgs)
      delete Arg.second;
  TCArgs.clear();

  // Redirect stdout/stderr to /dev/null.
  Redirects = {std::nullopt, {""}, {""}};

  // Temporary files added by diagnostics should be kept.
  ForceKeepTempFiles = true;
}

StringRef Compilation::getSysRoot() const {
  return getDriver().SysRoot;
}

void Compilation::Redirect(ArrayRef<std::optional<StringRef>> Redirects) {
  this->Redirects = Redirects;
}
