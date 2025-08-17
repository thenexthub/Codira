//===--- CreateInvocationFromCommandLine.cpp - CompilerInvocation from Args ==//
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
// Construct a compiler invocation object for command line driver arguments
//
//===----------------------------------------------------------------------===//

#include "language/Core/Basic/DiagnosticOptions.h"
#include "language/Core/Driver/Action.h"
#include "language/Core/Driver/Compilation.h"
#include "language/Core/Driver/Driver.h"
#include "language/Core/Driver/Options.h"
#include "language/Core/Driver/Tool.h"
#include "language/Core/Frontend/CompilerInstance.h"
#include "language/Core/Frontend/FrontendDiagnostic.h"
#include "language/Core/Frontend/Utils.h"
#include "toolchain/ADT/STLExtras.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Option/ArgList.h"
#include "toolchain/Support/VirtualFileSystem.h"
#include "toolchain/TargetParser/Host.h"
using namespace language::Core;
using namespace toolchain::opt;

std::unique_ptr<CompilerInvocation>
language::Core::createInvocation(ArrayRef<const char *> ArgList,
                        CreateInvocationOptions Opts) {
  assert(!ArgList.empty());
  std::optional<DiagnosticOptions> LocalDiagOpts;
  IntrusiveRefCntPtr<DiagnosticsEngine> Diags;
  if (Opts.Diags) {
    Diags = std::move(Opts.Diags);
  } else {
    LocalDiagOpts.emplace();
    Diags = CompilerInstance::createDiagnostics(
        Opts.VFS ? *Opts.VFS : *toolchain::vfs::getRealFileSystem(), *LocalDiagOpts);
  }

  SmallVector<const char *, 16> Args(ArgList);

  // FIXME: Find a cleaner way to force the driver into restricted modes.
  Args.insert(
      toolchain::find_if(
          Args, [](const char *Elem) { return toolchain::StringRef(Elem) == "--"; }),
      "-fsyntax-only");

  // FIXME: We shouldn't have to pass in the path info.
  driver::Driver TheDriver(Args[0], toolchain::sys::getDefaultTargetTriple(), *Diags,
                           "clang LLVM compiler", Opts.VFS);

  // Don't check that inputs exist, they may have been remapped.
  TheDriver.setCheckInputsExist(false);
  TheDriver.setProbePrecompiled(Opts.ProbePrecompiled);

  std::unique_ptr<driver::Compilation> C(TheDriver.BuildCompilation(Args));
  if (!C)
    return nullptr;

  if (C->getArgs().hasArg(driver::options::OPT_fdriver_only))
    return nullptr;

  // Just print the cc1 options if -### was present.
  if (C->getArgs().hasArg(driver::options::OPT__HASH_HASH_HASH)) {
    C->getJobs().Print(toolchain::errs(), "\n", true);
    return nullptr;
  }

  // We expect to get back exactly one command job, if we didn't something
  // failed. Offload compilation is an exception as it creates multiple jobs. If
  // that's the case, we proceed with the first job. If caller needs a
  // particular job, it should be controlled via options (e.g.
  // --cuda-{host|device}-only for CUDA) passed to the driver.
  const driver::JobList &Jobs = C->getJobs();
  bool OffloadCompilation = false;
  if (Jobs.size() > 1) {
    for (auto &A : C->getActions()){
      // On MacOSX real actions may end up being wrapped in BindArchAction
      if (isa<driver::BindArchAction>(A))
        A = *A->input_begin();
      if (isa<driver::OffloadAction>(A)) {
        OffloadCompilation = true;
        break;
      }
    }
  }

  bool PickFirstOfMany = OffloadCompilation || Opts.RecoverOnError;
  if (Jobs.size() == 0 || (Jobs.size() > 1 && !PickFirstOfMany)) {
    SmallString<256> Msg;
    toolchain::raw_svector_ostream OS(Msg);
    Jobs.Print(OS, "; ", true);
    Diags->Report(diag::err_fe_expected_compiler_job) << OS.str();
    return nullptr;
  }
  auto Cmd = toolchain::find_if(Jobs, [](const driver::Command &Cmd) {
    return StringRef(Cmd.getCreator().getName()) == "clang";
  });
  if (Cmd == Jobs.end()) {
    Diags->Report(diag::err_fe_expected_clang_command);
    return nullptr;
  }

  const ArgStringList &CCArgs = Cmd->getArguments();
  if (Opts.CC1Args)
    *Opts.CC1Args = {CCArgs.begin(), CCArgs.end()};
  auto CI = std::make_unique<CompilerInvocation>();
  if (!CompilerInvocation::CreateFromArgs(*CI, CCArgs, *Diags, Args[0]) &&
      !Opts.RecoverOnError)
    return nullptr;
  return CI;
}
