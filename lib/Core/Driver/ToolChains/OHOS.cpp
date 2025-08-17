//===--- OHOS.cpp - OHOS ToolChain Implementations --------*- C++ -*-===//
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

#include "OHOS.h"
#include "Arch/ARM.h"
#include "language/Core/Config/config.h"
#include "language/Core/Driver/CommonArgs.h"
#include "language/Core/Driver/Compilation.h"
#include "language/Core/Driver/Driver.h"
#include "language/Core/Driver/Options.h"
#include "language/Core/Driver/SanitizerArgs.h"
#include "toolchain/Option/ArgList.h"
#include "toolchain/ProfileData/InstrProf.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/VirtualFileSystem.h"

using namespace language::Core::driver;
using namespace language::Core::driver::toolchains;
using namespace language::Core::driver::tools;
using namespace language::Core;
using namespace toolchain::opt;
using namespace language::Core::driver::tools::arm;

using tools::addMultilibFlag;
using tools::addPathIfExists;

static bool findOHOSMuslMultilibs(const Driver &D,
                                  const Multilib::flags_list &Flags,
                                  DetectedMultilibs &Result) {
  MultilibSet Multilibs;
  Multilibs.push_back(Multilib());
  // -mcpu=cortex-a7
  // -mfloat-abi=soft -mfloat-abi=softfp -mfloat-abi=hard
  // -mfpu=neon-vfpv4
  Multilibs.push_back(
      Multilib("/a7_soft", {}, {}, {"-mcpu=cortex-a7", "-mfloat-abi=soft"}));

  Multilibs.push_back(
      Multilib("/a7_softfp_neon-vfpv4", {}, {},
               {"-mcpu=cortex-a7", "-mfloat-abi=softfp", "-mfpu=neon-vfpv4"}));

  Multilibs.push_back(
      Multilib("/a7_hard_neon-vfpv4", {}, {},
               {"-mcpu=cortex-a7", "-mfloat-abi=hard", "-mfpu=neon-vfpv4"}));

  if (Multilibs.select(D, Flags, Result.SelectedMultilibs)) {
    Result.Multilibs = Multilibs;
    return true;
  }
  return false;
}

static bool findOHOSMultilibs(const Driver &D,
                                      const ToolChain &TC,
                                      const toolchain::Triple &TargetTriple,
                                      StringRef Path, const ArgList &Args,
                                      DetectedMultilibs &Result) {
  Multilib::flags_list Flags;
  bool IsA7 = false;
  if (const Arg *A = Args.getLastArg(options::OPT_mcpu_EQ))
    IsA7 = A->getValue() == StringRef("cortex-a7");
  addMultilibFlag(IsA7, "-mcpu=cortex-a7", Flags);

  bool IsMFPU = false;
  if (const Arg *A = Args.getLastArg(options::OPT_mfpu_EQ))
    IsMFPU = A->getValue() == StringRef("neon-vfpv4");
  addMultilibFlag(IsMFPU, "-mfpu=neon-vfpv4", Flags);

  tools::arm::FloatABI ARMFloatABI = getARMFloatABI(D, TargetTriple, Args);
  addMultilibFlag((ARMFloatABI == tools::arm::FloatABI::Soft),
                  "-mfloat-abi=soft", Flags);
  addMultilibFlag((ARMFloatABI == tools::arm::FloatABI::SoftFP),
                  "-mfloat-abi=softfp", Flags);
  addMultilibFlag((ARMFloatABI == tools::arm::FloatABI::Hard),
                  "-mfloat-abi=hard", Flags);

  return findOHOSMuslMultilibs(D, Flags, Result);
}

