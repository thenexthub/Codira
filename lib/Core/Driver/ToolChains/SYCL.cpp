//===--- SYCL.cpp - SYCL Tool and ToolChain Implementations -----*- C++ -*-===//
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
#include "SYCL.h"
#include "language/Core/Driver/CommonArgs.h"

using namespace language::Core::driver;
using namespace language::Core::driver::toolchains;
using namespace language::Core::driver::tools;
using namespace language::Core;
using namespace toolchain::opt;

SYCLInstallationDetector::SYCLInstallationDetector(
    const Driver &D, const toolchain::Triple &HostTriple,
    const toolchain::opt::ArgList &Args) {}

void SYCLInstallationDetector::addSYCLIncludeArgs(
    const ArgList &DriverArgs, ArgStringList &CC1Args) const {
  if (DriverArgs.hasArg(language::Core::driver::options::OPT_nobuiltininc))
    return;

  // Add the SYCL header search locations in the specified order.
  // FIXME: Add the header file locations once the SYCL library and headers
  //        are properly established within the build.
}

// Unsupported options for SYCL device compilation.
static ArrayRef<options::ID> getUnsupportedOpts() {
  static constexpr options::ID UnsupportedOpts[] = {
      options::OPT_fsanitize_EQ,      // -fsanitize
      options::OPT_fcf_protection_EQ, // -fcf-protection
      options::OPT_fprofile_generate,
      options::OPT_fprofile_generate_EQ,
      options::OPT_fno_profile_generate, // -f[no-]profile-generate
      options::OPT_ftest_coverage,
      options::OPT_fno_test_coverage, // -f[no-]test-coverage
      options::OPT_fcoverage_mapping,
      options::OPT_fno_coverage_mapping, // -f[no-]coverage-mapping
      options::OPT_coverage,             // --coverage
      options::OPT_fprofile_instr_generate,
      options::OPT_fprofile_instr_generate_EQ,
      options::OPT_fno_profile_instr_generate, // -f[no-]profile-instr-generate
      options::OPT_fprofile_arcs,
      options::OPT_fno_profile_arcs, // -f[no-]profile-arcs
      options::OPT_fcreate_profile,  // -fcreate-profile
      options::OPT_fprofile_instr_use,
      options::OPT_fprofile_instr_use_EQ, // -fprofile-instr-use
      options::OPT_fcs_profile_generate,  // -fcs-profile-generate
      options::OPT_fcs_profile_generate_EQ,
  };
  return UnsupportedOpts;
}

SYCLToolChain::SYCLToolChain(const Driver &D, const toolchain::Triple &Triple,
                             const ToolChain &HostTC, const ArgList &Args)
    : ToolChain(D, Triple, Args), HostTC(HostTC),
      SYCLInstallation(D, Triple, Args) {
  // Lookup binaries into the driver directory, this is used to discover any
  // dependent SYCL offload compilation tools.
  getProgramPaths().push_back(getDriver().Dir);

  // Diagnose unsupported options only once.
  for (OptSpecifier Opt : getUnsupportedOpts()) {
    if (const Arg *A = Args.getLastArg(Opt)) {
      D.Diag(language::Core::diag::warn_drv_unsupported_option_for_target)
          << A->getAsString(Args) << getTriple().str();
    }
  }
}

void SYCLToolChain::addClangTargetOptions(
    const toolchain::opt::ArgList &DriverArgs, toolchain::opt::ArgStringList &CC1Args,
    Action::OffloadKind DeviceOffloadingKind) const {
  HostTC.addClangTargetOptions(DriverArgs, CC1Args, DeviceOffloadingKind);
}

toolchain::opt::DerivedArgList *
SYCLToolChain::TranslateArgs(const toolchain::opt::DerivedArgList &Args,
                             StringRef BoundArch,
                             Action::OffloadKind DeviceOffloadKind) const {
  DerivedArgList *DAL =
      HostTC.TranslateArgs(Args, BoundArch, DeviceOffloadKind);

  bool IsNewDAL = false;
  if (!DAL) {
    DAL = new DerivedArgList(Args.getBaseArgs());
    IsNewDAL = true;
  }

  for (Arg *A : Args) {
    // Filter out any options we do not want to pass along to the device
    // compilation.
    auto Opt(A->getOption());
    bool Unsupported = false;
    for (OptSpecifier UnsupportedOpt : getUnsupportedOpts()) {
      if (Opt.matches(UnsupportedOpt)) {
        if (Opt.getID() == options::OPT_fsanitize_EQ &&
            A->getValues().size() == 1) {
          std::string SanitizeVal = A->getValue();
          if (SanitizeVal == "address") {
            if (IsNewDAL)
              DAL->append(A);
            continue;
          }
        }
        if (!IsNewDAL)
          DAL->eraseArg(Opt.getID());
        Unsupported = true;
      }
    }
    if (Unsupported)
      continue;
    if (IsNewDAL)
      DAL->append(A);
  }

  const OptTable &Opts = getDriver().getOpts();
  if (!BoundArch.empty()) {
    DAL->eraseArg(options::OPT_march_EQ);
    DAL->AddJoinedArg(nullptr, Opts.getOption(options::OPT_march_EQ),
                      BoundArch);
  }
  return DAL;
}

void SYCLToolChain::addClangWarningOptions(ArgStringList &CC1Args) const {
  HostTC.addClangWarningOptions(CC1Args);
}

ToolChain::CXXStdlibType
SYCLToolChain::GetCXXStdlibType(const ArgList &Args) const {
  return HostTC.GetCXXStdlibType(Args);
}

void SYCLToolChain::addSYCLIncludeArgs(const ArgList &DriverArgs,
                                       ArgStringList &CC1Args) const {
  SYCLInstallation.addSYCLIncludeArgs(DriverArgs, CC1Args);
}

void SYCLToolChain::AddClangSystemIncludeArgs(const ArgList &DriverArgs,
                                              ArgStringList &CC1Args) const {
  HostTC.AddClangSystemIncludeArgs(DriverArgs, CC1Args);
}

void SYCLToolChain::AddClangCXXStdlibIncludeArgs(const ArgList &Args,
                                                 ArgStringList &CC1Args) const {
  HostTC.AddClangCXXStdlibIncludeArgs(Args, CC1Args);
}
