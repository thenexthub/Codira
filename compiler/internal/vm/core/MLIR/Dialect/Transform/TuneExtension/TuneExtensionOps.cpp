//===- TuneExtensionOps.cpp - Tune extension for the Transform dialect ----===//
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

#include "mlir/Dialect/Transform/IR/TransformOps.h"
#include "mlir/Dialect/Transform/Interfaces/TransformInterfaces.h"
#include "mlir/IR/OpImplementation.h"
#include "mlir/Interfaces/ControlFlowInterfaces.h"
#include "vm/core/Support/Debug.h"

#include "mlir/Dialect/Transform/TuneExtension/TuneExtensionOps.h"

using namespace mlir;

static ParseResult parseAlternativesOpSelectedRegion(
    OpAsmParser &parser, IntegerAttr &selectedRegionAttr,
    std::optional<OpAsmParser::UnresolvedOperand> &selectedRegionParam);

static void printAlternativesOpSelectedRegion(OpAsmPrinter &printer,
                                              Operation *op,
                                              IntegerAttr selectedRegionAttr,
                                              Value selectedRegionParam);

#define GET_OP_CLASSES
#include "mlir/Dialect/Transform/TuneExtension/TuneExtensionOps.cpp.inc"

#define DEBUG_TYPE "transform-tune"
#define DBGS() (toolchain::dbgs() << "[" DEBUG_TYPE "] ")

//===----------------------------------------------------------------------===//
// KnobOp
//===----------------------------------------------------------------------===//

void transform::tune::KnobOp::getEffects(
    SmallVectorImpl<MemoryEffects::EffectInstance> &effects) {
  producesHandle(getOperation()->getOpResults(), effects);
  onlyReadsPayload(effects);
}

DiagnosedSilenceableFailure
transform::tune::KnobOp::apply(transform::TransformRewriter &rewriter,
                               transform::TransformResults &results,
                               transform::TransformState &state) {
  if (getSelected()) {
    results.setParams(toolchain::cast<OpResult>(getResult()), *getSelected());
    return DiagnosedSilenceableFailure::success();
  }

  return emitDefiniteFailure()
         << "non-deterministic choice " << getName()
         << " is only resolved through providing a `selected` attr";
}

LogicalResult transform::tune::KnobOp::verify() {
  if (auto selected = getSelected()) {
    if (auto optionsArray = dyn_cast<ArrayAttr>(getOptions())) {
      if (!toolchain::is_contained(optionsArray, selected))
        return emitOpError("provided `selected` attribute is not an element of "
                           "`options` array of attributes");
    } else
      LLVM_DEBUG(DBGS() << "cannot verify `selected` attribute " << selected
                        << " is an element of `options` attribute "
                        << getOptions());
  }

  return success();
}

//===----------------------------------------------------------------------===//
// AlternativesOp
//===----------------------------------------------------------------------===//

static ParseResult parseAlternativesOpSelectedRegion(
    OpAsmParser &parser, IntegerAttr &selectedRegionAttr,
    std::optional<OpAsmParser::UnresolvedOperand> &selectedRegionParam) {
  size_t selectedRegionIdx;
  OptionalParseResult attrParseRes =
      parser.parseOptionalInteger(selectedRegionIdx);
  if (attrParseRes.has_value()) {
    if (failed(*attrParseRes))
      return failure();

    selectedRegionAttr = parser.getBuilder().getIndexAttr(selectedRegionIdx);
    return success();
  }

  OpAsmParser::UnresolvedOperand param;
  auto paramParseRes = parser.parseOptionalOperand(param);
  if (paramParseRes.has_value()) {
    if (failed(*paramParseRes))
      return failure();

    selectedRegionParam = param;
    return success();
  }

  return parser.emitError(parser.getCurrentLocation())
         << "expected either an integer attribute or a transform.param operand";
}

static void printAlternativesOpSelectedRegion(OpAsmPrinter &printer,
                                              Operation *op,
                                              IntegerAttr selectedRegionAttr,
                                              Value selectedRegionParam) {
  if (selectedRegionAttr)
    printer << selectedRegionAttr.getValue();
  if (selectedRegionParam)
    printer << selectedRegionParam;
}

OperandRange transform::tune::AlternativesOp::getEntrySuccessorOperands(
    RegionSuccessor successor) {
  // No operands will be forwarded to the region(s).
  return getOperands().slice(0, 0);
}

void transform::tune::AlternativesOp::getSuccessorRegions(
    RegionBranchPoint point, SmallVectorImpl<RegionSuccessor> &regions) {
  if (point.isParent())
    if (auto selectedRegionIdx = getSelectedRegionAttr())
      regions.emplace_back(
          &getAlternatives()[selectedRegionIdx->getSExtValue()],
          Block::BlockArgListType());
    else
      for (Region &alternative : getAlternatives())
        regions.emplace_back(&alternative, Block::BlockArgListType());
  else
    regions.push_back(RegionSuccessor::parent(getResults()));
}

