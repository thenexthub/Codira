//===--- SPIR.cpp - Implement SPIR and SPIR-V target feature support ------===//
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
//
// This file implements SPIR and SPIR-V TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#include "SPIR.h"
#include "AMDGPU.h"
#include "language/Core/Basic/MacroBuilder.h"
#include "language/Core/Basic/TargetBuiltins.h"
#include "toolchain/TargetParser/TargetParser.h"

using namespace language::Core;
using namespace language::Core::targets;

static constexpr int NumBuiltins =
    language::Core::SPIRV::LastTSBuiltin - Builtin::FirstTSBuiltin;

#define GET_BUILTIN_STR_TABLE
#include "language/Core/Basic/BuiltinsSPIRVCommon.inc"
#undef GET_BUILTIN_STR_TABLE

static constexpr Builtin::Info BuiltinInfos[] = {
#define GET_BUILTIN_INFOS
#include "language/Core/Basic/BuiltinsSPIRVCommon.inc"
#undef GET_BUILTIN_INFOS
};

namespace CL {
#define GET_BUILTIN_STR_TABLE
#include "language/Core/Basic/BuiltinsSPIRVCL.inc"
#undef GET_BUILTIN_STR_TABLE

static constexpr Builtin::Info BuiltinInfos[] = {
#define GET_BUILTIN_INFOS
#include "language/Core/Basic/BuiltinsSPIRVCL.inc"
#undef GET_BUILTIN_INFOS
};
} // namespace CL

namespace VK {
#define GET_BUILTIN_STR_TABLE
#include "language/Core/Basic/BuiltinsSPIRVVK.inc"
#undef GET_BUILTIN_STR_TABLE

static constexpr Builtin::Info BuiltinInfos[] = {
#define GET_BUILTIN_INFOS
#include "language/Core/Basic/BuiltinsSPIRVVK.inc"
#undef GET_BUILTIN_INFOS
};
} // namespace VK

static_assert(std::size(BuiltinInfos) + std::size(CL::BuiltinInfos) +
                  std::size(VK::BuiltinInfos) ==
              NumBuiltins);

toolchain::SmallVector<Builtin::InfosShard>
BaseSPIRVTargetInfo::getTargetBuiltins() const {
  return {{&BuiltinStrings, BuiltinInfos},
          {&VK::BuiltinStrings, VK::BuiltinInfos},
          {&CL::BuiltinStrings, CL::BuiltinInfos}};
}

void SPIRTargetInfo::getTargetDefines(const LangOptions &Opts,
                                      MacroBuilder &Builder) const {
  DefineStd(Builder, "SPIR", Opts);
}

void SPIR32TargetInfo::getTargetDefines(const LangOptions &Opts,
                                        MacroBuilder &Builder) const {
  SPIRTargetInfo::getTargetDefines(Opts, Builder);
  DefineStd(Builder, "SPIR32", Opts);
}

void SPIR64TargetInfo::getTargetDefines(const LangOptions &Opts,
                                        MacroBuilder &Builder) const {
  SPIRTargetInfo::getTargetDefines(Opts, Builder);
  DefineStd(Builder, "SPIR64", Opts);
}

void BaseSPIRVTargetInfo::getTargetDefines(const LangOptions &Opts,
                                           MacroBuilder &Builder) const {
  DefineStd(Builder, "SPIRV", Opts);
  if (Opts.HLSL)
    DefineStd(Builder, "spirv", Opts);
}

void SPIRVTargetInfo::getTargetDefines(const LangOptions &Opts,
                                       MacroBuilder &Builder) const {
  BaseSPIRVTargetInfo::getTargetDefines(Opts, Builder);
}

void SPIRV32TargetInfo::getTargetDefines(const LangOptions &Opts,
                                         MacroBuilder &Builder) const {
  BaseSPIRVTargetInfo::getTargetDefines(Opts, Builder);
  DefineStd(Builder, "SPIRV32", Opts);
}

