//===------ Primitives.h - Types for the constexpr VM -----------*- C++ -*-===//
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
// Utilities and helper functions for all primitive types:
//  - Integral
//  - Floating
//  - Boolean
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_AST_INTERP_PRIMITIVES_H
#define LANGUAGE_CORE_AST_INTERP_PRIMITIVES_H

#include "language/Core/AST/ComparisonCategories.h"

namespace language::Core {
namespace interp {

/// Helper to compare two comparable types.
template <typename T> ComparisonCategoryResult Compare(const T &X, const T &Y) {
  if (X < Y)
    return ComparisonCategoryResult::Less;
  if (X > Y)
    return ComparisonCategoryResult::Greater;
  return ComparisonCategoryResult::Equal;
}

} // namespace interp
} // namespace language::Core

#endif
