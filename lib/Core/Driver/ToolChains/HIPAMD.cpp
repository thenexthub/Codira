//===--- HIPAMD.cpp - HIP Tool and ToolChain Implementations ----*- C++ -*-===//
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

#include "HIPAMD.h"
#include "AMDGPU.h"
#include "HIPUtility.h"
#include "SPIRV.h"
#include "language/Core/Basic/Cuda.h"
#include "language/Core/Driver/CommonArgs.h"
#include "language/Core/Driver/Compilation.h"
#include "language/Core/Driver/Driver.h"
#include "language/Core/Driver/InputInfo.h"
#include "language/Core/Driver/Options.h"
#include "language/Core/Driver/SanitizerArgs.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/Path.h"
#include "toolchain/TargetParser/TargetParser.h"

using namespace language::Core::driver;
using namespace language::Core::driver::toolchains;
using namespace language::Core::driver::tools;
using namespace language::Core;
using namespace toolchain::opt;

#if defined(_WIN32) || defined(_WIN64)
#define NULL_FILE "nul"
#else
#define NULL_FILE "/dev/null"
#endif

void AMDGCN::Linker::constructLlvmLinkCommand(Compilation &C,
                                         const JobAction &JA,
                                         const InputInfoList &Inputs,
                                         const InputInfo &Output,
                                         const toolchain::opt::ArgList &Args) const {
  // Construct toolchain-link command.
  // The output from toolchain-link is a bitcode file.
  ArgStringList LlvmLinkArgs;

  assert(!Inputs.empty() && "Must have at least one input.");

  LlvmLinkArgs.append({"-o", Output.getFilename()});
  for (auto Input : Inputs)
    LlvmLinkArgs.push_back(Input.getFilename());

  // Look for archive of bundled bitcode in arguments, and add temporary files
  // for the extracted archive of bitcode to inputs.
  auto TargetID = Args.getLastArgValue(options::OPT_mcpu_EQ);
  AddStaticDeviceLibsLinking(C, *this, JA, Inputs, Args, LlvmLinkArgs, "amdgcn",
                             TargetID, /*IsBitCodeSDL=*/true);

  const char *LlvmLink =
    Args.MakeArgString(getToolChain().GetProgramPath("toolchain-link"));
  C.addCommand(std::make_unique<Command>(JA, *this, ResponseFileSupport::None(),
                                         LlvmLink, LlvmLinkArgs, Inputs,
                                         Output));
}

