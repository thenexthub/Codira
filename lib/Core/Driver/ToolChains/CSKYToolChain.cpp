//===--- CSKYToolchain.cpp - CSKY ToolChain Implementations ---*- C++ -*-===//
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

#include "CSKYToolChain.h"
#include "language/Core/Driver/CommonArgs.h"
#include "language/Core/Driver/Compilation.h"
#include "language/Core/Driver/InputInfo.h"
#include "language/Core/Driver/Options.h"
#include "toolchain/Option/ArgList.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/Path.h"

using namespace language::Core::driver;
using namespace language::Core::driver::toolchains;
using namespace language::Core::driver::tools;
using namespace language::Core;
using namespace toolchain::opt;

static void addMultilibsFilePaths(const Driver &D, const MultilibSet &Multilibs,
                                  const Multilib &Multilib,
                                  StringRef InstallPath,
                                  ToolChain::path_list &Paths) {
  if (const auto &PathsCallback = Multilibs.filePathsCallback())
    for (const auto &Path : PathsCallback(Multilib))
      addPathIfExists(D, InstallPath + Path, Paths);
}

/// CSKY Toolchain
CSKYToolChain::CSKYToolChain(const Driver &D, const toolchain::Triple &Triple,
                             const ArgList &Args)
    : Generic_ELF(D, Triple, Args) {
  GCCInstallation.init(Triple, Args);
  if (GCCInstallation.isValid()) {
    Multilibs = GCCInstallation.getMultilibs();
    SelectedMultilibs.assign({GCCInstallation.getMultilib()});
    path_list &Paths = getFilePaths();
    // Add toolchain/multilib specific file paths.
    addMultilibsFilePaths(D, Multilibs, SelectedMultilibs.back(),
                          GCCInstallation.getInstallPath(), Paths);
    getFilePaths().push_back(GCCInstallation.getInstallPath().str() +
                             SelectedMultilibs.back().osSuffix());
    ToolChain::path_list &PPaths = getProgramPaths();
    // Multilib cross-compiler GCC installations put ld in a triple-prefixed
    // directory off of the parent of the GCC installation.
    PPaths.push_back(Twine(GCCInstallation.getParentLibPath() + "/../" +
                           GCCInstallation.getTriple().str() + "/bin")
                         .str());
    PPaths.push_back((GCCInstallation.getParentLibPath() + "/../bin").str());
    getFilePaths().push_back(computeSysRoot() + "/lib" +
                             SelectedMultilibs.back().osSuffix());
  } else {
    getProgramPaths().push_back(D.Dir);
    getFilePaths().push_back(computeSysRoot() + "/lib");
  }
}

Tool *CSKYToolChain::buildLinker() const {
  return new tools::CSKY::Linker(*this);
}

ToolChain::RuntimeLibType CSKYToolChain::GetDefaultRuntimeLibType() const {
  return GCCInstallation.isValid() ? ToolChain::RLT_Libgcc
                                   : ToolChain::RLT_CompilerRT;
}

ToolChain::UnwindLibType
CSKYToolChain::GetUnwindLibType(const toolchain::opt::ArgList &Args) const {
  return ToolChain::UNW_None;
}

void CSKYToolChain::addClangTargetOptions(const toolchain::opt::ArgList &DriverArgs,
                                          toolchain::opt::ArgStringList &CC1Args,
                                          Action::OffloadKind) const {
  CC1Args.push_back("-nostdsysteminc");
}

void CSKYToolChain::AddClangSystemIncludeArgs(const ArgList &DriverArgs,
                                              ArgStringList &CC1Args) const {
  if (DriverArgs.hasArg(options::OPT_nostdinc))
    return;

  if (!DriverArgs.hasArg(options::OPT_nostdlibinc)) {
    SmallString<128> Dir(computeSysRoot());
    toolchain::sys::path::append(Dir, "include");
    addSystemInclude(DriverArgs, CC1Args, Dir.str());
    SmallString<128> Dir2(computeSysRoot());
    toolchain::sys::path::append(Dir2, "sys-include");
    addSystemInclude(DriverArgs, CC1Args, Dir2.str());
  }
}

void CSKYToolChain::addLibStdCxxIncludePaths(
    const toolchain::opt::ArgList &DriverArgs,
    toolchain::opt::ArgStringList &CC1Args) const {
  const GCCVersion &Version = GCCInstallation.getVersion();
  StringRef TripleStr = GCCInstallation.getTriple().str();
  const Multilib &Multilib = GCCInstallation.getMultilib();
  addLibStdCXXIncludePaths(computeSysRoot() + "/include/c++/" + Version.Text,
                           TripleStr, Multilib.includeSuffix(), DriverArgs,
                           CC1Args);
}

