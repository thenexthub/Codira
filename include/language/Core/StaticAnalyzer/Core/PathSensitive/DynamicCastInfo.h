//===- DynamicCastInfo.h - Runtime cast information -------------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_DYNAMICCASTINFO_H
#define LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_DYNAMICCASTINFO_H

#include "language/Core/AST/Type.h"

namespace language::Core {
namespace ento {

class DynamicCastInfo {
public:
  enum CastResult { Success, Failure };

  DynamicCastInfo(QualType from, QualType to, CastResult resultKind)
      : From(from), To(to), ResultKind(resultKind) {}

  QualType from() const { return From; }
  QualType to() const { return To; }

  bool equals(QualType from, QualType to) const {
    return From == from && To == to;
  }

  bool succeeds() const { return ResultKind == CastResult::Success; }
  bool fails() const { return ResultKind == CastResult::Failure; }

  bool operator==(const DynamicCastInfo &RHS) const {
    return From == RHS.From && To == RHS.To;
  }
  bool operator<(const DynamicCastInfo &RHS) const {
    return From < RHS.From && To < RHS.To;
  }

  void Profile(toolchain::FoldingSetNodeID &ID) const {
    ID.Add(From);
    ID.Add(To);
    ID.AddInteger(ResultKind);
  }

private:
  QualType From, To;
  CastResult ResultKind;
};

} // namespace ento
} // namespace language::Core

#endif // LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_DYNAMICCASTINFO_H