void AMDGCN::Linker::constructLldCommand(Compilation &C, const JobAction &JA,
                                         const InputInfoList &Inputs,
                                         const InputInfo &Output,
                                         const toolchain::opt::ArgList &Args) const {
  // Construct lld command.
  // The output from ld.lld is an HSA code object file.
  ArgStringList LldArgs{"-flavor",
                        "gnu",
                        "-m",
                        "elf64_amdgpu",
                        "--no-undefined",
                        "-shared",
                        "-plugin-opt=-amdgpu-internalize-symbols"};
  if (Args.hasArg(options::OPT_hipstdpar))
    LldArgs.push_back("-plugin-opt=-amdgpu-enable-hipstdpar");

  auto &TC = getToolChain();
  auto &D = TC.getDriver();
  bool IsThinLTO = D.getOffloadLTOMode() == LTOK_Thin;
  addLTOOptions(TC, Args, LldArgs, Output, Inputs, IsThinLTO);

  // Extract all the -m options
  std::vector<toolchain::StringRef> Features;
  amdgpu::getAMDGPUTargetFeatures(D, TC.getTriple(), Args, Features);

  // Add features to mattr such as cumode
  std::string MAttrString = "-plugin-opt=-mattr=";
  for (auto OneFeature : unifyTargetFeatures(Features)) {
    MAttrString.append(Args.MakeArgString(OneFeature));
    if (OneFeature != Features.back())
      MAttrString.append(",");
  }
  if (!Features.empty())
    LldArgs.push_back(Args.MakeArgString(MAttrString));

  // ToDo: Remove this option after AMDGPU backend supports ISA-level linking.
  // Since AMDGPU backend currently does not support ISA-level linking, all
  // called functions need to be imported.
  if (IsThinLTO) {
    LldArgs.push_back(Args.MakeArgString("-plugin-opt=-force-import-all"));
    LldArgs.push_back(Args.MakeArgString("-plugin-opt=-avail-extern-to-local"));
    LldArgs.push_back(Args.MakeArgString(
        "-plugin-opt=-avail-extern-gv-in-addrspace-to-local=3"));
  }

  for (const Arg *A : Args.filtered(options::OPT_mtoolchain)) {
    LldArgs.push_back(
        Args.MakeArgString(Twine("-plugin-opt=") + A->getValue(0)));
  }

  if (C.getDriver().isSaveTempsEnabled())
    LldArgs.push_back("-save-temps");

  addLinkerCompressDebugSectionsOption(TC, Args, LldArgs);

  // Given that host and device linking happen in separate processes, the device
  // linker doesn't always have the visibility as to which device symbols are
  // needed by a program, especially for the device symbol dependencies that are
  // introduced through the host symbol resolution.
  // For example: host_A() (A.obj) --> host_B(B.obj) --> device_kernel_B()
  // (B.obj) In this case, the device linker doesn't know that A.obj actually
  // depends on the kernel functions in B.obj.  When linking to static device
  // library, the device linker may drop some of the device global symbols if
  // they aren't referenced.  As a workaround, we are adding to the
  // --whole-archive flag such that all global symbols would be linked in.
  LldArgs.push_back("--whole-archive");

  for (auto *Arg : Args.filtered(options::OPT_Xoffload_linker)) {
    StringRef ArgVal = Arg->getValue(1);
    auto SplitArg = ArgVal.split("-mtoolchain=");
    if (!SplitArg.second.empty()) {
      LldArgs.push_back(
          Args.MakeArgString(Twine("-plugin-opt=") + SplitArg.second));
    } else {
      LldArgs.push_back(Args.MakeArgString(ArgVal));
    }
    Arg->claim();
  }

  LldArgs.append({"-o", Output.getFilename()});
  for (auto Input : Inputs)
    LldArgs.push_back(Input.getFilename());

  // Look for archive of bundled bitcode in arguments, and add temporary files
  // for the extracted archive of bitcode to inputs.
  auto TargetID = Args.getLastArgValue(options::OPT_mcpu_EQ);
  AddStaticDeviceLibsLinking(C, *this, JA, Inputs, Args, LldArgs, "amdgcn",
                             TargetID, /*IsBitCodeSDL=*/true);

  LldArgs.push_back("--no-whole-archive");

  const char *Lld = Args.MakeArgString(getToolChain().GetProgramPath("lld"));
  C.addCommand(std::make_unique<Command>(JA, *this, ResponseFileSupport::None(),
                                         Lld, LldArgs, Inputs, Output));
}

// For SPIR-V the inputs for the job are device AMDGCN SPIR-V flavoured bitcode
// and the output is either a compiled SPIR-V binary or bitcode (-emit-toolchain). It
// calls toolchain-link and then the toolchain-spirv translator. Once the SPIR-V BE will
// be promoted from experimental, we will switch to using that. TODO: consider
// if we want to run any targeted optimisations over IR here, over generic
// SPIR-V.
void AMDGCN::Linker::constructLinkAndEmitSpirvCommand(
    Compilation &C, const JobAction &JA, const InputInfoList &Inputs,
    const InputInfo &Output, const toolchain::opt::ArgList &Args) const {
  assert(!Inputs.empty() && "Must have at least one input.");

  constructLlvmLinkCommand(C, JA, Inputs, Output, Args);

  // Linked BC is now in Output

  // Emit SPIR-V binary.
  toolchain::opt::ArgStringList TrArgs{
      "--spirv-max-version=1.6",
      "--spirv-ext=+all",
      "--spirv-allow-unknown-intrinsics",
      "--spirv-lower-const-expr",
      "--spirv-preserve-auxdata",
      "--spirv-debug-info-version=nonsemantic-shader-200"};
  SPIRV::constructTranslateCommand(C, *this, JA, Output, Output, TrArgs);
}

