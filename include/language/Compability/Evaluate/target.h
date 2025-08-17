//===-- language/Compability/Evaluate/target.h -------------------------*- C++ -*-===//
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

// Represents the minimal amount of target architecture information required by
// semantics.

#ifndef LANGUAGE_COMPABILITY_EVALUATE_TARGET_H_
#define LANGUAGE_COMPABILITY_EVALUATE_TARGET_H_

#include "language/Compability/Common/enum-class.h"
#include "language/Compability/Common/enum-set.h"
#include "language/Compability/Common/target-rounding.h"
#include "language/Compability/Common/type-kinds.h"
#include "language/Compability/Evaluate/common.h"
#include "language/Compability/Support/Fortran.h"
#include <cstdint>

namespace language::Compability::evaluate {

using common::Rounding;

ENUM_CLASS(IeeeFeature, Denormal, Divide, Flags, Halting, Inf, Io, NaN,
    Rounding, Sqrt, Standard, Subnormal, UnderflowControl)

using IeeeFeatures = common::EnumSet<IeeeFeature, 16>;

class TargetCharacteristics {
public:
  TargetCharacteristics();
  TargetCharacteristics &operator=(const TargetCharacteristics &) = default;

  bool isBigEndian() const { return isBigEndian_; }
  void set_isBigEndian(bool isBig = true);

  bool haltingSupportIsUnknownAtCompileTime() const {
    return haltingSupportIsUnknownAtCompileTime_;
  }
  void set_haltingSupportIsUnknownAtCompileTime(bool yes = true) {
    haltingSupportIsUnknownAtCompileTime_ = yes;
  }

  bool areSubnormalsFlushedToZero() const {
    return areSubnormalsFlushedToZero_;
  }
  void set_areSubnormalsFlushedToZero(bool yes = true);

  // Check if a given real kind has flushing control.
  bool hasSubnormalFlushingControl(int kind) const;
  // Check if any or all real kinds have flushing control.
  bool hasSubnormalFlushingControl(bool any = false) const;
  void set_hasSubnormalFlushingControl(int kind, bool yes = true);

  // Check if a given real kind has support for raising a nonstandard
  // ieee_denorm exception.
  bool hasSubnormalExceptionSupport(int kind) const;
  // Check if all real kinds have support for the ieee_denorm exception.
  bool hasSubnormalExceptionSupport() const;
  void set_hasSubnormalExceptionSupport(int kind, bool yes = true);

  Rounding roundingMode() const { return roundingMode_; }
  void set_roundingMode(Rounding);

  void set_ieeeFeature(IeeeFeature ieeeFeature, bool yes = true) {
    if (yes) {
      ieeeFeatures_.set(ieeeFeature);
    } else {
      ieeeFeatures_.reset(ieeeFeature);
    }
  }

  std::size_t procedurePointerByteSize() const {
    return procedurePointerByteSize_;
  }
  std::size_t procedurePointerAlignment() const {
    return procedurePointerAlignment_;
  }
  std::size_t descriptorAlignment() const { return descriptorAlignment_; }
  std::size_t maxByteSize() const { return maxByteSize_; }
  std::size_t maxAlignment() const { return maxAlignment_; }

  static bool CanSupportType(common::TypeCategory, std::int64_t kind);
  bool EnableType(common::TypeCategory category, std::int64_t kind,
      std::size_t byteSize, std::size_t align);
  void DisableType(common::TypeCategory category, std::int64_t kind);

  std::size_t GetByteSize(
      common::TypeCategory category, std::int64_t kind) const;
  std::size_t GetAlignment(
      common::TypeCategory category, std::int64_t kind) const;
  bool IsTypeEnabled(common::TypeCategory category, std::int64_t kind) const;

  int SelectedIntKind(std::int64_t precision = 0) const;
  int SelectedLogicalKind(std::int64_t bits = 1) const;
  int SelectedRealKind(std::int64_t precision = 0, std::int64_t range = 0,
      std::int64_t radix = 2) const;

  static Rounding defaultRounding;

  const std::string &compilerOptionsString() const {
    return compilerOptionsString_;
  };
  TargetCharacteristics &set_compilerOptionsString(std::string x) {
    compilerOptionsString_ = x;
    return *this;
  }

  const std::string &compilerVersionString() const {
    return compilerVersionString_;
  };
  TargetCharacteristics &set_compilerVersionString(std::string x) {
    compilerVersionString_ = x;
    return *this;
  }

  bool isPPC() const { return isPPC_; }
  void set_isPPC(bool isPPC = false);

  bool isSPARC() const { return isSPARC_; }
  void set_isSPARC(bool isSPARC = false);

  bool isOSWindows() const { return isOSWindows_; }
  void set_isOSWindows(bool isOSWindows = false) {
    isOSWindows_ = isOSWindows;
  };

  IeeeFeatures &ieeeFeatures() { return ieeeFeatures_; }
  const IeeeFeatures &ieeeFeatures() const { return ieeeFeatures_; }

  std::size_t integerKindForPointer() { return integerKindForPointer_; }
  void set_integerKindForPointer(std::size_t newKind) {
    integerKindForPointer_ = newKind;
  }

private:
  static constexpr int maxKind{common::maxKind};
  std::uint8_t byteSize_[common::TypeCategory_enumSize][maxKind + 1]{};
  std::uint8_t align_[common::TypeCategory_enumSize][maxKind + 1]{};
  bool isBigEndian_{false};
  bool isPPC_{false};
  bool isSPARC_{false};
  bool isOSWindows_{false};
  bool haltingSupportIsUnknownAtCompileTime_{false};
  bool areSubnormalsFlushedToZero_{false};
  bool hasSubnormalFlushingControl_[maxKind + 1]{};
  bool hasSubnormalExceptionSupport_[maxKind + 1]{};
  Rounding roundingMode_{defaultRounding};
  std::size_t procedurePointerByteSize_{8};
  std::size_t procedurePointerAlignment_{8};
  std::size_t descriptorAlignment_{8};
  std::size_t maxByteSize_{8 /*at least*/};
  std::size_t maxAlignment_{8 /*at least*/};
  std::string compilerOptionsString_;
  std::string compilerVersionString_;
  IeeeFeatures ieeeFeatures_{IeeeFeature::Denormal, IeeeFeature::Divide,
      IeeeFeature::Flags, IeeeFeature::Halting, IeeeFeature::Inf,
      IeeeFeature::Io, IeeeFeature::NaN, IeeeFeature::Rounding,
      IeeeFeature::Sqrt, IeeeFeature::Standard, IeeeFeature::Subnormal,
      IeeeFeature::UnderflowControl};
  std::size_t integerKindForPointer_{8}; /* For 64 bit pointer */
};

} // namespace language::Compability::evaluate
#endif // FORTRAN_EVALUATE_TARGET_H_
