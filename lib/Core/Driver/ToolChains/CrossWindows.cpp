//===-- CrossWindows.cpp - Cross Windows Tool Chain -----------------------===//
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

#include "CrossWindows.h"
#include "language/Core/Driver/CommonArgs.h"
#include "language/Core/Driver/Compilation.h"
#include "language/Core/Driver/Driver.h"
#include "language/Core/Driver/Options.h"
#include "language/Core/Driver/SanitizerArgs.h"
#include "toolchain/Option/ArgList.h"
#include "toolchain/Support/Path.h"

using namespace language::Core::driver;
using namespace language::Core::driver::toolchains;

using toolchain::opt::ArgList;
using toolchain::opt::ArgStringList;

void tools::CrossWindows::Assembler::ConstructJob(
    Compilation &C, const JobAction &JA, const InputInfo &Output,
    const InputInfoList &Inputs, const ArgList &Args,
    const char *LinkingOutput) const {
  claimNoWarnArgs(Args);
  const auto &TC =
      static_cast<const toolchains::CrossWindowsToolChain &>(getToolChain());
  ArgStringList CmdArgs;
  const char *Exec;

  switch (TC.getArch()) {
  default:
    toolchain_unreachable("unsupported architecture");
  case toolchain::Triple::arm:
  case toolchain::Triple::thumb:
  case toolchain::Triple::aarch64:
    break;
  case toolchain::Triple::x86:
    CmdArgs.push_back("--32");
    break;
  case toolchain::Triple::x86_64:
    CmdArgs.push_back("--64");
    break;
  }

  Args.AddAllArgValues(CmdArgs, options::OPT_Wa_COMMA, options::OPT_Xassembler);

  CmdArgs.push_back("-o");
  CmdArgs.push_back(Output.getFilename());

  for (const auto &Input : Inputs)
    CmdArgs.push_back(Input.getFilename());

  const std::string Assembler = TC.GetProgramPath("as");
  Exec = Args.MakeArgString(Assembler);

  C.addCommand(std::make_unique<Command>(JA, *this, ResponseFileSupport::None(),
                                         Exec, CmdArgs, Inputs, Output));
}