// For amdgcn the inputs of the linker job are device bitcode and output is
// either an object file or bitcode (-emit-toolchain). It calls toolchain-link, opt,
// llc, then lld steps.
void AMDGCN::Linker::ConstructJob(Compilation &C, const JobAction &JA,
                                  const InputInfo &Output,
                                  const InputInfoList &Inputs,
                                  const ArgList &Args,
                                  const char *LinkingOutput) const {
  if (Inputs.size() > 0 &&
      Inputs[0].getType() == types::TY_Image &&
      JA.getType() == types::TY_Object)
    return HIP::constructGenerateObjFileFromHIPFatBinary(C, Output, Inputs,
                                                         Args, JA, *this);

  if (JA.getType() == types::TY_HIP_FATBIN)
    return HIP::constructHIPFatbinCommand(C, JA, Output.getFilename(), Inputs,
                                          Args, *this);

  if (JA.getType() == types::TY_LLVM_BC)
    return constructLlvmLinkCommand(C, JA, Inputs, Output, Args);

  if (getToolChain().getEffectiveTriple().isSPIRV())
    return constructLinkAndEmitSpirvCommand(C, JA, Inputs, Output, Args);

  return constructLldCommand(C, JA, Inputs, Output, Args);
}

HIPAMDToolChain::HIPAMDToolChain(const Driver &D, const toolchain::Triple &Triple,
                                 const ToolChain &HostTC, const ArgList &Args)
    : ROCMToolChain(D, Triple, Args), HostTC(HostTC) {
  // Lookup binaries into the driver directory, this is used to
  // discover the clang-offload-bundler executable.
  getProgramPaths().push_back(getDriver().Dir);
  // Diagnose unsupported sanitizer options only once.
  diagnoseUnsupportedSanitizers(Args);
}

void HIPAMDToolChain::addClangTargetOptions(
    const toolchain::opt::ArgList &DriverArgs, toolchain::opt::ArgStringList &CC1Args,
    Action::OffloadKind DeviceOffloadingKind) const {
  HostTC.addClangTargetOptions(DriverArgs, CC1Args, DeviceOffloadingKind);

  assert(DeviceOffloadingKind == Action::OFK_HIP &&
         "Only HIP offloading kinds are supported for GPUs.");

  CC1Args.append({"-fcuda-is-device", "-fno-threadsafe-statics"});

  if (!DriverArgs.hasFlag(options::OPT_fgpu_rdc, options::OPT_fno_gpu_rdc,
                          false)) {
    CC1Args.append({"-mtoolchain", "-amdgpu-internalize-symbols"});
    if (DriverArgs.hasArgNoClaim(options::OPT_hipstdpar))
      CC1Args.append({"-mtoolchain", "-amdgpu-enable-hipstdpar"});
  }

  StringRef MaxThreadsPerBlock =
      DriverArgs.getLastArgValue(options::OPT_gpu_max_threads_per_block_EQ);
  if (!MaxThreadsPerBlock.empty()) {
    std::string ArgStr =
        (Twine("--gpu-max-threads-per-block=") + MaxThreadsPerBlock).str();
    CC1Args.push_back(DriverArgs.MakeArgStringRef(ArgStr));
  }

  CC1Args.push_back("-fcuda-allow-variadic-functions");

  // Default to "hidden" visibility, as object level linking will not be
  // supported for the foreseeable future.
  if (!DriverArgs.hasArg(options::OPT_fvisibility_EQ,
                         options::OPT_fvisibility_ms_compat)) {
    CC1Args.append({"-fvisibility=hidden"});
    CC1Args.push_back("-fapply-global-visibility-to-externs");
  }

  if (getEffectiveTriple().isSPIRV()) {
    // For SPIR-V we embed the command-line into the generated binary, in order
    // to retrieve it at JIT time and be able to do target specific compilation
    // with options that match the user-supplied ones.
    if (!DriverArgs.hasArg(options::OPT_fembed_bitcode_marker))
      CC1Args.push_back("-fembed-bitcode=marker");
    return; // No DeviceLibs for SPIR-V.
  }

  for (auto BCFile : getDeviceLibs(DriverArgs, DeviceOffloadingKind)) {
    CC1Args.push_back(BCFile.ShouldInternalize ? "-mlink-builtin-bitcode"
                                               : "-mlink-bitcode-file");
    CC1Args.push_back(DriverArgs.MakeArgString(BCFile.Path));
  }
}

