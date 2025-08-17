//===--- MinGW.cpp - MinGWToolChain Implementation ------------------------===//
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

#include "MinGW.h"
#include "language/Core/Config/config.h"
#include "language/Core/Driver/CommonArgs.h"
#include "language/Core/Driver/Compilation.h"
#include "language/Core/Driver/Driver.h"
#include "language/Core/Driver/InputInfo.h"
#include "language/Core/Driver/Options.h"
#include "language/Core/Driver/SanitizerArgs.h"
#include "toolchain/Config/toolchain-config.h" // for LLVM_HOST_TRIPLE
#include "toolchain/Option/ArgList.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/VirtualFileSystem.h"
#include <system_error>

using namespace language::Core::diag;
using namespace language::Core::driver;
using namespace language::Core;
using namespace toolchain::opt;

/// MinGW Tools
void tools::MinGW::Assembler::ConstructJob(Compilation &C, const JobAction &JA,
                                           const InputInfo &Output,
                                           const InputInfoList &Inputs,
                                           const ArgList &Args,
                                           const char *LinkingOutput) const {
  claimNoWarnArgs(Args);
  ArgStringList CmdArgs;

  if (getToolChain().getArch() == toolchain::Triple::x86) {
    CmdArgs.push_back("--32");
  } else if (getToolChain().getArch() == toolchain::Triple::x86_64) {
    CmdArgs.push_back("--64");
  }

  Args.AddAllArgValues(CmdArgs, options::OPT_Wa_COMMA, options::OPT_Xassembler);

  CmdArgs.push_back("-o");
  CmdArgs.push_back(Output.getFilename());

  for (const auto &II : Inputs)
    CmdArgs.push_back(II.getFilename());

  const char *Exec = Args.MakeArgString(getToolChain().GetProgramPath("as"));
  C.addCommand(std::make_unique<Command>(JA, *this, ResponseFileSupport::None(),
                                         Exec, CmdArgs, Inputs, Output));

  if (Args.hasArg(options::OPT_gsplit_dwarf))
    SplitDebugInfo(getToolChain(), C, *this, JA, Args, Output,
                   SplitDebugName(JA, Args, Inputs[0], Output));
}

void tools::MinGW::Linker::AddLibGCC(const ArgList &Args,
                                     ArgStringList &CmdArgs) const {
  if (Args.hasArg(options::OPT_mthreads))
    CmdArgs.push_back("-lmingwthrd");
  CmdArgs.push_back("-lmingw32");

  // Make use of compiler-rt if --rtlib option is used
  ToolChain::RuntimeLibType RLT = getToolChain().GetRuntimeLibType(Args);
  if (RLT == ToolChain::RLT_Libgcc) {
    bool Static = Args.hasArg(options::OPT_static_libgcc) ||
                  Args.hasArg(options::OPT_static);
    bool Shared = Args.hasArg(options::OPT_shared);
    bool CXX = getToolChain().getDriver().CCCIsCXX();

    if (Static || (!CXX && !Shared)) {
      CmdArgs.push_back("-lgcc");
      CmdArgs.push_back("-lgcc_eh");
    } else {
      CmdArgs.push_back("-lgcc_s");
      CmdArgs.push_back("-lgcc");
    }
  } else {
    AddRunTimeLibs(getToolChain(), getToolChain().getDriver(), CmdArgs, Args);
  }

  CmdArgs.push_back("-lmoldname");
  CmdArgs.push_back("-lmingwex");
  for (auto Lib : Args.getAllArgValues(options::OPT_l)) {
    if (StringRef(Lib).starts_with("msvcr") ||
        StringRef(Lib).starts_with("ucrt") ||
        StringRef(Lib).starts_with("crtdll")) {
      std::string CRTLib = (toolchain::Twine("-l") + Lib).str();
      // Respect the user's chosen crt variant, but still provide it
      // again as the last linker argument, because some of the libraries
      // we added above may depend on it.
      CmdArgs.push_back(Args.MakeArgStringRef(CRTLib));
      return;
    }
  }
  CmdArgs.push_back("-lmsvcrt");
}

