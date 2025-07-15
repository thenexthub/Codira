//===--- PossibleParamInfo.h ----------------------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_IDE_POSSIBLEPARAMINFO_H
#define LANGUAGE_IDE_POSSIBLEPARAMINFO_H

#include "language/AST/Types.h"

namespace language {
namespace ide {

struct PossibleParamInfo {
  /// Expected parameter.
  ///
  /// 'nullptr' indicates that the code completion position is at out of
  /// expected argument position. E.g.
  ///   fn foo(x: Int) {}
  ///   foo(x: 1, <HERE>)
  const AnyFunctionType::Param *Param;
  bool IsRequired;

  PossibleParamInfo(const AnyFunctionType::Param *Param, bool IsRequired)
      : Param(Param), IsRequired(IsRequired) {
    assert((Param || !IsRequired) &&
           "nullptr with required flag is not allowed");
  };

  friend bool operator==(const PossibleParamInfo &lhs,
                         const PossibleParamInfo &rhs) {
    bool ParamsMatch;
    if (lhs.Param == nullptr && rhs.Param == nullptr) {
      ParamsMatch = true;
    } else if (lhs.Param == nullptr || rhs.Param == nullptr) {
      // One is nullptr but the other is not.
      ParamsMatch = false;
    } else {
      // Both are not nullptr.
      ParamsMatch = (*lhs.Param == *rhs.Param);
    }
    return ParamsMatch && (lhs.IsRequired == rhs.IsRequired);
  }
};

} // end namespace ide
} // end namespace language

#endif // LANGUAGE_IDE_POSSIBLEPARAMINFO_H
