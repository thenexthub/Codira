//===--- OperatorKinds.h - C++ Overloaded Operators -------------*- C++ -*-===//
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
/// Defines an enumeration for C++ overloaded operators.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_BASIC_OPERATORKINDS_H
#define LANGUAGE_CORE_BASIC_OPERATORKINDS_H

namespace language::Core {

/// Enumeration specifying the different kinds of C++ overloaded
/// operators.
enum OverloadedOperatorKind : int {
  OO_None,                ///< Not an overloaded operator
#define OVERLOADED_OPERATOR(Name,Spelling,Token,Unary,Binary,MemberOnly) \
  OO_##Name,
#include "language/Core/Basic/OperatorKinds.def"
  NUM_OVERLOADED_OPERATORS
};

/// Retrieve the spelling of the given overloaded operator, without
/// the preceding "operator" keyword.
const char *getOperatorSpelling(OverloadedOperatorKind Operator);

/// Get the other overloaded operator that the given operator can be rewritten
/// into, if any such operator exists.
inline OverloadedOperatorKind
getRewrittenOverloadedOperator(OverloadedOperatorKind Kind) {
  switch (Kind) {
  case OO_Less:
  case OO_LessEqual:
  case OO_Greater:
  case OO_GreaterEqual:
    return OO_Spaceship;

  case OO_ExclaimEqual:
    return OO_EqualEqual;

  default:
    return OO_None;
  }
}

/// Determine if this is a compound assignment operator.
inline bool isCompoundAssignmentOperator(OverloadedOperatorKind Kind) {
  return Kind >= OO_PlusEqual && Kind <= OO_PipeEqual;
}

} // end namespace language::Core

#endif
