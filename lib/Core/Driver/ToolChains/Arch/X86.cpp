//===--- X86.cpp - X86 Helpers for Tools ------------------------*- C++ -*-===//
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

#include "X86.h"
#include "language/Core/Driver/Driver.h"
#include "language/Core/Driver/Options.h"
#include "toolchain/ADT/StringExtras.h"
#include "toolchain/ADT/StringMap.h"
#include "toolchain/Option/ArgList.h"
#include "toolchain/TargetParser/Host.h"

using namespace language::Core::driver;
using namespace language::Core::driver::tools;
using namespace language::Core;
using namespace toolchain::opt;

std::string x86::getX86TargetCPU(const Driver &D, const ArgList &Args,
                                 const toolchain::Triple &Triple) {
  if (const Arg *A = Args.getLastArg(language::Core::driver::options::OPT_march_EQ)) {
    StringRef CPU = A->getValue();
    if (CPU != "native")
      return std::string(CPU);

    // FIXME: Reject attempts to use -march=native unless the target matches
    // the host.
    CPU = toolchain::sys::getHostCPUName();
    if (!CPU.empty() && CPU != "generic")
      return std::string(CPU);
  }

  if (const Arg *A = Args.getLastArg(options::OPT__SLASH_arch)) {
    // Mapping built by looking at lib/Basic's X86TargetInfo::initFeatureMap().
    // The keys are case-sensitive; this matches link.exe.
    // 32-bit and 64-bit /arch: flags.
    toolchain::StringMap<StringRef> ArchMap({
        {"AVX", "sandybridge"},
        {"AVX2", "haswell"},
        {"AVX512F", "knl"},
        {"AVX512", "skylake-avx512"},
    });
    if (Triple.getArch() == toolchain::Triple::x86) {
      // 32-bit-only /arch: flags.
      ArchMap.insert({
          {"IA32", "i386"},
          {"SSE", "pentium3"},
          {"SSE2", "pentium4"},
      });
    }
    StringRef CPU = ArchMap.lookup(A->getValue());
    if (CPU.empty()) {
      std::vector<StringRef> ValidArchs{ArchMap.keys().begin(),
                                        ArchMap.keys().end()};
      sort(ValidArchs);
      D.Diag(diag::warn_drv_invalid_arch_name_with_suggestion)
          << A->getValue() << (Triple.getArch() == toolchain::Triple::x86)
          << join(ValidArchs, ", ");
    }
    return std::string(CPU);
  }

  // Select the default CPU if none was given (or detection failed).

  if (!Triple.isX86())
    return ""; // This routine is only handling x86 targets.

  bool Is64Bit = Triple.getArch() == toolchain::Triple::x86_64;

  // FIXME: Need target hooks.
  if (Triple.isOSDarwin()) {
    if (Triple.getArchName() == "x86_64h")
      return "core-avx2";
    // macosx10.12 drops support for all pre-Penryn Macs.
    // Simulators can still run on 10.11 though, like Xcode.
    if (Triple.isMacOSX() && !Triple.isOSVersionLT(10, 12))
      return "penryn";

    if (Triple.isDriverKit())
      return "nehalem";

    // The oldest x86_64 Macs have core2/Merom; the oldest x86 Macs have Yonah.
    return Is64Bit ? "core2" : "yonah";
  }

  // Set up default CPU name for PS4/PS5 compilers.
  if (Triple.isPS4())
    return "btver2";
  if (Triple.isPS5())
    return "znver2";

  // On Android use targets compatible with gcc
  if (Triple.isAndroid())
    return Is64Bit ? "x86-64" : "i686";

  // Everything else goes to x86-64 in 64-bit mode.
  if (Is64Bit)
    return "x86-64";

  switch (Triple.getOS()) {
  case toolchain::Triple::NetBSD:
    return "i486";
  case toolchain::Triple::Haiku:
  case toolchain::Triple::OpenBSD:
    return "i586";
  case toolchain::Triple::FreeBSD:
    return "i686";
  default:
    // Fallback to p4.
    return "pentium4";
  }
}

