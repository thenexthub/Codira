//===- BaseSubobject.h - BaseSubobject class --------------------*- C++ -*-===//
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
// This file provides a definition of the BaseSubobject class.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_AST_BASESUBOBJECT_H
#define LANGUAGE_CORE_AST_BASESUBOBJECT_H

#include "language/Core/AST/CharUnits.h"
#include "language/Core/AST/DeclCXX.h"
#include "toolchain/ADT/DenseMapInfo.h"
#include "toolchain/Support/type_traits.h"
#include <cstdint>
#include <utility>

namespace language::Core {

class CXXRecordDecl;

// BaseSubobject - Uniquely identifies a direct or indirect base class.
// Stores both the base class decl and the offset from the most derived class to
// the base class. Used for vtable and VTT generation.
class BaseSubobject {
  /// Base - The base class declaration.
  const CXXRecordDecl *Base;

  /// BaseOffset - The offset from the most derived class to the base class.
  CharUnits BaseOffset;

public:
  BaseSubobject() = default;
  BaseSubobject(const CXXRecordDecl *Base, CharUnits BaseOffset)
      : Base(Base), BaseOffset(BaseOffset) {}

  /// getBase - Returns the base class declaration.
  const CXXRecordDecl *getBase() const { return Base; }

  /// getBaseOffset - Returns the base class offset.
  CharUnits getBaseOffset() const { return BaseOffset; }

  friend bool operator==(const BaseSubobject &LHS, const BaseSubobject &RHS) {
    return LHS.Base == RHS.Base && LHS.BaseOffset == RHS.BaseOffset;
 }
};

} // namespace language::Core

namespace toolchain {

template<> struct DenseMapInfo<language::Core::BaseSubobject> {
  static language::Core::BaseSubobject getEmptyKey() {
    return language::Core::BaseSubobject(
      DenseMapInfo<const language::Core::CXXRecordDecl *>::getEmptyKey(),
      language::Core::CharUnits::fromQuantity(DenseMapInfo<int64_t>::getEmptyKey()));
  }

  static language::Core::BaseSubobject getTombstoneKey() {
    return language::Core::BaseSubobject(
      DenseMapInfo<const language::Core::CXXRecordDecl *>::getTombstoneKey(),
      language::Core::CharUnits::fromQuantity(DenseMapInfo<int64_t>::getTombstoneKey()));
  }

  static unsigned getHashValue(const language::Core::BaseSubobject &Base) {
    using PairTy = std::pair<const language::Core::CXXRecordDecl *, language::Core::CharUnits>;

    return DenseMapInfo<PairTy>::getHashValue(PairTy(Base.getBase(),
                                                     Base.getBaseOffset()));
  }

  static bool isEqual(const language::Core::BaseSubobject &LHS,
                      const language::Core::BaseSubobject &RHS) {
    return LHS == RHS;
  }
};

} // namespace toolchain

#endif // LANGUAGE_CORE_AST_BASESUBOBJECT_H
