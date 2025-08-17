//===--- AArch64.cpp - AArch64 (not ARM) Helpers for Tools ------*- C++ -*-===//
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

#include "AArch64.h"
#include "language/Core/Driver/CommonArgs.h"
#include "language/Core/Driver/Driver.h"
#include "language/Core/Driver/Options.h"
#include "toolchain/Option/ArgList.h"
#include "toolchain/TargetParser/AArch64TargetParser.h"
#include "toolchain/TargetParser/Host.h"

using namespace language::Core::driver;
using namespace language::Core::driver::tools;
using namespace language::Core;
using namespace toolchain::opt;

/// \returns true if the given triple can determine the default CPU type even
/// if -arch is not specified.
static bool isCPUDeterminedByTriple(const toolchain::Triple &Triple) {
  return Triple.isOSDarwin();
}

/// getAArch64TargetCPU - Get the (LLVM) name of the AArch64 cpu we are
/// targeting. Set \p A to the Arg corresponding to the -mcpu argument if it is
/// provided, or to nullptr otherwise.
std::string aarch64::getAArch64TargetCPU(const ArgList &Args,
                                         const toolchain::Triple &Triple, Arg *&A) {
  std::string CPU;
  // If we have -mcpu, use that.
  if ((A = Args.getLastArg(options::OPT_mcpu_EQ))) {
    StringRef Mcpu = A->getValue();
    CPU = Mcpu.split("+").first.lower();
  }

  CPU = toolchain::AArch64::resolveCPUAlias(CPU);

  // Handle CPU name is 'native'.
  if (CPU == "native")
    return std::string(toolchain::sys::getHostCPUName());

  if (CPU.size())
    return CPU;

  if (Triple.isTargetMachineMac() &&
      Triple.getArch() == toolchain::Triple::aarch64) {
    // Apple Silicon macs default to M1 CPUs.
    return "apple-m1";
  }

  if (Triple.getOS() == toolchain::Triple::IOS) {
    assert(!Triple.isSimulatorEnvironment() && "iossim should be mac-like");
    // iOS 26 only runs on apple-a12 and later CPUs.
    if (!Triple.isOSVersionLT(26))
      return "apple-a12";
  }

  if (Triple.isWatchOS()) {
    assert(!Triple.isSimulatorEnvironment() && "watchossim should be mac-like");
    // arm64_32/arm64e watchOS requires S4 before watchOS 26, S6 after.
    if (Triple.getArch() == toolchain::Triple::aarch64_32 || Triple.isArm64e())
      return Triple.isOSVersionLT(26) ? "apple-s4" : "apple-s6";
    // arm64 (non-e, non-32) watchOS comes later, and requires S6 anyway.
    return "apple-s6";
  }

  if (Triple.isXROS()) {
    // The xrOS simulator runs on M1 as well, it should have been covered above.
    assert(!Triple.isSimulatorEnvironment() && "xrossim should be mac-like");
    return "apple-a12";
  }
  // arm64e requires v8.3a and only runs on apple-a12 and later CPUs.
  if (Triple.isArm64e())
    return "apple-a12";

  // Make sure we pick the appropriate Apple CPU when targetting a Darwin OS.
  if (Triple.isOSDarwin())
    return Triple.getArch() == toolchain::Triple::aarch64_32 ? "apple-s4"
                                                        : "apple-a7";

  return "generic";
}

// Decode AArch64 features from string like +[no]featureA+[no]featureB+...
static bool DecodeAArch64Features(const Driver &D, StringRef text,
                                  toolchain::AArch64::ExtensionSet &Extensions) {
  SmallVector<StringRef, 8> Split;
  text.split(Split, StringRef("+"), -1, false);

  for (StringRef Feature : Split) {
    if (Feature == "neon" || Feature == "noneon") {
      D.Diag(language::Core::diag::err_drv_no_neon_modifier);
      continue;
    }
    if (!Extensions.parseModifier(Feature))
      return false;
  }

  return true;
}