void x86::getX86TargetFeatures(const Driver &D, const toolchain::Triple &Triple,
                               const ArgList &Args,
                               std::vector<StringRef> &Features) {
  // Claim and report unsupported -mabi=. Note: we don't support "sysv_abi" or
  // "ms_abi" as default function attributes.
  if (const Arg *A = Args.getLastArg(language::Core::driver::options::OPT_mabi_EQ)) {
    StringRef DefaultAbi =
        (Triple.isOSWindows() || Triple.isUEFI()) ? "ms" : "sysv";
    if (A->getValue() != DefaultAbi)
      D.Diag(diag::err_drv_unsupported_opt_for_target)
          << A->getSpelling() << Triple.getTriple();
  }

  // If -march=native, autodetect the feature list.
  if (const Arg *A = Args.getLastArg(language::Core::driver::options::OPT_march_EQ)) {
    if (StringRef(A->getValue()) == "native") {
      for (auto &F : toolchain::sys::getHostCPUFeatures())
        Features.push_back(
            Args.MakeArgString((F.second ? "+" : "-") + F.first()));
    }
  }

  if (Triple.getArchName() == "x86_64h") {
    // x86_64h implies quite a few of the more modern subtarget features
    // for Haswell class CPUs, but not all of them. Opt-out of a few.
    Features.push_back("-rdrnd");
    Features.push_back("-aes");
    Features.push_back("-pclmul");
    Features.push_back("-rtm");
    Features.push_back("-fsgsbase");
  }

  const toolchain::Triple::ArchType ArchType = Triple.getArch();
  // Add features to be compatible with gcc for Android.
  if (Triple.isAndroid()) {
    if (ArchType == toolchain::Triple::x86_64) {
      Features.push_back("+sse4.2");
      Features.push_back("+popcnt");
      Features.push_back("+cx16");
    } else
      Features.push_back("+ssse3");
  }

  // Translate the high level `-mretpoline` flag to the specific target feature
  // flags. We also detect if the user asked for retpoline external thunks but
  // failed to ask for retpolines themselves (through any of the different
  // flags). This is a bit hacky but keeps existing usages working. We should
  // consider deprecating this and instead warn if the user requests external
  // retpoline thunks and *doesn't* request some form of retpolines.
  auto SpectreOpt = language::Core::driver::options::ID::OPT_INVALID;
  if (Args.hasArgNoClaim(options::OPT_mretpoline, options::OPT_mno_retpoline,
                         options::OPT_mspeculative_load_hardening,
                         options::OPT_mno_speculative_load_hardening)) {
    if (Args.hasFlag(options::OPT_mretpoline, options::OPT_mno_retpoline,
                     false)) {
      Features.push_back("+retpoline-indirect-calls");
      Features.push_back("+retpoline-indirect-branches");
      SpectreOpt = options::OPT_mretpoline;
    } else if (Args.hasFlag(options::OPT_mspeculative_load_hardening,
                            options::OPT_mno_speculative_load_hardening,
                            false)) {
      // On x86, speculative load hardening relies on at least using retpolines
      // for indirect calls.
      Features.push_back("+retpoline-indirect-calls");
      SpectreOpt = options::OPT_mspeculative_load_hardening;
    }
  } else if (Args.hasFlag(options::OPT_mretpoline_external_thunk,
                          options::OPT_mno_retpoline_external_thunk, false)) {
    // FIXME: Add a warning about failing to specify `-mretpoline` and
    // eventually switch to an error here.
    Features.push_back("+retpoline-indirect-calls");
    Features.push_back("+retpoline-indirect-branches");
    SpectreOpt = options::OPT_mretpoline_external_thunk;
  }

  auto LVIOpt = language::Core::driver::options::ID::OPT_INVALID;
  if (Args.hasFlag(options::OPT_mlvi_hardening, options::OPT_mno_lvi_hardening,
                   false)) {
    Features.push_back("+lvi-load-hardening");
    Features.push_back("+lvi-cfi"); // load hardening implies CFI protection
    LVIOpt = options::OPT_mlvi_hardening;
  } else if (Args.hasFlag(options::OPT_mlvi_cfi, options::OPT_mno_lvi_cfi,
                          false)) {
    Features.push_back("+lvi-cfi");
    LVIOpt = options::OPT_mlvi_cfi;
  }

  if (Args.hasFlag(options::OPT_m_seses, options::OPT_mno_seses, false)) {
    if (LVIOpt == options::OPT_mlvi_hardening)
      D.Diag(diag::err_drv_argument_not_allowed_with)
          << D.getOpts().getOptionName(options::OPT_mlvi_hardening)
          << D.getOpts().getOptionName(options::OPT_m_seses);

    if (SpectreOpt != language::Core::driver::options::ID::OPT_INVALID)
      D.Diag(diag::err_drv_argument_not_allowed_with)
          << D.getOpts().getOptionName(SpectreOpt)
          << D.getOpts().getOptionName(options::OPT_m_seses);

    Features.push_back("+seses");
    if (!Args.hasArg(options::OPT_mno_lvi_cfi)) {
      Features.push_back("+lvi-cfi");
      LVIOpt = options::OPT_mlvi_cfi;
    }
  }

  if (SpectreOpt != language::Core::driver::options::ID::OPT_INVALID &&
      LVIOpt != language::Core::driver::options::ID::OPT_INVALID) {
    D.Diag(diag::err_drv_argument_not_allowed_with)
        << D.getOpts().getOptionName(SpectreOpt)
        << D.getOpts().getOptionName(LVIOpt);
  }

  for (const Arg *A : Args.filtered(options::OPT_m_x86_AVX10_Features_Group)) {
    StringRef Name = A->getOption().getName();
    A->claim();

    // Skip over "-m".
    assert(Name.starts_with("m") && "Invalid feature name.");
    Name = Name.substr(1);

    bool IsNegative = Name.consume_front("no-");

    StringRef Version, Width;
    std::tie(Version, Width) = Name.substr(6).split('-');
    assert(Name.starts_with("avx10.") && "Invalid AVX10 feature name.");
    assert((Version == "1" || Version == "2") && "Invalid AVX10 feature name.");

    if (Width == "") {
      if (IsNegative)
        Features.push_back(Args.MakeArgString("-" + Name + "-256"));
      else
        Features.push_back(Args.MakeArgString("+" + Name + "-512"));
    } else {
      if (Width == "512")
        D.Diag(diag::warn_drv_deprecated_arg) << Name << 1 << Name.drop_back(4);
      else if (Width == "256")
        D.Diag(diag::warn_drv_deprecated_custom)
            << Name
            << "no alternative argument provided because "
               "AVX10/256 is not supported and will be removed";
      else
        assert((Width == "256" || Width == "512") && "Invalid vector length.");
      Features.push_back(Args.MakeArgString((IsNegative ? "-" : "+") + Name));
    }
  }

  // Now add any that the user explicitly requested on the command line,
  // which may override the defaults.
  for (const Arg *A : Args.filtered(options::OPT_m_x86_Features_Group,
                                    options::OPT_mgeneral_regs_only)) {
    StringRef Name = A->getOption().getName();
    A->claim();

    // Skip over "-m".
    assert(Name.starts_with("m") && "Invalid feature name.");
    Name = Name.substr(1);

    // Replace -mgeneral-regs-only with -x87, -mmx, -sse
    if (A->getOption().getID() == options::OPT_mgeneral_regs_only) {
      Features.insert(Features.end(), {"-x87", "-mmx", "-sse"});
      continue;
    }

    bool IsNegative = Name.starts_with("no-");

    bool Not64Bit = ArchType != toolchain::Triple::x86_64;
    if (Not64Bit && Name == "uintr")
      D.Diag(diag::err_drv_unsupported_opt_for_target)
          << A->getSpelling() << Triple.getTriple();

    if (A->getOption().matches(options::OPT_mevex512) ||
        A->getOption().matches(options::OPT_mno_evex512))
      D.Diag(diag::warn_drv_deprecated_custom)
          << Name
          << "no alternative argument provided because "
             "AVX10/256 is not supported and will be removed";

    if (A->getOption().matches(options::OPT_mapx_features_EQ) ||
        A->getOption().matches(options::OPT_mno_apx_features_EQ)) {

      if (Not64Bit && !IsNegative)
        D.Diag(diag::err_drv_unsupported_opt_for_target)
            << StringRef(A->getSpelling().str() + "|-mapxf")
            << Triple.getTriple();

      for (StringRef Value : A->getValues()) {
        if (Value != "egpr" && Value != "push2pop2" && Value != "ppx" &&
            Value != "ndd" && Value != "ccmp" && Value != "nf" &&
            Value != "cf" && Value != "zu")
          D.Diag(language::Core::diag::err_drv_unsupported_option_argument)
              << A->getSpelling() << Value;

        Features.push_back(
            Args.MakeArgString((IsNegative ? "-" : "+") + Value));
      }
      continue;
    }
    if (IsNegative)
      Name = Name.substr(3);
    Features.push_back(Args.MakeArgString((IsNegative ? "-" : "+") + Name));
  }

  // Enable/disable straight line speculation hardening.
  if (Arg *A = Args.getLastArg(options::OPT_mharden_sls_EQ)) {
    StringRef Scope = A->getValue();
    if (Scope == "all") {
      Features.push_back("+harden-sls-ijmp");
      Features.push_back("+harden-sls-ret");
    } else if (Scope == "return") {
      Features.push_back("+harden-sls-ret");
    } else if (Scope == "indirect-jmp") {
      Features.push_back("+harden-sls-ijmp");
    } else if (Scope != "none") {
      D.Diag(diag::err_drv_unsupported_option_argument)
          << A->getSpelling() << Scope;
    }
  }

  // -mno-gather, -mno-scatter support
  if (Args.hasArg(options::OPT_mno_gather))
    Features.push_back("+prefer-no-gather");
  if (Args.hasArg(options::OPT_mno_scatter))
    Features.push_back("+prefer-no-scatter");
  if (Args.hasArg(options::OPT_mapx_inline_asm_use_gpr32))
    Features.push_back("+inline-asm-use-gpr32");

  // Warn for removed 3dnow support
  if (const Arg *A =
          Args.getLastArg(options::OPT_m3dnowa, options::OPT_mno_3dnowa,
                          options::OPT_mno_3dnow)) {
    if (A->getOption().matches(options::OPT_m3dnowa))
      D.Diag(diag::warn_drv_clang_unsupported) << A->getAsString(Args);
  }
  if (const Arg *A =
          Args.getLastArg(options::OPT_m3dnow, options::OPT_mno_3dnow)) {
    if (A->getOption().matches(options::OPT_m3dnow))
      D.Diag(diag::warn_drv_clang_unsupported) << A->getAsString(Args);
  }
}
