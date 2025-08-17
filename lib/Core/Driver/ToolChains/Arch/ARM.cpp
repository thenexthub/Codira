//===--- ARM.cpp - ARM (not AArch64) Helpers for Tools ----------*- C++ -*-===//
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

#include "ARM.h"
#include "language/Core/Driver/Driver.h"
#include "language/Core/Driver/Options.h"
#include "toolchain/ADT/StringSwitch.h"
#include "toolchain/Option/ArgList.h"
#include "toolchain/TargetParser/ARMTargetParser.h"
#include "toolchain/TargetParser/Host.h"

using namespace language::Core::driver;
using namespace language::Core::driver::tools;
using namespace language::Core;
using namespace toolchain::opt;

// Get SubArch (vN).
int arm::getARMSubArchVersionNumber(const toolchain::Triple &Triple) {
  toolchain::StringRef Arch = Triple.getArchName();
  return toolchain::ARM::parseArchVersion(Arch);
}

// True if M-profile.
bool arm::isARMMProfile(const toolchain::Triple &Triple) {
  toolchain::StringRef Arch = Triple.getArchName();
  return toolchain::ARM::parseArchProfile(Arch) == toolchain::ARM::ProfileKind::M;
}

// On Arm the endianness of the output file is determined by the target and
// can be overridden by the pseudo-target flags '-mlittle-endian'/'-EL' and
// '-mbig-endian'/'-EB'. Unlike other targets the flag does not result in a
// normalized triple so we must handle the flag here.
bool arm::isARMBigEndian(const toolchain::Triple &Triple, const ArgList &Args) {
  if (Arg *A = Args.getLastArg(options::OPT_mlittle_endian,
                               options::OPT_mbig_endian)) {
    return !A->getOption().matches(options::OPT_mlittle_endian);
  }

  return Triple.getArch() == toolchain::Triple::armeb ||
         Triple.getArch() == toolchain::Triple::thumbeb;
}

// True if A-profile.
bool arm::isARMAProfile(const toolchain::Triple &Triple) {
  toolchain::StringRef Arch = Triple.getArchName();
  return toolchain::ARM::parseArchProfile(Arch) == toolchain::ARM::ProfileKind::A;
}

/// Is the triple {arm,armeb,thumb,thumbeb}-none-none-{eabi,eabihf} ?
bool arm::isARMEABIBareMetal(const toolchain::Triple &Triple) {
  auto arch = Triple.getArch();
  if (arch != toolchain::Triple::arm && arch != toolchain::Triple::thumb &&
      arch != toolchain::Triple::armeb && arch != toolchain::Triple::thumbeb)
    return false;

  if (Triple.getVendor() != toolchain::Triple::UnknownVendor)
    return false;

  if (Triple.getOS() != toolchain::Triple::UnknownOS)
    return false;

  if (Triple.getEnvironment() != toolchain::Triple::EABI &&
      Triple.getEnvironment() != toolchain::Triple::EABIHF)
    return false;

  return true;
}

// Get Arch/CPU from args.
void arm::getARMArchCPUFromArgs(const ArgList &Args, toolchain::StringRef &Arch,
                                toolchain::StringRef &CPU, bool FromAs) {
  if (const Arg *A = Args.getLastArg(language::Core::driver::options::OPT_mcpu_EQ))
    CPU = A->getValue();
  if (const Arg *A = Args.getLastArg(options::OPT_march_EQ))
    Arch = A->getValue();
  if (!FromAs)
    return;

  for (const Arg *A :
       Args.filtered(options::OPT_Wa_COMMA, options::OPT_Xassembler)) {
    // Use getValues because -Wa can have multiple arguments
    // e.g. -Wa,-mcpu=foo,-mcpu=bar
    for (StringRef Value : A->getValues()) {
      if (Value.starts_with("-mcpu="))
        CPU = Value.substr(6);
      if (Value.starts_with("-march="))
        Arch = Value.substr(7);
    }
  }
}

// Handle -mhwdiv=.
// FIXME: Use ARMTargetParser.
static void getARMHWDivFeatures(const Driver &D, const Arg *A,
                                const ArgList &Args, StringRef HWDiv,
                                std::vector<StringRef> &Features) {
  uint64_t HWDivID = toolchain::ARM::parseHWDiv(HWDiv);
  if (!toolchain::ARM::getHWDivFeatures(HWDivID, Features))
    D.Diag(language::Core::diag::err_drv_clang_unsupported) << A->getAsString(Args);
}

// Handle -mfpu=.
static toolchain::ARM::FPUKind getARMFPUFeatures(const Driver &D, const Arg *A,
                                            const ArgList &Args, StringRef FPU,
                                            std::vector<StringRef> &Features) {
  toolchain::ARM::FPUKind FPUKind = toolchain::ARM::parseFPU(FPU);
  if (!toolchain::ARM::getFPUFeatures(FPUKind, Features))
    D.Diag(language::Core::diag::err_drv_clang_unsupported) << A->getAsString(Args);
  return FPUKind;
}

// Decode ARM features from string like +[no]featureA+[no]featureB+...
static bool DecodeARMFeatures(const Driver &D, StringRef text, StringRef CPU,
                              toolchain::ARM::ArchKind ArchKind,
                              std::vector<StringRef> &Features,
                              toolchain::ARM::FPUKind &ArgFPUKind) {
  SmallVector<StringRef, 8> Split;
  text.split(Split, StringRef("+"), -1, false);

  for (StringRef Feature : Split) {
    if (!appendArchExtFeatures(CPU, ArchKind, Feature, Features, ArgFPUKind))
      return false;
  }
  return true;
}

static void DecodeARMFeaturesFromCPU(const Driver &D, StringRef CPU,
                                     std::vector<StringRef> &Features) {
  CPU = CPU.split("+").first;
  if (CPU != "generic") {
    toolchain::ARM::ArchKind ArchKind = toolchain::ARM::parseCPUArch(CPU);
    uint64_t Extension = toolchain::ARM::getDefaultExtensions(CPU, ArchKind);
    toolchain::ARM::getExtensionFeatures(Extension, Features);
  }
}

// Check if -march is valid by checking if it can be canonicalised and parsed.
// getARMArch is used here instead of just checking the -march value in order
// to handle -march=native correctly.
static void checkARMArchName(const Driver &D, const Arg *A, const ArgList &Args,
                             toolchain::StringRef ArchName, toolchain::StringRef CPUName,
                             std::vector<StringRef> &Features,
                             const toolchain::Triple &Triple,
                             toolchain::ARM::FPUKind &ArgFPUKind) {
  std::pair<StringRef, StringRef> Split = ArchName.split("+");

  std::string MArch = arm::getARMArch(ArchName, Triple);
  toolchain::ARM::ArchKind ArchKind = toolchain::ARM::parseArch(MArch);
  if (ArchKind == toolchain::ARM::ArchKind::INVALID ||
      (Split.second.size() &&
       !DecodeARMFeatures(D, Split.second, CPUName, ArchKind, Features,
                          ArgFPUKind)))
    D.Diag(language::Core::diag::err_drv_unsupported_option_argument)
        << A->getSpelling() << A->getValue();
}