toolchain::opt::DerivedArgList *
HIPAMDToolChain::TranslateArgs(const toolchain::opt::DerivedArgList &Args,
                               StringRef BoundArch,
                               Action::OffloadKind DeviceOffloadKind) const {
  DerivedArgList *DAL =
      HostTC.TranslateArgs(Args, BoundArch, DeviceOffloadKind);
  if (!DAL)
    DAL = new DerivedArgList(Args.getBaseArgs());

  const OptTable &Opts = getDriver().getOpts();

  for (Arg *A : Args) {
    if (!shouldSkipSanitizeOption(*this, Args, BoundArch, A))
      DAL->append(A);
  }

  if (!BoundArch.empty()) {
    DAL->eraseArg(options::OPT_mcpu_EQ);
    DAL->AddJoinedArg(nullptr, Opts.getOption(options::OPT_mcpu_EQ), BoundArch);
    checkTargetID(*DAL);
  }

  if (!Args.hasArg(options::OPT_flto_partitions_EQ))
    DAL->AddJoinedArg(nullptr, Opts.getOption(options::OPT_flto_partitions_EQ),
                      "8");

  return DAL;
}

Tool *HIPAMDToolChain::buildLinker() const {
  assert(getTriple().isAMDGCN() ||
         getTriple().getArch() == toolchain::Triple::spirv64);
  return new tools::AMDGCN::Linker(*this);
}

void HIPAMDToolChain::addClangWarningOptions(ArgStringList &CC1Args) const {
  AMDGPUToolChain::addClangWarningOptions(CC1Args);
  HostTC.addClangWarningOptions(CC1Args);
}

ToolChain::CXXStdlibType
HIPAMDToolChain::GetCXXStdlibType(const ArgList &Args) const {
  return HostTC.GetCXXStdlibType(Args);
}

void HIPAMDToolChain::AddClangSystemIncludeArgs(const ArgList &DriverArgs,
                                                ArgStringList &CC1Args) const {
  HostTC.AddClangSystemIncludeArgs(DriverArgs, CC1Args);
}

void HIPAMDToolChain::AddClangCXXStdlibIncludeArgs(
    const ArgList &Args, ArgStringList &CC1Args) const {
  HostTC.AddClangCXXStdlibIncludeArgs(Args, CC1Args);
}

void HIPAMDToolChain::AddIAMCUIncludeArgs(const ArgList &Args,
                                          ArgStringList &CC1Args) const {
  HostTC.AddIAMCUIncludeArgs(Args, CC1Args);
}

void HIPAMDToolChain::AddHIPIncludeArgs(const ArgList &DriverArgs,
                                        ArgStringList &CC1Args) const {
  RocmInstallation->AddHIPIncludeArgs(DriverArgs, CC1Args);
}

SanitizerMask HIPAMDToolChain::getSupportedSanitizers() const {
  // The HIPAMDToolChain only supports sanitizers in the sense that it allows
  // sanitizer arguments on the command line if they are supported by the host
  // toolchain. The HIPAMDToolChain will actually ignore any command line
  // arguments for any of these "supported" sanitizers. That means that no
  // sanitization of device code is actually supported at this time.
  //
  // This behavior is necessary because the host and device toolchains
  // invocations often share the command line, so the device toolchain must
  // tolerate flags meant only for the host toolchain.
  return HostTC.getSupportedSanitizers();
}