void tools::MinGW::Linker::ConstructJob(Compilation &C, const JobAction &JA,
                                        const InputInfo &Output,
                                        const InputInfoList &Inputs,
                                        const ArgList &Args,
                                        const char *LinkingOutput) const {
  const ToolChain &TC = getToolChain();
  const Driver &D = TC.getDriver();
  const SanitizerArgs &Sanitize = TC.getSanitizerArgs(Args);

  ArgStringList CmdArgs;

  // Silence warning for "clang -g foo.o -o foo"
  Args.ClaimAllArgs(options::OPT_g_Group);
  // and "clang -emit-toolchain foo.o -o foo"
  Args.ClaimAllArgs(options::OPT_emit_toolchain);
  // and for "clang -w foo.o -o foo". Other warning options are already
  // handled somewhere else.
  Args.ClaimAllArgs(options::OPT_w);

  if (!D.SysRoot.empty())
    CmdArgs.push_back(Args.MakeArgString("--sysroot=" + D.SysRoot));

  if (Args.hasArg(options::OPT_s))
    CmdArgs.push_back("-s");

  CmdArgs.push_back("-m");
  switch (TC.getArch()) {
  case toolchain::Triple::x86:
    CmdArgs.push_back("i386pe");
    break;
  case toolchain::Triple::x86_64:
    CmdArgs.push_back("i386pep");
    break;
  case toolchain::Triple::arm:
  case toolchain::Triple::thumb:
    // FIXME: this is incorrect for WinCE
    CmdArgs.push_back("thumb2pe");
    break;
  case toolchain::Triple::aarch64:
    if (Args.hasArg(options::OPT_marm64x))
      CmdArgs.push_back("arm64xpe");
    else if (TC.getEffectiveTriple().isWindowsArm64EC())
      CmdArgs.push_back("arm64ecpe");
    else
      CmdArgs.push_back("arm64pe");
    break;
  case toolchain::Triple::mipsel:
    CmdArgs.push_back("mipspe");
    break;
  default:
    D.Diag(diag::err_target_unknown_triple) << TC.getEffectiveTriple().str();
  }

  Arg *SubsysArg =
      Args.getLastArg(options::OPT_mwindows, options::OPT_mconsole);
  if (SubsysArg && SubsysArg->getOption().matches(options::OPT_mwindows)) {
    CmdArgs.push_back("--subsystem");
    CmdArgs.push_back("windows");
  } else if (SubsysArg &&
             SubsysArg->getOption().matches(options::OPT_mconsole)) {
    CmdArgs.push_back("--subsystem");
    CmdArgs.push_back("console");
  }

  if (Args.hasArg(options::OPT_mdll))
    CmdArgs.push_back("--dll");
  else if (Args.hasArg(options::OPT_shared))
    CmdArgs.push_back("--shared");
  if (Args.hasArg(options::OPT_static))
    CmdArgs.push_back("-Bstatic");
  else
    CmdArgs.push_back("-Bdynamic");
  if (Args.hasArg(options::OPT_mdll) || Args.hasArg(options::OPT_shared)) {
    CmdArgs.push_back("-e");
    if (TC.getArch() == toolchain::Triple::x86)
      CmdArgs.push_back("_DllMainCRTStartup@12");
    else
      CmdArgs.push_back("DllMainCRTStartup");
    CmdArgs.push_back("--enable-auto-image-base");
  }

  if (Args.hasArg(options::OPT_Z_Xlinker__no_demangle))
    CmdArgs.push_back("--no-demangle");

  if (!Args.hasFlag(options::OPT_fauto_import, options::OPT_fno_auto_import,
                    true))
    CmdArgs.push_back("--disable-auto-import");

  if (Arg *A = Args.getLastArg(options::OPT_mguard_EQ)) {
    StringRef GuardArgs = A->getValue();
    if (GuardArgs == "none")
      CmdArgs.push_back("--no-guard-cf");
    else if (GuardArgs == "cf" || GuardArgs == "cf-nochecks")
      CmdArgs.push_back("--guard-cf");
    else
      D.Diag(diag::err_drv_unsupported_option_argument)
          << A->getSpelling() << GuardArgs;
  }

  if (Args.hasArg(options::OPT_fms_hotpatch))
    CmdArgs.push_back("--functionpadmin");

  CmdArgs.push_back("-o");
  const char *OutputFile = Output.getFilename();
  // GCC implicitly adds an .exe extension if it is given an output file name
  // that lacks an extension.
  // GCC used to do this only when the compiler itself runs on windows, but
  // since GCC 8 it does the same when cross compiling as well.
  if (!toolchain::sys::path::has_extension(OutputFile)) {
    CmdArgs.push_back(Args.MakeArgString(Twine(OutputFile) + ".exe"));
    OutputFile = CmdArgs.back();
  } else
    CmdArgs.push_back(OutputFile);

  // FIXME: add -N, -n flags
  Args.AddLastArg(CmdArgs, options::OPT_r);
  Args.AddLastArg(CmdArgs, options::OPT_s);
  Args.AddLastArg(CmdArgs, options::OPT_t);
  Args.AddAllArgs(CmdArgs, options::OPT_u_Group);

  // Add asan_dynamic as the first import lib before other libs. This allows
  // asan to be initialized as early as possible to increase its instrumentation
  // coverage to include other user DLLs which has not been built with asan.
  if (Sanitize.needsAsanRt() && !Args.hasArg(options::OPT_nostdlib) &&
      !Args.hasArg(options::OPT_nodefaultlibs)) {
    // MinGW always links against a shared MSVCRT.
    CmdArgs.push_back(
        TC.getCompilerRTArgString(Args, "asan_dynamic", ToolChain::FT_Shared));
  }

  if (!Args.hasArg(options::OPT_nostdlib, options::OPT_nostartfiles)) {
    if (Args.hasArg(options::OPT_shared) || Args.hasArg(options::OPT_mdll)) {
      CmdArgs.push_back(Args.MakeArgString(TC.GetFilePath("dllcrt2.o")));
    } else {
      if (Args.hasArg(options::OPT_municode))
        CmdArgs.push_back(Args.MakeArgString(TC.GetFilePath("crt2u.o")));
      else
        CmdArgs.push_back(Args.MakeArgString(TC.GetFilePath("crt2.o")));
    }
    if (Args.hasArg(options::OPT_pg))
      CmdArgs.push_back(Args.MakeArgString(TC.GetFilePath("gcrt2.o")));
    CmdArgs.push_back(Args.MakeArgString(TC.GetFilePath("crtbegin.o")));
  }

  Args.AddAllArgs(CmdArgs, options::OPT_L);
  TC.AddFilePathLibArgs(Args, CmdArgs);

  // Add the compiler-rt library directories if they exist to help
  // the linker find the various sanitizer, builtin, and profiling runtimes.
  for (const auto &LibPath : TC.getLibraryPaths()) {
    if (TC.getVFS().exists(LibPath))
      CmdArgs.push_back(Args.MakeArgString("-L" + LibPath));
  }
  auto CRTPath = TC.getCompilerRTPath();
  if (TC.getVFS().exists(CRTPath))
    CmdArgs.push_back(Args.MakeArgString("-L" + CRTPath));

  AddLinkerInputs(TC, Inputs, Args, CmdArgs, JA);

  if (D.isUsingLTO())
    addLTOOptions(TC, Args, CmdArgs, Output, Inputs,
                  D.getLTOMode() == LTOK_Thin);

  if (C.getDriver().IsFlangMode() &&
      !Args.hasArg(options::OPT_nostdlib, options::OPT_nodefaultlibs)) {
    TC.addFortranRuntimeLibraryPath(Args, CmdArgs);
    TC.addFortranRuntimeLibs(Args, CmdArgs);
  }

  // TODO: Add profile stuff here

  if (TC.ShouldLinkCXXStdlib(Args)) {
    bool OnlyLibstdcxxStatic = Args.hasArg(options::OPT_static_libstdcxx) &&
                               !Args.hasArg(options::OPT_static);
    if (OnlyLibstdcxxStatic)
      CmdArgs.push_back("-Bstatic");
    TC.AddCXXStdlibLibArgs(Args, CmdArgs);
    if (OnlyLibstdcxxStatic)
      CmdArgs.push_back("-Bdynamic");
  }

  bool HasWindowsApp = false;
  for (auto Lib : Args.getAllArgValues(options::OPT_l)) {
    if (Lib == "windowsapp") {
      HasWindowsApp = true;
      break;
    }
  }

  if (!Args.hasArg(options::OPT_nostdlib)) {
    if (!Args.hasArg(options::OPT_nodefaultlibs)) {
      if (Args.hasArg(options::OPT_static))
        CmdArgs.push_back("--start-group");

      if (Args.hasArg(options::OPT_fstack_protector) ||
          Args.hasArg(options::OPT_fstack_protector_strong) ||
          Args.hasArg(options::OPT_fstack_protector_all)) {
        CmdArgs.push_back("-lssp_nonshared");
        CmdArgs.push_back("-lssp");
      }

      if (Args.hasFlag(options::OPT_fopenmp, options::OPT_fopenmp_EQ,
                       options::OPT_fno_openmp, false)) {
        switch (TC.getDriver().getOpenMPRuntime(Args)) {
        case Driver::OMPRT_OMP:
          CmdArgs.push_back("-lomp");
          break;
        case Driver::OMPRT_IOMP5:
          CmdArgs.push_back("-liomp5md");
          break;
        case Driver::OMPRT_GOMP:
          CmdArgs.push_back("-lgomp");
          break;
        case Driver::OMPRT_Unknown:
          // Already diagnosed.
          break;
        }
      }

      AddLibGCC(Args, CmdArgs);

      if (Args.hasArg(options::OPT_pg))
        CmdArgs.push_back("-lgmon");

      if (Args.hasArg(options::OPT_pthread))
        CmdArgs.push_back("-lpthread");

      if (Sanitize.needsAsanRt()) {
        // MinGW always links against a shared MSVCRT.
        CmdArgs.push_back(TC.getCompilerRTArgString(Args, "asan_dynamic",
                                                    ToolChain::FT_Shared));
        CmdArgs.push_back(
            TC.getCompilerRTArgString(Args, "asan_dynamic_runtime_thunk"));
        CmdArgs.push_back("--require-defined");
        CmdArgs.push_back(TC.getArch() == toolchain::Triple::x86
                              ? "___asan_seh_interceptor"
                              : "__asan_seh_interceptor");
        // Make sure the linker consider all object files from the dynamic
        // runtime thunk.
        CmdArgs.push_back("--whole-archive");
        CmdArgs.push_back(
            TC.getCompilerRTArgString(Args, "asan_dynamic_runtime_thunk"));
        CmdArgs.push_back("--no-whole-archive");
      }

      TC.addProfileRTLibs(Args, CmdArgs);

      if (!HasWindowsApp) {
        // Add system libraries. If linking to libwindowsapp.a, that import
        // library replaces all these and we shouldn't accidentally try to
        // link to the normal desktop mode dlls.
        if (Args.hasArg(options::OPT_mwindows)) {
          CmdArgs.push_back("-lgdi32");
          CmdArgs.push_back("-lcomdlg32");
        }
        CmdArgs.push_back("-ladvapi32");
        CmdArgs.push_back("-lshell32");
        CmdArgs.push_back("-luser32");
        CmdArgs.push_back("-lkernel32");
      }

      if (Args.hasArg(options::OPT_static)) {
        CmdArgs.push_back("--end-group");
      } else {
        AddLibGCC(Args, CmdArgs);
        if (!HasWindowsApp)
          CmdArgs.push_back("-lkernel32");
      }
    }

    if (!Args.hasArg(options::OPT_nostartfiles)) {
      // Add crtfastmath.o if available and fast math is enabled.
      TC.addFastMathRuntimeIfAvailable(Args, CmdArgs);

      CmdArgs.push_back(Args.MakeArgString(TC.GetFilePath("crtend.o")));
    }
  }
  const char *Exec = Args.MakeArgString(TC.GetLinkerPath());
  C.addCommand(std::make_unique<Command>(JA, *this,
                                         ResponseFileSupport::AtFileUTF8(),
                                         Exec, CmdArgs, Inputs, Output));
}

