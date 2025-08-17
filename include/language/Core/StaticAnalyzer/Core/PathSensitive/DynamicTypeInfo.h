//===- DynamicTypeInfo.h - Runtime type information -------------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_DYNAMICTYPEINFO_H
#define LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_DYNAMICTYPEINFO_H

#include "language/Core/AST/Type.h"

namespace language::Core {
namespace ento {

/// Stores the currently inferred strictest bound on the runtime type
/// of a region in a given state along the analysis path.
class DynamicTypeInfo {
public:
  DynamicTypeInfo() {}

  DynamicTypeInfo(QualType Ty, bool CanBeSub = true)
      : DynTy(Ty), CanBeASubClass(CanBeSub) {}

  /// Returns false if the type information is precise (the type 'DynTy' is
  /// the only type in the lattice), true otherwise.
  bool canBeASubClass() const { return CanBeASubClass; }

  /// Returns true if the dynamic type info is available.
  bool isValid() const { return !DynTy.isNull(); }

  /// Returns the currently inferred upper bound on the runtime type.
  QualType getType() const { return DynTy; }

  operator bool() const { return isValid(); }

  bool operator==(const DynamicTypeInfo &RHS) const {
    return DynTy == RHS.DynTy && CanBeASubClass == RHS.CanBeASubClass;
  }

  void Profile(toolchain::FoldingSetNodeID &ID) const {
    ID.Add(DynTy);
    ID.AddBoolean(CanBeASubClass);
  }

private:
  QualType DynTy;
  bool CanBeASubClass;
};

} // namespace ento
} // namespace language::Core

#endif // LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_DYNAMICTYPEINFO_H
