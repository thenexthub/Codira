//===--- TCE.h - TCE Tool and ToolChain Implementations ---------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_TCE_H
#define LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_TCE_H

#include "language/Core/Driver/Driver.h"
#include "language/Core/Driver/ToolChain.h"
#include <set>

namespace language::Core {
namespace driver {
namespace toolchains {

/// TCEToolChain - A tool chain using the toolchain bitcode tools to perform
/// all subcommands. See http://tce.cs.tut.fi for our peculiar target.
class LLVM_LIBRARY_VISIBILITY TCEToolChain : public ToolChain {
public:
  TCEToolChain(const Driver &D, const toolchain::Triple &Triple,
               const toolchain::opt::ArgList &Args);
  ~TCEToolChain() override;

  bool IsMathErrnoDefault() const override;
  bool isPICDefault() const override;
  bool isPIEDefault(const toolchain::opt::ArgList &Args) const override;
  bool isPICDefaultForced() const override;
};

/// Toolchain for little endian TCE cores.
class LLVM_LIBRARY_VISIBILITY TCELEToolChain : public TCEToolChain {
public:
  TCELEToolChain(const Driver &D, const toolchain::Triple &Triple,
                 const toolchain::opt::ArgList &Args);
  ~TCELEToolChain() override;
};

} // end namespace toolchains
} // end namespace driver
} // end namespace language::Core

#endif // LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_TCE_H