std::string OHOS::getMultiarchTriple(const toolchain::Triple &T) const {
  // For most architectures, just use whatever we have rather than trying to be
  // clever.
  switch (T.getArch()) {
  default:
    break;

  // We use the existence of '/lib/<triple>' as a directory to detect some
  // common linux triples that don't quite match the Clang triple for both
  // 32-bit and 64-bit targets. Multiarch fixes its install triples to these
  // regardless of what the actual target triple is.
  case toolchain::Triple::arm:
  case toolchain::Triple::thumb:
    return T.isOSLiteOS() ? "arm-liteos-ohos" : "arm-linux-ohos";
  case toolchain::Triple::riscv32:
    return "riscv32-linux-ohos";
  case toolchain::Triple::riscv64:
    return "riscv64-linux-ohos";
  case toolchain::Triple::mipsel:
    return "mipsel-linux-ohos";
  case toolchain::Triple::x86:
    return "i686-linux-ohos";
  case toolchain::Triple::x86_64:
    return "x86_64-linux-ohos";
  case toolchain::Triple::aarch64:
    return "aarch64-linux-ohos";
  case toolchain::Triple::loongarch64:
    return "loongarch64-linux-ohos";
  }
  return T.str();
}

std::string OHOS::getMultiarchTriple(const Driver &D,
                                     const toolchain::Triple &TargetTriple,
                                     StringRef SysRoot) const {
  return getMultiarchTriple(TargetTriple);
}

static std::string makePath(const std::initializer_list<std::string> &IL) {
  SmallString<128> P;
  for (const auto &S : IL)
    toolchain::sys::path::append(P, S);
  return static_cast<std::string>(P.str());
}

/// OHOS Toolchain
OHOS::OHOS(const Driver &D, const toolchain::Triple &Triple, const ArgList &Args)
    : Generic_ELF(D, Triple, Args) {
  std::string SysRoot = computeSysRoot();

  // Select the correct multilib according to the given arguments.
  DetectedMultilibs Result;
  findOHOSMultilibs(D, *this, Triple, "", Args, Result);
  Multilibs = Result.Multilibs;
  SelectedMultilibs = Result.SelectedMultilibs;
  if (!SelectedMultilibs.empty()) {
    SelectedMultilib = SelectedMultilibs.back();
  }

  getFilePaths().clear();
  for (const auto &CandidateLibPath : getArchSpecificLibPaths())
    if (getVFS().exists(CandidateLibPath))
      getFilePaths().push_back(CandidateLibPath);

  getLibraryPaths().clear();
  for (auto &Path : getRuntimePaths())
    if (getVFS().exists(Path))
      getLibraryPaths().push_back(Path);

  // OHOS sysroots contain a library directory for each supported OS
  // version as well as some unversioned libraries in the usual multiarch
  // directory. Support --target=aarch64-linux-ohosX.Y.Z or
  // --target=aarch64-linux-ohosX.Y or --target=aarch64-linux-ohosX
  path_list &Paths = getFilePaths();
  std::string SysRootLibPath = makePath({SysRoot, "usr", "lib"});
  std::string MultiarchTriple = getMultiarchTriple(getTriple());
  addPathIfExists(D, makePath({SysRootLibPath, SelectedMultilib.gccSuffix()}),
                  Paths);
  addPathIfExists(D,
                  makePath({D.Dir, "..", "lib", MultiarchTriple,
                            SelectedMultilib.gccSuffix()}),
                  Paths);

  addPathIfExists(
      D,
      makePath({SysRootLibPath, MultiarchTriple, SelectedMultilib.gccSuffix()}),
      Paths);
}

ToolChain::RuntimeLibType OHOS::GetRuntimeLibType(
    const ArgList &Args) const {
  if (Arg *A = Args.getLastArg(language::Core::driver::options::OPT_rtlib_EQ)) {
    StringRef Value = A->getValue();
    if (Value != "compiler-rt")
      getDriver().Diag(language::Core::diag::err_drv_invalid_rtlib_name)
          << A->getAsString(Args);
  }

  return ToolChain::RLT_CompilerRT;
}

ToolChain::CXXStdlibType
OHOS::GetCXXStdlibType(const ArgList &Args) const {
  if (Arg *A = Args.getLastArg(options::OPT_stdlib_EQ)) {
    StringRef Value = A->getValue();
    if (Value != "libc++")
      getDriver().Diag(diag::err_drv_invalid_stdlib_name)
        << A->getAsString(Args);
  }

  return ToolChain::CST_Libcxx;
}

