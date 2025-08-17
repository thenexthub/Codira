//===--- ARM.h - ARM-specific (not AArch64) Tool Helpers --------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_ARCH_ARM_H
#define LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_ARCH_ARM_H

#include "language/Core/Driver/ToolChain.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Option/ArgList.h"
#include "toolchain/Option/Option.h"
#include "toolchain/TargetParser/ARMTargetParser.h"
#include "toolchain/TargetParser/Triple.h"
#include <string>
#include <vector>

namespace language::Core {
namespace driver {
namespace tools {
namespace arm {

std::string getARMTargetCPU(StringRef CPU, toolchain::StringRef Arch,
                            const toolchain::Triple &Triple);
std::string getARMArch(toolchain::StringRef Arch, const toolchain::Triple &Triple);
StringRef getARMCPUForMArch(toolchain::StringRef Arch, const toolchain::Triple &Triple);
toolchain::ARM::ArchKind getLLVMArchKindForARM(StringRef CPU, StringRef Arch,
                                          const toolchain::Triple &Triple);
StringRef getLLVMArchSuffixForARM(toolchain::StringRef CPU, toolchain::StringRef Arch,
                                  const toolchain::Triple &Triple);

void appendBE8LinkFlag(const toolchain::opt::ArgList &Args,
                       toolchain::opt::ArgStringList &CmdArgs,
                       const toolchain::Triple &Triple);
enum class ReadTPMode {
  Invalid,
  Soft,
  TPIDRURW,
  TPIDRURO,
  TPIDRPRW,
};

enum class FloatABI {
  Invalid,
  Soft,
  SoftFP,
  Hard,
};

FloatABI getDefaultFloatABI(const toolchain::Triple &Triple);
FloatABI getARMFloatABI(const ToolChain &TC, const toolchain::opt::ArgList &Args);
FloatABI getARMFloatABI(const Driver &D, const toolchain::Triple &Triple,
                        const toolchain::opt::ArgList &Args);
void setFloatABIInTriple(const Driver &D, const toolchain::opt::ArgList &Args,
                         toolchain::Triple &triple);
bool isHardTPSupported(const toolchain::Triple &Triple);
ReadTPMode getReadTPMode(const Driver &D, const toolchain::opt::ArgList &Args,
                         const toolchain::Triple &Triple, bool ForAS);
void setArchNameInTriple(const Driver &D, const toolchain::opt::ArgList &Args,
                         types::ID InputType, toolchain::Triple &Triple);

bool useAAPCSForMachO(const toolchain::Triple &T);
void getARMArchCPUFromArgs(const toolchain::opt::ArgList &Args,
                           toolchain::StringRef &Arch, toolchain::StringRef &CPU,
                           bool FromAs = false);
toolchain::ARM::FPUKind getARMTargetFeatures(const Driver &D,
                                        const toolchain::Triple &Triple,
                                        const toolchain::opt::ArgList &Args,
                                        std::vector<toolchain::StringRef> &Features,
                                        bool ForAS, bool ForMultilib = false);
int getARMSubArchVersionNumber(const toolchain::Triple &Triple);
bool isARMMProfile(const toolchain::Triple &Triple);
bool isARMAProfile(const toolchain::Triple &Triple);
bool isARMBigEndian(const toolchain::Triple &Triple, const toolchain::opt::ArgList &Args);
bool isARMEABIBareMetal(const toolchain::Triple &Triple);

} // end namespace arm
} // end namespace tools
} // end namespace driver
} // end namespace language::Core

#endif // LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_ARCH_ARM_H
