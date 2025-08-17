//===--- PPC.cpp - PPC Helpers for Tools ------------------------*- C++ -*-===//
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

#include "PPC.h"
#include "language/Core/Driver/CommonArgs.h"
#include "language/Core/Driver/Driver.h"
#include "language/Core/Driver/Options.h"
#include "toolchain/ADT/StringSwitch.h"
#include "toolchain/Option/ArgList.h"
#include "toolchain/TargetParser/Host.h"

using namespace language::Core::driver;
using namespace language::Core::driver::tools;
using namespace language::Core;
using namespace toolchain::opt;

const char *ppc::getPPCAsmModeForCPU(StringRef Name) {
  return toolchain::StringSwitch<const char *>(Name)
      .Case("pwr7", "-mpower7")
      .Case("power7", "-mpower7")
      .Case("pwr8", "-mpower8")
      .Case("power8", "-mpower8")
      .Case("ppc64le", "-mpower8")
      .Case("pwr9", "-mpower9")
      .Case("power9", "-mpower9")
      .Case("pwr10", "-mpower10")
      .Case("power10", "-mpower10")
      .Case("pwr11", "-mpower11")
      .Case("power11", "-mpower11")
      .Default("-many");
}

void ppc::getPPCTargetFeatures(const Driver &D, const toolchain::Triple &Triple,
                               const ArgList &Args,
                               std::vector<StringRef> &Features) {
  if (Triple.getSubArch() == toolchain::Triple::PPCSubArch_spe)
    Features.push_back("+spe");

  handleTargetFeaturesGroup(D, Triple, Args, Features,
                            options::OPT_m_ppc_Features_Group);

  ppc::FloatABI FloatABI = ppc::getPPCFloatABI(D, Args);
  if (FloatABI == ppc::FloatABI::Soft)
    Features.push_back("-hard-float");

  ppc::ReadGOTPtrMode ReadGOT = ppc::getPPCReadGOTPtrMode(D, Triple, Args);
  if (ReadGOT == ppc::ReadGOTPtrMode::SecurePlt)
    Features.push_back("+secure-plt");

  bool UseSeparateSections = isUseSeparateSections(Triple);
  bool HasDefaultDataSections = Triple.isOSBinFormatXCOFF();
  if (Args.hasArg(options::OPT_maix_small_local_exec_tls) ||
      Args.hasArg(options::OPT_maix_small_local_dynamic_tls)) {
    if (!Triple.isOSAIX() || !Triple.isArch64Bit())
      D.Diag(diag::err_opt_not_valid_on_target)
          << "-maix-small-local-[exec|dynamic]-tls";

    // The -maix-small-local-[exec|dynamic]-tls option should only be used with
    // -fdata-sections, as having data sections turned off with this option
    // is not ideal for performance. Moreover, the
    // small-local-[exec|dynamic]-tls region is a limited resource, and should
    // not be used for variables that may be replaced.
    if (!Args.hasFlag(options::OPT_fdata_sections,
                      options::OPT_fno_data_sections,
                      UseSeparateSections || HasDefaultDataSections))
      D.Diag(diag::err_drv_argument_only_allowed_with)
          << "-maix-small-local-[exec|dynamic]-tls" << "-fdata-sections";
  }

  if (Args.hasArg(options::OPT_maix_shared_lib_tls_model_opt) &&
      !(Triple.isOSAIX() && Triple.isArch64Bit()))
    D.Diag(diag::err_opt_not_valid_on_target)
        << "-maix-shared-lib-tls-model-opt";
}

ppc::ReadGOTPtrMode ppc::getPPCReadGOTPtrMode(const Driver &D, const toolchain::Triple &Triple,
                                              const ArgList &Args) {
  if (Args.getLastArg(options::OPT_msecure_plt))
    return ppc::ReadGOTPtrMode::SecurePlt;
  if (Triple.isPPC32SecurePlt())
    return ppc::ReadGOTPtrMode::SecurePlt;
  else
    return ppc::ReadGOTPtrMode::Bss;
}

ppc::FloatABI ppc::getPPCFloatABI(const Driver &D, const ArgList &Args) {
  ppc::FloatABI ABI = ppc::FloatABI::Invalid;
  if (Arg *A =
          Args.getLastArg(options::OPT_msoft_float, options::OPT_mhard_float,
                          options::OPT_mfloat_abi_EQ)) {
    if (A->getOption().matches(options::OPT_msoft_float))
      ABI = ppc::FloatABI::Soft;
    else if (A->getOption().matches(options::OPT_mhard_float))
      ABI = ppc::FloatABI::Hard;
    else {
      ABI = toolchain::StringSwitch<ppc::FloatABI>(A->getValue())
                .Case("soft", ppc::FloatABI::Soft)
                .Case("hard", ppc::FloatABI::Hard)
                .Default(ppc::FloatABI::Invalid);
      if (ABI == ppc::FloatABI::Invalid && !StringRef(A->getValue()).empty()) {
        D.Diag(language::Core::diag::err_drv_invalid_mfloat_abi) << A->getAsString(Args);
        ABI = ppc::FloatABI::Hard;
      }
    }
  }

  // If unspecified, choose the default based on the platform.
  if (ABI == ppc::FloatABI::Invalid) {
    ABI = ppc::FloatABI::Hard;
  }

  return ABI;
}

bool ppc::hasPPCAbiArg(const ArgList &Args, const char *Value) {
  Arg *A = Args.getLastArg(options::OPT_mabi_EQ);
  return A && (A->getValue() == StringRef(Value));
}