void OHOS::AddClangSystemIncludeArgs(const ArgList &DriverArgs,
                                        ArgStringList &CC1Args) const {
  const Driver &D = getDriver();
  const toolchain::Triple &Triple = getTriple();
  std::string SysRoot = computeSysRoot();

  if (DriverArgs.hasArg(options::OPT_nostdinc))
    return;

  if (!DriverArgs.hasArg(options::OPT_nobuiltininc)) {
    SmallString<128> P(D.ResourceDir);
    toolchain::sys::path::append(P, "include");
    addSystemInclude(DriverArgs, CC1Args, P);
  }

  if (DriverArgs.hasArg(options::OPT_nostdlibinc))
    return;

  // Check for configure-time C include directories.
  StringRef CIncludeDirs(C_INCLUDE_DIRS);
  if (CIncludeDirs != "") {
    SmallVector<StringRef, 5> dirs;
    CIncludeDirs.split(dirs, ":");
    for (StringRef dir : dirs) {
      StringRef Prefix =
          toolchain::sys::path::is_absolute(dir) ? StringRef(SysRoot) : "";
      addExternCSystemInclude(DriverArgs, CC1Args, Prefix + dir);
    }
    return;
  }

  addExternCSystemInclude(DriverArgs, CC1Args,
                          SysRoot + "/usr/include/" +
                              getMultiarchTriple(Triple));
  addExternCSystemInclude(DriverArgs, CC1Args, SysRoot + "/include");
  addExternCSystemInclude(DriverArgs, CC1Args, SysRoot + "/usr/include");
}

void OHOS::AddClangCXXStdlibIncludeArgs(const ArgList &DriverArgs,
                                        ArgStringList &CC1Args) const {
  if (DriverArgs.hasArg(options::OPT_nostdlibinc) ||
      DriverArgs.hasArg(options::OPT_nostdincxx))
    return;

  switch (GetCXXStdlibType(DriverArgs)) {
  case ToolChain::CST_Libcxx: {
    std::string IncPath = makePath({getDriver().Dir, "..", "include"});
    std::string IncTargetPath =
        makePath({IncPath, getMultiarchTriple(getTriple()), "c++", "v1"});
    if (getVFS().exists(IncTargetPath)) {
      addSystemInclude(DriverArgs, CC1Args, makePath({IncPath, "c++", "v1"}));
      addSystemInclude(DriverArgs, CC1Args, IncTargetPath);
    }
    break;
  }

  default:
    toolchain_unreachable("invalid stdlib name");
  }
}

void OHOS::AddCXXStdlibLibArgs(const ArgList &Args,
                                  ArgStringList &CmdArgs) const {
  switch (GetCXXStdlibType(Args)) {
  case ToolChain::CST_Libcxx:
    CmdArgs.push_back("-lc++");
    CmdArgs.push_back("-lc++abi");
    CmdArgs.push_back("-lunwind");
    break;

  case ToolChain::CST_Libstdcxx:
    toolchain_unreachable("invalid stdlib name");
  }
}

std::string OHOS::computeSysRoot() const {
  std::string SysRoot =
      !getDriver().SysRoot.empty()
          ? getDriver().SysRoot
          : makePath({getDriver().Dir, "..", "..", "sysroot"});
  if (!toolchain::sys::fs::exists(SysRoot))
    return std::string();

  std::string ArchRoot = makePath({SysRoot, getMultiarchTriple(getTriple())});
  return toolchain::sys::fs::exists(ArchRoot) ? ArchRoot : SysRoot;
}

ToolChain::path_list OHOS::getRuntimePaths() const {
  SmallString<128> P;
  path_list Paths;
  const Driver &D = getDriver();
  const toolchain::Triple &Triple = getTriple();

  // First try the triple passed to driver as --target=<triple>.
  P.assign(D.ResourceDir);
  toolchain::sys::path::append(P, "lib", D.getTargetTriple(), SelectedMultilib.gccSuffix());
  Paths.push_back(P.c_str());

  // Second try the normalized triple.
  P.assign(D.ResourceDir);
  toolchain::sys::path::append(P, "lib", Triple.str(), SelectedMultilib.gccSuffix());
  Paths.push_back(P.c_str());

  // Third try the effective triple.
  P.assign(D.ResourceDir);
  toolchain::sys::path::append(P, "lib", getMultiarchTriple(Triple),
                          SelectedMultilib.gccSuffix());
  Paths.push_back(P.c_str());

  return Paths;
}