static bool isCrossCompiling(const toolchain::Triple &T, bool RequireArchMatch) {
  toolchain::Triple HostTriple(toolchain::Triple::normalize(LLVM_HOST_TRIPLE));
  if (HostTriple.getOS() != toolchain::Triple::Win32)
    return true;
  if (RequireArchMatch && HostTriple.getArch() != T.getArch())
    return true;
  return false;
}

// Simplified from Generic_GCC::GCCInstallationDetector::ScanLibDirForGCCTriple.
static bool findGccVersion(StringRef LibDir, std::string &GccLibDir,
                           std::string &Ver,
                           toolchains::Generic_GCC::GCCVersion &Version) {
  Version = toolchains::Generic_GCC::GCCVersion::Parse("0.0.0");
  std::error_code EC;
  for (toolchain::sys::fs::directory_iterator LI(LibDir, EC), LE; !EC && LI != LE;
       LI = LI.increment(EC)) {
    StringRef VersionText = toolchain::sys::path::filename(LI->path());
    auto CandidateVersion =
        toolchains::Generic_GCC::GCCVersion::Parse(VersionText);
    if (CandidateVersion.Major == -1)
      continue;
    if (CandidateVersion <= Version)
      continue;
    Version = CandidateVersion;
    Ver = std::string(VersionText);
    GccLibDir = LI->path();
  }
  return Ver.size();
}

