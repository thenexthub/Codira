//===--- TCE.cpp - TCE ToolChain Implementations ----------------*- C++ -*-===//
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

#include "TCE.h"

using namespace language::Core::driver;
using namespace language::Core::driver::toolchains;
using namespace language::Core;
using namespace toolchain::opt;

/// TCEToolChain - A tool chain using the toolchain bitcode tools to perform
/// all subcommands. See http://tce.cs.tut.fi for our peculiar target.
/// Currently does not support anything else but compilation.

TCEToolChain::TCEToolChain(const Driver &D, const toolchain::Triple &Triple,
                           const ArgList &Args)
    : ToolChain(D, Triple, Args) {
  // Path mangling to find libexec
  std::string Path(getDriver().Dir);

  Path += "/../libexec";
  getProgramPaths().push_back(Path);
}

TCEToolChain::~TCEToolChain() {}

bool TCEToolChain::IsMathErrnoDefault() const { return true; }

bool TCEToolChain::isPICDefault() const { return false; }

bool TCEToolChain::isPIEDefault(const toolchain::opt::ArgList &Args) const {
  return false;
}

bool TCEToolChain::isPICDefaultForced() const { return false; }

TCELEToolChain::TCELEToolChain(const Driver &D, const toolchain::Triple& Triple,
                               const ArgList &Args)
  : TCEToolChain(D, Triple, Args) {
}

TCELEToolChain::~TCELEToolChain() {}
