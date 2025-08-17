//===-------------- TypeOrdering.h - Total ordering for types ---*- C++ -*-===//
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
///
/// \file
/// Allows QualTypes to be sorted and hence used in maps and sets.
///
/// Defines language::Core::QualTypeOrdering, a total ordering on language::Core::QualType,
/// and hence enables QualType values to be sorted and to be used in
/// std::maps, std::sets, toolchain::DenseMaps, and toolchain::DenseSets.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_AST_TYPEORDERING_H
#define LANGUAGE_CORE_AST_TYPEORDERING_H

#include "language/Core/AST/CanonicalType.h"
#include "language/Core/AST/Type.h"
#include <functional>

namespace language::Core {

/// Function object that provides a total ordering on QualType values.
struct QualTypeOrdering {
  bool operator()(QualType T1, QualType T2) const {
    return std::less<void*>()(T1.getAsOpaquePtr(), T2.getAsOpaquePtr());
  }
};

}

namespace toolchain {

  template<> struct DenseMapInfo<language::Core::QualType> {
    static inline language::Core::QualType getEmptyKey() { return language::Core::QualType(); }

    static inline language::Core::QualType getTombstoneKey() {
      using language::Core::QualType;
      return QualType::getFromOpaquePtr(reinterpret_cast<language::Core::Type *>(-1));
    }

    static unsigned getHashValue(language::Core::QualType Val) {
      return (unsigned)((uintptr_t)Val.getAsOpaquePtr()) ^
            ((unsigned)((uintptr_t)Val.getAsOpaquePtr() >> 9));
    }

    static bool isEqual(language::Core::QualType LHS, language::Core::QualType RHS) {
      return LHS == RHS;
    }
  };

  template<> struct DenseMapInfo<language::Core::CanQualType> {
    static inline language::Core::CanQualType getEmptyKey() {
      return language::Core::CanQualType();
    }

    static inline language::Core::CanQualType getTombstoneKey() {
      using language::Core::CanQualType;
      return CanQualType::getFromOpaquePtr(reinterpret_cast<language::Core::Type *>(-1));
    }

    static unsigned getHashValue(language::Core::CanQualType Val) {
      return (unsigned)((uintptr_t)Val.getAsOpaquePtr()) ^
      ((unsigned)((uintptr_t)Val.getAsOpaquePtr() >> 9));
    }

    static bool isEqual(language::Core::CanQualType LHS, language::Core::CanQualType RHS) {
      return LHS == RHS;
    }
  };
}

#endif
