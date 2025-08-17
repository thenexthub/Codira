//===-- PPCLinux.cpp - PowerPC ToolChain Implementations --------*- C++ -*-===//
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

#include "PPCLinux.h"
#include "language/Core/Driver/Driver.h"
#include "language/Core/Driver/Options.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/Path.h"

using namespace language::Core::driver;
using namespace language::Core::driver::toolchains;
using namespace toolchain::opt;
using namespace toolchain::sys;

// Glibc older than 2.32 doesn't fully support IEEE float128. Here we check
// glibc version by looking at dynamic linker name.
static bool GlibcSupportsFloat128(const std::string &Linker) {
  toolchain::SmallVector<char, 16> Path;

  // Resolve potential symlinks to linker.
  if (fs::real_path(Linker, Path))
    return false;
  toolchain::StringRef LinkerName =
      path::filename(toolchain::StringRef(Path.data(), Path.size()));

  // Since glibc 2.34, the installed .so file is not symlink anymore. But we can
  // still safely assume it's newer than 2.32.
  if (LinkerName.starts_with("ld64.so"))
    return true;

  if (!LinkerName.starts_with("ld-2."))
    return false;
  unsigned Minor = (LinkerName[5] - '0') * 10 + (LinkerName[6] - '0');
  if (Minor < 32)
    return false;

  return true;
}

PPCLinuxToolChain::PPCLinuxToolChain(const Driver &D,
                                     const toolchain::Triple &Triple,
                                     const toolchain::opt::ArgList &Args)
    : Linux(D, Triple, Args) {
  if (Arg *A = Args.getLastArg(options::OPT_mabi_EQ)) {
    StringRef ABIName = A->getValue();

    if ((ABIName == "ieeelongdouble" &&
         !SupportIEEEFloat128(D, Triple, Args)) ||
        (ABIName == "ibmlongdouble" && !supportIBMLongDouble(D, Args)))
      D.Diag(diag::warn_drv_unsupported_float_abi_by_lib) << ABIName;
  }
}

void PPCLinuxToolChain::AddClangSystemIncludeArgs(const ArgList &DriverArgs,
                                                  ArgStringList &CC1Args) const {
  if (!DriverArgs.hasArg(language::Core::driver::options::OPT_nostdinc) &&
      !DriverArgs.hasArg(options::OPT_nobuiltininc)) {
    const Driver &D = getDriver();
    SmallString<128> P(D.ResourceDir);
    toolchain::sys::path::append(P, "include", "ppc_wrappers");
    addSystemInclude(DriverArgs, CC1Args, P);
  }

  Linux::AddClangSystemIncludeArgs(DriverArgs, CC1Args);
}

bool PPCLinuxToolChain::supportIBMLongDouble(
    const Driver &D, const toolchain::opt::ArgList &Args) const {
  if (Args.hasArg(options::OPT_nostdlib, options::OPT_nostdlibxx))
    return true;

  CXXStdlibType StdLib = ToolChain::GetCXXStdlibType(Args);
  if (StdLib == CST_Libstdcxx)
    return true;

  return StdLib == CST_Libcxx && !defaultToIEEELongDouble();
}

bool PPCLinuxToolChain::SupportIEEEFloat128(
    const Driver &D, const toolchain::Triple &Triple,
    const toolchain::opt::ArgList &Args) const {
  if (!Triple.isLittleEndian() || !Triple.isPPC64())
    return false;

  if (Args.hasArg(options::OPT_nostdlib, options::OPT_nostdlibxx))
    return true;

  CXXStdlibType StdLib = ToolChain::GetCXXStdlibType(Args);
  bool HasUnsupportedCXXLib =
      (StdLib == CST_Libcxx && !defaultToIEEELongDouble()) ||
      (StdLib == CST_Libstdcxx &&
       GCCInstallation.getVersion().isOlderThan(12, 1, 0));

  std::string Linker = Linux::getDynamicLinker(Args);
  return GlibcSupportsFloat128((Twine(D.DyldPrefix) + Linker).str()) &&
         !(D.CCCIsCXX() && HasUnsupportedCXXLib);
}
