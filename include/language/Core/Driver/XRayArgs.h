//===--- XRayArgs.h - Arguments for XRay ------------------------*- C++ -*-===//
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
#ifndef LANGUAGE_CORE_DRIVER_XRAYARGS_H
#define LANGUAGE_CORE_DRIVER_XRAYARGS_H

#include "language/Core/Basic/XRayInstr.h"
#include "language/Core/Driver/Types.h"
#include "toolchain/Option/Arg.h"
#include "toolchain/Option/ArgList.h"

namespace language::Core {
namespace driver {

class ToolChain;

class XRayArgs {
  std::vector<std::string> AlwaysInstrumentFiles;
  std::vector<std::string> NeverInstrumentFiles;
  std::vector<std::string> AttrListFiles;
  std::vector<std::string> ExtraDeps;
  std::vector<std::string> Modes;
  XRayInstrSet InstrumentationBundle;
  toolchain::opt::Arg *XRayInstrument = nullptr;
  bool XRayRT = true;
  bool XRayShared = false;

public:
  /// Parses the XRay arguments from an argument list.
  XRayArgs(const ToolChain &TC, const toolchain::opt::ArgList &Args);
  void addArgs(const ToolChain &TC, const toolchain::opt::ArgList &Args,
               toolchain::opt::ArgStringList &CmdArgs, types::ID InputType) const;

  bool needsXRayRt() const { return XRayInstrument && XRayRT; }
  bool needsXRayDSORt() const { return XRayInstrument && XRayRT && XRayShared; }
  toolchain::ArrayRef<std::string> modeList() const { return Modes; }
  XRayInstrSet instrumentationBundle() const { return InstrumentationBundle; }
};

} // namespace driver
} // namespace language::Core

#endif // LANGUAGE_CORE_DRIVER_XRAYARGS_H