// Check -mcpu=. Needs ArchName to handle -mcpu=generic.
static void checkARMCPUName(const Driver &D, const Arg *A, const ArgList &Args,
                            toolchain::StringRef CPUName, toolchain::StringRef ArchName,
                            std::vector<StringRef> &Features,
                            const toolchain::Triple &Triple,
                            toolchain::ARM::FPUKind &ArgFPUKind) {
  std::pair<StringRef, StringRef> Split = CPUName.split("+");

  std::string CPU = arm::getARMTargetCPU(CPUName, ArchName, Triple);
  toolchain::ARM::ArchKind ArchKind =
    arm::getLLVMArchKindForARM(CPU, ArchName, Triple);
  if (ArchKind == toolchain::ARM::ArchKind::INVALID ||
      (Split.second.size() && !DecodeARMFeatures(D, Split.second, CPU, ArchKind,
                                                 Features, ArgFPUKind)))
    D.Diag(language::Core::diag::err_drv_unsupported_option_argument)
        << A->getSpelling() << A->getValue();
}

// If -mfloat-abi=hard or -mhard-float are specified explicitly then check that
// floating point registers are available on the target CPU.
static void checkARMFloatABI(const Driver &D, const ArgList &Args,
                             bool HasFPRegs) {
  if (HasFPRegs)
    return;
  const Arg *A =
      Args.getLastArg(options::OPT_msoft_float, options::OPT_mhard_float,
                      options::OPT_mfloat_abi_EQ);
  if (A && (A->getOption().matches(options::OPT_mhard_float) ||
            (A->getOption().matches(options::OPT_mfloat_abi_EQ) &&
             A->getValue() == StringRef("hard"))))
    D.Diag(language::Core::diag::warn_drv_no_floating_point_registers)
        << A->getAsString(Args);
}

bool arm::useAAPCSForMachO(const toolchain::Triple &T) {
  // The backend is hardwired to assume AAPCS for M-class processors, ensure
  // the frontend matches that.
  return T.getEnvironment() == toolchain::Triple::EABI ||
         T.getEnvironment() == toolchain::Triple::EABIHF ||
         T.getOS() == toolchain::Triple::UnknownOS || isARMMProfile(T);
}

// Check whether the architecture backend has support for the MRC/MCR
// instructions that are used to set the hard thread pointer ("CP15 C13
// Thread id").
// This is not identical to ability to use the instruction, as the ARMV6K
// variants can only use it in Arm mode since they don't support Thumb2
// encoding.
bool arm::isHardTPSupported(const toolchain::Triple &Triple) {
  int Ver = getARMSubArchVersionNumber(Triple);
  toolchain::ARM::ArchKind AK = toolchain::ARM::parseArch(Triple.getArchName());
  return AK == toolchain::ARM::ArchKind::ARMV6K ||
         AK == toolchain::ARM::ArchKind::ARMV6KZ ||
         (Ver >= 7 && !isARMMProfile(Triple));
}

// Checks whether the architecture is capable of supporting the Thumb2 encoding
static bool supportsThumb2Encoding(const toolchain::Triple &Triple) {
  int Ver = arm::getARMSubArchVersionNumber(Triple);
  toolchain::ARM::ArchKind AK = toolchain::ARM::parseArch(Triple.getArchName());
  return AK == toolchain::ARM::ArchKind::ARMV6T2 ||
         (Ver >= 7 && AK != toolchain::ARM::ArchKind::ARMV8MBaseline);
}

// Select mode for reading thread pointer (-mtp=soft/cp15).
arm::ReadTPMode arm::getReadTPMode(const Driver &D, const ArgList &Args,
                                   const toolchain::Triple &Triple, bool ForAS) {
  Arg *A = Args.getLastArg(options::OPT_mtp_mode_EQ);
  if (A && A->getValue() != StringRef("auto")) {
    arm::ReadTPMode ThreadPointer =
        toolchain::StringSwitch<arm::ReadTPMode>(A->getValue())
            .Case("cp15", ReadTPMode::TPIDRURO)
            .Case("tpidrurw", ReadTPMode::TPIDRURW)
            .Case("tpidruro", ReadTPMode::TPIDRURO)
            .Case("tpidrprw", ReadTPMode::TPIDRPRW)
            .Case("soft", ReadTPMode::Soft)
            .Default(ReadTPMode::Invalid);
    if ((ThreadPointer == ReadTPMode::TPIDRURW ||
         ThreadPointer == ReadTPMode::TPIDRURO ||
         ThreadPointer == ReadTPMode::TPIDRPRW) &&
        !isHardTPSupported(Triple) && !ForAS) {
      D.Diag(diag::err_target_unsupported_tp_hard) << Triple.getArchName();
      return ReadTPMode::Invalid;
    }
    if (ThreadPointer != ReadTPMode::Invalid)
      return ThreadPointer;
    if (StringRef(A->getValue()).empty())
      D.Diag(diag::err_drv_missing_arg_mtp) << A->getAsString(Args);
    else
      D.Diag(diag::err_drv_invalid_mtp) << A->getAsString(Args);
    return ReadTPMode::Invalid;
  }
  // In auto mode we enable HW mode only if both the hardware supports it and
  // the thumb2 encoding. For example ARMV6T2 supports thumb2, but not hardware.
  // ARMV6K has HW suport, but not thumb2. Otherwise we could enable it for
  // ARMV6K in thumb mode.
  bool autoUseHWTPMode =
      isHardTPSupported(Triple) && supportsThumb2Encoding(Triple);
  return autoUseHWTPMode ? ReadTPMode::TPIDRURO : ReadTPMode::Soft;
}

