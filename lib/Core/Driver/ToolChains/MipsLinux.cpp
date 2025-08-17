//===-- MipsLinux.cpp - Mips ToolChain Implementations ----------*- C++ -*-===//
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

#include "MipsLinux.h"
#include "Arch/Mips.h"
#include "language/Core/Driver/Driver.h"
#include "language/Core/Driver/Options.h"
#include "toolchain/Option/ArgList.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/Path.h"

using namespace language::Core::driver;
using namespace language::Core::driver::toolchains;
using namespace language::Core;
using namespace toolchain::opt;

/// Mips Toolchain
MipsLLVMToolChain::MipsLLVMToolChain(const Driver &D,
                                     const toolchain::Triple &Triple,
                                     const ArgList &Args)
    : Linux(D, Triple, Args) {
  // Select the correct multilib according to the given arguments.
  DetectedMultilibs Result;
  findMIPSMultilibs(D, Triple, "", Args, Result);
  Multilibs = Result.Multilibs;
  SelectedMultilibs = Result.SelectedMultilibs;

  // Find out the library suffix based on the ABI.
  LibSuffix = tools::mips::getMipsABILibSuffix(Args, Triple);
  getFilePaths().clear();
  getFilePaths().push_back(computeSysRoot() + "/usr/lib" + LibSuffix);
}

void MipsLLVMToolChain::AddClangSystemIncludeArgs(
    const ArgList &DriverArgs, ArgStringList &CC1Args) const {
  if (DriverArgs.hasArg(language::Core::driver::options::OPT_nostdinc))
    return;

  const Driver &D = getDriver();

  if (!DriverArgs.hasArg(options::OPT_nobuiltininc)) {
    SmallString<128> P(D.ResourceDir);
    toolchain::sys::path::append(P, "include");
    addSystemInclude(DriverArgs, CC1Args, P);
  }

  if (DriverArgs.hasArg(options::OPT_nostdlibinc))
    return;

  const auto &Callback = Multilibs.includeDirsCallback();
  if (Callback) {
    for (const auto &Path : Callback(SelectedMultilibs.back()))
      addExternCSystemIncludeIfExists(DriverArgs, CC1Args, D.Dir + Path);
  }
}

Tool *MipsLLVMToolChain::buildLinker() const {
  return new tools::gnutools::Linker(*this);
}

std::string MipsLLVMToolChain::computeSysRoot() const {
  if (!getDriver().SysRoot.empty())
    return getDriver().SysRoot + SelectedMultilibs.back().osSuffix();

  const std::string InstalledDir(getDriver().Dir);
  std::string SysRootPath =
      InstalledDir + "/../sysroot" + SelectedMultilibs.back().osSuffix();
  if (toolchain::sys::fs::exists(SysRootPath))
    return SysRootPath;

  return std::string();
}

ToolChain::CXXStdlibType
MipsLLVMToolChain::GetCXXStdlibType(const ArgList &Args) const {
  Arg *A = Args.getLastArg(options::OPT_stdlib_EQ);
  if (A) {
    StringRef Value = A->getValue();
    if (Value != "libc++")
      getDriver().Diag(language::Core::diag::err_drv_invalid_stdlib_name)
          << A->getAsString(Args);
  }

  return ToolChain::CST_Libcxx;
}

void MipsLLVMToolChain::addLibCxxIncludePaths(
    const toolchain::opt::ArgList &DriverArgs,
    toolchain::opt::ArgStringList &CC1Args) const {
  if (const auto &Callback = Multilibs.includeDirsCallback()) {
    for (std::string Path : Callback(SelectedMultilibs.back())) {
      Path = getDriver().Dir + Path + "/c++/v1";
      if (toolchain::sys::fs::exists(Path)) {
        addSystemInclude(DriverArgs, CC1Args, Path);
        return;
      }
    }
  }
}

void MipsLLVMToolChain::AddCXXStdlibLibArgs(const ArgList &Args,
                                            ArgStringList &CmdArgs) const {
  assert((GetCXXStdlibType(Args) == ToolChain::CST_Libcxx) &&
         "Only -lc++ (aka libxx) is supported in this toolchain.");

  CmdArgs.push_back("-lc++");
  if (Args.hasArg(options::OPT_fexperimental_library))
    CmdArgs.push_back("-lc++experimental");
  CmdArgs.push_back("-lc++abi");
  CmdArgs.push_back("-lunwind");
}

std::string MipsLLVMToolChain::getCompilerRT(const ArgList &Args,
                                             StringRef Component, FileType Type,
                                             bool IsFortran) const {
  SmallString<128> Path(getDriver().ResourceDir);
  toolchain::sys::path::append(Path, SelectedMultilibs.back().osSuffix(), "lib" + LibSuffix,
                          getOS());
  const char *Suffix;
  switch (Type) {
  case ToolChain::FT_Object:
    Suffix = ".o";
    break;
  case ToolChain::FT_Static:
    Suffix = ".a";
    break;
  case ToolChain::FT_Shared:
    Suffix = ".so";
    break;
  }
  toolchain::sys::path::append(
      Path, Twine("libclang_rt." + Component + "-" + "mips" + Suffix));
  return std::string(Path);
}
