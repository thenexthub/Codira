//===-- lib/Support/default-kinds.cpp ---------------------------*- C++ -*-===//
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

#include "language/Compability/Support/default-kinds.h"
#include "language/Compability/Common/idioms.h"

namespace language::Compability::common {

IntrinsicTypeDefaultKinds::IntrinsicTypeDefaultKinds() {}

IntrinsicTypeDefaultKinds &IntrinsicTypeDefaultKinds::set_defaultIntegerKind(
    int k) {
  defaultIntegerKind_ = k;
  return *this;
}

IntrinsicTypeDefaultKinds &IntrinsicTypeDefaultKinds::set_subscriptIntegerKind(
    int k) {
  subscriptIntegerKind_ = k;
  return *this;
}

IntrinsicTypeDefaultKinds &IntrinsicTypeDefaultKinds::set_sizeIntegerKind(
    int k) {
  sizeIntegerKind_ = k;
  return *this;
}

IntrinsicTypeDefaultKinds &IntrinsicTypeDefaultKinds::set_defaultRealKind(
    int k) {
  defaultRealKind_ = k;
  return *this;
}

IntrinsicTypeDefaultKinds &IntrinsicTypeDefaultKinds::set_doublePrecisionKind(
    int k) {
  doublePrecisionKind_ = k;
  return *this;
}

IntrinsicTypeDefaultKinds &IntrinsicTypeDefaultKinds::set_quadPrecisionKind(
    int k) {
  quadPrecisionKind_ = k;
  return *this;
}

IntrinsicTypeDefaultKinds &IntrinsicTypeDefaultKinds::set_defaultCharacterKind(
    int k) {
  defaultCharacterKind_ = k;
  return *this;
}

IntrinsicTypeDefaultKinds &IntrinsicTypeDefaultKinds::set_defaultLogicalKind(
    int k) {
  defaultLogicalKind_ = k;
  return *this;
}

int IntrinsicTypeDefaultKinds::GetDefaultKind(TypeCategory category) const {
  switch (category) {
  case TypeCategory::Integer:
  case TypeCategory::Unsigned:
    return defaultIntegerKind_;
  case TypeCategory::Real:
  case TypeCategory::Complex:
    return defaultRealKind_;
  case TypeCategory::Character:
    return defaultCharacterKind_;
  case TypeCategory::Logical:
    return defaultLogicalKind_;
  default:
    CRASH_NO_CASE;
    return 0;
  }
}
} // namespace language::Compability::common
