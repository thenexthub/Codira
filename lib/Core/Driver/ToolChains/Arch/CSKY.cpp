//===--- CSKY.cpp - CSKY Helpers for Tools --------------------*- C++ -*-===//
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

#include "CSKY.h"
#include "language/Core/Driver/Driver.h"
#include "language/Core/Driver/Options.h"
#include "toolchain/ADT/StringSwitch.h"
#include "toolchain/Option/ArgList.h"
#include "toolchain/TargetParser/CSKYTargetParser.h"
#include "toolchain/TargetParser/Host.h"
#include "toolchain/TargetParser/TargetParser.h"

using namespace language::Core::driver;
using namespace language::Core::driver::tools;
using namespace language::Core;
using namespace toolchain::opt;

std::optional<toolchain::StringRef>
csky::getCSKYArchName(const Driver &D, const ArgList &Args,
                      const toolchain::Triple &Triple) {
  if (const Arg *A = Args.getLastArg(options::OPT_march_EQ)) {
    toolchain::CSKY::ArchKind ArchKind = toolchain::CSKY::parseArch(A->getValue());

    if (ArchKind == toolchain::CSKY::ArchKind::INVALID) {
      D.Diag(language::Core::diag::err_drv_invalid_arch_name) << A->getAsString(Args);
      return std::nullopt;
    }
    return std::optional<toolchain::StringRef>(A->getValue());
  }

  if (const Arg *A = Args.getLastArg(language::Core::driver::options::OPT_mcpu_EQ)) {
    toolchain::CSKY::ArchKind ArchKind = toolchain::CSKY::parseCPUArch(A->getValue());
    if (ArchKind == toolchain::CSKY::ArchKind::INVALID) {
      D.Diag(language::Core::diag::err_drv_clang_unsupported) << A->getAsString(Args);
      return std::nullopt;
    }
    return std::optional<toolchain::StringRef>(toolchain::CSKY::getArchName(ArchKind));
  }

  return std::optional<toolchain::StringRef>("ck810");
}

csky::FloatABI csky::getCSKYFloatABI(const Driver &D, const ArgList &Args) {
  csky::FloatABI ABI = FloatABI::Soft;
  if (Arg *A =
          Args.getLastArg(options::OPT_msoft_float, options::OPT_mhard_float,
                          options::OPT_mfloat_abi_EQ)) {
    if (A->getOption().matches(options::OPT_msoft_float)) {
      ABI = FloatABI::Soft;
    } else if (A->getOption().matches(options::OPT_mhard_float)) {
      ABI = FloatABI::Hard;
    } else {
      ABI = toolchain::StringSwitch<csky::FloatABI>(A->getValue())
                .Case("soft", FloatABI::Soft)
                .Case("softfp", FloatABI::SoftFP)
                .Case("hard", FloatABI::Hard)
                .Default(FloatABI::Invalid);
      if (ABI == FloatABI::Invalid) {
        D.Diag(diag::err_drv_invalid_mfloat_abi) << A->getAsString(Args);
        ABI = FloatABI::Soft;
      }
    }
  }

  return ABI;
}

// Handle -mfpu=.
static toolchain::CSKY::CSKYFPUKind
getCSKYFPUFeatures(const Driver &D, const Arg *A, const ArgList &Args,
                   StringRef FPU, std::vector<StringRef> &Features) {

  toolchain::CSKY::CSKYFPUKind FPUID =
      toolchain::StringSwitch<toolchain::CSKY::CSKYFPUKind>(FPU)
          .Case("auto", toolchain::CSKY::FK_AUTO)
          .Case("fpv2", toolchain::CSKY::FK_FPV2)
          .Case("fpv2_divd", toolchain::CSKY::FK_FPV2_DIVD)
          .Case("fpv2_sf", toolchain::CSKY::FK_FPV2_SF)
          .Case("fpv3", toolchain::CSKY::FK_FPV3)
          .Case("fpv3_hf", toolchain::CSKY::FK_FPV3_HF)
          .Case("fpv3_hsf", toolchain::CSKY::FK_FPV3_HSF)
          .Case("fpv3_sdf", toolchain::CSKY::FK_FPV3_SDF)
          .Default(toolchain::CSKY::FK_INVALID);
  if (FPUID == toolchain::CSKY::FK_INVALID) {
    D.Diag(language::Core::diag::err_drv_clang_unsupported) << A->getAsString(Args);
    return toolchain::CSKY::FK_INVALID;
  }

  auto RemoveTargetFPUFeature =
      [&Features](ArrayRef<const char *> FPUFeatures) {
        for (auto FPUFeature : FPUFeatures) {
          auto it = toolchain::find(Features, FPUFeature);
          if (it != Features.end())
            Features.erase(it);
        }
      };

  RemoveTargetFPUFeature({"+fpuv2_sf", "+fpuv2_df", "+fdivdu", "+fpuv3_hi",
                          "+fpuv3_hf", "+fpuv3_sf", "+fpuv3_df"});

  if (!toolchain::CSKY::getFPUFeatures(FPUID, Features)) {
    D.Diag(language::Core::diag::err_drv_clang_unsupported) << A->getAsString(Args);
    return toolchain::CSKY::FK_INVALID;
  }

  return FPUID;
}

void csky::getCSKYTargetFeatures(const Driver &D, const toolchain::Triple &Triple,
                                 const ArgList &Args, ArgStringList &CmdArgs,
                                 std::vector<toolchain::StringRef> &Features) {
  toolchain::StringRef archName;
  toolchain::StringRef cpuName;
  toolchain::CSKY::ArchKind ArchKind = toolchain::CSKY::ArchKind::INVALID;
  if (const Arg *A = Args.getLastArg(options::OPT_march_EQ)) {
    ArchKind = toolchain::CSKY::parseArch(A->getValue());
    if (ArchKind == toolchain::CSKY::ArchKind::INVALID) {
      D.Diag(language::Core::diag::err_drv_invalid_arch_name) << A->getAsString(Args);
      return;
    }
    archName = A->getValue();
  }

  if (const Arg *A = Args.getLastArg(language::Core::driver::options::OPT_mcpu_EQ)) {
    toolchain::CSKY::ArchKind Kind = toolchain::CSKY::parseCPUArch(A->getValue());
    if (Kind == toolchain::CSKY::ArchKind::INVALID) {
      D.Diag(language::Core::diag::err_drv_clang_unsupported) << A->getAsString(Args);
      return;
    }
    if (!archName.empty() && Kind != ArchKind) {
      D.Diag(language::Core::diag::err_drv_clang_unsupported) << A->getAsString(Args);
      return;
    }
    cpuName = A->getValue();
    if (archName.empty())
      archName = toolchain::CSKY::getArchName(Kind);
  }

  if (archName.empty() && cpuName.empty()) {
    archName = "ck810";
    cpuName = "ck810";
  } else if (!archName.empty() && cpuName.empty()) {
    cpuName = archName;
  }

  csky::FloatABI FloatABI = csky::getCSKYFloatABI(D, Args);

  if (FloatABI == csky::FloatABI::Hard) {
    Features.push_back("+hard-float-abi");
    Features.push_back("+hard-float");
  } else if (FloatABI == csky::FloatABI::SoftFP) {
    Features.push_back("+hard-float");
  }

  uint64_t Extension = toolchain::CSKY::getDefaultExtensions(cpuName);
  toolchain::CSKY::getExtensionFeatures(Extension, Features);

  if (const Arg *FPUArg = Args.getLastArg(options::OPT_mfpu_EQ))
    getCSKYFPUFeatures(D, FPUArg, Args, FPUArg->getValue(), Features);
}
