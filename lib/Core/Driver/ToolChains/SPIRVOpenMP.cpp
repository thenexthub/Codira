//==- SPIRVOpenMP.cpp - SPIR-V OpenMP Tool Implementations --------*- C++ -*==//
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
//==------------------------------------------------------------------------==//
#include "SPIRVOpenMP.h"
#include "language/Core/Driver/CommonArgs.h"

using namespace language::Core::driver;
using namespace language::Core::driver::toolchains;
using namespace language::Core::driver::tools;
using namespace toolchain::opt;

namespace language::Core::driver::toolchains {
SPIRVOpenMPToolChain::SPIRVOpenMPToolChain(const Driver &D,
                                           const toolchain::Triple &Triple,
                                           const ToolChain &HostToolchain,
                                           const ArgList &Args)
    : SPIRVToolChain(D, Triple, Args), HostTC(HostToolchain) {}

void SPIRVOpenMPToolChain::addClangTargetOptions(
    const toolchain::opt::ArgList &DriverArgs, toolchain::opt::ArgStringList &CC1Args,
    Action::OffloadKind DeviceOffloadingKind) const {

  if (DeviceOffloadingKind != Action::OFK_OpenMP)
    return;

  if (!DriverArgs.hasFlag(options::OPT_offloadlib, options::OPT_no_offloadlib,
                          true))
    return;
  addOpenMPDeviceRTL(getDriver(), DriverArgs, CC1Args, "", getTriple(), HostTC);
}
} // namespace language::Core::driver::toolchains
