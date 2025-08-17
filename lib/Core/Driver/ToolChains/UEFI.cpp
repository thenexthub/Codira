//===--- UEFI.cpp - UEFI ToolChain Implementations -----------------------===//
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

#include "UEFI.h"
#include "language/Core/Config/config.h"
#include "language/Core/Driver/CommonArgs.h"
#include "language/Core/Driver/Compilation.h"
#include "language/Core/Driver/Driver.h"
#include "language/Core/Driver/Options.h"
#include "language/Core/Driver/SanitizerArgs.h"
#include "toolchain/Option/Arg.h"
#include "toolchain/Option/ArgList.h"
#include "toolchain/Support/VirtualFileSystem.h"
#include "toolchain/TargetParser/Host.h"

using namespace language::Core::driver;
using namespace language::Core::driver::toolchains;
using namespace language::Core;
using namespace toolchain::opt;

UEFI::UEFI(const Driver &D, const toolchain::Triple &Triple, const ArgList &Args)
    : ToolChain(D, Triple, Args) {}

Tool *UEFI::buildLinker() const { return new tools::uefi::Linker(*this); }

void UEFI::AddClangSystemIncludeArgs(const ArgList &DriverArgs,
                                     ArgStringList &CC1Args) const {
  if (DriverArgs.hasArg(options::OPT_nostdinc))
    return;

  if (!DriverArgs.hasArg(options::OPT_nobuiltininc)) {
    SmallString<128> Dir(getDriver().ResourceDir);
    toolchain::sys::path::append(Dir, "include");
    addSystemInclude(DriverArgs, CC1Args, Dir.str());
  }

  if (DriverArgs.hasArg(options::OPT_nostdlibinc))
    return;

  if (std::optional<std::string> Path = getStdlibIncludePath())
    addSystemInclude(DriverArgs, CC1Args, *Path);
}

void tools::uefi::Linker::ConstructJob(Compilation &C, const JobAction &JA,
                                       const InputInfo &Output,
                                       const InputInfoList &Inputs,
                                       const ArgList &Args,
                                       const char *LinkingOutput) const {
  ArgStringList CmdArgs;
  auto &TC = static_cast<const toolchains::UEFI &>(getToolChain());

  assert((Output.isFilename() || Output.isNothing()) && "invalid output");
  if (Output.isFilename())
    CmdArgs.push_back(
        Args.MakeArgString(std::string("-out:") + Output.getFilename()));

  CmdArgs.push_back("-nologo");

  // TODO: Other UEFI binary subsystems that are currently unsupported:
  // efi_boot_service_driver, efi_rom, efi_runtime_driver.
  CmdArgs.push_back("-subsystem:efi_application");

  // Default entry function name according to the TianoCore reference
  // implementation is EfiMain.
  // TODO: Provide a flag to override the entry function name.
  CmdArgs.push_back("-entry:EfiMain");

  // "Terminal Service Aware" flag is not needed for UEFI applications.
  CmdArgs.push_back("-tsaware:no");

  if (Args.hasArg(options::OPT_g_Group, options::OPT__SLASH_Z7))
    CmdArgs.push_back("-debug");

  Args.AddAllArgValues(CmdArgs, options::OPT__SLASH_link);

  AddLinkerInputs(TC, Inputs, Args, CmdArgs, JA);

  // This should ideally be handled by ToolChain::GetLinkerPath but we need
  // to special case some linker paths. In the case of lld, we need to
  // translate 'lld' into 'lld-link'.
  StringRef Linker = Args.getLastArgValue(options::OPT_fuse_ld_EQ,
                                          TC.getDriver().getPreferredLinker());
  if (Linker.empty() || Linker == "lld")
    Linker = "lld-link";

  auto LinkerPath = TC.GetProgramPath(Linker.str().c_str());
  auto LinkCmd = std::make_unique<Command>(
      JA, *this, ResponseFileSupport::AtFileUTF16(),
      Args.MakeArgString(LinkerPath), CmdArgs, Inputs, Output);
  C.addCommand(std::move(LinkCmd));
}