VersionTuple HIPAMDToolChain::computeMSVCVersion(const Driver *D,
                                                 const ArgList &Args) const {
  return HostTC.computeMSVCVersion(D, Args);
}

toolchain::SmallVector<ToolChain::BitCodeLibraryInfo, 12>
HIPAMDToolChain::getDeviceLibs(const toolchain::opt::ArgList &DriverArgs,
                               Action::OffloadKind DeviceOffloadingKind) const {
  toolchain::SmallVector<BitCodeLibraryInfo, 12> BCLibs;
  if (!DriverArgs.hasFlag(options::OPT_offloadlib, options::OPT_no_offloadlib,
                          true) ||
      getGPUArch(DriverArgs) == "amdgcnspirv")
    return {};
  ArgStringList LibraryPaths;

  // Find in --hip-device-lib-path and HIP_LIBRARY_PATH.
  for (StringRef Path : RocmInstallation->getRocmDeviceLibPathArg())
    LibraryPaths.push_back(DriverArgs.MakeArgString(Path));

  addDirectoryList(DriverArgs, LibraryPaths, "", "HIP_DEVICE_LIB_PATH");

  // Maintain compatability with --hip-device-lib.
  auto BCLibArgs = DriverArgs.getAllArgValues(options::OPT_hip_device_lib_EQ);
  if (!BCLibArgs.empty()) {
    for (StringRef BCName : BCLibArgs) {
      StringRef FullName;
      bool Found = false;
      for (StringRef LibraryPath : LibraryPaths) {
        SmallString<128> Path(LibraryPath);
        toolchain::sys::path::append(Path, BCName);
        FullName = Path;
        if (toolchain::sys::fs::exists(FullName)) {
          BCLibs.emplace_back(FullName);
          Found = true;
          break;
        }
      }
      if (!Found)
        getDriver().Diag(diag::err_drv_no_such_file) << BCName;
    }
  } else {
    if (!RocmInstallation->hasDeviceLibrary()) {
      getDriver().Diag(diag::err_drv_no_rocm_device_lib) << 0;
      return {};
    }
    StringRef GpuArch = getGPUArch(DriverArgs);
    assert(!GpuArch.empty() && "Must have an explicit GPU arch.");

    // Add common device libraries like ocml etc.
    for (auto N : getCommonDeviceLibNames(DriverArgs, GpuArch.str(),
                                          DeviceOffloadingKind))
      BCLibs.emplace_back(N);

    // Add instrument lib.
    auto InstLib =
        DriverArgs.getLastArgValue(options::OPT_gpu_instrument_lib_EQ);
    if (InstLib.empty())
      return BCLibs;
    if (toolchain::sys::fs::exists(InstLib))
      BCLibs.emplace_back(InstLib);
    else
      getDriver().Diag(diag::err_drv_no_such_file) << InstLib;
  }

  return BCLibs;
}

void HIPAMDToolChain::checkTargetID(
    const toolchain::opt::ArgList &DriverArgs) const {
  auto PTID = getParsedTargetID(DriverArgs);
  if (PTID.OptionalTargetID && !PTID.OptionalGPUArch &&
      PTID.OptionalTargetID != "amdgcnspirv")
    getDriver().Diag(language::Core::diag::err_drv_bad_target_id)
        << *PTID.OptionalTargetID;
}

SPIRVAMDToolChain::SPIRVAMDToolChain(const Driver &D,
                                     const toolchain::Triple &Triple,
                                     const ArgList &Args)
    : ROCMToolChain(D, Triple, Args) {
  getProgramPaths().push_back(getDriver().Dir);
}

Tool *SPIRVAMDToolChain::buildLinker() const {
  assert(getTriple().getArch() == toolchain::Triple::spirv64);
  return new tools::AMDGCN::Linker(*this);
}
