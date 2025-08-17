//===-- UnresolvedSet.h - Unresolved sets of declarations  ------*- C++ -*-===//
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
//  This file defines the WeakInfo class, which is used to store
//  information about the target of a #pragma weak directive.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_SEMA_WEAK_H
#define LANGUAGE_CORE_SEMA_WEAK_H

#include "language/Core/Basic/SourceLocation.h"
#include "toolchain/ADT/DenseMapInfo.h"

namespace language::Core {

class IdentifierInfo;

/// Captures information about a \#pragma weak directive.
class WeakInfo {
  const IdentifierInfo *alias = nullptr; // alias (optional)
  SourceLocation loc;                    // for diagnostics
public:
  WeakInfo() = default;
  WeakInfo(const IdentifierInfo *Alias, SourceLocation Loc)
      : alias(Alias), loc(Loc) {}
  inline const IdentifierInfo *getAlias() const { return alias; }
  inline SourceLocation getLocation() const { return loc; }
  bool operator==(WeakInfo RHS) const = delete;
  bool operator!=(WeakInfo RHS) const = delete;

  struct DenseMapInfoByAliasOnly
      : private toolchain::DenseMapInfo<const IdentifierInfo *> {
    static inline WeakInfo getEmptyKey() {
      return WeakInfo(DenseMapInfo::getEmptyKey(), SourceLocation());
    }
    static inline WeakInfo getTombstoneKey() {
      return WeakInfo(DenseMapInfo::getTombstoneKey(), SourceLocation());
    }
    static unsigned getHashValue(const WeakInfo &W) {
      return DenseMapInfo::getHashValue(W.getAlias());
    }
    static bool isEqual(const WeakInfo &LHS, const WeakInfo &RHS) {
      return DenseMapInfo::isEqual(LHS.getAlias(), RHS.getAlias());
    }
  };
};

} // end namespace language::Core

#endif // LANGUAGE_CORE_SEMA_WEAK_H