// Check if the CPU name and feature modifiers in -mcpu are legal. If yes,
// decode CPU and feature.
static bool DecodeAArch64Mcpu(const Driver &D, StringRef Mcpu, StringRef &CPU,
                              toolchain::AArch64::ExtensionSet &Extensions) {
  std::pair<StringRef, StringRef> Split = Mcpu.split("+");
  CPU = Split.first;

  if (CPU == "native")
    CPU = toolchain::sys::getHostCPUName();

  const std::optional<toolchain::AArch64::CpuInfo> CpuInfo =
      toolchain::AArch64::parseCpu(CPU);
  if (!CpuInfo)
    return false;

  Extensions.addCPUDefaults(*CpuInfo);

  if (Split.second.size() &&
      !DecodeAArch64Features(D, Split.second, Extensions))
    return false;

  return true;
}

static bool
getAArch64ArchFeaturesFromMarch(const Driver &D, StringRef March,
                                const ArgList &Args,
                                toolchain::AArch64::ExtensionSet &Extensions) {
  std::string MarchLowerCase = March.lower();
  std::pair<StringRef, StringRef> Split = StringRef(MarchLowerCase).split("+");

  const toolchain::AArch64::ArchInfo *ArchInfo =
      toolchain::AArch64::parseArch(Split.first);
  if (Split.first == "native")
    ArchInfo = toolchain::AArch64::getArchForCpu(toolchain::sys::getHostCPUName().str());
  if (!ArchInfo)
    return false;

  Extensions.addArchDefaults(*ArchInfo);

  if ((Split.second.size() &&
       !DecodeAArch64Features(D, Split.second, Extensions)))
    return false;

  return true;
}

static bool getAArch64ArchFeaturesFromMcpu(
    const Driver &D, StringRef Mcpu, const ArgList &Args,
    toolchain::AArch64::ExtensionSet &Extensions, std::vector<StringRef> &Features) {
  StringRef CPU;
  std::string McpuLowerCase = Mcpu.lower();
  if (!DecodeAArch64Mcpu(D, McpuLowerCase, CPU, Extensions))
    return false;

  if (Mcpu == "native") {
    toolchain::StringMap<bool> HostFeatures = toolchain::sys::getHostCPUFeatures();
    for (auto &[Feature, Enabled] : HostFeatures) {
      Features.push_back(Args.MakeArgString((Enabled ? "+" : "-") + Feature));
    }
  }

  return true;
}

static bool
getAArch64MicroArchFeaturesFromMtune(const Driver &D, StringRef Mtune,
                                     const ArgList &Args,
                                     std::vector<StringRef> &Features) {
  // Check CPU name is valid, but ignore any extensions on it.
  std::string MtuneLowerCase = Mtune.lower();
  toolchain::AArch64::ExtensionSet Extensions;
  StringRef Tune;
  return DecodeAArch64Mcpu(D, MtuneLowerCase, Tune, Extensions);
}

static bool
getAArch64MicroArchFeaturesFromMcpu(const Driver &D, StringRef Mcpu,
                                    const ArgList &Args,
                                    std::vector<StringRef> &Features) {
  return getAArch64MicroArchFeaturesFromMtune(D, Mcpu, Args, Features);
}