std::string CSKYToolChain::computeSysRoot() const {
  if (!getDriver().SysRoot.empty())
    return getDriver().SysRoot;

  SmallString<128> SysRootDir;
  if (GCCInstallation.isValid()) {
    StringRef LibDir = GCCInstallation.getParentLibPath();
    StringRef TripleStr = GCCInstallation.getTriple().str();
    toolchain::sys::path::append(SysRootDir, LibDir, "..", TripleStr);
  } else {
    // Use the triple as provided to the driver. Unlike the parsed triple
    // this has not been normalized to always contain every field.
    toolchain::sys::path::append(SysRootDir, getDriver().Dir, "..",
                            getDriver().getTargetTriple());
  }

  if (!toolchain::sys::fs::exists(SysRootDir))
    return std::string();

  return std::string(SysRootDir);
}

void CSKY::Linker::ConstructJob(Compilation &C, const JobAction &JA,
                                const InputInfo &Output,
                                const InputInfoList &Inputs,
                                const ArgList &Args,
                                const char *LinkingOutput) const {
  const ToolChain &ToolChain = getToolChain();
  const Driver &D = ToolChain.getDriver();
  ArgStringList CmdArgs;

  if (!D.SysRoot.empty())
    CmdArgs.push_back(Args.MakeArgString("--sysroot=" + D.SysRoot));

  CmdArgs.push_back("-m");
  CmdArgs.push_back("cskyelf");

  std::string Linker = getToolChain().GetLinkerPath();

  bool WantCRTs =
      !Args.hasArg(options::OPT_nostdlib, options::OPT_nostartfiles);

  const char *crtbegin, *crtend;
  auto RuntimeLib = ToolChain.GetRuntimeLibType(Args);
  if (RuntimeLib == ToolChain::RLT_Libgcc) {
    crtbegin = "crtbegin.o";
    crtend = "crtend.o";
  } else {
    assert(RuntimeLib == ToolChain::RLT_CompilerRT);
    crtbegin = ToolChain.getCompilerRTArgString(Args, "crtbegin",
                                                ToolChain::FT_Object);
    crtend =
        ToolChain.getCompilerRTArgString(Args, "crtend", ToolChain::FT_Object);
  }

  if (WantCRTs) {
    CmdArgs.push_back(Args.MakeArgString(ToolChain.GetFilePath("crt0.o")));
    CmdArgs.push_back(Args.MakeArgString(ToolChain.GetFilePath("crti.o")));
    CmdArgs.push_back(Args.MakeArgString(ToolChain.GetFilePath(crtbegin)));
  }

  Args.AddAllArgs(CmdArgs, options::OPT_L);
  ToolChain.AddFilePathLibArgs(Args, CmdArgs);
  Args.addAllArgs(CmdArgs, {options::OPT_T_Group, options::OPT_s,
                            options::OPT_t, options::OPT_r});

  AddLinkerInputs(ToolChain, Inputs, Args, CmdArgs, JA);

  // TODO: add C++ includes and libs if compiling C++.

  if (!Args.hasArg(options::OPT_nostdlib) &&
      !Args.hasArg(options::OPT_nodefaultlibs)) {
    if (ToolChain.ShouldLinkCXXStdlib(Args))
      ToolChain.AddCXXStdlibLibArgs(Args, CmdArgs);
    CmdArgs.push_back("--start-group");
    CmdArgs.push_back("-lc");
    if (Args.hasArg(options::OPT_msim))
      CmdArgs.push_back("-lsemi");
    else
      CmdArgs.push_back("-lnosys");
    CmdArgs.push_back("--end-group");
    AddRunTimeLibs(ToolChain, ToolChain.getDriver(), CmdArgs, Args);
  }

  if (WantCRTs) {
    CmdArgs.push_back(Args.MakeArgString(ToolChain.GetFilePath(crtend)));
    CmdArgs.push_back(Args.MakeArgString(ToolChain.GetFilePath("crtn.o")));
  }

  CmdArgs.push_back("-o");
  CmdArgs.push_back(Output.getFilename());
  C.addCommand(std::make_unique<Command>(
      JA, *this, ResponseFileSupport::AtFileCurCP(), Args.MakeArgString(Linker),
      CmdArgs, Inputs, Output));
}
// CSKY tools end.
