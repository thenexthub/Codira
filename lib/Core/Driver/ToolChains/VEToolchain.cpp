//===--- VE.cpp - VE ToolChain Implementations ------------------*- C++ -*-===//
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

#include "VEToolchain.h"
#include "language/Core/Driver/CommonArgs.h"
#include "language/Core/Driver/Compilation.h"
#include "language/Core/Driver/Driver.h"
#include "language/Core/Driver/Options.h"
#include "toolchain/Option/ArgList.h"
#include "toolchain/Support/Path.h"
#include <cstdlib> // ::getenv

using namespace language::Core::driver;
using namespace language::Core::driver::toolchains;
using namespace language::Core;
using namespace toolchain::opt;

/// VE tool chain
VEToolChain::VEToolChain(const Driver &D, const toolchain::Triple &Triple,
                         const ArgList &Args)
    : Linux(D, Triple, Args) {
  getProgramPaths().push_back("/opt/nec/ve/bin");
  // ProgramPaths are found via 'PATH' environment variable.

  // Default library paths are following:
  //   ${RESOURCEDIR}/lib/ve-unknown-linux-gnu,
  // These are OK.

  // Default file paths are following:
  //   ${RESOURCEDIR}/lib/ve-unknown-linux-gnu, (== getArchSpecificLibPaths)
  //   ${RESOURCEDIR}/lib/linux/ve, (== getArchSpecificLibPaths)
  //   /lib/../lib64,
  //   /usr/lib/../lib64,
  //   ${BINPATH}/../lib,
  //   /lib,
  //   /usr/lib,
  // These are OK for host, but no go for VE.

  // Define file paths from scratch here.
  getFilePaths().clear();

  // Add library directories:
  //   ${BINPATH}/../lib/ve-unknown-linux-gnu, (== getStdlibPath)
  //   ${RESOURCEDIR}/lib/ve-unknown-linux-gnu, (== getArchSpecificLibPaths)
  //   ${RESOURCEDIR}/lib/linux/ve, (== getArchSpecificLibPaths)
  //   ${SYSROOT}/opt/nec/ve/lib,
  if (std::optional<std::string> Path = getStdlibPath())
    getFilePaths().push_back(std::move(*Path));
  for (const auto &Path : getArchSpecificLibPaths())
    getFilePaths().push_back(Path);
  getFilePaths().push_back(computeSysRoot() + "/opt/nec/ve/lib");
}

Tool *VEToolChain::buildAssembler() const {
  return new tools::gnutools::Assembler(*this);
}

Tool *VEToolChain::buildLinker() const {
  return new tools::gnutools::Linker(*this);
}

bool VEToolChain::isPICDefault() const { return false; }

bool VEToolChain::isPIEDefault(const toolchain::opt::ArgList &Args) const {
  return false;
}

bool VEToolChain::isPICDefaultForced() const { return false; }

bool VEToolChain::SupportsProfiling() const { return false; }

bool VEToolChain::hasBlocksRuntime() const { return false; }

void VEToolChain::AddClangSystemIncludeArgs(const ArgList &DriverArgs,
                                            ArgStringList &CC1Args) const {
  if (DriverArgs.hasArg(language::Core::driver::options::OPT_nostdinc))
    return;

  if (DriverArgs.hasArg(options::OPT_nobuiltininc) &&
      DriverArgs.hasArg(options::OPT_nostdlibinc))
    return;

  if (!DriverArgs.hasArg(options::OPT_nobuiltininc)) {
    SmallString<128> P(getDriver().ResourceDir);
    toolchain::sys::path::append(P, "include");
    addSystemInclude(DriverArgs, CC1Args, P);
  }

  if (!DriverArgs.hasArg(options::OPT_nostdlibinc)) {
    if (const char *cl_include_dir = getenv("NCC_C_INCLUDE_PATH")) {
      SmallVector<StringRef, 4> Dirs;
      const char EnvPathSeparatorStr[] = {toolchain::sys::EnvPathSeparator, '\0'};
      StringRef(cl_include_dir).split(Dirs, StringRef(EnvPathSeparatorStr));
      ArrayRef<StringRef> DirVec(Dirs);
      addSystemIncludes(DriverArgs, CC1Args, DirVec);
    } else {
      addSystemInclude(DriverArgs, CC1Args,
                       getDriver().SysRoot + "/opt/nec/ve/include");
    }
  }
}

void VEToolChain::addClangTargetOptions(const ArgList &DriverArgs,
                                        ArgStringList &CC1Args,
                                        Action::OffloadKind) const {
  CC1Args.push_back("-nostdsysteminc");
  bool UseInitArrayDefault = true;
  if (!DriverArgs.hasFlag(options::OPT_fuse_init_array,
                          options::OPT_fno_use_init_array, UseInitArrayDefault))
    CC1Args.push_back("-fno-use-init-array");
}

void VEToolChain::AddClangCXXStdlibIncludeArgs(const ArgList &DriverArgs,
                                               ArgStringList &CC1Args) const {
  if (DriverArgs.hasArg(language::Core::driver::options::OPT_nostdinc) ||
      DriverArgs.hasArg(options::OPT_nostdlibinc) ||
      DriverArgs.hasArg(options::OPT_nostdincxx))
    return;
  if (const char *cl_include_dir = getenv("NCC_CPLUS_INCLUDE_PATH")) {
    SmallVector<StringRef, 4> Dirs;
    const char EnvPathSeparatorStr[] = {toolchain::sys::EnvPathSeparator, '\0'};
    StringRef(cl_include_dir).split(Dirs, StringRef(EnvPathSeparatorStr));
    ArrayRef<StringRef> DirVec(Dirs);
    addSystemIncludes(DriverArgs, CC1Args, DirVec);
  } else {
    // Add following paths for multiple target installation.
    //   ${INSTALLDIR}/include/ve-unknown-linux-gnu/c++/v1,
    //   ${INSTALLDIR}/include/c++/v1,
    addLibCxxIncludePaths(DriverArgs, CC1Args);
  }
}

void VEToolChain::AddCXXStdlibLibArgs(const ArgList &Args,
                                      ArgStringList &CmdArgs) const {
  assert((GetCXXStdlibType(Args) == ToolChain::CST_Libcxx) &&
         "Only -lc++ (aka libxx) is supported in this toolchain.");

  tools::addArchSpecificRPath(*this, Args, CmdArgs);

  // Add paths for libc++.so and other shared libraries.
  if (std::optional<std::string> Path = getStdlibPath()) {
    CmdArgs.push_back("-rpath");
    CmdArgs.push_back(Args.MakeArgString(*Path));
  }

  CmdArgs.push_back("-lc++");
  if (Args.hasArg(options::OPT_fexperimental_library))
    CmdArgs.push_back("-lc++experimental");
  CmdArgs.push_back("-lc++abi");
  CmdArgs.push_back("-lunwind");
  // libc++ requires -lpthread under glibc environment
  CmdArgs.push_back("-lpthread");
  // libunwind requires -ldl under glibc environment
  CmdArgs.push_back("-ldl");
}

toolchain::ExceptionHandling
VEToolChain::GetExceptionModel(const ArgList &Args) const {
  // VE uses SjLj exceptions.
  return toolchain::ExceptionHandling::SjLj;
}