void transform::tune::AlternativesOp::getRegionInvocationBounds(
    ArrayRef<Attribute> operands, SmallVectorImpl<InvocationBounds> &bounds) {
  (void)operands;
  bounds.reserve(getNumRegions());

  if (auto selectedRegionIdx = getSelectedRegionAttr()) {
    bounds.resize(getNumRegions(), InvocationBounds(0, 0));
    bounds[selectedRegionIdx->getSExtValue()] = InvocationBounds(1, 1);
  } else {
    bounds.resize(getNumRegions(), InvocationBounds(0, 1));
  }
}

void transform::tune::AlternativesOp::getEffects(
    SmallVectorImpl<MemoryEffects::EffectInstance> &effects) {
  onlyReadsHandle(getSelectedRegionParamMutable(), effects);
  producesHandle(getOperation()->getOpResults(), effects);
  // TODO: should effects from regions be forwarded?
}

DiagnosedSilenceableFailure
transform::tune::AlternativesOp::apply(transform::TransformRewriter &rewriter,
                                       transform::TransformResults &results,
                                       transform::TransformState &state) {
  std::optional<int64_t> selectedRegionIdx;

  if (auto selectedRegionAttr = getSelectedRegionAttr())
    selectedRegionIdx = selectedRegionAttr->getSExtValue();

  if (Value selectedRegionParam = getSelectedRegionParam()) {
    ArrayRef<Attribute> associatedAttrs = state.getParams(selectedRegionParam);
    IntegerAttr selectedRegionAttr;
    if (associatedAttrs.size() != 1 ||
        !(selectedRegionAttr = dyn_cast<IntegerAttr>(associatedAttrs[0])))
      return emitDefiniteFailure()
             << "param should hold exactly one integer attribute, got: "
             << associatedAttrs[0];
    selectedRegionIdx = selectedRegionAttr.getValue().getSExtValue();
  }

  if (!selectedRegionIdx)
    return emitDefiniteFailure() << "non-deterministic choice " << getName()
                                 << " is only resolved through providing a "
                                    "`selected_region` attr/param";

  if (*selectedRegionIdx < 0 || *selectedRegionIdx >= getNumRegions())
    return emitDefiniteFailure()
           << "'selected_region' attribute/param specifies region at index "
           << *selectedRegionIdx << " while op has only " << getNumRegions()
           << " regions";

  Region &selectedRegion = getRegion(*selectedRegionIdx);
  auto scope = state.make_region_scope(selectedRegion);
  Block &block = selectedRegion.front();
  // Apply the region's ops one by one.
  for (Operation &transform : block.without_terminator()) {
    DiagnosedSilenceableFailure result =
        state.applyTransform(cast<transform::TransformOpInterface>(transform));
    if (result.isDefiniteFailure())
      return result;

    if (result.isSilenceableFailure()) {
      for (const auto &res : getResults())
        results.set(res, {});
      return result;
    }
  }
  // Forward the operation mapping for values yielded from the region to the
  // values produced by the alternatives op.
  transform::detail::forwardTerminatorOperands(&block, state, results);
  return DiagnosedSilenceableFailure::success();
}

LogicalResult transform::tune::AlternativesOp::verify() {
  for (auto *region : getRegions()) {
    auto yieldTerminator =
        toolchain::dyn_cast_if_present<transform::YieldOp>(region->front().back());
    if (!yieldTerminator)
      return emitOpError() << "expected '"
                           << transform::YieldOp::getOperationName()
                           << "' as terminator";

    if (yieldTerminator->getNumOperands() != getNumResults())
      return yieldTerminator.emitOpError()
             << "expected terminator to have as many operands as the parent op "
                "has results";

    for (auto [i, operandType, resultType] : toolchain::zip_equal(
             toolchain::seq<unsigned>(0, yieldTerminator->getNumOperands()),
             yieldTerminator->getOperands().getType(), getResultTypes())) {
      if (operandType == resultType)
        continue;
      return yieldTerminator.emitOpError()
             << "the type of the terminator operand #" << i
             << " must match the type of the corresponding parent op result ("
             << operandType << " vs " << resultType << ")";
    }
  }

  if (auto selectedRegionAttr = getSelectedRegionAttr()) {
    int64_t regionIdx = selectedRegionAttr->getSExtValue();
    if (regionIdx < 0 || regionIdx >= getNumRegions())
      return emitOpError()
             << "'selected_region' attribute specifies region at index "
             << regionIdx << " while op has only " << getNumRegions()
             << " regions";
  }

  return success();
}