void SPIRV64TargetInfo::getTargetDefines(const LangOptions &Opts,
                                         MacroBuilder &Builder) const {
  BaseSPIRVTargetInfo::getTargetDefines(Opts, Builder);
  DefineStd(Builder, "SPIRV64", Opts);
}

static const AMDGPUTargetInfo AMDGPUTI(toolchain::Triple("amdgcn-amd-amdhsa"), {});

ArrayRef<const char *> SPIRV64AMDGCNTargetInfo::getGCCRegNames() const {
  return AMDGPUTI.getGCCRegNames();
}

bool SPIRV64AMDGCNTargetInfo::initFeatureMap(
    toolchain::StringMap<bool> &Features, DiagnosticsEngine &Diags, StringRef,
    const std::vector<std::string> &FeatureVec) const {
  toolchain::AMDGPU::fillAMDGPUFeatureMap({}, getTriple(), Features);

  return TargetInfo::initFeatureMap(Features, Diags, {}, FeatureVec);
}

bool SPIRV64AMDGCNTargetInfo::validateAsmConstraint(
    const char *&Name, TargetInfo::ConstraintInfo &Info) const {
  return AMDGPUTI.validateAsmConstraint(Name, Info);
}

std::string
SPIRV64AMDGCNTargetInfo::convertConstraint(const char *&Constraint) const {
  return AMDGPUTI.convertConstraint(Constraint);
}

toolchain::SmallVector<Builtin::InfosShard>
SPIRV64AMDGCNTargetInfo::getTargetBuiltins() const {
  return AMDGPUTI.getTargetBuiltins();
}

void SPIRV64AMDGCNTargetInfo::getTargetDefines(const LangOptions &Opts,
                                               MacroBuilder &Builder) const {
  BaseSPIRVTargetInfo::getTargetDefines(Opts, Builder);
  DefineStd(Builder, "SPIRV64", Opts);

  Builder.defineMacro("__AMD__");
  Builder.defineMacro("__AMDGPU__");
  Builder.defineMacro("__AMDGCN__");
}

void SPIRV64AMDGCNTargetInfo::setAuxTarget(const TargetInfo *Aux) {
  assert(Aux && "Cannot invoke setAuxTarget without a valid auxiliary target!");

  // This is a 1:1 copy of AMDGPUTargetInfo::setAuxTarget()
  assert(HalfFormat == Aux->HalfFormat);
  assert(FloatFormat == Aux->FloatFormat);
  assert(DoubleFormat == Aux->DoubleFormat);

  // On x86_64 long double is 80-bit extended precision format, which is
  // not supported by AMDGPU. 128-bit floating point format is also not
  // supported by AMDGPU. Therefore keep its own format for these two types.
  auto SaveLongDoubleFormat = LongDoubleFormat;
  auto SaveFloat128Format = Float128Format;
  auto SaveLongDoubleWidth = LongDoubleWidth;
  auto SaveLongDoubleAlign = LongDoubleAlign;
  copyAuxTarget(Aux);
  LongDoubleFormat = SaveLongDoubleFormat;
  Float128Format = SaveFloat128Format;
  LongDoubleWidth = SaveLongDoubleWidth;
  LongDoubleAlign = SaveLongDoubleAlign;
  // For certain builtin types support on the host target, claim they are
  // supported to pass the compilation of the host code during the device-side
  // compilation.
  // FIXME: As the side effect, we also accept `__float128` uses in the device
  // code. To reject these builtin types supported in the host target but not in
  // the device target, one approach would support `device_builtin` attribute
  // so that we could tell the device builtin types from the host ones. This
  // also solves the different representations of the same builtin type, such
  // as `size_t` in the MSVC environment.
  if (Aux->hasFloat128Type()) {
    HasFloat128 = true;
    Float128Format = DoubleFormat;
  }
}