void arm::setArchNameInTriple(const Driver &D, const ArgList &Args,
                              types::ID InputType, toolchain::Triple &Triple) {
  StringRef MCPU, MArch;
  if (const Arg *A = Args.getLastArg(options::OPT_mcpu_EQ))
    MCPU = A->getValue();
  if (const Arg *A = Args.getLastArg(options::OPT_march_EQ))
    MArch = A->getValue();

  std::string CPU = Triple.isOSBinFormatMachO()
                        ? tools::arm::getARMCPUForMArch(MArch, Triple).str()
                        : tools::arm::getARMTargetCPU(MCPU, MArch, Triple);
  StringRef Suffix = tools::arm::getLLVMArchSuffixForARM(CPU, MArch, Triple);

  bool IsBigEndian = Triple.getArch() == toolchain::Triple::armeb ||
                     Triple.getArch() == toolchain::Triple::thumbeb;
  // Handle pseudo-target flags '-mlittle-endian'/'-EL' and
  // '-mbig-endian'/'-EB'.
  if (Arg *A = Args.getLastArg(options::OPT_mlittle_endian,
                               options::OPT_mbig_endian)) {
    IsBigEndian = !A->getOption().matches(options::OPT_mlittle_endian);
  }
  std::string ArchName = IsBigEndian ? "armeb" : "arm";

  // FIXME: Thumb should just be another -target-feaure, not in the triple.
  bool IsMProfile =
      toolchain::ARM::parseArchProfile(Suffix) == toolchain::ARM::ProfileKind::M;
  bool ThumbDefault = IsMProfile ||
                      // Thumb2 is the default for V7 on Darwin.
                      (toolchain::ARM::parseArchVersion(Suffix) == 7 &&
                       Triple.isOSBinFormatMachO()) ||
                      // FIXME: this is invalid for WindowsCE
                      Triple.isOSWindows();

  // Check if ARM ISA was explicitly selected (using -mno-thumb or -marm) for
  // M-Class CPUs/architecture variants, which is not supported.
  bool ARMModeRequested =
      !Args.hasFlag(options::OPT_mthumb, options::OPT_mno_thumb, ThumbDefault);
  if (IsMProfile && ARMModeRequested) {
    if (MCPU.size())
      D.Diag(diag::err_cpu_unsupported_isa) << CPU << "ARM";
    else
      D.Diag(diag::err_arch_unsupported_isa)
          << tools::arm::getARMArch(MArch, Triple) << "ARM";
  }

  // Check to see if an explicit choice to use thumb has been made via
  // -mthumb. For assembler files we must check for -mthumb in the options
  // passed to the assembler via -Wa or -Xassembler.
  bool IsThumb = false;
  if (InputType != types::TY_PP_Asm)
    IsThumb =
        Args.hasFlag(options::OPT_mthumb, options::OPT_mno_thumb, ThumbDefault);
  else {
    // Ideally we would check for these flags in
    // CollectArgsForIntegratedAssembler but we can't change the ArchName at
    // that point.
    toolchain::StringRef WaMArch, WaMCPU;
    for (const auto *A :
         Args.filtered(options::OPT_Wa_COMMA, options::OPT_Xassembler)) {
      for (StringRef Value : A->getValues()) {
        // There is no assembler equivalent of -mno-thumb, -marm, or -mno-arm.
        if (Value == "-mthumb")
          IsThumb = true;
        else if (Value.starts_with("-march="))
          WaMArch = Value.substr(7);
        else if (Value.starts_with("-mcpu="))
          WaMCPU = Value.substr(6);
      }
    }

    if (WaMCPU.size() || WaMArch.size()) {
      // The way this works means that we prefer -Wa,-mcpu's architecture
      // over -Wa,-march. Which matches the compiler behaviour.
      Suffix = tools::arm::getLLVMArchSuffixForARM(WaMCPU, WaMArch, Triple);
    }
  }

  // Assembly files should start in ARM mode, unless arch is M-profile, or
  // -mthumb has been passed explicitly to the assembler. Windows is always
  // thumb.
  if (IsThumb || IsMProfile || Triple.isOSWindows()) {
    if (IsBigEndian)
      ArchName = "thumbeb";
    else
      ArchName = "thumb";
  }
  Triple.setArchName(ArchName + Suffix.str());
}

void arm::setFloatABIInTriple(const Driver &D, const ArgList &Args,
                              toolchain::Triple &Triple) {
  if (Triple.isOSLiteOS()) {
    Triple.setEnvironment(toolchain::Triple::OpenHOS);
    return;
  }

  bool isHardFloat =
      (arm::getARMFloatABI(D, Triple, Args) == arm::FloatABI::Hard);

  switch (Triple.getEnvironment()) {
  case toolchain::Triple::GNUEABI:
  case toolchain::Triple::GNUEABIHF:
    Triple.setEnvironment(isHardFloat ? toolchain::Triple::GNUEABIHF
                                      : toolchain::Triple::GNUEABI);
    break;
  case toolchain::Triple::GNUEABIT64:
  case toolchain::Triple::GNUEABIHFT64:
    Triple.setEnvironment(isHardFloat ? toolchain::Triple::GNUEABIHFT64
                                      : toolchain::Triple::GNUEABIT64);
    break;
  case toolchain::Triple::EABI:
  case toolchain::Triple::EABIHF:
    Triple.setEnvironment(isHardFloat ? toolchain::Triple::EABIHF
                                      : toolchain::Triple::EABI);
    break;
  case toolchain::Triple::MuslEABI:
  case toolchain::Triple::MuslEABIHF:
    Triple.setEnvironment(isHardFloat ? toolchain::Triple::MuslEABIHF
                                      : toolchain::Triple::MuslEABI);
    break;
  case toolchain::Triple::OpenHOS:
    break;
  default: {
    arm::FloatABI DefaultABI = arm::getDefaultFloatABI(Triple);
    if (DefaultABI != arm::FloatABI::Invalid &&
        isHardFloat != (DefaultABI == arm::FloatABI::Hard)) {
      Arg *ABIArg =
          Args.getLastArg(options::OPT_msoft_float, options::OPT_mhard_float,
                          options::OPT_mfloat_abi_EQ);
      assert(ABIArg && "Non-default float abi expected to be from arg");
      D.Diag(diag::err_drv_unsupported_opt_for_target)
          << ABIArg->getAsString(Args) << Triple.getTriple();
    }
    break;
  }
  }
}

arm::FloatABI arm::getARMFloatABI(const ToolChain &TC, const ArgList &Args) {
  return arm::getARMFloatABI(TC.getDriver(), TC.getEffectiveTriple(), Args);
}

arm::FloatABI arm::getDefaultFloatABI(const toolchain::Triple &Triple) {
  auto SubArch = getARMSubArchVersionNumber(Triple);
  switch (Triple.getOS()) {
  case toolchain::Triple::Darwin:
  case toolchain::Triple::MacOSX:
  case toolchain::Triple::IOS:
  case toolchain::Triple::TvOS:
  case toolchain::Triple::DriverKit:
  case toolchain::Triple::XROS:
    // Darwin defaults to "softfp" for v6 and v7.
    if (Triple.isWatchABI())
      return FloatABI::Hard;
    else
      return (SubArch == 6 || SubArch == 7) ? FloatABI::SoftFP : FloatABI::Soft;

  case toolchain::Triple::WatchOS:
    return FloatABI::Hard;

  // FIXME: this is invalid for WindowsCE
  case toolchain::Triple::Win32:
    // It is incorrect to select hard float ABI on MachO platforms if the ABI is
    // "apcs-gnu".
    if (Triple.isOSBinFormatMachO() && !useAAPCSForMachO(Triple))
      return FloatABI::Soft;
    return FloatABI::Hard;

  case toolchain::Triple::NetBSD:
    switch (Triple.getEnvironment()) {
    case toolchain::Triple::EABIHF:
    case toolchain::Triple::GNUEABIHF:
      return FloatABI::Hard;
    default:
      return FloatABI::Soft;
    }
    break;

  case toolchain::Triple::FreeBSD:
    switch (Triple.getEnvironment()) {
    case toolchain::Triple::GNUEABIHF:
      return FloatABI::Hard;
    default:
      // FreeBSD defaults to soft float
      return FloatABI::Soft;
    }
    break;

  case toolchain::Triple::Haiku:
  case toolchain::Triple::OpenBSD:
    return FloatABI::SoftFP;

  default:
    if (Triple.isOHOSFamily())
      return FloatABI::Soft;
    switch (Triple.getEnvironment()) {
    case toolchain::Triple::GNUEABIHF:
    case toolchain::Triple::GNUEABIHFT64:
    case toolchain::Triple::MuslEABIHF:
    case toolchain::Triple::EABIHF:
      return FloatABI::Hard;
    case toolchain::Triple::Android:
    case toolchain::Triple::GNUEABI:
    case toolchain::Triple::GNUEABIT64:
    case toolchain::Triple::MuslEABI:
    case toolchain::Triple::EABI:
      // EABI is always AAPCS, and if it was not marked 'hard', it's softfp
      return FloatABI::SoftFP;
    default:
      return FloatABI::Invalid;
    }
  }
  return FloatABI::Invalid;
}