static toolchain::Triple getLiteralTriple(const Driver &D, const toolchain::Triple &T) {
  toolchain::Triple LiteralTriple(D.getTargetTriple());
  // The arch portion of the triple may be overridden by -m32/-m64.
  LiteralTriple.setArchName(T.getArchName());
  return LiteralTriple;
}

void toolchains::MinGW::findGccLibDir(const toolchain::Triple &LiteralTriple) {
  toolchain::SmallVector<toolchain::SmallString<32>, 5> SubdirNames;
  SubdirNames.emplace_back(LiteralTriple.str());
  SubdirNames.emplace_back(getTriple().str());
  SubdirNames.emplace_back(getTriple().getArchName());
  SubdirNames.back() += "-w64-mingw32";
  SubdirNames.emplace_back(getTriple().getArchName());
  SubdirNames.back() += "-w64-mingw32ucrt";
  SubdirNames.emplace_back("mingw32");
  if (SubdirName.empty()) {
    SubdirName = getTriple().getArchName();
    SubdirName += "-w64-mingw32";
  }
  // lib: Arch Linux, Ubuntu, Windows
  // lib64: openSUSE Linux
  for (StringRef CandidateLib : {"lib", "lib64"}) {
    for (StringRef CandidateSysroot : SubdirNames) {
      toolchain::SmallString<1024> LibDir(Base);
      toolchain::sys::path::append(LibDir, CandidateLib, "gcc", CandidateSysroot);
      if (findGccVersion(LibDir, GccLibDir, Ver, GccVer)) {
        SubdirName = std::string(CandidateSysroot);
        return;
      }
    }
  }
}