void tools::CrossWindows::Linker::ConstructJob(
    Compilation &C, const JobAction &JA, const InputInfo &Output,
    const InputInfoList &Inputs, const ArgList &Args,
    const char *LinkingOutput) const {
  const auto &TC =
      static_cast<const toolchains::CrossWindowsToolChain &>(getToolChain());
  const toolchain::Triple &T = TC.getTriple();
  const Driver &D = TC.getDriver();
  SmallString<128> EntryPoint;
  ArgStringList CmdArgs;
  const char *Exec;

  // Silence warning for "clang -g foo.o -o foo"
  Args.ClaimAllArgs(options::OPT_g_Group);
  // and "clang -emit-toolchain foo.o -o foo"
  Args.ClaimAllArgs(options::OPT_emit_toolchain);
  // and for "clang -w foo.o -o foo"
  Args.ClaimAllArgs(options::OPT_w);
  // Other warning options are already handled somewhere else.

  if (!D.SysRoot.empty())
    CmdArgs.push_back(Args.MakeArgString("--sysroot=" + D.SysRoot));

  if (Args.hasArg(options::OPT_pie))
    CmdArgs.push_back("-pie");
  if (Args.hasArg(options::OPT_rdynamic))
    CmdArgs.push_back("-export-dynamic");
  if (Args.hasArg(options::OPT_s))
    CmdArgs.push_back("--strip-all");

  CmdArgs.push_back("-m");
  switch (TC.getArch()) {
  default:
    D.Diag(diag::err_target_unknown_triple) << TC.getEffectiveTriple().str();
    break;
  case toolchain::Triple::arm:
  case toolchain::Triple::thumb:
    // FIXME: this is incorrect for WinCE
    CmdArgs.push_back("thumb2pe");
    break;
  case toolchain::Triple::aarch64:
    CmdArgs.push_back("arm64pe");
    break;
  case toolchain::Triple::x86:
    CmdArgs.push_back("i386pe");
    EntryPoint.append("_");
    break;
  case toolchain::Triple::x86_64:
    CmdArgs.push_back("i386pep");
    break;
  }

  if (Args.hasArg(options::OPT_shared)) {
    switch (T.getArch()) {
    default:
      toolchain_unreachable("unsupported architecture");
    case toolchain::Triple::aarch64:
    case toolchain::Triple::arm:
    case toolchain::Triple::thumb:
    case toolchain::Triple::x86_64:
      EntryPoint.append("_DllMainCRTStartup");
      break;
    case toolchain::Triple::x86:
      EntryPoint.append("_DllMainCRTStartup@12");
      break;
    }

    CmdArgs.push_back("-shared");
    CmdArgs.push_back(Args.hasArg(options::OPT_static) ? "-Bstatic"
                                                       : "-Bdynamic");

    CmdArgs.push_back("--enable-auto-image-base");

    CmdArgs.push_back("--entry");
    CmdArgs.push_back(Args.MakeArgString(EntryPoint));
  } else {
    EntryPoint.append("mainCRTStartup");

    CmdArgs.push_back(Args.hasArg(options::OPT_static) ? "-Bstatic"
                                                       : "-Bdynamic");

    if (!Args.hasArg(options::OPT_nostdlib, options::OPT_nostartfiles)) {
      CmdArgs.push_back("--entry");
      CmdArgs.push_back(Args.MakeArgString(EntryPoint));
    }

    // FIXME: handle subsystem
  }

  // NOTE: deal with multiple definitions on Windows (e.g. COMDAT)
  CmdArgs.push_back("--allow-multiple-definition");

  CmdArgs.push_back("-o");
  CmdArgs.push_back(Output.getFilename());

  if (Args.hasArg(options::OPT_shared) || Args.hasArg(options::OPT_rdynamic)) {
    SmallString<261> ImpLib(Output.getFilename());
    toolchain::sys::path::replace_extension(ImpLib, ".lib");

    CmdArgs.push_back("--out-implib");
    CmdArgs.push_back(Args.MakeArgString(ImpLib));
  }

  Args.AddAllArgs(CmdArgs, options::OPT_L);
  TC.AddFilePathLibArgs(Args, CmdArgs);
  AddLinkerInputs(TC, Inputs, Args, CmdArgs, JA);

  if (TC.ShouldLinkCXXStdlib(Args)) {
    bool StaticCXX = Args.hasArg(options::OPT_static_libstdcxx) &&
                     !Args.hasArg(options::OPT_static);
    if (StaticCXX)
      CmdArgs.push_back("-Bstatic");
    TC.AddCXXStdlibLibArgs(Args, CmdArgs);
    if (StaticCXX)
      CmdArgs.push_back("-Bdynamic");
  }

  if (!Args.hasArg(options::OPT_nostdlib)) {
    if (!Args.hasArg(options::OPT_nodefaultlibs)) {
      // TODO handle /MT[d] /MD[d]
      CmdArgs.push_back("-lmsvcrt");
      AddRunTimeLibs(TC, D, CmdArgs, Args);
    }
  }

  if (TC.getSanitizerArgs(Args).needsAsanRt()) {
    // TODO handle /MT[d] /MD[d]
    if (Args.hasArg(options::OPT_shared)) {
      CmdArgs.push_back(TC.getCompilerRTArgString(Args, "asan_dll_thunk"));
    } else {
      for (const auto &Lib : {"asan_dynamic", "asan_dynamic_runtime_thunk"})
        CmdArgs.push_back(TC.getCompilerRTArgString(Args, Lib));
      // Make sure the dynamic runtime thunk is not optimized out at link time
      // to ensure proper SEH handling.
      CmdArgs.push_back(Args.MakeArgString("--undefined"));
      CmdArgs.push_back(Args.MakeArgString(TC.getArch() == toolchain::Triple::x86
                                               ? "___asan_seh_interceptor"
                                               : "__asan_seh_interceptor"));
    }
  }

  Exec = Args.MakeArgString(TC.GetLinkerPath());

  C.addCommand(std::make_unique<Command>(JA, *this,
                                         ResponseFileSupport::AtFileUTF8(),
                                         Exec, CmdArgs, Inputs, Output));
}

