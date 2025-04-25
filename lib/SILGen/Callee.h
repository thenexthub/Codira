//===--- Callee.h -----------------------------------------------*- C++ -*-===//
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

#ifndef SWIFT_SILGEN_CALLEE_H
#define SWIFT_SILGEN_CALLEE_H

#include "language/AST/ForeignAsyncConvention.h"
#include "language/AST/ForeignErrorConvention.h"
#include "language/AST/ForeignInfo.h"
#include "language/AST/Types.h"
#include "language/SIL/AbstractionPattern.h"

namespace language {
namespace Lowering {

class CalleeTypeInfo {
public:
  std::optional<AbstractionPattern> origFormalType;
  CanSILFunctionType substFnType;
  std::optional<AbstractionPattern> origResultType;
  CanType substResultType;
  ForeignInfo foreign;

private:
  std::optional<SILFunctionTypeRepresentation> overrideRep;

public:
  CalleeTypeInfo() = default;

  CalleeTypeInfo(
      CanSILFunctionType substFnType, AbstractionPattern origResultType,
      CanType substResultType,
      const std::optional<ForeignErrorConvention> &foreignError,
      const std::optional<ForeignAsyncConvention> &foreignAsync,
      ImportAsMemberStatus foreignSelf,
      std::optional<SILFunctionTypeRepresentation> overrideRep = std::nullopt)
      : origFormalType(std::nullopt), substFnType(substFnType),
        origResultType(origResultType), substResultType(substResultType),
        foreign{foreignSelf, foreignError, foreignAsync},
        overrideRep(overrideRep) {}

  CalleeTypeInfo(
      CanSILFunctionType substFnType, AbstractionPattern origResultType,
      CanType substResultType,
      std::optional<SILFunctionTypeRepresentation> overrideRep = std::nullopt)
      : origFormalType(std::nullopt), substFnType(substFnType),
        origResultType(origResultType), substResultType(substResultType),
        foreign(), overrideRep(overrideRep) {}

  SILFunctionTypeRepresentation getOverrideRep() const {
    return overrideRep.value_or(substFnType->getRepresentation());
  }
};

} // namespace Lowering
} // namespace language

#endif