static toolchain::ErrorOr<std::string> findGcc(const toolchain::Triple &LiteralTriple,
                                          const toolchain::Triple &T) {
  toolchain::SmallVector<toolchain::SmallString<32>, 5> Gccs;
  Gccs.emplace_back(LiteralTriple.str());
  Gccs.back() += "-gcc";
  Gccs.emplace_back(T.str());
  Gccs.back() += "-gcc";
  Gccs.emplace_back(T.getArchName());
  Gccs.back() += "-w64-mingw32-gcc";
  Gccs.emplace_back(T.getArchName());
  Gccs.back() += "-w64-mingw32ucrt-gcc";
  Gccs.emplace_back("mingw32-gcc");
  // Please do not add "gcc" here
  for (StringRef CandidateGcc : Gccs)
    if (toolchain::ErrorOr<std::string> GPPName = toolchain::sys::findProgramByName(CandidateGcc))
      return GPPName;
  return make_error_code(std::errc::no_such_file_or_directory);
}

static toolchain::ErrorOr<std::string>
findClangRelativeSysroot(const Driver &D, const toolchain::Triple &LiteralTriple,
                         const toolchain::Triple &T, std::string &SubdirName) {
  toolchain::SmallVector<toolchain::SmallString<32>, 4> Subdirs;
  Subdirs.emplace_back(LiteralTriple.str());
  Subdirs.emplace_back(T.str());
  Subdirs.emplace_back(T.getArchName());
  Subdirs.back() += "-w64-mingw32";
  Subdirs.emplace_back(T.getArchName());
  Subdirs.back() += "-w64-mingw32ucrt";
  StringRef ClangRoot = toolchain::sys::path::parent_path(D.Dir);
  StringRef Sep = toolchain::sys::path::get_separator();
  for (StringRef CandidateSubdir : Subdirs) {
    if (toolchain::sys::fs::is_directory(ClangRoot + Sep + CandidateSubdir)) {
      SubdirName = std::string(CandidateSubdir);
      return (ClangRoot + Sep + CandidateSubdir).str();
    }
  }
  return make_error_code(std::errc::no_such_file_or_directory);
}

static bool looksLikeMinGWSysroot(const std::string &Directory) {
  StringRef Sep = toolchain::sys::path::get_separator();
  if (!toolchain::sys::fs::exists(Directory + Sep + "include" + Sep + "_mingw.h"))
    return false;
  if (!toolchain::sys::fs::exists(Directory + Sep + "lib" + Sep + "libkernel32.a"))
    return false;
  return true;
}

toolchains::MinGW::MinGW(const Driver &D, const toolchain::Triple &Triple,
                         const ArgList &Args)
    : ToolChain(D, Triple, Args), CudaInstallation(D, Triple, Args),
      RocmInstallation(D, Triple, Args) {
  getProgramPaths().push_back(getDriver().Dir);

  std::string InstallBase =
      std::string(toolchain::sys::path::parent_path(getDriver().Dir));
  // The sequence for detecting a sysroot here should be kept in sync with
  // the testTriple function below.
  toolchain::Triple LiteralTriple = getLiteralTriple(D, getTriple());
  if (getDriver().SysRoot.size())
    Base = getDriver().SysRoot;
  // Look for <clang-bin>/../<triplet>; if found, use <clang-bin>/.. as the
  // base as it could still be a base for a gcc setup with libgcc.
  else if (toolchain::ErrorOr<std::string> TargetSubdir = findClangRelativeSysroot(
               getDriver(), LiteralTriple, getTriple(), SubdirName))
    Base = std::string(toolchain::sys::path::parent_path(TargetSubdir.get()));
  // If the install base of Clang seems to have mingw sysroot files directly
  // in the toplevel include and lib directories, use this as base instead of
  // looking for a triple prefixed GCC in the path.
  else if (looksLikeMinGWSysroot(InstallBase))
    Base = InstallBase;
  else if (toolchain::ErrorOr<std::string> GPPName =
               findGcc(LiteralTriple, getTriple()))
    Base = std::string(toolchain::sys::path::parent_path(
        toolchain::sys::path::parent_path(GPPName.get())));
  else
    Base = InstallBase;

  Base += toolchain::sys::path::get_separator();
  findGccLibDir(LiteralTriple);
  TripleDirName = SubdirName;
  // GccLibDir must precede Base/lib so that the
  // correct crtbegin.o ,cetend.o would be found.
  getFilePaths().push_back(GccLibDir);

  // openSUSE/Fedora
  std::string CandidateSubdir = SubdirName + "/sys-root/mingw";
  if (getDriver().getVFS().exists(Base + CandidateSubdir))
    SubdirName = CandidateSubdir;

  getFilePaths().push_back(
      (Base + SubdirName + toolchain::sys::path::get_separator() + "lib").str());

  // Gentoo
  getFilePaths().push_back(
      (Base + SubdirName + toolchain::sys::path::get_separator() + "mingw/lib").str());

  // Only include <base>/lib if we're not cross compiling (not even for
  // windows->windows to a different arch), or if the sysroot has been set
  // (where we presume the user has pointed it at an arch specific
  // subdirectory).
  if (!::isCrossCompiling(getTriple(), /*RequireArchMatch=*/true) ||
      getDriver().SysRoot.size())
    getFilePaths().push_back(Base + "lib");

  NativeLLVMSupport =
      Args.getLastArgValue(options::OPT_fuse_ld_EQ, D.getPreferredLinker())
          .equals_insensitive("lld");
}