// Select the float ABI as determined by -msoft-float, -mhard-float, and
// -mfloat-abi=.
arm::FloatABI arm::getARMFloatABI(const Driver &D, const toolchain::Triple &Triple,
                                  const ArgList &Args) {
  arm::FloatABI ABI = FloatABI::Invalid;
  if (Arg *A =
          Args.getLastArg(options::OPT_msoft_float, options::OPT_mhard_float,
                          options::OPT_mfloat_abi_EQ)) {
    if (A->getOption().matches(options::OPT_msoft_float)) {
      ABI = FloatABI::Soft;
    } else if (A->getOption().matches(options::OPT_mhard_float)) {
      ABI = FloatABI::Hard;
    } else {
      ABI = toolchain::StringSwitch<arm::FloatABI>(A->getValue())
                .Case("soft", FloatABI::Soft)
                .Case("softfp", FloatABI::SoftFP)
                .Case("hard", FloatABI::Hard)
                .Default(FloatABI::Invalid);
      if (ABI == FloatABI::Invalid && !StringRef(A->getValue()).empty()) {
        D.Diag(diag::err_drv_invalid_mfloat_abi) << A->getAsString(Args);
        ABI = FloatABI::Soft;
      }
    }
  }

  // If unspecified, choose the default based on the platform.
  if (ABI == FloatABI::Invalid)
    ABI = arm::getDefaultFloatABI(Triple);

  if (ABI == FloatABI::Invalid) {
    // Assume "soft", but warn the user we are guessing.
    if (Triple.isOSBinFormatMachO() &&
        Triple.getSubArch() == toolchain::Triple::ARMSubArch_v7em)
      ABI = FloatABI::Hard;
    else
      ABI = FloatABI::Soft;

    if (Triple.getOS() != toolchain::Triple::UnknownOS ||
        !Triple.isOSBinFormatMachO())
      D.Diag(diag::warn_drv_assuming_mfloat_abi_is) << "soft";
  }

  assert(ABI != FloatABI::Invalid && "must select an ABI");
  return ABI;
}

static bool hasIntegerMVE(const std::vector<StringRef> &F) {
  auto MVE = toolchain::find(toolchain::reverse(F), "+mve");
  auto NoMVE = toolchain::find(toolchain::reverse(F), "-mve");
  return MVE != F.rend() &&
         (NoMVE == F.rend() || std::distance(MVE, NoMVE) > 0);
}

