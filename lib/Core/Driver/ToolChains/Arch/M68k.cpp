//===--- M68k.cpp - M68k Helpers for Tools -------------------*- C++-*-===//
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

#include "M68k.h"
#include "language/Core/Driver/Driver.h"
#include "language/Core/Driver/Options.h"
#include "toolchain/ADT/StringSwitch.h"
#include "toolchain/Option/ArgList.h"
#include "toolchain/Support/Regex.h"
#include "toolchain/TargetParser/Host.h"

using namespace language::Core::driver;
using namespace language::Core::driver::tools;
using namespace language::Core;
using namespace toolchain::opt;

/// getM68kTargetCPU - Get the (LLVM) name of the 68000 cpu we are targeting.
std::string m68k::getM68kTargetCPU(const ArgList &Args) {
  if (Arg *A = Args.getLastArg(language::Core::driver::options::OPT_mcpu_EQ)) {
    // The canonical CPU name is captalize. However, we allow
    // starting with lower case or numbers only
    StringRef CPUName = A->getValue();

    if (CPUName == "native") {
      std::string CPU = std::string(toolchain::sys::getHostCPUName());
      if (!CPU.empty() && CPU != "generic")
        return CPU;
    }

    if (CPUName == "common")
      return "generic";

    return toolchain::StringSwitch<std::string>(CPUName)
        .Cases("m68000", "68000", "M68000")
        .Cases("m68010", "68010", "M68010")
        .Cases("m68020", "68020", "M68020")
        .Cases("m68030", "68030", "M68030")
        .Cases("m68040", "68040", "M68040")
        .Cases("m68060", "68060", "M68060")
        .Default(CPUName.str());
  }
  // FIXME: Throw error when multiple sub-architecture flag exist
  if (Args.hasArg(language::Core::driver::options::OPT_m68000))
    return "M68000";
  if (Args.hasArg(language::Core::driver::options::OPT_m68010))
    return "M68010";
  if (Args.hasArg(language::Core::driver::options::OPT_m68020))
    return "M68020";
  if (Args.hasArg(language::Core::driver::options::OPT_m68030))
    return "M68030";
  if (Args.hasArg(language::Core::driver::options::OPT_m68040))
    return "M68040";
  if (Args.hasArg(language::Core::driver::options::OPT_m68060))
    return "M68060";

  return "";
}

static void addFloatABIFeatures(const toolchain::opt::ArgList &Args,
                                std::vector<toolchain::StringRef> &Features) {
  Arg *A = Args.getLastArg(options::OPT_msoft_float, options::OPT_mhard_float,
                           options::OPT_m68881);
  // Opt out FPU even for newer CPUs.
  if (A && A->getOption().matches(options::OPT_msoft_float)) {
    Features.push_back("-isa-68881");
    Features.push_back("-isa-68882");
    return;
  }

  std::string CPU = m68k::getM68kTargetCPU(Args);
  // Only enable M68881 for CPU < 68020 if the related flags are present.
  if ((A && (CPU == "M68000" || CPU == "M68010")) ||
      // Otherwise, by default we assume newer CPUs have M68881/2.
      CPU == "M68020")
    Features.push_back("+isa-68881");
  else if (CPU == "M68030" || CPU == "M68040" || CPU == "M68060")
    // Note that although CPU >= M68040 imply M68882, we still add `isa-68882`
    // anyway so that it's easier to add or not add the corresponding macro
    // definitions later, in case we want to disable 68881/2 in newer CPUs
    // (with -msoft-float, for instance).
    Features.push_back("+isa-68882");
}

void m68k::getM68kTargetFeatures(const Driver &D, const toolchain::Triple &Triple,
                                 const ArgList &Args,
                                 std::vector<StringRef> &Features) {
  addFloatABIFeatures(Args, Features);

  // Handle '-ffixed-<register>' flags
  if (Args.hasArg(options::OPT_ffixed_a0))
    Features.push_back("+reserve-a0");
  if (Args.hasArg(options::OPT_ffixed_a1))
    Features.push_back("+reserve-a1");
  if (Args.hasArg(options::OPT_ffixed_a2))
    Features.push_back("+reserve-a2");
  if (Args.hasArg(options::OPT_ffixed_a3))
    Features.push_back("+reserve-a3");
  if (Args.hasArg(options::OPT_ffixed_a4))
    Features.push_back("+reserve-a4");
  if (Args.hasArg(options::OPT_ffixed_a5))
    Features.push_back("+reserve-a5");
  if (Args.hasArg(options::OPT_ffixed_a6))
    Features.push_back("+reserve-a6");
  if (Args.hasArg(options::OPT_ffixed_d0))
    Features.push_back("+reserve-d0");
  if (Args.hasArg(options::OPT_ffixed_d1))
    Features.push_back("+reserve-d1");
  if (Args.hasArg(options::OPT_ffixed_d2))
    Features.push_back("+reserve-d2");
  if (Args.hasArg(options::OPT_ffixed_d3))
    Features.push_back("+reserve-d3");
  if (Args.hasArg(options::OPT_ffixed_d4))
    Features.push_back("+reserve-d4");
  if (Args.hasArg(options::OPT_ffixed_d5))
    Features.push_back("+reserve-d5");
  if (Args.hasArg(options::OPT_ffixed_d6))
    Features.push_back("+reserve-d6");
  if (Args.hasArg(options::OPT_ffixed_d7))
    Features.push_back("+reserve-d7");
}