Tool *toolchains::MinGW::getTool(Action::ActionClass AC) const {
  switch (AC) {
  case Action::PreprocessJobClass:
    if (!Preprocessor)
      Preprocessor.reset(new tools::gcc::Preprocessor(*this));
    return Preprocessor.get();
  case Action::CompileJobClass:
    if (!Compiler)
      Compiler.reset(new tools::gcc::Compiler(*this));
    return Compiler.get();
  default:
    return ToolChain::getTool(AC);
  }
}

Tool *toolchains::MinGW::buildAssembler() const {
  return new tools::MinGW::Assembler(*this);
}

Tool *toolchains::MinGW::buildLinker() const {
  return new tools::MinGW::Linker(*this);
}

bool toolchains::MinGW::HasNativeLLVMSupport() const {
  return NativeLLVMSupport;
}

ToolChain::UnwindTableLevel
toolchains::MinGW::getDefaultUnwindTableLevel(const ArgList &Args) const {
  Arg *ExceptionArg = Args.getLastArg(options::OPT_fsjlj_exceptions,
                                      options::OPT_fseh_exceptions,
                                      options::OPT_fdwarf_exceptions);
  if (ExceptionArg &&
      ExceptionArg->getOption().matches(options::OPT_fseh_exceptions))
    return UnwindTableLevel::Asynchronous;

  if (getArch() == toolchain::Triple::x86_64 || getArch() == toolchain::Triple::arm ||
      getArch() == toolchain::Triple::thumb || getArch() == toolchain::Triple::aarch64)
    return UnwindTableLevel::Asynchronous;
  return UnwindTableLevel::None;
}

bool toolchains::MinGW::isPICDefault() const {
  return getArch() == toolchain::Triple::x86_64 ||
         getArch() == toolchain::Triple::aarch64;
}

bool toolchains::MinGW::isPIEDefault(const toolchain::opt::ArgList &Args) const {
  return false;
}

bool toolchains::MinGW::isPICDefaultForced() const { return true; }

toolchain::ExceptionHandling
toolchains::MinGW::GetExceptionModel(const ArgList &Args) const {
  if (getArch() == toolchain::Triple::x86_64 || getArch() == toolchain::Triple::aarch64 ||
      getArch() == toolchain::Triple::arm || getArch() == toolchain::Triple::thumb)
    return toolchain::ExceptionHandling::WinEH;
  return toolchain::ExceptionHandling::DwarfCFI;
}

SanitizerMask toolchains::MinGW::getSupportedSanitizers() const {
  SanitizerMask Res = ToolChain::getSupportedSanitizers();
  Res |= SanitizerKind::Address;
  Res |= SanitizerKind::PointerCompare;
  Res |= SanitizerKind::PointerSubtract;
  Res |= SanitizerKind::Vptr;
  return Res;
}

void toolchains::MinGW::AddCudaIncludeArgs(const ArgList &DriverArgs,
                                           ArgStringList &CC1Args) const {
  CudaInstallation->AddCudaIncludeArgs(DriverArgs, CC1Args);
}

void toolchains::MinGW::AddHIPIncludeArgs(const ArgList &DriverArgs,
                                          ArgStringList &CC1Args) const {
  RocmInstallation->AddHIPIncludeArgs(DriverArgs, CC1Args);
}

void toolchains::MinGW::printVerboseInfo(raw_ostream &OS) const {
  CudaInstallation->print(OS);
  RocmInstallation->print(OS);
}

// Include directories for various hosts:

// Windows, mingw.org
// c:\mingw\lib\gcc\mingw32\4.8.1\include\c++
// c:\mingw\lib\gcc\mingw32\4.8.1\include\c++\mingw32
// c:\mingw\lib\gcc\mingw32\4.8.1\include\c++\backward
// c:\mingw\include
// c:\mingw\mingw32\include

// Windows, mingw-w64 mingw-builds
// c:\mingw32\i686-w64-mingw32\include
// c:\mingw32\i686-w64-mingw32\include\c++
// c:\mingw32\i686-w64-mingw32\include\c++\i686-w64-mingw32
// c:\mingw32\i686-w64-mingw32\include\c++\backward

// Windows, mingw-w64 msys2
// c:\msys64\mingw32\include
// c:\msys64\mingw32\i686-w64-mingw32\include
// c:\msys64\mingw32\include\c++\4.9.2
// c:\msys64\mingw32\include\c++\4.9.2\i686-w64-mingw32
// c:\msys64\mingw32\include\c++\4.9.2\backward

// openSUSE
// /usr/lib64/gcc/x86_64-w64-mingw32/5.1.0/include/c++
// /usr/lib64/gcc/x86_64-w64-mingw32/5.1.0/include/c++/x86_64-w64-mingw32
// /usr/lib64/gcc/x86_64-w64-mingw32/5.1.0/include/c++/backward
// /usr/x86_64-w64-mingw32/sys-root/mingw/include