std::string OHOS::getDynamicLinker(const ArgList &Args) const {
  const toolchain::Triple &Triple = getTriple();
  const toolchain::Triple::ArchType Arch = getArch();

  assert(Triple.isMusl());
  std::string ArchName;
  bool IsArm = false;

  switch (Arch) {
  case toolchain::Triple::arm:
  case toolchain::Triple::thumb:
    ArchName = "arm";
    IsArm = true;
    break;
  case toolchain::Triple::armeb:
  case toolchain::Triple::thumbeb:
    ArchName = "armeb";
    IsArm = true;
    break;
  default:
    ArchName = Triple.getArchName().str();
  }
  if (IsArm &&
      (tools::arm::getARMFloatABI(*this, Args) == tools::arm::FloatABI::Hard))
    ArchName += "hf";

  return "/lib/ld-musl-" + ArchName + ".so.1";
}

std::string OHOS::getCompilerRT(const ArgList &Args, StringRef Component,
                                FileType Type, bool IsFortran) const {
  SmallString<128> Path(getDriver().ResourceDir);
  toolchain::sys::path::append(Path, "lib", getMultiarchTriple(getTriple()),
                          SelectedMultilib.gccSuffix());
  const char *Prefix =
      Type == ToolChain::FT_Object ? "" : "lib";
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
      Path, Prefix + Twine("clang_rt.") + Component + Suffix);
  return static_cast<std::string>(Path.str());
}

void OHOS::addExtraOpts(toolchain::opt::ArgStringList &CmdArgs) const {
  CmdArgs.push_back("-z");
  CmdArgs.push_back("now");
  CmdArgs.push_back("-z");
  CmdArgs.push_back("relro");
  CmdArgs.push_back("-z");
  CmdArgs.push_back(getArch() == toolchain::Triple::loongarch64
                        ? "max-page-size=16384"
                        : "max-page-size=4096");
  // .gnu.hash section is not compatible with the MIPS target
  if (getArch() != toolchain::Triple::mipsel)
    CmdArgs.push_back("--hash-style=both");
#ifdef ENABLE_LINKER_BUILD_ID
  CmdArgs.push_back("--build-id");
#endif
  CmdArgs.push_back("--enable-new-dtags");
}

SanitizerMask OHOS::getSupportedSanitizers() const {
  SanitizerMask Res = ToolChain::getSupportedSanitizers();
  Res |= SanitizerKind::Address;
  Res |= SanitizerKind::PointerCompare;
  Res |= SanitizerKind::PointerSubtract;
  Res |= SanitizerKind::Fuzzer;
  Res |= SanitizerKind::FuzzerNoLink;
  Res |= SanitizerKind::Memory;
  Res |= SanitizerKind::Vptr;
  Res |= SanitizerKind::SafeStack;
  Res |= SanitizerKind::Scudo;
  // TODO: kASAN for liteos ??
  // TODO: Support TSAN and HWASAN and update mask.
  return Res;
}

// TODO: Make a base class for Linux and OHOS and move this there.
void OHOS::addProfileRTLibs(const toolchain::opt::ArgList &Args,
                             toolchain::opt::ArgStringList &CmdArgs) const {
  // Add linker option -u__toolchain_profile_runtime to cause runtime
  // initialization module to be linked in.
  if (needsProfileRT(Args))
    CmdArgs.push_back(Args.MakeArgString(
        Twine("-u", toolchain::getInstrProfRuntimeHookVarName())));
  ToolChain::addProfileRTLibs(Args, CmdArgs);
}

ToolChain::path_list OHOS::getArchSpecificLibPaths() const {
  ToolChain::path_list Paths;
  toolchain::Triple Triple = getTriple();
  Paths.push_back(
      makePath({getDriver().ResourceDir, "lib", getMultiarchTriple(Triple)}));
  return Paths;
}

ToolChain::UnwindLibType OHOS::GetUnwindLibType(const toolchain::opt::ArgList &Args) const {
  if (Args.getLastArg(options::OPT_unwindlib_EQ))
    return Generic_ELF::GetUnwindLibType(Args);
  return GetDefaultUnwindLibType();
}
