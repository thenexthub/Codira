//===--- SystemZ.cpp - SystemZ Helpers for Tools ----------------*- C++ -*-===//
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

#include "SystemZ.h"
#include "language/Core/Config/config.h"
#include "language/Core/Driver/Options.h"
#include "toolchain/Option/ArgList.h"
#include "toolchain/TargetParser/Host.h"

using namespace language::Core::driver;
using namespace language::Core::driver::tools;
using namespace language::Core;
using namespace toolchain::opt;

systemz::FloatABI systemz::getSystemZFloatABI(const Driver &D,
                                              const ArgList &Args) {
  // Hard float is the default.
  systemz::FloatABI ABI = systemz::FloatABI::Hard;
  if (Args.hasArg(options::OPT_mfloat_abi_EQ))
    D.Diag(diag::err_drv_unsupported_opt)
      << Args.getLastArg(options::OPT_mfloat_abi_EQ)->getAsString(Args);

  if (Arg *A = Args.getLastArg(language::Core::driver::options::OPT_msoft_float,
                               options::OPT_mhard_float))
    if (A->getOption().matches(language::Core::driver::options::OPT_msoft_float))
      ABI = systemz::FloatABI::Soft;

  return ABI;
}

std::string systemz::getSystemZTargetCPU(const ArgList &Args,
                                         const toolchain::Triple &T) {
  if (const Arg *A = Args.getLastArg(language::Core::driver::options::OPT_march_EQ)) {
    toolchain::StringRef CPUName = A->getValue();

    if (CPUName == "native") {
      std::string CPU = std::string(toolchain::sys::getHostCPUName());
      if (!CPU.empty() && CPU != "generic")
        return CPU;
      else
        return "";
    }

    return std::string(CPUName);
  }
  if (T.isOSzOS())
    return "zEC12";
  return CLANG_SYSTEMZ_DEFAULT_ARCH;
}

void systemz::getSystemZTargetFeatures(const Driver &D, const ArgList &Args,
                                       std::vector<toolchain::StringRef> &Features) {
  // -m(no-)htm overrides use of the transactional-execution facility.
  if (Arg *A = Args.getLastArg(options::OPT_mhtm, options::OPT_mno_htm)) {
    if (A->getOption().matches(options::OPT_mhtm))
      Features.push_back("+transactional-execution");
    else
      Features.push_back("-transactional-execution");
  }
  // -m(no-)vx overrides use of the vector facility.
  if (Arg *A = Args.getLastArg(options::OPT_mvx, options::OPT_mno_vx)) {
    if (A->getOption().matches(options::OPT_mvx))
      Features.push_back("+vector");
    else
      Features.push_back("-vector");
  }

  systemz::FloatABI FloatABI = systemz::getSystemZFloatABI(D, Args);
  if (FloatABI == systemz::FloatABI::Soft)
    Features.push_back("+soft-float");

  if (const Arg *A = Args.getLastArg(options::OPT_munaligned_symbols,
                                     options::OPT_mno_unaligned_symbols)) {
    if (A->getOption().matches(options::OPT_munaligned_symbols))
      Features.push_back("+unaligned-symbols");
    else
      Features.push_back("-unaligned-symbols");
  }
}
