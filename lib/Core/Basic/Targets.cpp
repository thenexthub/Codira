//===--- Targets.cpp - Implement target feature support -------------------===//
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
//
// This file implements construction of a TargetInfo object from a
// target triple.
//
//===----------------------------------------------------------------------===//

#include "Targets.h"

#include "Targets/AArch64.h"
#include "Targets/AMDGPU.h"
#include "Targets/ARC.h"
#include "Targets/ARM.h"
#include "Targets/AVR.h"
#include "Targets/BPF.h"
#include "Targets/CSKY.h"
#include "Targets/DirectX.h"
#include "Targets/Hexagon.h"
#include "Targets/Lanai.h"
#include "Targets/LoongArch.h"
#include "Targets/M68k.h"
#include "Targets/MSP430.h"
#include "Targets/Mips.h"
#include "Targets/NVPTX.h"
#include "Targets/OSTargets.h"
#include "Targets/PPC.h"
#include "Targets/RISCV.h"
#include "Targets/SPIR.h"
#include "Targets/Sparc.h"
#include "Targets/SystemZ.h"
#include "Targets/TCE.h"
#include "Targets/VE.h"
#include "Targets/WebAssembly.h"
#include "Targets/X86.h"
#include "Targets/XCore.h"
#include "Targets/Xtensa.h"
#include "language/Core/Basic/Diagnostic.h"
#include "language/Core/Basic/DiagnosticFrontend.h"
#include "toolchain/ADT/StringExtras.h"
#include "toolchain/TargetParser/Triple.h"

using namespace language::Core;