void aarch64::getAArch64TargetFeatures(const Driver &D,
                                       const toolchain::Triple &Triple,
                                       const ArgList &Args,
                                       std::vector<StringRef> &Features,
                                       bool ForAS, bool ForMultilib) {
  Arg *A;
  bool success = true;
  toolchain::StringRef WaMArch;
  toolchain::AArch64::ExtensionSet Extensions;
  if (ForAS)
    for (const auto *A :
         Args.filtered(options::OPT_Wa_COMMA, options::OPT_Xassembler))
      for (StringRef Value : A->getValues())
        if (Value.starts_with("-march="))
          WaMArch = Value.substr(7);
  // Call getAArch64ArchFeaturesFromMarch only if "-Wa,-march=" or
  // "-Xassembler -march" is detected. Otherwise it may return false
  // and causes Clang to error out.
  if (!WaMArch.empty())
    success = getAArch64ArchFeaturesFromMarch(D, WaMArch, Args, Extensions);
  else if ((A = Args.getLastArg(options::OPT_march_EQ)))
    success =
        getAArch64ArchFeaturesFromMarch(D, A->getValue(), Args, Extensions);
  else if ((A = Args.getLastArg(options::OPT_mcpu_EQ)))
    success = getAArch64ArchFeaturesFromMcpu(D, A->getValue(), Args, Extensions,
                                             Features);
  else if (isCPUDeterminedByTriple(Triple))
    success = getAArch64ArchFeaturesFromMcpu(
        D, getAArch64TargetCPU(Args, Triple, A), Args, Extensions, Features);
  else
    // Default to 'A' profile if the architecture is not specified.
    success = getAArch64ArchFeaturesFromMarch(D, "armv8-a", Args, Extensions);

  if (success && (A = Args.getLastArg(language::Core::driver::options::OPT_mtune_EQ)))
    success =
        getAArch64MicroArchFeaturesFromMtune(D, A->getValue(), Args, Features);
  else if (success && (A = Args.getLastArg(options::OPT_mcpu_EQ)))
    success =
        getAArch64MicroArchFeaturesFromMcpu(D, A->getValue(), Args, Features);
  else if (success && isCPUDeterminedByTriple(Triple))
    success = getAArch64MicroArchFeaturesFromMcpu(
        D, getAArch64TargetCPU(Args, Triple, A), Args, Features);

  if (!success) {
    auto Diag = D.Diag(diag::err_drv_unsupported_option_argument);
    // If "-Wa,-march=" is used, 'WaMArch' will contain the argument's value,
    // while 'A' is uninitialized. Only dereference 'A' in the other case.
    if (!WaMArch.empty())
      Diag << "-march=" << WaMArch;
    else
      Diag << A->getSpelling() << A->getValue();
  }

  // -mgeneral-regs-only disables all floating-point features.
  if (Args.getLastArg(options::OPT_mgeneral_regs_only)) {
    Extensions.disable(toolchain::AArch64::AEK_FP);
  }

  // En/disable crc
  if (Arg *A = Args.getLastArg(options::OPT_mcrc, options::OPT_mnocrc)) {
    if (A->getOption().matches(options::OPT_mcrc))
      Extensions.enable(toolchain::AArch64::AEK_CRC);
    else
      Extensions.disable(toolchain::AArch64::AEK_CRC);
  }

  // At this point all hardware features are decided, so convert the extensions
  // set to a feature list.
  Extensions.toLLVMFeatureList(Features);

  if (Arg *A = Args.getLastArg(options::OPT_mtp_mode_EQ)) {
    StringRef Mtp = A->getValue();
    if (Mtp == "el3" || Mtp == "tpidr_el3")
      Features.push_back("+tpidr-el3");
    else if (Mtp == "el2" || Mtp == "tpidr_el2")
      Features.push_back("+tpidr-el2");
    else if (Mtp == "el1" || Mtp == "tpidr_el1")
      Features.push_back("+tpidr-el1");
    else if (Mtp == "tpidrro_el0")
      Features.push_back("+tpidrro-el0");
    else if (Mtp != "el0" && Mtp != "tpidr_el0")
      D.Diag(diag::err_drv_invalid_mtp) << A->getAsString(Args);
  }

  // Enable/disable straight line speculation hardening.
  if (Arg *A = Args.getLastArg(options::OPT_mharden_sls_EQ)) {
    StringRef Scope = A->getValue();
    bool EnableRetBr = false;
    bool EnableBlr = false;
    bool DisableComdat = false;
    if (Scope != "none") {
      SmallVector<StringRef, 4> Opts;
      Scope.split(Opts, ",");
      for (auto Opt : Opts) {
        Opt = Opt.trim();
        if (Opt == "all") {
          EnableBlr = true;
          EnableRetBr = true;
          continue;
        }
        if (Opt == "retbr") {
          EnableRetBr = true;
          continue;
        }
        if (Opt == "blr") {
          EnableBlr = true;
          continue;
        }
        if (Opt == "comdat") {
          DisableComdat = false;
          continue;
        }
        if (Opt == "nocomdat") {
          DisableComdat = true;
          continue;
        }
        D.Diag(diag::err_drv_unsupported_option_argument)
            << A->getSpelling() << Scope;
        break;
      }
    }

    if (EnableRetBr)
      Features.push_back("+harden-sls-retbr");
    if (EnableBlr)
      Features.push_back("+harden-sls-blr");
    if (DisableComdat) {
      Features.push_back("+harden-sls-nocomdat");
    }
  }

  if (Arg *A = Args.getLastArg(
          options::OPT_mstrict_align, options::OPT_mno_strict_align,
          options::OPT_mno_unaligned_access, options::OPT_munaligned_access)) {
    if (A->getOption().matches(options::OPT_mstrict_align) ||
        A->getOption().matches(options::OPT_mno_unaligned_access))
      Features.push_back("+strict-align");
  } else if (Triple.isOSOpenBSD())
    Features.push_back("+strict-align");

  // Generate execute-only output (no data access to code sections).
  // This only makes sense for the compiler, not for the assembler.
  // It's not needed for multilib selection and may hide an unused
  // argument diagnostic if the code is always run.
  if (!ForAS && !ForMultilib) {
    if (Arg *A = Args.getLastArg(options::OPT_mexecute_only,
                                 options::OPT_mno_execute_only)) {
      if (A->getOption().matches(options::OPT_mexecute_only)) {
        Features.push_back("+execute-only");
      }
    }
  }

  if (Args.hasArg(options::OPT_ffixed_x1))
    Features.push_back("+reserve-x1");

  if (Args.hasArg(options::OPT_ffixed_x2))
    Features.push_back("+reserve-x2");

  if (Args.hasArg(options::OPT_ffixed_x3))
    Features.push_back("+reserve-x3");

  if (Args.hasArg(options::OPT_ffixed_x4))
    Features.push_back("+reserve-x4");

  if (Args.hasArg(options::OPT_ffixed_x5))
    Features.push_back("+reserve-x5");

  if (Args.hasArg(options::OPT_ffixed_x6))
    Features.push_back("+reserve-x6");

  if (Args.hasArg(options::OPT_ffixed_x7))
    Features.push_back("+reserve-x7");

  if (Args.hasArg(options::OPT_ffixed_x9))
    Features.push_back("+reserve-x9");

  if (Args.hasArg(options::OPT_ffixed_x10))
    Features.push_back("+reserve-x10");

  if (Args.hasArg(options::OPT_ffixed_x11))
    Features.push_back("+reserve-x11");

  if (Args.hasArg(options::OPT_ffixed_x12))
    Features.push_back("+reserve-x12");

  if (Args.hasArg(options::OPT_ffixed_x13))
    Features.push_back("+reserve-x13");

  if (Args.hasArg(options::OPT_ffixed_x14))
    Features.push_back("+reserve-x14");

  if (Args.hasArg(options::OPT_ffixed_x15))
    Features.push_back("+reserve-x15");

  if (Args.hasArg(options::OPT_ffixed_x18))
    Features.push_back("+reserve-x18");

  if (Args.hasArg(options::OPT_ffixed_x20))
    Features.push_back("+reserve-x20");

  if (Args.hasArg(options::OPT_ffixed_x21))
    Features.push_back("+reserve-x21");

  if (Args.hasArg(options::OPT_ffixed_x22))
    Features.push_back("+reserve-x22");

  if (Args.hasArg(options::OPT_ffixed_x23))
    Features.push_back("+reserve-x23");

  if (Args.hasArg(options::OPT_ffixed_x24))
    Features.push_back("+reserve-x24");

  if (Args.hasArg(options::OPT_ffixed_x25))
    Features.push_back("+reserve-x25");

  if (Args.hasArg(options::OPT_ffixed_x26))
    Features.push_back("+reserve-x26");

  if (Args.hasArg(options::OPT_ffixed_x27))
    Features.push_back("+reserve-x27");

  if (Args.hasArg(options::OPT_ffixed_x28))
    Features.push_back("+reserve-x28");

  if (Args.hasArg(options::OPT_mlr_for_calls_only))
    Features.push_back("+reserve-lr-for-ra");

  if (Args.hasArg(options::OPT_fcall_saved_x8))
    Features.push_back("+call-saved-x8");

  if (Args.hasArg(options::OPT_fcall_saved_x9))
    Features.push_back("+call-saved-x9");

  if (Args.hasArg(options::OPT_fcall_saved_x10))
    Features.push_back("+call-saved-x10");

  if (Args.hasArg(options::OPT_fcall_saved_x11))
    Features.push_back("+call-saved-x11");

  if (Args.hasArg(options::OPT_fcall_saved_x12))
    Features.push_back("+call-saved-x12");

  if (Args.hasArg(options::OPT_fcall_saved_x13))
    Features.push_back("+call-saved-x13");

  if (Args.hasArg(options::OPT_fcall_saved_x14))
    Features.push_back("+call-saved-x14");

  if (Args.hasArg(options::OPT_fcall_saved_x15))
    Features.push_back("+call-saved-x15");

  if (Args.hasArg(options::OPT_fcall_saved_x18))
    Features.push_back("+call-saved-x18");

  if (Args.hasArg(options::OPT_mno_neg_immediates))
    Features.push_back("+no-neg-immediates");

  if (Arg *A = Args.getLastArg(options::OPT_mfix_cortex_a53_835769,
                               options::OPT_mno_fix_cortex_a53_835769)) {
    if (A->getOption().matches(options::OPT_mfix_cortex_a53_835769))
      Features.push_back("+fix-cortex-a53-835769");
    else
      Features.push_back("-fix-cortex-a53-835769");
  } else if (Triple.isAndroid() || Triple.isOHOSFamily()) {
    // Enabled A53 errata (835769) workaround by default on android
    Features.push_back("+fix-cortex-a53-835769");
  } else if (Triple.isOSFuchsia()) {
    std::string CPU = getCPUName(D, Args, Triple);
    if (CPU.empty() || CPU == "generic" || CPU == "cortex-a53")
      Features.push_back("+fix-cortex-a53-835769");
  }

  if (Args.getLastArg(options::OPT_mno_bti_at_return_twice))
    Features.push_back("+no-bti-at-return-twice");
}