// Arch Linux
// /usr/i686-w64-mingw32/include/c++/5.1.0
// /usr/i686-w64-mingw32/include/c++/5.1.0/i686-w64-mingw32
// /usr/i686-w64-mingw32/include/c++/5.1.0/backward
// /usr/i686-w64-mingw32/include

// Ubuntu
// /usr/include/c++/4.8
// /usr/include/c++/4.8/x86_64-w64-mingw32
// /usr/include/c++/4.8/backward
// /usr/x86_64-w64-mingw32/include

// Fedora
// /usr/x86_64-w64-mingw32ucrt/sys-root/mingw/include/c++/x86_64-w64-mingw32ucrt
// /usr/x86_64-w64-mingw32ucrt/sys-root/mingw/include/c++/backward
// /usr/x86_64-w64-mingw32ucrt/sys-root/mingw/include
// /usr/lib/gcc/x86_64-w64-mingw32ucrt/12.2.1/include-fixed

void toolchains::MinGW::AddClangSystemIncludeArgs(const ArgList &DriverArgs,
                                                  ArgStringList &CC1Args) const {
  if (DriverArgs.hasArg(options::OPT_nostdinc))
    return;

  if (!DriverArgs.hasArg(options::OPT_nobuiltininc)) {
    SmallString<1024> P(getDriver().ResourceDir);
    toolchain::sys::path::append(P, "include");
    addSystemInclude(DriverArgs, CC1Args, P.str());
  }

  if (DriverArgs.hasArg(options::OPT_nostdlibinc))
    return;

  addSystemInclude(DriverArgs, CC1Args,
                   Base + SubdirName + toolchain::sys::path::get_separator() +
                       "include");

  // Gentoo
  addSystemInclude(DriverArgs, CC1Args,
                   Base + SubdirName + toolchain::sys::path::get_separator() + "usr/include");

  // Only include <base>/include if we're not cross compiling (but do allow it
  // if we're on Windows and building for Windows on another architecture),
  // or if the sysroot has been set (where we presume the user has pointed it
  // at an arch specific subdirectory).
  if (!::isCrossCompiling(getTriple(), /*RequireArchMatch=*/false) ||
      getDriver().SysRoot.size())
    addSystemInclude(DriverArgs, CC1Args, Base + "include");
}

void toolchains::MinGW::addClangTargetOptions(
    const toolchain::opt::ArgList &DriverArgs, toolchain::opt::ArgStringList &CC1Args,
    Action::OffloadKind DeviceOffloadKind) const {
  if (Arg *A = DriverArgs.getLastArg(options::OPT_mguard_EQ)) {
    StringRef GuardArgs = A->getValue();
    if (GuardArgs == "none") {
      // Do nothing.
    } else if (GuardArgs == "cf") {
      // Emit CFG instrumentation and the table of address-taken functions.
      CC1Args.push_back("-cfguard");
    } else if (GuardArgs == "cf-nochecks") {
      // Emit only the table of address-taken functions.
      CC1Args.push_back("-cfguard-no-checks");
    } else {
      getDriver().Diag(diag::err_drv_unsupported_option_argument)
          << A->getSpelling() << GuardArgs;
    }
  }

  // Default to not enabling sized deallocation, but let user provided options
  // override it.
  //
  // If using sized deallocation, user code that invokes delete will end up
  // calling delete(void*,size_t). If the user wanted to override the
  // operator delete(void*), there may be a fallback operator
  // delete(void*,size_t) which calls the regular operator delete(void*).
  //
  // However, if the C++ standard library is linked in the form of a DLL,
  // and the fallback operator delete(void*,size_t) is within this DLL (which is
  // the case for libc++ at least) it will only redirect towards the library's
  // default operator delete(void*), not towards the user's provided operator
  // delete(void*).
  //
  // This issue can be avoided, if the fallback operators are linked statically
  // into the callers, even if the C++ standard library is linked as a DLL.
  //
  // This is meant as a temporary workaround until libc++ implements this
  // technique, which is tracked in
  // https://github.com/toolchain/toolchain-project/issues/96899.
  if (!DriverArgs.hasArgNoClaim(options::OPT_fsized_deallocation,
                                options::OPT_fno_sized_deallocation))
    CC1Args.push_back("-fno-sized-deallocation");

  CC1Args.push_back("-fno-use-init-array");

  for (auto Opt : {options::OPT_mthreads, options::OPT_mwindows,
                   options::OPT_mconsole, options::OPT_mdll}) {
    if (Arg *A = DriverArgs.getLastArgNoClaim(Opt))
      A->ignoreTargetSpecific();
  }
}

