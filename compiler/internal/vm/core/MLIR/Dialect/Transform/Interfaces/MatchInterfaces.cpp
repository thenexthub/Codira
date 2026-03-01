//===- MatchInterfaces.cpp - Transform Dialect Interfaces -----------------===//
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

#include "mlir/Dialect/Transform/Interfaces/MatchInterfaces.h"

#include "vm/core/Support/InterleavedRange.h"

using namespace mlir;

//===----------------------------------------------------------------------===//
// Printing and parsing for match ops.
//===----------------------------------------------------------------------===//

/// Keyword syntax for positional specification inversion.
constexpr const static toolchain::StringLiteral kDimExceptKeyword = "except";

/// Keyword syntax for full inclusion in positional specification.
constexpr const static toolchain::StringLiteral kDimAllKeyword = "all";

ParseResult transform::parseTransformMatchDims(OpAsmParser &parser,
                                               DenseI64ArrayAttr &rawDimList,
                                               UnitAttr &isInverted,
                                               UnitAttr &isAll) {
  Builder &builder = parser.getBuilder();
  if (parser.parseOptionalKeyword(kDimAllKeyword).succeeded()) {
    rawDimList = builder.getDenseI64ArrayAttr({});
    isInverted = nullptr;
    isAll = builder.getUnitAttr();
    return success();
  }

  isAll = nullptr;
  isInverted = nullptr;
  if (parser.parseOptionalKeyword(kDimExceptKeyword).succeeded()) {
    isInverted = builder.getUnitAttr();
  }

  if (isInverted) {
    if (parser.parseLParen().failed())
      return failure();
  }

  SmallVector<int64_t> values;
  ParseResult listResult = parser.parseCommaSeparatedList(
      [&]() { return parser.parseInteger(values.emplace_back()); });
  if (listResult.failed())
    return failure();

  rawDimList = builder.getDenseI64ArrayAttr(values);

  if (isInverted) {
    if (parser.parseRParen().failed())
      return failure();
  }
  return success();
}

void transform::printTransformMatchDims(OpAsmPrinter &printer, Operation *op,
                                        DenseI64ArrayAttr rawDimList,
                                        UnitAttr isInverted, UnitAttr isAll) {
  if (isAll) {
    printer << kDimAllKeyword;
    return;
  }
  if (isInverted) {
    printer << kDimExceptKeyword << "(";
  }
  printer << toolchain::interleaved(rawDimList.asArrayRef());
  if (isInverted) {
    printer << ")";
  }
}

LogicalResult transform::verifyTransformMatchDimsOp(Operation *op,
                                                    ArrayRef<int64_t> raw,
                                                    bool inverted, bool all) {
  if (all) {
    if (inverted) {
      return op->emitOpError()
             << "cannot request both 'all' and 'inverted' values in the list";
    }
    if (!raw.empty()) {
      return op->emitOpError()
             << "cannot both request 'all' and specific values in the list";
    }
  }
  if (!all && raw.empty()) {
    return op->emitOpError() << "must request specific values in the list if "
                                "'all' is not specified";
  }
  SmallVector<int64_t> rawVector = toolchain::to_vector(raw);
  auto *it = toolchain::unique(rawVector);
  if (it != rawVector.end())
    return op->emitOpError() << "expected the listed values to be unique";

  return success();
}

DiagnosedSilenceableFailure transform::expandTargetSpecification(
    Location loc, bool isAll, bool isInverted, ArrayRef<int64_t> rawList,
    int64_t maxNumber, SmallVectorImpl<int64_t> &result) {
  assert(maxNumber > 0 && "expected size to be positive");
  assert(!(isAll && isInverted) && "cannot invert all");
  if (isAll) {
    result = toolchain::to_vector(toolchain::seq<int64_t>(0, maxNumber));
    return DiagnosedSilenceableFailure::success();
  }

  SmallVector<int64_t> expanded;
  toolchain::SmallDenseSet<int64_t> visited;
  expanded.reserve(rawList.size());
  SmallVectorImpl<int64_t> &target = isInverted ? expanded : result;
  for (int64_t raw : rawList) {
    int64_t updated = raw < 0 ? maxNumber + raw : raw;
    if (updated >= maxNumber) {
      return emitSilenceableFailure(loc)
             << "position overflow " << updated << " (updated from " << raw
             << ") for maximum " << maxNumber;
    }
    if (updated < 0) {
      return emitSilenceableFailure(loc) << "position underflow " << updated
                                         << " (updated from " << raw << ")";
    }
    if (!visited.insert(updated).second) {
      return emitSilenceableFailure(loc) << "repeated position " << updated
                                         << " (updated from " << raw << ")";
    }
    target.push_back(updated);
  }

  if (!isInverted)
    return DiagnosedSilenceableFailure::success();

  result.reserve(result.size() + (maxNumber - expanded.size()));
  for (int64_t candidate : toolchain::seq<int64_t>(0, maxNumber)) {
    if (toolchain::is_contained(expanded, candidate))
      continue;
    result.push_back(candidate);
  }

  return DiagnosedSilenceableFailure::success();
}

//===----------------------------------------------------------------------===//
// Generated interface implementation.
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/Transform/Interfaces/MatchInterfaces.cpp.inc"