void aarch64::setPAuthABIInTriple(const Driver &D, const ArgList &Args,
                                  toolchain::Triple &Triple) {
  Arg *ABIArg = Args.getLastArg(options::OPT_mabi_EQ);
  bool HasPAuthABI =
      ABIArg ? (StringRef(ABIArg->getValue()) == "pauthtest") : false;

  switch (Triple.getEnvironment()) {
  case toolchain::Triple::UnknownEnvironment:
    if (HasPAuthABI)
      Triple.setEnvironment(toolchain::Triple::PAuthTest);
    break;
  case toolchain::Triple::PAuthTest:
    break;
  default:
    if (HasPAuthABI)
      D.Diag(diag::err_drv_unsupported_opt_for_target)
          << ABIArg->getAsString(Args) << Triple.getTriple();
    break;
  }
}

/// Is the triple {aarch64.aarch64_be}-none-elf?
bool aarch64::isAArch64BareMetal(const toolchain::Triple &Triple) {
  if (Triple.getArch() != toolchain::Triple::aarch64 &&
      Triple.getArch() != toolchain::Triple::aarch64_be)
    return false;

  if (Triple.getVendor() != toolchain::Triple::UnknownVendor)
    return false;

  if (Triple.getOS() != toolchain::Triple::UnknownOS)
    return false;

  return Triple.getEnvironmentName() == "elf";
}