CrossWindowsToolChain::CrossWindowsToolChain(const Driver &D,
                                             const toolchain::Triple &T,
                                             const toolchain::opt::ArgList &Args)
    : Generic_GCC(D, T, Args) {}

ToolChain::UnwindTableLevel
CrossWindowsToolChain::getDefaultUnwindTableLevel(const ArgList &Args) const {
  // FIXME: all non-x86 targets need unwind tables, however, LLVM currently does
  // not know how to emit them.
  return getArch() == toolchain::Triple::x86_64 ? UnwindTableLevel::Asynchronous : UnwindTableLevel::None;
}

bool CrossWindowsToolChain::isPICDefault() const {
  return getArch() == toolchain::Triple::x86_64;
}

bool CrossWindowsToolChain::isPIEDefault(const toolchain::opt::ArgList &Args) const {
  return getArch() == toolchain::Triple::x86_64;
}

bool CrossWindowsToolChain::isPICDefaultForced() const {
  return getArch() == toolchain::Triple::x86_64;
}

void CrossWindowsToolChain::
AddClangSystemIncludeArgs(const toolchain::opt::ArgList &DriverArgs,
                          toolchain::opt::ArgStringList &CC1Args) const {
  const Driver &D = getDriver();
  const std::string &SysRoot = D.SysRoot;

  auto AddSystemAfterIncludes = [&]() {
    for (const auto &P : DriverArgs.getAllArgValues(options::OPT_isystem_after))
      addSystemInclude(DriverArgs, CC1Args, P);
  };

  if (DriverArgs.hasArg(options::OPT_nostdinc)) {
    AddSystemAfterIncludes();
    return;
  }

  addSystemInclude(DriverArgs, CC1Args, SysRoot + "/usr/local/include");
  if (!DriverArgs.hasArg(options::OPT_nobuiltininc)) {
    SmallString<128> ResourceDir(D.ResourceDir);
    toolchain::sys::path::append(ResourceDir, "include");
    addSystemInclude(DriverArgs, CC1Args, ResourceDir);
  }
  AddSystemAfterIncludes();
  addExternCSystemInclude(DriverArgs, CC1Args, SysRoot + "/usr/include");
}

void CrossWindowsToolChain::
AddClangCXXStdlibIncludeArgs(const toolchain::opt::ArgList &DriverArgs,
                             toolchain::opt::ArgStringList &CC1Args) const {
  const std::string &SysRoot = getDriver().SysRoot;

  if (DriverArgs.hasArg(options::OPT_nostdinc) ||
      DriverArgs.hasArg(options::OPT_nostdincxx))
    return;

  if (GetCXXStdlibType(DriverArgs) == ToolChain::CST_Libcxx)
    addSystemInclude(DriverArgs, CC1Args, SysRoot + "/usr/include/c++/v1");
}

void CrossWindowsToolChain::
AddCXXStdlibLibArgs(const toolchain::opt::ArgList &Args,
                    toolchain::opt::ArgStringList &CmdArgs) const {
  if (GetCXXStdlibType(Args) == ToolChain::CST_Libcxx) {
    CmdArgs.push_back("-lc++");
    if (Args.hasArg(options::OPT_fexperimental_library))
      CmdArgs.push_back("-lc++experimental");
  }
}

language::Core::SanitizerMask CrossWindowsToolChain::getSupportedSanitizers() const {
  SanitizerMask Res = ToolChain::getSupportedSanitizers();
  Res |= SanitizerKind::Address;
  Res |= SanitizerKind::PointerCompare;
  Res |= SanitizerKind::PointerSubtract;
  return Res;
}

Tool *CrossWindowsToolChain::buildLinker() const {
  return new tools::CrossWindows::Linker(*this);
}

Tool *CrossWindowsToolChain::buildAssembler() const {
  return new tools::CrossWindows::Assembler(*this);
}