namespace language::Core {
namespace targets {
//===----------------------------------------------------------------------===//
//  Common code shared among targets.
//===----------------------------------------------------------------------===//

/// DefineStd - Define a macro name and standard variants.  For example if
/// MacroName is "unix", then this will define "__unix", "__unix__", and "unix"
/// when in GNU mode.
void DefineStd(MacroBuilder &Builder, StringRef MacroName,
               const LangOptions &Opts) {
  assert(MacroName[0] != '_' && "Identifier should be in the user's namespace");

  // If in GNU mode (e.g. -std=gnu99 but not -std=c99) define the raw identifier
  // in the user's namespace.
  if (Opts.GNUMode)
    Builder.defineMacro(MacroName);

  // Define __unix.
  Builder.defineMacro("__" + MacroName);

  // Define __unix__.
  Builder.defineMacro("__" + MacroName + "__");
}

void defineCPUMacros(MacroBuilder &Builder, StringRef CPUName, bool Tuning) {
  Builder.defineMacro("__" + CPUName);
  Builder.defineMacro("__" + CPUName + "__");
  if (Tuning)
    Builder.defineMacro("__tune_" + CPUName + "__");
}

void addCygMingDefines(const LangOptions &Opts, MacroBuilder &Builder) {
  // Mingw and cygwin define __declspec(a) to __attribute__((a)).  Clang
  // supports __declspec natively under -fdeclspec (also enabled with
  // -fms-extensions), but we define a no-op __declspec macro anyway for
  // pre-processor compatibility.
  if (Opts.DeclSpecKeyword)
    Builder.defineMacro("__declspec", "__declspec");
  else
    Builder.defineMacro("__declspec(a)", "__attribute__((a))");

  if (!Opts.MicrosoftExt) {
    // Provide macros for all the calling convention keywords.  Provide both
    // single and double underscore prefixed variants.  These are available on
    // x64 as well as x86, even though they have no effect.
    const char *CCs[] = {"cdecl", "stdcall", "fastcall", "thiscall", "pascal"};
    for (const char *CC : CCs) {
      std::string GCCSpelling = "__attribute__((__";
      GCCSpelling += CC;
      GCCSpelling += "__))";
      Builder.defineMacro(Twine("_") + CC, GCCSpelling);
      Builder.defineMacro(Twine("__") + CC, GCCSpelling);
    }
  }
}

//===----------------------------------------------------------------------===//
// Driver code
//===----------------------------------------------------------------------===//

std::unique_ptr<TargetInfo> AllocateTarget(const toolchain::Triple &Triple,
                                           const TargetOptions &Opts) {
  toolchain::Triple::OSType os = Triple.getOS();

  switch (Triple.getArch()) {
  default:
    return nullptr;

  case toolchain::Triple::arc:
    return std::make_unique<ARCTargetInfo>(Triple, Opts);

  case toolchain::Triple::xcore:
    return std::make_unique<XCoreTargetInfo>(Triple, Opts);

  case toolchain::Triple::hexagon:
    if (os == toolchain::Triple::Linux &&
        Triple.getEnvironment() == toolchain::Triple::Musl)
      return std::make_unique<LinuxTargetInfo<HexagonTargetInfo>>(Triple, Opts);
    return std::make_unique<HexagonTargetInfo>(Triple, Opts);

  case toolchain::Triple::lanai:
    return std::make_unique<LanaiTargetInfo>(Triple, Opts);

  case toolchain::Triple::aarch64_32:
    if (Triple.isOSDarwin())
      return std::make_unique<DarwinAArch64TargetInfo>(Triple, Opts);
    else if (Triple.isAppleMachO())
      return std::make_unique<AppleMachOAArch64TargetInfo>(Triple, Opts);

    return nullptr;
  case toolchain::Triple::aarch64:
    if (Triple.isOSDarwin())
      return std::make_unique<DarwinAArch64TargetInfo>(Triple, Opts);
    else if (Triple.isAppleMachO())
      return std::make_unique<AppleMachOAArch64TargetInfo>(Triple, Opts);

    switch (os) {
    case toolchain::Triple::FreeBSD:
      return std::make_unique<FreeBSDTargetInfo<AArch64leTargetInfo>>(Triple,
                                                                      Opts);
    case toolchain::Triple::Fuchsia:
      return std::make_unique<FuchsiaTargetInfo<AArch64leTargetInfo>>(Triple,
                                                                      Opts);
    case toolchain::Triple::Haiku:
      return std::make_unique<HaikuTargetInfo<AArch64leTargetInfo>>(Triple,
                                                                    Opts);
    case toolchain::Triple::Linux:
      switch (Triple.getEnvironment()) {
      default:
        return std::make_unique<LinuxTargetInfo<AArch64leTargetInfo>>(Triple,
                                                                      Opts);
      case toolchain::Triple::OpenHOS:
        return std::make_unique<OHOSTargetInfo<AArch64leTargetInfo>>(Triple,
                                                                     Opts);
      }
    case toolchain::Triple::Managarm:
      return std::make_unique<ManagarmTargetInfo<AArch64leTargetInfo>>(Triple,
                                                                       Opts);
    case toolchain::Triple::NetBSD:
      return std::make_unique<NetBSDTargetInfo<AArch64leTargetInfo>>(Triple,
                                                                     Opts);
    case toolchain::Triple::OpenBSD:
      return std::make_unique<OpenBSDTargetInfo<AArch64leTargetInfo>>(Triple,
                                                                      Opts);
    case toolchain::Triple::Win32:
      switch (Triple.getEnvironment()) {
      case toolchain::Triple::GNU:
        return std::make_unique<MinGWARM64TargetInfo>(Triple, Opts);
      case toolchain::Triple::MSVC:
      default: // Assume MSVC for unknown environments
        return std::make_unique<MicrosoftARM64TargetInfo>(Triple, Opts);
      }
    default:
      return std::make_unique<AArch64leTargetInfo>(Triple, Opts);
    }

  case toolchain::Triple::aarch64_be:
    switch (os) {
    case toolchain::Triple::FreeBSD:
      return std::make_unique<FreeBSDTargetInfo<AArch64beTargetInfo>>(Triple,
                                                                      Opts);
    case toolchain::Triple::Fuchsia:
      return std::make_unique<FuchsiaTargetInfo<AArch64beTargetInfo>>(Triple,
                                                                      Opts);
    case toolchain::Triple::Linux:
      return std::make_unique<LinuxTargetInfo<AArch64beTargetInfo>>(Triple,
                                                                    Opts);
    case toolchain::Triple::NetBSD:
      return std::make_unique<NetBSDTargetInfo<AArch64beTargetInfo>>(Triple,
                                                                     Opts);
    default:
      return std::make_unique<AArch64beTargetInfo>(Triple, Opts);
    }

  case toolchain::Triple::arm:
  case toolchain::Triple::thumb:
    if (Triple.isOSBinFormatMachO())
      return std::make_unique<DarwinARMTargetInfo>(Triple, Opts);

    switch (os) {
    case toolchain::Triple::Linux:
      switch (Triple.getEnvironment()) {
      default:
        return std::make_unique<LinuxTargetInfo<ARMleTargetInfo>>(Triple, Opts);
      case toolchain::Triple::OpenHOS:
        return std::make_unique<OHOSTargetInfo<ARMleTargetInfo>>(Triple, Opts);
      }
    case toolchain::Triple::LiteOS:
      return std::make_unique<OHOSTargetInfo<ARMleTargetInfo>>(Triple, Opts);
    case toolchain::Triple::FreeBSD:
      return std::make_unique<FreeBSDTargetInfo<ARMleTargetInfo>>(Triple, Opts);
    case toolchain::Triple::NetBSD:
      return std::make_unique<NetBSDTargetInfo<ARMleTargetInfo>>(Triple, Opts);
    case toolchain::Triple::OpenBSD:
      return std::make_unique<OpenBSDTargetInfo<ARMleTargetInfo>>(Triple, Opts);
    case toolchain::Triple::RTEMS:
      return std::make_unique<RTEMSTargetInfo<ARMleTargetInfo>>(Triple, Opts);
    case toolchain::Triple::Haiku:
      return std::make_unique<HaikuTargetInfo<ARMleTargetInfo>>(Triple, Opts);
    case toolchain::Triple::Win32:
      switch (Triple.getEnvironment()) {
      case toolchain::Triple::Cygnus:
        return std::make_unique<CygwinARMTargetInfo>(Triple, Opts);
      case toolchain::Triple::GNU:
        return std::make_unique<MinGWARMTargetInfo>(Triple, Opts);
      case toolchain::Triple::Itanium:
        return std::make_unique<ItaniumWindowsARMleTargetInfo>(Triple, Opts);
      case toolchain::Triple::MSVC:
      default: // Assume MSVC for unknown environments
        return std::make_unique<MicrosoftARMleTargetInfo>(Triple, Opts);
      }
    default:
      return std::make_unique<ARMleTargetInfo>(Triple, Opts);
    }

  case toolchain::Triple::armeb:
  case toolchain::Triple::thumbeb:
    if (Triple.isOSDarwin())
      return std::make_unique<DarwinARMTargetInfo>(Triple, Opts);
    else if (Triple.isAppleMachO())
      return std::make_unique<AppleMachOARMTargetInfo>(Triple, Opts);

    switch (os) {
    case toolchain::Triple::Linux:
      return std::make_unique<LinuxTargetInfo<ARMbeTargetInfo>>(Triple, Opts);
    case toolchain::Triple::NetBSD:
      return std::make_unique<NetBSDTargetInfo<ARMbeTargetInfo>>(Triple, Opts);
    case toolchain::Triple::RTEMS:
      return std::make_unique<RTEMSTargetInfo<ARMbeTargetInfo>>(Triple, Opts);
    default:
      return std::make_unique<ARMbeTargetInfo>(Triple, Opts);
    }

  case toolchain::Triple::avr:
    return std::make_unique<AVRTargetInfo>(Triple, Opts);
  case toolchain::Triple::bpfeb:
  case toolchain::Triple::bpfel:
    return std::make_unique<BPFTargetInfo>(Triple, Opts);

  case toolchain::Triple::msp430:
    return std::make_unique<MSP430TargetInfo>(Triple, Opts);

  case toolchain::Triple::mips:
    switch (os) {
    case toolchain::Triple::Linux:
      return std::make_unique<LinuxTargetInfo<MipsTargetInfo>>(Triple, Opts);
    case toolchain::Triple::RTEMS:
      return std::make_unique<RTEMSTargetInfo<MipsTargetInfo>>(Triple, Opts);
    case toolchain::Triple::FreeBSD:
      return std::make_unique<FreeBSDTargetInfo<MipsTargetInfo>>(Triple, Opts);
    case toolchain::Triple::NetBSD:
      return std::make_unique<NetBSDTargetInfo<MipsTargetInfo>>(Triple, Opts);
    default:
      return std::make_unique<MipsTargetInfo>(Triple, Opts);
    }

  case toolchain::Triple::mipsel:
    switch (os) {
    case toolchain::Triple::Linux:
      switch (Triple.getEnvironment()) {
      default:
        return std::make_unique<LinuxTargetInfo<MipsTargetInfo>>(Triple, Opts);
      case toolchain::Triple::OpenHOS:
        return std::make_unique<OHOSTargetInfo<MipsTargetInfo>>(Triple, Opts);
      }
    case toolchain::Triple::RTEMS:
      return std::make_unique<RTEMSTargetInfo<MipsTargetInfo>>(Triple, Opts);
    case toolchain::Triple::FreeBSD:
      return std::make_unique<FreeBSDTargetInfo<MipsTargetInfo>>(Triple, Opts);
    case toolchain::Triple::NetBSD:
      return std::make_unique<NetBSDTargetInfo<MipsTargetInfo>>(Triple, Opts);
    case toolchain::Triple::Win32:
      switch (Triple.getEnvironment()) {
      case toolchain::Triple::GNU:
        return std::make_unique<MinGWMipsTargetInfo>(Triple, Opts);
      case toolchain::Triple::MSVC:
      default: // Assume MSVC for unknown environments
        return std::make_unique<MicrosoftMipsTargetInfo>(Triple, Opts);
      }
    default:
      return std::make_unique<MipsTargetInfo>(Triple, Opts);
    }

  case toolchain::Triple::mips64:
    switch (os) {
    case toolchain::Triple::Linux:
      return std::make_unique<LinuxTargetInfo<MipsTargetInfo>>(Triple, Opts);
    case toolchain::Triple::RTEMS:
      return std::make_unique<RTEMSTargetInfo<MipsTargetInfo>>(Triple, Opts);
    case toolchain::Triple::FreeBSD:
      return std::make_unique<FreeBSDTargetInfo<MipsTargetInfo>>(Triple, Opts);
    case toolchain::Triple::NetBSD:
      return std::make_unique<NetBSDTargetInfo<MipsTargetInfo>>(Triple, Opts);
    case toolchain::Triple::OpenBSD:
      return std::make_unique<OpenBSDTargetInfo<MipsTargetInfo>>(Triple, Opts);
    default:
      return std::make_unique<MipsTargetInfo>(Triple, Opts);
    }

  case toolchain::Triple::mips64el:
    switch (os) {
    case toolchain::Triple::Linux:
      return std::make_unique<LinuxTargetInfo<MipsTargetInfo>>(Triple, Opts);
    case toolchain::Triple::RTEMS:
      return std::make_unique<RTEMSTargetInfo<MipsTargetInfo>>(Triple, Opts);
    case toolchain::Triple::FreeBSD:
      return std::make_unique<FreeBSDTargetInfo<MipsTargetInfo>>(Triple, Opts);
    case toolchain::Triple::NetBSD:
      return std::make_unique<NetBSDTargetInfo<MipsTargetInfo>>(Triple, Opts);
    case toolchain::Triple::OpenBSD:
      return std::make_unique<OpenBSDTargetInfo<MipsTargetInfo>>(Triple, Opts);
    default:
      return std::make_unique<MipsTargetInfo>(Triple, Opts);
    }

  case toolchain::Triple::m68k:
    switch (os) {
    case toolchain::Triple::Linux:
      return std::make_unique<LinuxTargetInfo<M68kTargetInfo>>(Triple, Opts);
    case toolchain::Triple::NetBSD:
      return std::make_unique<NetBSDTargetInfo<M68kTargetInfo>>(Triple, Opts);
    default:
      return std::make_unique<M68kTargetInfo>(Triple, Opts);
    }

  case toolchain::Triple::ppc:
    switch (os) {
    case toolchain::Triple::Linux:
      return std::make_unique<LinuxTargetInfo<PPC32TargetInfo>>(Triple, Opts);
    case toolchain::Triple::FreeBSD:
      return std::make_unique<FreeBSDTargetInfo<PPC32TargetInfo>>(Triple, Opts);
    case toolchain::Triple::NetBSD:
      return std::make_unique<NetBSDTargetInfo<PPC32TargetInfo>>(Triple, Opts);
    case toolchain::Triple::OpenBSD:
      return std::make_unique<OpenBSDTargetInfo<PPC32TargetInfo>>(Triple, Opts);
    case toolchain::Triple::RTEMS:
      return std::make_unique<RTEMSTargetInfo<PPC32TargetInfo>>(Triple, Opts);
    case toolchain::Triple::AIX:
      return std::make_unique<AIXPPC32TargetInfo>(Triple, Opts);
    default:
      return std::make_unique<PPC32TargetInfo>(Triple, Opts);
    }

  case toolchain::Triple::ppcle:
    switch (os) {
    case toolchain::Triple::Linux:
      return std::make_unique<LinuxTargetInfo<PPC32TargetInfo>>(Triple, Opts);
    case toolchain::Triple::FreeBSD:
      return std::make_unique<FreeBSDTargetInfo<PPC32TargetInfo>>(Triple, Opts);
    default:
      return std::make_unique<PPC32TargetInfo>(Triple, Opts);
    }

  case toolchain::Triple::ppc64:
    switch (os) {
    case toolchain::Triple::Linux:
      return std::make_unique<LinuxTargetInfo<PPC64TargetInfo>>(Triple, Opts);
    case toolchain::Triple::Lv2:
      return std::make_unique<PS3PPUTargetInfo<PPC64TargetInfo>>(Triple, Opts);
    case toolchain::Triple::FreeBSD:
      return std::make_unique<FreeBSDTargetInfo<PPC64TargetInfo>>(Triple, Opts);
    case toolchain::Triple::NetBSD:
      return std::make_unique<NetBSDTargetInfo<PPC64TargetInfo>>(Triple, Opts);
    case toolchain::Triple::OpenBSD:
      return std::make_unique<OpenBSDTargetInfo<PPC64TargetInfo>>(Triple, Opts);
    case toolchain::Triple::AIX:
      return std::make_unique<AIXPPC64TargetInfo>(Triple, Opts);
    default:
      return std::make_unique<PPC64TargetInfo>(Triple, Opts);
    }

  case toolchain::Triple::ppc64le:
    switch (os) {
    case toolchain::Triple::Linux:
      return std::make_unique<LinuxTargetInfo<PPC64TargetInfo>>(Triple, Opts);
    case toolchain::Triple::FreeBSD:
      return std::make_unique<FreeBSDTargetInfo<PPC64TargetInfo>>(Triple, Opts);
    case toolchain::Triple::NetBSD:
      return std::make_unique<NetBSDTargetInfo<PPC64TargetInfo>>(Triple, Opts);
    case toolchain::Triple::OpenBSD:
      return std::make_unique<OpenBSDTargetInfo<PPC64TargetInfo>>(Triple, Opts);
    default:
      return std::make_unique<PPC64TargetInfo>(Triple, Opts);
    }

  case toolchain::Triple::nvptx:
    return std::make_unique<NVPTXTargetInfo>(Triple, Opts,
                                             /*TargetPointerWidth=*/32);
  case toolchain::Triple::nvptx64:
    return std::make_unique<NVPTXTargetInfo>(Triple, Opts,
                                             /*TargetPointerWidth=*/64);

  case toolchain::Triple::amdgcn:
  case toolchain::Triple::r600:
    return std::make_unique<AMDGPUTargetInfo>(Triple, Opts);

  case toolchain::Triple::riscv32:
    switch (os) {
    case toolchain::Triple::NetBSD:
      return std::make_unique<NetBSDTargetInfo<RISCV32TargetInfo>>(Triple,
                                                                   Opts);
    case toolchain::Triple::Linux:
      return std::make_unique<LinuxTargetInfo<RISCV32TargetInfo>>(Triple, Opts);
    default:
      return std::make_unique<RISCV32TargetInfo>(Triple, Opts);
    }

  case toolchain::Triple::riscv64:
    switch (os) {
    case toolchain::Triple::FreeBSD:
      return std::make_unique<FreeBSDTargetInfo<RISCV64TargetInfo>>(Triple,
                                                                    Opts);
    case toolchain::Triple::NetBSD:
      return std::make_unique<NetBSDTargetInfo<RISCV64TargetInfo>>(Triple,
                                                                   Opts);
    case toolchain::Triple::OpenBSD:
      return std::make_unique<OpenBSDTargetInfo<RISCV64TargetInfo>>(Triple,
                                                                    Opts);
    case toolchain::Triple::Fuchsia:
      return std::make_unique<FuchsiaTargetInfo<RISCV64TargetInfo>>(Triple,
                                                                    Opts);
    case toolchain::Triple::Haiku:
      return std::make_unique<HaikuTargetInfo<RISCV64TargetInfo>>(Triple,
                                                                  Opts);
    case toolchain::Triple::Linux:
      switch (Triple.getEnvironment()) {
      default:
        return std::make_unique<LinuxTargetInfo<RISCV64TargetInfo>>(Triple,
                                                                    Opts);
      case toolchain::Triple::OpenHOS:
        return std::make_unique<OHOSTargetInfo<RISCV64TargetInfo>>(Triple,
                                                                   Opts);
      }
    case toolchain::Triple::Managarm:
      return std::make_unique<ManagarmTargetInfo<RISCV64TargetInfo>>(Triple,
                                                                     Opts);
    default:
      return std::make_unique<RISCV64TargetInfo>(Triple, Opts);
    }

  case toolchain::Triple::sparc:
    switch (os) {
    case toolchain::Triple::Linux:
      return std::make_unique<LinuxTargetInfo<SparcV8TargetInfo>>(Triple, Opts);
    case toolchain::Triple::Solaris:
      return std::make_unique<SolarisTargetInfo<SparcV8TargetInfo>>(Triple,
                                                                    Opts);
    case toolchain::Triple::NetBSD:
      return std::make_unique<NetBSDTargetInfo<SparcV8TargetInfo>>(Triple,
                                                                   Opts);
    case toolchain::Triple::RTEMS:
      return std::make_unique<RTEMSTargetInfo<SparcV8TargetInfo>>(Triple, Opts);
    default:
      return std::make_unique<SparcV8TargetInfo>(Triple, Opts);
    }

  case toolchain::Triple::sparcel:
    switch (os) {
    case toolchain::Triple::Linux:
      return std::make_unique<LinuxTargetInfo<SparcV8elTargetInfo>>(Triple,
                                                                    Opts);
    case toolchain::Triple::RTEMS:
      return std::make_unique<RTEMSTargetInfo<SparcV8elTargetInfo>>(Triple,
                                                                    Opts);
    default:
      return std::make_unique<SparcV8elTargetInfo>(Triple, Opts);
    }

  case toolchain::Triple::sparcv9:
    switch (os) {
    case toolchain::Triple::Linux:
      return std::make_unique<LinuxTargetInfo<SparcV9TargetInfo>>(Triple, Opts);
    case toolchain::Triple::Solaris:
      return std::make_unique<SolarisTargetInfo<SparcV9TargetInfo>>(Triple,
                                                                    Opts);
    case toolchain::Triple::NetBSD:
      return std::make_unique<NetBSDTargetInfo<SparcV9TargetInfo>>(Triple,
                                                                   Opts);
    case toolchain::Triple::OpenBSD:
      return std::make_unique<OpenBSDTargetInfo<SparcV9TargetInfo>>(Triple,
                                                                    Opts);
    case toolchain::Triple::FreeBSD:
      return std::make_unique<FreeBSDTargetInfo<SparcV9TargetInfo>>(Triple,
                                                                    Opts);
    default:
      return std::make_unique<SparcV9TargetInfo>(Triple, Opts);
    }

  case toolchain::Triple::systemz:
    switch (os) {
    case toolchain::Triple::Linux:
      return std::make_unique<LinuxTargetInfo<SystemZTargetInfo>>(Triple, Opts);
    case toolchain::Triple::ZOS:
      return std::make_unique<ZOSTargetInfo<SystemZTargetInfo>>(Triple, Opts);
    default:
      return std::make_unique<SystemZTargetInfo>(Triple, Opts);
    }

  case toolchain::Triple::tce:
    return std::make_unique<TCETargetInfo>(Triple, Opts);

  case toolchain::Triple::tcele:
    return std::make_unique<TCELETargetInfo>(Triple, Opts);

  case toolchain::Triple::x86:
    if (Triple.isOSDarwin())
      return std::make_unique<DarwinI386TargetInfo>(Triple, Opts);
    else if (Triple.isAppleMachO())
      return std::make_unique<AppleMachOI386TargetInfo>(Triple, Opts);

    switch (os) {
    case toolchain::Triple::Linux: {
      switch (Triple.getEnvironment()) {
      default:
        return std::make_unique<LinuxTargetInfo<X86_32TargetInfo>>(Triple,
                                                                   Opts);
      case toolchain::Triple::Android:
        return std::make_unique<AndroidX86_32TargetInfo>(Triple, Opts);
      }
    }
    case toolchain::Triple::DragonFly:
      return std::make_unique<DragonFlyBSDTargetInfo<X86_32TargetInfo>>(Triple,
                                                                        Opts);
    case toolchain::Triple::NetBSD:
      return std::make_unique<NetBSDI386TargetInfo>(Triple, Opts);
    case toolchain::Triple::OpenBSD:
      return std::make_unique<OpenBSDI386TargetInfo>(Triple, Opts);
    case toolchain::Triple::FreeBSD:
      return std::make_unique<FreeBSDTargetInfo<X86_32TargetInfo>>(Triple,
                                                                   Opts);
    case toolchain::Triple::Fuchsia:
      return std::make_unique<FuchsiaTargetInfo<X86_32TargetInfo>>(Triple,
                                                                   Opts);
    case toolchain::Triple::KFreeBSD:
      return std::make_unique<KFreeBSDTargetInfo<X86_32TargetInfo>>(Triple,
                                                                    Opts);
    case toolchain::Triple::Solaris:
      return std::make_unique<SolarisTargetInfo<X86_32TargetInfo>>(Triple,
                                                                   Opts);
    case toolchain::Triple::Win32: {
      switch (Triple.getEnvironment()) {
      case toolchain::Triple::Cygnus:
        return std::make_unique<CygwinX86_32TargetInfo>(Triple, Opts);
      case toolchain::Triple::GNU:
        return std::make_unique<MinGWX86_32TargetInfo>(Triple, Opts);
      case toolchain::Triple::Itanium:
      case toolchain::Triple::MSVC:
      default: // Assume MSVC for unknown environments
        return std::make_unique<MicrosoftX86_32TargetInfo>(Triple, Opts);
      }
    }
    case toolchain::Triple::Haiku:
      return std::make_unique<HaikuX86_32TargetInfo>(Triple, Opts);
    case toolchain::Triple::RTEMS:
      return std::make_unique<RTEMSX86_32TargetInfo>(Triple, Opts);
    case toolchain::Triple::ELFIAMCU:
      return std::make_unique<MCUX86_32TargetInfo>(Triple, Opts);
    case toolchain::Triple::Hurd:
      return std::make_unique<HurdTargetInfo<X86_32TargetInfo>>(Triple, Opts);
    default:
      return std::make_unique<X86_32TargetInfo>(Triple, Opts);
    }

  case toolchain::Triple::x86_64:
    if (Triple.isOSDarwin() || Triple.isOSBinFormatMachO())
      return std::make_unique<DarwinX86_64TargetInfo>(Triple, Opts);

    switch (os) {
    case toolchain::Triple::Linux: {
      switch (Triple.getEnvironment()) {
      default:
        return std::make_unique<LinuxTargetInfo<X86_64TargetInfo>>(Triple,
                                                                   Opts);
      case toolchain::Triple::Android:
        return std::make_unique<AndroidX86_64TargetInfo>(Triple, Opts);
      case toolchain::Triple::OpenHOS:
        return std::make_unique<OHOSX86_64TargetInfo>(Triple, Opts);
      }
    }
    case toolchain::Triple::DragonFly:
      return std::make_unique<DragonFlyBSDTargetInfo<X86_64TargetInfo>>(Triple,
                                                                        Opts);
    case toolchain::Triple::NetBSD:
      return std::make_unique<NetBSDTargetInfo<X86_64TargetInfo>>(Triple, Opts);
    case toolchain::Triple::OpenBSD:
      return std::make_unique<OpenBSDX86_64TargetInfo>(Triple, Opts);
    case toolchain::Triple::FreeBSD:
      return std::make_unique<FreeBSDTargetInfo<X86_64TargetInfo>>(Triple,
                                                                   Opts);
    case toolchain::Triple::Fuchsia:
      return std::make_unique<FuchsiaTargetInfo<X86_64TargetInfo>>(Triple,
                                                                   Opts);
    case toolchain::Triple::KFreeBSD:
      return std::make_unique<KFreeBSDTargetInfo<X86_64TargetInfo>>(Triple,
                                                                    Opts);
    case toolchain::Triple::Solaris:
      return std::make_unique<SolarisTargetInfo<X86_64TargetInfo>>(Triple,
                                                                   Opts);
    case toolchain::Triple::UEFI:
      return std::make_unique<UEFIX86_64TargetInfo>(Triple, Opts);

    case toolchain::Triple::Win32: {
      switch (Triple.getEnvironment()) {
      case toolchain::Triple::Cygnus:
        return std::make_unique<CygwinX86_64TargetInfo>(Triple, Opts);
      case toolchain::Triple::GNU:
        return std::make_unique<MinGWX86_64TargetInfo>(Triple, Opts);
      case toolchain::Triple::MSVC:
      default: // Assume MSVC for unknown environments
        return std::make_unique<MicrosoftX86_64TargetInfo>(Triple, Opts);
      }
    }
    case toolchain::Triple::Haiku:
      return std::make_unique<HaikuTargetInfo<X86_64TargetInfo>>(Triple, Opts);
    case toolchain::Triple::PS4:
      return std::make_unique<PS4OSTargetInfo<X86_64TargetInfo>>(Triple, Opts);
    case toolchain::Triple::PS5:
      return std::make_unique<PS5OSTargetInfo<X86_64TargetInfo>>(Triple, Opts);
    case toolchain::Triple::Hurd:
      return std::make_unique<HurdTargetInfo<X86_64TargetInfo>>(Triple, Opts);
    case toolchain::Triple::Managarm:
      return std::make_unique<ManagarmTargetInfo<X86_64TargetInfo>>(Triple,
                                                                    Opts);
    default:
      return std::make_unique<X86_64TargetInfo>(Triple, Opts);
    }

  case toolchain::Triple::spir: {
    if (os != toolchain::Triple::UnknownOS ||
        Triple.getEnvironment() != toolchain::Triple::UnknownEnvironment)
      return nullptr;
    return std::make_unique<SPIR32TargetInfo>(Triple, Opts);
  }
  case toolchain::Triple::spir64: {
    if (os != toolchain::Triple::UnknownOS ||
        Triple.getEnvironment() != toolchain::Triple::UnknownEnvironment)
      return nullptr;
    return std::make_unique<SPIR64TargetInfo>(Triple, Opts);
  }
  case toolchain::Triple::spirv: {
    return std::make_unique<SPIRVTargetInfo>(Triple, Opts);
  }
  case toolchain::Triple::spirv32: {
    if (os != toolchain::Triple::UnknownOS ||
        Triple.getEnvironment() != toolchain::Triple::UnknownEnvironment)
      return nullptr;
    return std::make_unique<SPIRV32TargetInfo>(Triple, Opts);
  }
  case toolchain::Triple::spirv64: {
    if (os != toolchain::Triple::UnknownOS ||
        Triple.getEnvironment() != toolchain::Triple::UnknownEnvironment) {
      if (os == toolchain::Triple::OSType::AMDHSA)
        return std::make_unique<SPIRV64AMDGCNTargetInfo>(Triple, Opts);
      return nullptr;
    }
    return std::make_unique<SPIRV64TargetInfo>(Triple, Opts);
  }
  case toolchain::Triple::wasm32:
    if (Triple.getSubArch() != toolchain::Triple::NoSubArch ||
        Triple.getVendor() != toolchain::Triple::UnknownVendor ||
        !Triple.isOSBinFormatWasm())
      return nullptr;
    switch (os) {
      case toolchain::Triple::WASI:
      return std::make_unique<WASITargetInfo<WebAssembly32TargetInfo>>(Triple,
                                                                       Opts);
      case toolchain::Triple::Emscripten:
      return std::make_unique<EmscriptenTargetInfo<WebAssembly32TargetInfo>>(
          Triple, Opts);
      case toolchain::Triple::UnknownOS:
      return std::make_unique<WebAssemblyOSTargetInfo<WebAssembly32TargetInfo>>(
          Triple, Opts);
      default:
        return nullptr;
    }
  case toolchain::Triple::wasm64:
    if (Triple.getSubArch() != toolchain::Triple::NoSubArch ||
        Triple.getVendor() != toolchain::Triple::UnknownVendor ||
        !Triple.isOSBinFormatWasm())
      return nullptr;
    switch (os) {
      case toolchain::Triple::WASI:
      return std::make_unique<WASITargetInfo<WebAssembly64TargetInfo>>(Triple,
                                                                       Opts);
      case toolchain::Triple::Emscripten:
      return std::make_unique<EmscriptenTargetInfo<WebAssembly64TargetInfo>>(
          Triple, Opts);
      case toolchain::Triple::UnknownOS:
      return std::make_unique<WebAssemblyOSTargetInfo<WebAssembly64TargetInfo>>(
          Triple, Opts);
      default:
        return nullptr;
    }

  case toolchain::Triple::dxil:
    return std::make_unique<DirectXTargetInfo>(Triple, Opts);

  case toolchain::Triple::ve:
    return std::make_unique<LinuxTargetInfo<VETargetInfo>>(Triple, Opts);

  case toolchain::Triple::csky:
    switch (os) {
    case toolchain::Triple::Linux:
      return std::make_unique<LinuxTargetInfo<CSKYTargetInfo>>(Triple, Opts);
    default:
      return std::make_unique<CSKYTargetInfo>(Triple, Opts);
    }
  case toolchain::Triple::loongarch32:
    switch (os) {
    case toolchain::Triple::Linux:
      return std::make_unique<LinuxTargetInfo<LoongArch32TargetInfo>>(Triple,
                                                                      Opts);
    default:
      return std::make_unique<LoongArch32TargetInfo>(Triple, Opts);
    }
  case toolchain::Triple::loongarch64:
    switch (os) {
    case toolchain::Triple::Linux:
      switch (Triple.getEnvironment()) {
      default:
        return std::make_unique<LinuxTargetInfo<LoongArch64TargetInfo>>(Triple,
                                                                        Opts);
      case toolchain::Triple::OpenHOS:
        return std::make_unique<OHOSTargetInfo<LoongArch64TargetInfo>>(Triple,
                                                                       Opts);
      }
    case toolchain::Triple::FreeBSD:
      return std::make_unique<FreeBSDTargetInfo<LoongArch64TargetInfo>>(Triple,
                                                                        Opts);
    case toolchain::Triple::OpenBSD:
      return std::make_unique<OpenBSDTargetInfo<LoongArch64TargetInfo>>(Triple,
                                                                        Opts);
    default:
      return std::make_unique<LoongArch64TargetInfo>(Triple, Opts);
    }

  case toolchain::Triple::xtensa:
    return std::make_unique<XtensaTargetInfo>(Triple, Opts);
  }
}
} // namespace targets
} // namespace language::Core

