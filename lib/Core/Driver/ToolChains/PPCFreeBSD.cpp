//===-- PPCFreeBSD.cpp - PowerPC ToolChain Implementations ------*- C++ -*-===//
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

#include "PPCFreeBSD.h"
#include "language/Core/Driver/Driver.h"
#include "language/Core/Driver/Options.h"
#include "toolchain/Support/Path.h"

using namespace language::Core::driver::toolchains;
using namespace toolchain::opt;

void PPCFreeBSDToolChain::AddClangSystemIncludeArgs(
    const ArgList &DriverArgs, ArgStringList &CC1Args) const {
  if (!DriverArgs.hasArg(language::Core::driver::options::OPT_nostdinc) &&
      !DriverArgs.hasArg(options::OPT_nobuiltininc)) {
    const Driver &D = getDriver();
    SmallString<128> P(D.ResourceDir);
    toolchain::sys::path::append(P, "include", "ppc_wrappers");
    addSystemInclude(DriverArgs, CC1Args, P);
  }

  FreeBSD::AddClangSystemIncludeArgs(DriverArgs, CC1Args);
}