toolchain::ARM::FPUKind arm::getARMTargetFeatures(const Driver &D,
                                             const toolchain::Triple &Triple,
                                             const ArgList &Args,
                                             std::vector<StringRef> &Features,
                                             bool ForAS, bool ForMultilib) {
  bool KernelOrKext =
      Args.hasArg(options::OPT_mkernel, options::OPT_fapple_kext);
  arm::FloatABI ABI = arm::getARMFloatABI(D, Triple, Args);
  std::optional<std::pair<const Arg *, StringRef>> WaCPU, WaFPU, WaHDiv, WaArch;

  // This vector will accumulate features from the architecture
  // extension suffixes on -mcpu and -march (e.g. the 'bar' in
  // -mcpu=foo+bar). We want to apply those after the features derived
  // from the FPU, in case -mfpu generates a negative feature which
  // the +bar is supposed to override.
  std::vector<StringRef> ExtensionFeatures;

  if (!ForAS) {
    // FIXME: Note, this is a hack, the LLVM backend doesn't actually use these
    // yet (it uses the -mfloat-abi and -msoft-float options), and it is
    // stripped out by the ARM target. We should probably pass this a new
    // -target-option, which is handled by the -cc1/-cc1as invocation.
    //
    // FIXME2:  For consistency, it would be ideal if we set up the target
    // machine state the same when using the frontend or the assembler. We don't
    // currently do that for the assembler, we pass the options directly to the
    // backend and never even instantiate the frontend TargetInfo. If we did,
    // and used its handleTargetFeatures hook, then we could ensure the
    // assembler and the frontend behave the same.

    // Use software floating point operations?
    if (ABI == arm::FloatABI::Soft)
      Features.push_back("+soft-float");

    // Use software floating point argument passing?
    if (ABI != arm::FloatABI::Hard)
      Features.push_back("+soft-float-abi");
  } else {
    // Here, we make sure that -Wa,-mfpu/cpu/arch/hwdiv will be passed down
    // to the assembler correctly.
    for (const Arg *A :
         Args.filtered(options::OPT_Wa_COMMA, options::OPT_Xassembler)) {
      // We use getValues here because you can have many options per -Wa
      // We will keep the last one we find for each of these
      for (StringRef Value : A->getValues()) {
        if (Value.starts_with("-mfpu=")) {
          WaFPU = std::make_pair(A, Value.substr(6));
        } else if (Value.starts_with("-mcpu=")) {
          WaCPU = std::make_pair(A, Value.substr(6));
        } else if (Value.starts_with("-mhwdiv=")) {
          WaHDiv = std::make_pair(A, Value.substr(8));
        } else if (Value.starts_with("-march=")) {
          WaArch = std::make_pair(A, Value.substr(7));
        }
      }
    }

    // The integrated assembler doesn't implement e_flags setting behavior for
    // -meabi=gnu (gcc -mabi={apcs-gnu,atpcs} passes -meabi=gnu to gas). For
    // compatibility we accept but warn.
    if (Arg *A = Args.getLastArgNoClaim(options::OPT_mabi_EQ))
      A->ignoreTargetSpecific();
  }

  arm::ReadTPMode TPMode = getReadTPMode(D, Args, Triple, ForAS);

  if (TPMode == ReadTPMode::TPIDRURW)
    Features.push_back("+read-tp-tpidrurw");
  else if (TPMode == ReadTPMode::TPIDRPRW)
    Features.push_back("+read-tp-tpidrprw");
  else if (TPMode == ReadTPMode::TPIDRURO)
    Features.push_back("+read-tp-tpidruro");

  const Arg *ArchArg = Args.getLastArg(options::OPT_march_EQ);
  const Arg *CPUArg = Args.getLastArg(options::OPT_mcpu_EQ);
  StringRef ArchName;
  StringRef CPUName;
  toolchain::ARM::FPUKind ArchArgFPUKind = toolchain::ARM::FK_INVALID;
  toolchain::ARM::FPUKind CPUArgFPUKind = toolchain::ARM::FK_INVALID;

  // Check -mcpu. ClangAs gives preference to -Wa,-mcpu=.
  if (WaCPU) {
    if (CPUArg)
      D.Diag(language::Core::diag::warn_drv_unused_argument)
          << CPUArg->getAsString(Args);
    CPUName = WaCPU->second;
    CPUArg = WaCPU->first;
  } else if (CPUArg)
    CPUName = CPUArg->getValue();

  // Check -march. ClangAs gives preference to -Wa,-march=.
  if (WaArch) {
    if (ArchArg)
      D.Diag(language::Core::diag::warn_drv_unused_argument)
          << ArchArg->getAsString(Args);
    ArchName = WaArch->second;
    // This will set any features after the base architecture.
    checkARMArchName(D, WaArch->first, Args, ArchName, CPUName,
                     ExtensionFeatures, Triple, ArchArgFPUKind);
    // The base architecture was handled in ToolChain::ComputeLLVMTriple because
    // triple is read only by this point.
  } else if (ArchArg) {
    ArchName = ArchArg->getValue();
    checkARMArchName(D, ArchArg, Args, ArchName, CPUName, ExtensionFeatures,
                     Triple, ArchArgFPUKind);
  }

  // Add CPU features for generic CPUs
  if (CPUName == "native") {
    for (auto &F : toolchain::sys::getHostCPUFeatures())
      Features.push_back(
          Args.MakeArgString((F.second ? "+" : "-") + F.first()));
  } else if (!CPUName.empty()) {
    // This sets the default features for the specified CPU. We certainly don't
    // want to override the features that have been explicitly specified on the
    // command line. Therefore, process them directly instead of appending them
    // at the end later.
    DecodeARMFeaturesFromCPU(D, CPUName, Features);
  }

  if (CPUArg)
    checkARMCPUName(D, CPUArg, Args, CPUName, ArchName, ExtensionFeatures,
                    Triple, CPUArgFPUKind);

  // TODO Handle -mtune=. Suppress -Wunused-command-line-argument as a
  // longstanding behavior.
  (void)Args.getLastArg(options::OPT_mtune_EQ);

  // Honor -mfpu=. ClangAs gives preference to -Wa,-mfpu=.
  toolchain::ARM::FPUKind FPUKind = toolchain::ARM::FK_INVALID;
  const Arg *FPUArg = Args.getLastArg(options::OPT_mfpu_EQ);
  if (WaFPU) {
    if (FPUArg)
      D.Diag(language::Core::diag::warn_drv_unused_argument)
          << FPUArg->getAsString(Args);
    (void)getARMFPUFeatures(D, WaFPU->first, Args, WaFPU->second, Features);
  } else if (FPUArg) {
    FPUKind = getARMFPUFeatures(D, FPUArg, Args, FPUArg->getValue(), Features);
  } else if (Triple.isAndroid() && getARMSubArchVersionNumber(Triple) == 7) {
    const char *AndroidFPU = "neon";
    FPUKind = toolchain::ARM::parseFPU(AndroidFPU);
    if (!toolchain::ARM::getFPUFeatures(FPUKind, Features))
      D.Diag(language::Core::diag::err_drv_clang_unsupported)
          << std::string("-mfpu=") + AndroidFPU;
  } else if (ArchArgFPUKind != toolchain::ARM::FK_INVALID ||
             CPUArgFPUKind != toolchain::ARM::FK_INVALID) {
    FPUKind =
        CPUArgFPUKind != toolchain::ARM::FK_INVALID ? CPUArgFPUKind : ArchArgFPUKind;
    (void)toolchain::ARM::getFPUFeatures(FPUKind, Features);
  } else {
    std::string CPU = arm::getARMTargetCPU(CPUName, ArchName, Triple);
    bool Generic = CPU == "generic";
    if (Generic && (Triple.isOSWindows() || Triple.isOSDarwin()) &&
        getARMSubArchVersionNumber(Triple) >= 7) {
      FPUKind = toolchain::ARM::parseFPU("neon");
    } else {
      toolchain::ARM::ArchKind ArchKind =
          arm::getLLVMArchKindForARM(CPU, ArchName, Triple);
      FPUKind = toolchain::ARM::getDefaultFPU(CPU, ArchKind);
    }
    (void)toolchain::ARM::getFPUFeatures(FPUKind, Features);
  }

  // Now we've finished accumulating features from arch, cpu and fpu,
  // we can append the ones for architecture extensions that we
  // collected separately.
  Features.insert(std::end(Features),
                  std::begin(ExtensionFeatures), std::end(ExtensionFeatures));

  // Honor -mhwdiv=. ClangAs gives preference to -Wa,-mhwdiv=.
  const Arg *HDivArg = Args.getLastArg(options::OPT_mhwdiv_EQ);
  if (WaHDiv) {
    if (HDivArg)
      D.Diag(language::Core::diag::warn_drv_unused_argument)
          << HDivArg->getAsString(Args);
    getARMHWDivFeatures(D, WaHDiv->first, Args, WaHDiv->second, Features);
  } else if (HDivArg)
    getARMHWDivFeatures(D, HDivArg, Args, HDivArg->getValue(), Features);

  // Handle (arch-dependent) fp16fml/fullfp16 relationship.
  // Must happen before any features are disabled due to soft-float.
  // FIXME: this fp16fml option handling will be reimplemented after the
  // TargetParser rewrite.
  const auto ItRNoFullFP16 = std::find(Features.rbegin(), Features.rend(), "-fullfp16");
  const auto ItRFP16FML = std::find(Features.rbegin(), Features.rend(), "+fp16fml");
  if (Triple.getSubArch() == toolchain::Triple::SubArchType::ARMSubArch_v8_4a) {
    const auto ItRFullFP16  = std::find(Features.rbegin(), Features.rend(), "+fullfp16");
    if (ItRFullFP16 < ItRNoFullFP16 && ItRFullFP16 < ItRFP16FML) {
      // Only entangled feature that can be to the right of this +fullfp16 is -fp16fml.
      // Only append the +fp16fml if there is no -fp16fml after the +fullfp16.
      if (std::find(Features.rbegin(), ItRFullFP16, "-fp16fml") == ItRFullFP16)
        Features.push_back("+fp16fml");
    }
    else
      goto fp16_fml_fallthrough;
  }
  else {
fp16_fml_fallthrough:
    // In both of these cases, putting the 'other' feature on the end of the vector will
    // result in the same effect as placing it immediately after the current feature.
    if (ItRNoFullFP16 < ItRFP16FML)
      Features.push_back("-fp16fml");
    else if (ItRNoFullFP16 > ItRFP16FML)
      Features.push_back("+fullfp16");
  }

  // Setting -msoft-float/-mfloat-abi=soft, -mfpu=none, or adding +nofp to
  // -march/-mcpu effectively disables the FPU (GCC ignores the -mfpu options in
  // this case). Note that the ABI can also be set implicitly by the target
  // selected.
  bool HasFPRegs = true;
  if (ABI == arm::FloatABI::Soft) {
    toolchain::ARM::getFPUFeatures(toolchain::ARM::FK_NONE, Features);

    // Disable all features relating to hardware FP, not already disabled by the
    // above call.
    Features.insert(Features.end(),
                    {"-dotprod", "-fp16fml", "-bf16", "-mve", "-mve.fp"});
    HasFPRegs = false;
    FPUKind = toolchain::ARM::FK_NONE;
  } else if (FPUKind == toolchain::ARM::FK_NONE ||
             ArchArgFPUKind == toolchain::ARM::FK_NONE ||
             CPUArgFPUKind == toolchain::ARM::FK_NONE) {
    // -mfpu=none, -march=armvX+nofp or -mcpu=X+nofp is *very* similar to
    // -mfloat-abi=soft, only that it should not disable MVE-I. They disable the
    // FPU, but not the FPU registers, thus MVE-I, which depends only on the
    // latter, is still supported.
    Features.insert(Features.end(),
                    {"-dotprod", "-fp16fml", "-bf16", "-mve.fp"});
    HasFPRegs = hasIntegerMVE(Features);
    FPUKind = toolchain::ARM::FK_NONE;
  }
  if (!HasFPRegs)
    Features.emplace_back("-fpregs");

  // En/disable crc code generation.
  if (Arg *A = Args.getLastArg(options::OPT_mcrc, options::OPT_mnocrc)) {
    if (A->getOption().matches(options::OPT_mcrc))
      Features.push_back("+crc");
    else
      Features.push_back("-crc");
  }

  // Invalid value of the __ARM_FEATURE_MVE macro when an explicit -mfpu= option
  // disables MVE-FP -mfpu=fpv5-d16 or -mfpu=fpv5-sp-d16 disables the scalar
  // half-precision floating-point operations feature. Therefore, because the
  // M-profile Vector Extension (MVE) floating-point feature requires the scalar
  // half-precision floating-point operations, this option also disables the MVE
  // floating-point feature: -mve.fp
  if (FPUKind == toolchain::ARM::FK_FPV5_D16 || FPUKind == toolchain::ARM::FK_FPV5_SP_D16)
    Features.push_back("-mve.fp");

  // If SIMD has been disabled and the selected FPU supports NEON, then features
  // that rely on NEON instructions should also be disabled.
  bool HasSimd = false;
  const auto ItSimd =
      toolchain::find_if(toolchain::reverse(Features),
                    [](const StringRef F) { return F.contains("neon"); });
  const bool FPUSupportsNeon = (toolchain::ARM::FPUNames[FPUKind].NeonSupport ==
                                toolchain::ARM::NeonSupportLevel::Neon) ||
                               (toolchain::ARM::FPUNames[FPUKind].NeonSupport ==
                                toolchain::ARM::NeonSupportLevel::Crypto);
  if (ItSimd != Features.rend())
    HasSimd = ItSimd->starts_with("+");
  if (!HasSimd && FPUSupportsNeon)
    Features.insert(Features.end(),
                    {"-sha2", "-aes", "-crypto", "-dotprod", "-bf16", "-i8mm"});

  // For Arch >= ARMv8.0 && A or R profile:  crypto = sha2 + aes
  // Rather than replace within the feature vector, determine whether each
  // algorithm is enabled and append this to the end of the vector.
  // The algorithms can be controlled by their specific feature or the crypto
  // feature, so their status can be determined by the last occurance of
  // either in the vector. This allows one to supercede the other.
  // e.g. +crypto+noaes in -march/-mcpu should enable sha2, but not aes
  // FIXME: this needs reimplementation after the TargetParser rewrite
  bool HasSHA2 = false;
  bool HasAES = false;
  bool HasBF16 = false;
  bool HasDotprod = false;
  bool HasI8MM = false;
  const auto ItCrypto =
      toolchain::find_if(toolchain::reverse(Features), [](const StringRef F) {
        return F.contains("crypto");
      });
  const auto ItSHA2 =
      toolchain::find_if(toolchain::reverse(Features), [](const StringRef F) {
        return F.contains("crypto") || F.contains("sha2");
      });
  const auto ItAES =
      toolchain::find_if(toolchain::reverse(Features), [](const StringRef F) {
        return F.contains("crypto") || F.contains("aes");
      });
  const auto ItBF16 =
      toolchain::find_if(toolchain::reverse(Features),
                    [](const StringRef F) { return F.contains("bf16"); });
  const auto ItDotprod =
      toolchain::find_if(toolchain::reverse(Features),
                    [](const StringRef F) { return F.contains("dotprod"); });
  const auto ItI8MM =
      toolchain::find_if(toolchain::reverse(Features),
                    [](const StringRef F) { return F.contains("i8mm"); });
  if (ItSHA2 != Features.rend())
    HasSHA2 = ItSHA2->starts_with("+");
  if (ItAES != Features.rend())
    HasAES = ItAES->starts_with("+");
  if (ItBF16 != Features.rend())
    HasBF16 = ItBF16->starts_with("+");
  if (ItDotprod != Features.rend())
    HasDotprod = ItDotprod->starts_with("+");
  if (ItI8MM != Features.rend())
    HasI8MM = ItI8MM->starts_with("+");
  if (ItCrypto != Features.rend()) {
    if (HasSHA2 && HasAES)
      Features.push_back("+crypto");
    else
      Features.push_back("-crypto");
    if (HasSHA2)
      Features.push_back("+sha2");
    else
      Features.push_back("-sha2");
    if (HasAES)
      Features.push_back("+aes");
    else
      Features.push_back("-aes");
  }
  // If any of these features are enabled, NEON should also be enabled.
  if (HasAES || HasSHA2 || HasBF16 || HasDotprod || HasI8MM)
    Features.push_back("+neon");

  if (HasSHA2 || HasAES) {
    StringRef ArchSuffix = arm::getLLVMArchSuffixForARM(
        arm::getARMTargetCPU(CPUName, ArchName, Triple), ArchName, Triple);
    toolchain::ARM::ProfileKind ArchProfile =
        toolchain::ARM::parseArchProfile(ArchSuffix);
    if (!((toolchain::ARM::parseArchVersion(ArchSuffix) >= 8) &&
          (ArchProfile == toolchain::ARM::ProfileKind::A ||
           ArchProfile == toolchain::ARM::ProfileKind::R))) {
      if (HasSHA2)
        D.Diag(language::Core::diag::warn_target_unsupported_extension)
            << "sha2"
            << toolchain::ARM::getArchName(toolchain::ARM::parseArch(ArchSuffix));
      if (HasAES)
        D.Diag(language::Core::diag::warn_target_unsupported_extension)
            << "aes"
            << toolchain::ARM::getArchName(toolchain::ARM::parseArch(ArchSuffix));
      // With -fno-integrated-as -mfpu=crypto-neon-fp-armv8 some assemblers such
      // as the GNU assembler will permit the use of crypto instructions as the
      // fpu will override the architecture. We keep the crypto feature in this
      // case to preserve compatibility. In all other cases we remove the crypto
      // feature.
      if (!Args.hasArg(options::OPT_fno_integrated_as)) {
        Features.push_back("-sha2");
        Features.push_back("-aes");
      }
    }
  }

  // Propagate frame-chain model selection
  if (Arg *A = Args.getLastArg(options::OPT_mframe_chain)) {
    StringRef FrameChainOption = A->getValue();
    if (FrameChainOption.starts_with("aapcs"))
      Features.push_back("+aapcs-frame-chain");
  }

  // CMSE: Check for target 8M (for -mcmse to be applicable) is performed later.
  if (Args.getLastArg(options::OPT_mcmse))
    Features.push_back("+8msecext");

  if (Arg *A = Args.getLastArg(options::OPT_mfix_cmse_cve_2021_35465,
                               options::OPT_mno_fix_cmse_cve_2021_35465)) {
    if (!Args.getLastArg(options::OPT_mcmse))
      D.Diag(diag::err_opt_not_valid_without_opt)
          << A->getOption().getName() << "-mcmse";

    if (A->getOption().matches(options::OPT_mfix_cmse_cve_2021_35465))
      Features.push_back("+fix-cmse-cve-2021-35465");
    else
      Features.push_back("-fix-cmse-cve-2021-35465");
  }

  // This also handles the -m(no-)fix-cortex-a72-1655431 arguments via aliases.
  if (Arg *A = Args.getLastArg(options::OPT_mfix_cortex_a57_aes_1742098,
                               options::OPT_mno_fix_cortex_a57_aes_1742098)) {
    if (A->getOption().matches(options::OPT_mfix_cortex_a57_aes_1742098)) {
      Features.push_back("+fix-cortex-a57-aes-1742098");
    } else {
      Features.push_back("-fix-cortex-a57-aes-1742098");
    }
  }

  // Look for the last occurrence of -mlong-calls or -mno-long-calls. If
  // neither options are specified, see if we are compiling for kernel/kext and
  // decide whether to pass "+long-calls" based on the OS and its version.
  if (Arg *A = Args.getLastArg(options::OPT_mlong_calls,
                               options::OPT_mno_long_calls)) {
    if (A->getOption().matches(options::OPT_mlong_calls))
      Features.push_back("+long-calls");
  } else if (KernelOrKext && (!Triple.isiOS() || Triple.isOSVersionLT(6)) &&
             !Triple.isWatchOS() && !Triple.isXROS()) {
    Features.push_back("+long-calls");
  }

  // Generate execute-only output (no data access to code sections).
  // This only makes sense for the compiler, not for the assembler.
  // It's not needed for multilib selection and may hide an unused
  // argument diagnostic if the code is always run.
  if (!ForAS && !ForMultilib) {
    // Supported only on ARMv6T2 and ARMv7 and above.
    // Cannot be combined with -mno-movt.
    if (Arg *A = Args.getLastArg(options::OPT_mexecute_only, options::OPT_mno_execute_only)) {
      if (A->getOption().matches(options::OPT_mexecute_only)) {
        if (getARMSubArchVersionNumber(Triple) < 7 &&
            toolchain::ARM::parseArch(Triple.getArchName()) != toolchain::ARM::ArchKind::ARMV6T2 &&
            toolchain::ARM::parseArch(Triple.getArchName()) != toolchain::ARM::ArchKind::ARMV6M)
              D.Diag(diag::err_target_unsupported_execute_only) << Triple.getArchName();
        else if (toolchain::ARM::parseArch(Triple.getArchName()) == toolchain::ARM::ArchKind::ARMV6M) {
          if (Arg *PIArg = Args.getLastArg(options::OPT_fropi, options::OPT_frwpi,
                                           options::OPT_fpic, options::OPT_fpie,
                                           options::OPT_fPIC, options::OPT_fPIE))
            D.Diag(diag::err_opt_not_valid_with_opt_on_target)
                << A->getAsString(Args) << PIArg->getAsString(Args) << Triple.getArchName();
        } else if (Arg *B = Args.getLastArg(options::OPT_mno_movt))
          D.Diag(diag::err_opt_not_valid_with_opt)
              << A->getAsString(Args) << B->getAsString(Args);
        Features.push_back("+execute-only");
      }
    }
  }

  if (Arg *A = Args.getLastArg(options::OPT_mno_unaligned_access,
                                      options::OPT_munaligned_access,
                                      options::OPT_mstrict_align,
                                      options::OPT_mno_strict_align)) {
    // Kernel code has more strict alignment requirements.
    if (KernelOrKext ||
        A->getOption().matches(options::OPT_mno_unaligned_access) ||
        A->getOption().matches(options::OPT_mstrict_align)) {
      Features.push_back("+strict-align");
    } else {
      // No v6M core supports unaligned memory access (v6M ARM ARM A3.2).
      if (Triple.getSubArch() == toolchain::Triple::SubArchType::ARMSubArch_v6m)
        D.Diag(diag::err_target_unsupported_unaligned) << "v6m";
      // v8M Baseline follows on from v6M, so doesn't support unaligned memory
      // access either.
      else if (Triple.getSubArch() == toolchain::Triple::SubArchType::ARMSubArch_v8m_baseline)
        D.Diag(diag::err_target_unsupported_unaligned) << "v8m.base";
    }
  } else {
    // Assume pre-ARMv6 doesn't support unaligned accesses.
    //
    // ARMv6 may or may not support unaligned accesses depending on the
    // SCTLR.U bit, which is architecture-specific. We assume ARMv6
    // Darwin and NetBSD targets support unaligned accesses, and others don't.
    //
    // ARMv7 always has SCTLR.U set to 1, but it has a new SCTLR.A bit which
    // raises an alignment fault on unaligned accesses. Assume ARMv7+ supports
    // unaligned accesses, except ARMv6-M, and ARMv8-M without the Main
    // Extension. This aligns with the default behavior of ARM's downstream
    // versions of GCC and Clang.
    //
    // Users can change the default behavior via -m[no-]unaliged-access.
    int VersionNum = getARMSubArchVersionNumber(Triple);
    if (Triple.isOSDarwin() || Triple.isOSNetBSD()) {
      if (VersionNum < 6 ||
          Triple.getSubArch() == toolchain::Triple::SubArchType::ARMSubArch_v6m)
        Features.push_back("+strict-align");
    } else if (Triple.getVendor() == toolchain::Triple::Apple &&
               Triple.isOSBinFormatMachO()) {
      // Firmwares on Apple platforms are strict-align by default.
      Features.push_back("+strict-align");
    } else if (VersionNum < 7 ||
               Triple.getSubArch() ==
                   toolchain::Triple::SubArchType::ARMSubArch_v6m ||
               Triple.getSubArch() ==
                   toolchain::Triple::SubArchType::ARMSubArch_v8m_baseline) {
      Features.push_back("+strict-align");
    }
  }

  // toolchain does not support reserving registers in general. There is support
  // for reserving r9 on ARM though (defined as a platform-specific register
  // in ARM EABI).
  if (Args.hasArg(options::OPT_ffixed_r9))
    Features.push_back("+reserve-r9");

  // The kext linker doesn't know how to deal with movw/movt.
  if (KernelOrKext || Args.hasArg(options::OPT_mno_movt))
    Features.push_back("+no-movt");

  if (Args.hasArg(options::OPT_mno_neg_immediates))
    Features.push_back("+no-neg-immediates");

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

    if (EnableRetBr || EnableBlr)
      if (!(isARMAProfile(Triple) && getARMSubArchVersionNumber(Triple) >= 7))
        D.Diag(diag::err_sls_hardening_arm_not_supported)
            << Scope << A->getAsString(Args);

    if (EnableRetBr)
      Features.push_back("+harden-sls-retbr");
    if (EnableBlr)
      Features.push_back("+harden-sls-blr");
    if (DisableComdat) {
      Features.push_back("+harden-sls-nocomdat");
    }
  }

  if (Args.getLastArg(options::OPT_mno_bti_at_return_twice))
    Features.push_back("+no-bti-at-return-twice");

  checkARMFloatABI(D, Args, HasFPRegs);

  return FPUKind;
}