void toolchains::MinGW::AddClangCXXStdlibIncludeArgs(
    const ArgList &DriverArgs, ArgStringList &CC1Args) const {
  if (DriverArgs.hasArg(options::OPT_nostdinc, options::OPT_nostdlibinc,
                        options::OPT_nostdincxx))
    return;

  StringRef Slash = toolchain::sys::path::get_separator();

  switch (GetCXXStdlibType(DriverArgs)) {
  case ToolChain::CST_Libcxx: {
    std::string TargetDir = (Base + "include" + Slash + getTripleString() +
                             Slash + "c++" + Slash + "v1")
                                .str();
    if (getDriver().getVFS().exists(TargetDir))
      addSystemInclude(DriverArgs, CC1Args, TargetDir);
    addSystemInclude(DriverArgs, CC1Args,
                     Base + SubdirName + Slash + "include" + Slash + "c++" +
                         Slash + "v1");
    addSystemInclude(DriverArgs, CC1Args,
                     Base + "include" + Slash + "c++" + Slash + "v1");
    break;
  }

  case ToolChain::CST_Libstdcxx:
    toolchain::SmallVector<toolchain::SmallString<1024>, 7> CppIncludeBases;
    CppIncludeBases.emplace_back(Base);
    toolchain::sys::path::append(CppIncludeBases[0], SubdirName, "include", "c++");
    CppIncludeBases.emplace_back(Base);
    toolchain::sys::path::append(CppIncludeBases[1], SubdirName, "include", "c++",
                            Ver);
    CppIncludeBases.emplace_back(Base);
    toolchain::sys::path::append(CppIncludeBases[2], "include", "c++", Ver);
    CppIncludeBases.emplace_back(GccLibDir);
    toolchain::sys::path::append(CppIncludeBases[3], "include", "c++");
    CppIncludeBases.emplace_back(GccLibDir);
    toolchain::sys::path::append(CppIncludeBases[4], "include",
                            "g++-v" + GccVer.Text);
    CppIncludeBases.emplace_back(GccLibDir);
    toolchain::sys::path::append(CppIncludeBases[5], "include",
                            "g++-v" + GccVer.MajorStr + "." + GccVer.MinorStr);
    CppIncludeBases.emplace_back(GccLibDir);
    toolchain::sys::path::append(CppIncludeBases[6], "include",
                            "g++-v" + GccVer.MajorStr);
    for (auto &CppIncludeBase : CppIncludeBases) {
      addSystemInclude(DriverArgs, CC1Args, CppIncludeBase);
      CppIncludeBase += Slash;
      addSystemInclude(DriverArgs, CC1Args, CppIncludeBase + TripleDirName);
      addSystemInclude(DriverArgs, CC1Args, CppIncludeBase + "backward");
    }
    break;
  }
}

static bool testTriple(const Driver &D, const toolchain::Triple &Triple,
                       const ArgList &Args) {
  // If an explicit sysroot is set, that will be used and we shouldn't try to
  // detect anything else.
  std::string SubdirName;
  if (D.SysRoot.size())
    return true;
  toolchain::Triple LiteralTriple = getLiteralTriple(D, Triple);
  std::string InstallBase = std::string(toolchain::sys::path::parent_path(D.Dir));
  if (toolchain::ErrorOr<std::string> TargetSubdir =
          findClangRelativeSysroot(D, LiteralTriple, Triple, SubdirName))
    return true;
  // If the install base itself looks like a mingw sysroot, we'll use that
  // - don't use any potentially unrelated gcc to influence what triple to use.
  if (looksLikeMinGWSysroot(InstallBase))
    return false;
  if (toolchain::ErrorOr<std::string> GPPName = findGcc(LiteralTriple, Triple))
    return true;
  // If we neither found a colocated sysroot or a matching gcc executable,
  // conclude that we can't know if this is the correct spelling of the triple.
  return false;
}

static toolchain::Triple adjustTriple(const Driver &D, const toolchain::Triple &Triple,
                                 const ArgList &Args) {
  // First test if the original triple can find a sysroot with the triple
  // name.
  if (testTriple(D, Triple, Args))
    return Triple;
  toolchain::SmallVector<toolchain::StringRef, 3> Archs;
  // If not, test a couple other possible arch names that might be what was
  // intended.
  if (Triple.getArch() == toolchain::Triple::x86) {
    Archs.emplace_back("i386");
    Archs.emplace_back("i586");
    Archs.emplace_back("i686");
  } else if (Triple.getArch() == toolchain::Triple::arm ||
             Triple.getArch() == toolchain::Triple::thumb) {
    Archs.emplace_back("armv7");
  }
  for (auto A : Archs) {
    toolchain::Triple TestTriple(Triple);
    TestTriple.setArchName(A);
    if (testTriple(D, TestTriple, Args))
      return TestTriple;
  }
  // If none was found, just proceed with the original value.
  return Triple;
}

void toolchains::MinGW::fixTripleArch(const Driver &D, toolchain::Triple &Triple,
                                      const ArgList &Args) {
  if (Triple.getArch() == toolchain::Triple::x86 ||
      Triple.getArch() == toolchain::Triple::arm ||
      Triple.getArch() == toolchain::Triple::thumb)
    Triple = adjustTriple(D, Triple, Args);
}
