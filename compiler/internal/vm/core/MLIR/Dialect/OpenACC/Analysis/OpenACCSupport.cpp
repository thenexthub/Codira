//===- OpenACCSupport.cpp - OpenACCSupport Implementation -----------------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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
// This file implements the OpenACCSupport analysis interface.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/OpenACC/Analysis/OpenACCSupport.h"
#include "mlir/Dialect/OpenACC/OpenACCUtils.h"

namespace mlir {
namespace acc {

std::string OpenACCSupport::getVariableName(Value v) {
  if (impl)
    return impl->getVariableName(v);
  return acc::getVariableName(v);
}

std::string OpenACCSupport::getRecipeName(RecipeKind kind, Type type,
                                          Value var) {
  if (impl)
    return impl->getRecipeName(kind, type, var);
  // The default implementation assumes that only type matters
  // and the actual instance of variable is not relevant.
  auto recipeName = acc::getRecipeName(kind, type);
  if (recipeName.empty())
    emitNYI(var ? var.getLoc() : UnknownLoc::get(type.getContext()),
            "variable privatization (incomplete recipe name handling)");
  return recipeName;
}

InFlightDiagnostic OpenACCSupport::emitNYI(Location loc, const Twine &message) {
  if (impl)
    return impl->emitNYI(loc, message);
  return mlir::emitError(loc, "not yet implemented: " + message);
}

remark::detail::InFlightRemark
OpenACCSupport::emitRemark(Operation *op, const Twine &message,
                           toolchain::StringRef category) {
  if (impl)
    return impl->emitRemark(op, message, category);
  return acc::emitRemark(op, message, category);
}

bool OpenACCSupport::isValidSymbolUse(Operation *user, SymbolRefAttr symbol,
                                      Operation **definingOpPtr) {
  if (impl)
    return impl->isValidSymbolUse(user, symbol, definingOpPtr);
  return acc::isValidSymbolUse(user, symbol, definingOpPtr);
}

bool OpenACCSupport::isValidValueUse(Value v, Region &region) {
  if (impl)
    return impl->isValidValueUse(v, region);
  return acc::isValidValueUse(v, region);
}

} // namespace acc
} // namespace mlir
