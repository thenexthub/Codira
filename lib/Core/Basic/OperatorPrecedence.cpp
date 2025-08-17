//===--- OperatorPrecedence.cpp ---------------------------------*- C++ -*-===//
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
/// Defines and computes precedence levels for binary/ternary operators.
///
//===----------------------------------------------------------------------===//
#include "language/Core/Basic/OperatorPrecedence.h"

namespace language::Core {

prec::Level getBinOpPrecedence(tok::TokenKind Kind, bool GreaterThanIsOperator,
                               bool CPlusPlus11) {
  switch (Kind) {
  case tok::greater:
    // C++ [temp.names]p3:
    //   [...] When parsing a template-argument-list, the first
    //   non-nested > is taken as the ending delimiter rather than a
    //   greater-than operator. [...]
    if (GreaterThanIsOperator)
      return prec::Relational;
    return prec::Unknown;

  case tok::greatergreater:
    // C++11 [temp.names]p3:
    //
    //   [...] Similarly, the first non-nested >> is treated as two
    //   consecutive but distinct > tokens, the first of which is
    //   taken as the end of the template-argument-list and completes
    //   the template-id. [...]
    if (GreaterThanIsOperator || !CPlusPlus11)
      return prec::Shift;
    return prec::Unknown;

  default:                        return prec::Unknown;
  case tok::comma:                return prec::Comma;
  case tok::equal:
  case tok::starequal:
  case tok::slashequal:
  case tok::percentequal:
  case tok::plusequal:
  case tok::minusequal:
  case tok::lesslessequal:
  case tok::greatergreaterequal:
  case tok::ampequal:
  case tok::caretequal:
  case tok::pipeequal:            return prec::Assignment;
  case tok::question:             return prec::Conditional;
  case tok::pipepipe:             return prec::LogicalOr;
  case tok::ampamp:               return prec::LogicalAnd;
  case tok::pipe:                 return prec::InclusiveOr;
  case tok::caret:                return prec::ExclusiveOr;
  case tok::amp:                  return prec::And;
  case tok::exclaimequal:
  case tok::equalequal:           return prec::Equality;
  case tok::lessequal:
  case tok::less:
  case tok::greaterequal:         return prec::Relational;
  case tok::spaceship:            return prec::Spaceship;
  case tok::lessless:             return prec::Shift;
  case tok::plus:
  case tok::minus:                return prec::Additive;
  case tok::percent:
  case tok::slash:
  case tok::star:                 return prec::Multiplicative;
  case tok::periodstar:
  case tok::arrowstar:            return prec::PointerToMember;
  }
}

}  // namespace language::Core