std::string arm::getARMArch(StringRef Arch, const toolchain::Triple &Triple) {
  std::string MArch;
  if (!Arch.empty())
    MArch = std::string(Arch);
  else
    MArch = std::string(Triple.getArchName());
  MArch = StringRef(MArch).split("+").first.lower();

  // Handle -march=native.
  if (MArch == "native") {
    std::string CPU = std::string(toolchain::sys::getHostCPUName());
    if (CPU != "generic") {
      // Translate the native cpu into the architecture suffix for that CPU.
      StringRef Suffix = arm::getLLVMArchSuffixForARM(CPU, MArch, Triple);
      // If there is no valid architecture suffix for this CPU we don't know how
      // to handle it, so return no architecture.
      if (Suffix.empty())
        MArch = "";
      else
        MArch = std::string("arm") + Suffix.str();
    }
  }

  return MArch;
}

/// Get the (LLVM) name of the minimum ARM CPU for the arch we are targeting.
StringRef arm::getARMCPUForMArch(StringRef Arch, const toolchain::Triple &Triple) {
  std::string MArch = getARMArch(Arch, Triple);
  // getARMCPUForArch defaults to the triple if MArch is empty, but empty MArch
  // here means an -march=native that we can't handle, so instead return no CPU.
  if (MArch.empty())
    return StringRef();

  // We need to return an empty string here on invalid MArch values as the
  // various places that call this function can't cope with a null result.
  return toolchain::ARM::getARMCPUForArch(Triple, MArch);
}