using namespace language::Core::targets;
/// CreateTargetInfo - Return the target info object for the specified target
/// options.
TargetInfo *TargetInfo::CreateTargetInfo(DiagnosticsEngine &Diags,
                                         TargetOptions &OptsRef) {
  TargetOptions *Opts = &OptsRef;

  toolchain::Triple Triple(toolchain::Triple::normalize(Opts->Triple));

  // Construct the target
  std::unique_ptr<TargetInfo> Target = AllocateTarget(Triple, *Opts);
  if (!Target) {
    Diags.Report(diag::err_target_unknown_triple) << Triple.str();
    return nullptr;
  }
  Target->TargetOpts = Opts;

  // Set the target CPU if specified.
  if (!Opts->CPU.empty() && !Target->setCPU(Opts->CPU)) {
    Diags.Report(diag::err_target_unknown_cpu) << Opts->CPU;
    SmallVector<StringRef, 32> ValidList;
    Target->fillValidCPUList(ValidList);
    if (!ValidList.empty())
      Diags.Report(diag::note_valid_options) << toolchain::join(ValidList, ", ");
    return nullptr;
  }

  // Check the TuneCPU name if specified.
  if (!Opts->TuneCPU.empty() &&
      !Target->isValidTuneCPUName(Opts->TuneCPU)) {
    Diags.Report(diag::err_target_unknown_cpu) << Opts->TuneCPU;
    SmallVector<StringRef, 32> ValidList;
    Target->fillValidTuneCPUList(ValidList);
    if (!ValidList.empty())
      Diags.Report(diag::note_valid_options) << toolchain::join(ValidList, ", ");
    return nullptr;
  }

  // Set the target ABI if specified.
  if (!Opts->ABI.empty() && !Target->setABI(Opts->ABI)) {
    Diags.Report(diag::err_target_unknown_abi) << Opts->ABI;
    return nullptr;
  }

  // Set the fp math unit.
  if (!Opts->FPMath.empty() && !Target->setFPMath(Opts->FPMath)) {
    Diags.Report(diag::err_target_unknown_fpmath) << Opts->FPMath;
    return nullptr;
  }

  // Compute the default target features, we need the target to handle this
  // because features may have dependencies on one another.
  toolchain::erase_if(Opts->FeaturesAsWritten, [&](StringRef Name) {
    if (Target->isReadOnlyFeature(Name.substr(1))) {
      Diags.Report(diag::warn_fe_backend_readonly_feature_flag) << Name;
      return true;
    }
    return false;
  });
  if (!Target->initFeatureMap(Opts->FeatureMap, Diags, Opts->CPU,
                              Opts->FeaturesAsWritten))
    return nullptr;

  // Add the features to the compile options.
  Opts->Features.clear();
  for (const auto &F : Opts->FeatureMap)
    Opts->Features.push_back((F.getValue() ? "+" : "-") + F.getKey().str());
  // Sort here, so we handle the features in a predictable order. (This matters
  // when we're dealing with features that overlap.)
  toolchain::sort(Opts->Features);

  if (!Target->handleTargetFeatures(Opts->Features, Diags))
    return nullptr;

  Target->setSupportedOpenCLOpts();
  Target->setCommandLineOpenCLOpts();
  Target->setMaxAtomicWidth();

  if (!Opts->DarwinTargetVariantTriple.empty())
    Target->DarwinTargetVariantTriple =
        toolchain::Triple(Opts->DarwinTargetVariantTriple);

  if (!Target->validateTarget(Diags))
    return nullptr;

  Target->CheckFixedPointBits();

  return Target.release();
}
/// validateOpenCLTarget  - Check that OpenCL target has valid
/// options setting based on OpenCL version.
bool TargetInfo::validateOpenCLTarget(const LangOptions &Opts,
                                      DiagnosticsEngine &Diags) const {
  const toolchain::StringMap<bool> &OpenCLFeaturesMap = getSupportedOpenCLOpts();

  auto diagnoseNotSupportedCore = [&](toolchain::StringRef Name, auto... OptArgs) {
    if (OpenCLOptions::isOpenCLOptionCoreIn(Opts, OptArgs...) &&
        !hasFeatureEnabled(OpenCLFeaturesMap, Name))
      Diags.Report(diag::warn_opencl_unsupported_core_feature)
          << Name << Opts.OpenCLCPlusPlus
          << Opts.getOpenCLVersionTuple().getAsString();
  };
#define OPENCL_GENERIC_EXTENSION(Ext, ...)                                     \
  diagnoseNotSupportedCore(#Ext, __VA_ARGS__);
#include "language/Core/Basic/OpenCLExtensions.def"

  // Validate that feature macros are set properly for OpenCL C 3.0.
  // In other cases assume that target is always valid.
  if (Opts.getOpenCLCompatibleVersion() < 300)
    return true;

  return OpenCLOptions::diagnoseUnsupportedFeatureDependencies(*this, Diags) &&
         OpenCLOptions::diagnoseFeatureExtensionDifferences(*this, Diags);
}
