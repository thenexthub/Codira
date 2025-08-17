//===-- Tools/TargetSetup.h ------------------------------------- *-C++-*-===//
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

#ifndef LANGUAGE_COMPABILITY_TOOLS_TARGET_SETUP_H
#define LANGUAGE_COMPABILITY_TOOLS_TARGET_SETUP_H

#include "language/Compability/Common/float128.h"
#include "language/Compability/Evaluate/target.h"
#include "language/Compability/Frontend/TargetOptions.h"
#include "toolchain/Target/TargetMachine.h"
#include <cfloat>

namespace language::Compability::tools {

[[maybe_unused]] inline static void setUpTargetCharacteristics(
    language::Compability::evaluate::TargetCharacteristics &targetCharacteristics,
    const toolchain::TargetMachine &targetMachine,
    const language::Compability::frontend::TargetOptions &targetOptions,
    const std::string &compilerVersion, const std::string &compilerOptions) {

  const toolchain::Triple &targetTriple{targetMachine.getTargetTriple()};

  if (targetTriple.getArch() == toolchain::Triple::ArchType::x86_64) {
    targetCharacteristics.set_hasSubnormalFlushingControl(/*kind=*/3);
    targetCharacteristics.set_hasSubnormalFlushingControl(/*kind=*/4);
    targetCharacteristics.set_hasSubnormalFlushingControl(/*kind=*/8);
    // ieee_denorm exception support is nonstandard.
    targetCharacteristics.set_hasSubnormalExceptionSupport(/*kind=*/3);
    targetCharacteristics.set_hasSubnormalExceptionSupport(/*kind=*/4);
    targetCharacteristics.set_hasSubnormalExceptionSupport(/*kind=*/8);
  }

  if (targetTriple.isARM() || targetTriple.isAArch64()) {
    targetCharacteristics.set_haltingSupportIsUnknownAtCompileTime();
    targetCharacteristics.set_ieeeFeature(
        evaluate::IeeeFeature::Halting, false);
    targetCharacteristics.set_ieeeFeature(
        evaluate::IeeeFeature::Standard, false);
    targetCharacteristics.set_hasSubnormalFlushingControl(/*kind=*/3);
    targetCharacteristics.set_hasSubnormalFlushingControl(/*kind=*/4);
    targetCharacteristics.set_hasSubnormalFlushingControl(/*kind=*/8);
  }

  switch (targetTriple.getArch()) {
  case toolchain::Triple::ArchType::amdgcn:
  case toolchain::Triple::ArchType::x86_64:
    break;
  default:
    targetCharacteristics.DisableType(
        language::Compability::common::TypeCategory::Real, /*kind=*/10);
    targetCharacteristics.DisableType(
        language::Compability::common::TypeCategory::Complex, /*kind=*/10);
    break;
  }

  // Check for kind=16 support. See flang/runtime/Float128Math/math-entries.h.
  // TODO: Take this from TargetInfo::getLongDoubleFormat for cross compilation.
#ifdef FLANG_RUNTIME_F128_MATH_LIB
  constexpr bool f128Support = true; // use libquadmath wrappers
#elif HAS_LDBL128
  constexpr bool f128Support = true; // use libm wrappers
#else
  constexpr bool f128Support = false;
#endif

  if constexpr (!f128Support) {
    targetCharacteristics.DisableType(language::Compability::common::TypeCategory::Real, 16);
    targetCharacteristics.DisableType(
        language::Compability::common::TypeCategory::Complex, 16);
  }

  for (auto realKind : targetOptions.disabledRealKinds) {
    targetCharacteristics.DisableType(common::TypeCategory::Real, realKind);
    targetCharacteristics.DisableType(common::TypeCategory::Complex, realKind);
  }

  for (auto intKind : targetOptions.disabledIntegerKinds)
    targetCharacteristics.DisableType(common::TypeCategory::Integer, intKind);

  targetCharacteristics.set_compilerOptionsString(compilerOptions)
      .set_compilerVersionString(compilerVersion);

  if (targetTriple.isPPC())
    targetCharacteristics.set_isPPC(true);

  if (targetTriple.isSPARC())
    targetCharacteristics.set_isSPARC(true);

  if (targetTriple.isOSWindows())
    targetCharacteristics.set_isOSWindows(true);

  // Currently the integer kind happens to be the same as the byte size
  targetCharacteristics.set_integerKindForPointer(
      targetTriple.getArchPointerBitWidth() / 8);

  // TODO: use target machine data layout to set-up the target characteristics
  // type size and alignment info.
}

} // namespace language::Compability::tools

#endif // FORTRAN_TOOLS_TARGET_SETUP_H