/// getARMTargetCPU - Get the (LLVM) name of the ARM cpu we are targeting.
std::string arm::getARMTargetCPU(StringRef CPU, StringRef Arch,
                                 const toolchain::Triple &Triple) {
  // FIXME: Warn on inconsistent use of -mcpu and -march.
  // If we have -mcpu=, use that.
  if (!CPU.empty()) {
    std::string MCPU = StringRef(CPU).split("+").first.lower();
    // Handle -mcpu=native.
    if (MCPU == "native")
      return std::string(toolchain::sys::getHostCPUName());
    else
      return MCPU;
  }

  return std::string(getARMCPUForMArch(Arch, Triple));
}

/// getLLVMArchSuffixForARM - Get the LLVM ArchKind value to use for a
/// particular CPU (or Arch, if CPU is generic). This is needed to
/// pass to functions like toolchain::ARM::getDefaultFPU which need an
/// ArchKind as well as a CPU name.
toolchain::ARM::ArchKind arm::getLLVMArchKindForARM(StringRef CPU, StringRef Arch,
                                               const toolchain::Triple &Triple) {
  toolchain::ARM::ArchKind ArchKind;
  if (CPU == "generic" || CPU.empty()) {
    std::string ARMArch = tools::arm::getARMArch(Arch, Triple);
    ArchKind = toolchain::ARM::parseArch(ARMArch);
    if (ArchKind == toolchain::ARM::ArchKind::INVALID)
      // In case of generic Arch, i.e. "arm",
      // extract arch from default cpu of the Triple
      ArchKind =
          toolchain::ARM::parseCPUArch(toolchain::ARM::getARMCPUForArch(Triple, ARMArch));
  } else {
    // FIXME: horrible hack to get around the fact that Cortex-A7 is only an
    // armv7k triple if it's actually been specified via "-arch armv7k".
    ArchKind = (Arch == "armv7k" || Arch == "thumbv7k")
                          ? toolchain::ARM::ArchKind::ARMV7K
                          : toolchain::ARM::parseCPUArch(CPU);
  }
  return ArchKind;
}

/// getLLVMArchSuffixForARM - Get the LLVM arch name to use for a particular
/// CPU  (or Arch, if CPU is generic).
// FIXME: This is redundant with -mcpu, why does LLVM use this.
StringRef arm::getLLVMArchSuffixForARM(StringRef CPU, StringRef Arch,
                                       const toolchain::Triple &Triple) {
  toolchain::ARM::ArchKind ArchKind = getLLVMArchKindForARM(CPU, Arch, Triple);
  if (ArchKind == toolchain::ARM::ArchKind::INVALID)
    return "";
  return toolchain::ARM::getSubArch(ArchKind);
}

void arm::appendBE8LinkFlag(const ArgList &Args, ArgStringList &CmdArgs,
                            const toolchain::Triple &Triple) {
  if (Args.hasArg(options::OPT_r))
    return;

  // ARMv7 (and later) and ARMv6-M do not support BE-32, so instruct the linker
  // to generate BE-8 executables.
  if (arm::getARMSubArchVersionNumber(Triple) >= 7 || arm::isARMMProfile(Triple))
    CmdArgs.push_back("--be8");
}
