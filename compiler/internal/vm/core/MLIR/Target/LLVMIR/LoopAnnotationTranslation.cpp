//===- LoopAnnotationTranslation.cpp - Loop annotation export -------------===//
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

#include "LoopAnnotationTranslation.h"
#include "vm/core/IR/DebugInfoMetadata.h"

using namespace mlir;
using namespace mlir::LLVM;
using namespace mlir::LLVM::detail;

namespace {
/// Helper class that keeps the state of one attribute to metadata conversion.
struct LoopAnnotationConversion {
  LoopAnnotationConversion(LoopAnnotationAttr attr, Operation *op,
                           LoopAnnotationTranslation &loopAnnotationTranslation,
                           toolchain::LLVMContext &ctx)
      : attr(attr), op(op),
        loopAnnotationTranslation(loopAnnotationTranslation), ctx(ctx) {}

  /// Converts this struct's loop annotation into a corresponding LLVMIR
  /// metadata representation.
  toolchain::MDNode *convert();

  /// Conversion functions for different payload attribute kinds.
  void addUnitNode(StringRef name);
  void addUnitNode(StringRef name, BoolAttr attr);
  void addI32NodeWithVal(StringRef name, uint32_t val);
  void convertBoolNode(StringRef name, BoolAttr attr, bool negated = false);
  void convertI32Node(StringRef name, IntegerAttr attr);
  void convertFollowupNode(StringRef name, LoopAnnotationAttr attr);
  void convertLocation(FusedLoc attr);

  /// Conversion functions for each for each loop annotation sub-attribute.
  void convertLoopOptions(LoopVectorizeAttr options);
  void convertLoopOptions(LoopInterleaveAttr options);
  void convertLoopOptions(LoopUnrollAttr options);
  void convertLoopOptions(LoopUnrollAndJamAttr options);
  void convertLoopOptions(LoopLICMAttr options);
  void convertLoopOptions(LoopDistributeAttr options);
  void convertLoopOptions(LoopPipelineAttr options);
  void convertLoopOptions(LoopPeeledAttr options);
  void convertLoopOptions(LoopUnswitchAttr options);

  LoopAnnotationAttr attr;
  Operation *op;
  LoopAnnotationTranslation &loopAnnotationTranslation;
  toolchain::LLVMContext &ctx;
  toolchain::SmallVector<toolchain::Metadata *> metadataNodes;
};
} // namespace

void LoopAnnotationConversion::addUnitNode(StringRef name) {
  metadataNodes.push_back(
      toolchain::MDNode::get(ctx, {toolchain::MDString::get(ctx, name)}));
}

void LoopAnnotationConversion::addUnitNode(StringRef name, BoolAttr attr) {
  if (attr && attr.getValue())
    addUnitNode(name);
}

void LoopAnnotationConversion::addI32NodeWithVal(StringRef name, uint32_t val) {
  toolchain::Constant *cstValue = toolchain::ConstantInt::get(
      toolchain::IntegerType::get(ctx, /*NumBits=*/32), val, /*isSigned=*/false);
  metadataNodes.push_back(
      toolchain::MDNode::get(ctx, {toolchain::MDString::get(ctx, name),
                              toolchain::ConstantAsMetadata::get(cstValue)}));
}

void LoopAnnotationConversion::convertBoolNode(StringRef name, BoolAttr attr,
                                               bool negated) {
  if (!attr)
    return;
  bool val = negated ^ attr.getValue();
  toolchain::Constant *cstValue = toolchain::ConstantInt::getBool(ctx, val);
  metadataNodes.push_back(
      toolchain::MDNode::get(ctx, {toolchain::MDString::get(ctx, name),
                              toolchain::ConstantAsMetadata::get(cstValue)}));
}

void LoopAnnotationConversion::convertI32Node(StringRef name,
                                              IntegerAttr attr) {
  if (!attr)
    return;
  addI32NodeWithVal(name, attr.getInt());
}

void LoopAnnotationConversion::convertFollowupNode(StringRef name,
                                                   LoopAnnotationAttr attr) {
  if (!attr)
    return;

  toolchain::MDNode *node =
      loopAnnotationTranslation.translateLoopAnnotation(attr, op);

  metadataNodes.push_back(
      toolchain::MDNode::get(ctx, {toolchain::MDString::get(ctx, name), node}));
}

void LoopAnnotationConversion::convertLoopOptions(LoopVectorizeAttr options) {
  convertBoolNode("toolchain.loop.vectorize.enable", options.getDisable(), true);
  convertBoolNode("toolchain.loop.vectorize.predicate.enable",
                  options.getPredicateEnable());
  convertBoolNode("toolchain.loop.vectorize.scalable.enable",
                  options.getScalableEnable());
  convertI32Node("toolchain.loop.vectorize.width", options.getWidth());
  convertFollowupNode("toolchain.loop.vectorize.followup_vectorized",
                      options.getFollowupVectorized());
  convertFollowupNode("toolchain.loop.vectorize.followup_epilogue",
                      options.getFollowupEpilogue());
  convertFollowupNode("toolchain.loop.vectorize.followup_all",
                      options.getFollowupAll());
}

void LoopAnnotationConversion::convertLoopOptions(LoopInterleaveAttr options) {
  convertI32Node("toolchain.loop.interleave.count", options.getCount());
}

void LoopAnnotationConversion::convertLoopOptions(LoopUnrollAttr options) {
  if (auto disable = options.getDisable())
    addUnitNode(disable.getValue() ? "toolchain.loop.unroll.disable"
                                   : "toolchain.loop.unroll.enable");
  convertI32Node("toolchain.loop.unroll.count", options.getCount());
  convertBoolNode("toolchain.loop.unroll.runtime.disable",
                  options.getRuntimeDisable());
  addUnitNode("toolchain.loop.unroll.full", options.getFull());
  convertFollowupNode("toolchain.loop.unroll.followup_unrolled",
                      options.getFollowupUnrolled());
  convertFollowupNode("toolchain.loop.unroll.followup_remainder",
                      options.getFollowupRemainder());
  convertFollowupNode("toolchain.loop.unroll.followup_all",
                      options.getFollowupAll());
}

void LoopAnnotationConversion::convertLoopOptions(
    LoopUnrollAndJamAttr options) {
  if (auto disable = options.getDisable())
    addUnitNode(disable.getValue() ? "toolchain.loop.unroll_and_jam.disable"
                                   : "toolchain.loop.unroll_and_jam.enable");
  convertI32Node("toolchain.loop.unroll_and_jam.count", options.getCount());
  convertFollowupNode("toolchain.loop.unroll_and_jam.followup_outer",
                      options.getFollowupOuter());
  convertFollowupNode("toolchain.loop.unroll_and_jam.followup_inner",
                      options.getFollowupInner());
  convertFollowupNode("toolchain.loop.unroll_and_jam.followup_remainder_outer",
                      options.getFollowupRemainderOuter());
  convertFollowupNode("toolchain.loop.unroll_and_jam.followup_remainder_inner",
                      options.getFollowupRemainderInner());
  convertFollowupNode("toolchain.loop.unroll_and_jam.followup_all",
                      options.getFollowupAll());
}

void LoopAnnotationConversion::convertLoopOptions(LoopLICMAttr options) {
  addUnitNode("toolchain.licm.disable", options.getDisable());
  addUnitNode("toolchain.loop.licm_versioning.disable",
              options.getVersioningDisable());
}

void LoopAnnotationConversion::convertLoopOptions(LoopDistributeAttr options) {
  convertBoolNode("toolchain.loop.distribute.enable", options.getDisable(), true);
  convertFollowupNode("toolchain.loop.distribute.followup_coincident",
                      options.getFollowupCoincident());
  convertFollowupNode("toolchain.loop.distribute.followup_sequential",
                      options.getFollowupSequential());
  convertFollowupNode("toolchain.loop.distribute.followup_fallback",
                      options.getFollowupFallback());
  convertFollowupNode("toolchain.loop.distribute.followup_all",
                      options.getFollowupAll());
}

void LoopAnnotationConversion::convertLoopOptions(LoopPipelineAttr options) {
  convertBoolNode("toolchain.loop.pipeline.disable", options.getDisable());
  convertI32Node("toolchain.loop.pipeline.initiationinterval",
                 options.getInitiationinterval());
}

void LoopAnnotationConversion::convertLoopOptions(LoopPeeledAttr options) {
  convertI32Node("toolchain.loop.peeled.count", options.getCount());
}

void LoopAnnotationConversion::convertLoopOptions(LoopUnswitchAttr options) {
  addUnitNode("toolchain.loop.unswitch.partial.disable",
              options.getPartialDisable());
}

void LoopAnnotationConversion::convertLocation(FusedLoc location) {
  auto localScopeAttr =
      dyn_cast_or_null<DILocalScopeAttr>(location.getMetadata());
  if (!localScopeAttr)
    return;
  auto *localScope = dyn_cast<toolchain::DILocalScope>(
      loopAnnotationTranslation.moduleTranslation.translateDebugInfo(
          localScopeAttr));
  if (!localScope)
    return;
  toolchain::Metadata *loc =
      loopAnnotationTranslation.moduleTranslation.translateLoc(location,
                                                               localScope);
  metadataNodes.push_back(loc);
}

toolchain::MDNode *LoopAnnotationConversion::convert() {
  // Reserve operand 0 for loop id self reference.
  auto dummy = toolchain::MDNode::getTemporary(ctx, {});
  metadataNodes.push_back(dummy.get());

  if (FusedLoc startLoc = attr.getStartLoc())
    convertLocation(startLoc);

  if (FusedLoc endLoc = attr.getEndLoc())
    convertLocation(endLoc);

  addUnitNode("toolchain.loop.disable_nonforced", attr.getDisableNonforced());
  addUnitNode("toolchain.loop.mustprogress", attr.getMustProgress());
  // "isvectorized" is encoded as an i32 value.
  if (BoolAttr isVectorized = attr.getIsVectorized())
    addI32NodeWithVal("toolchain.loop.isvectorized", isVectorized.getValue());

  if (auto options = attr.getVectorize())
    convertLoopOptions(options);
  if (auto options = attr.getInterleave())
    convertLoopOptions(options);
  if (auto options = attr.getUnroll())
    convertLoopOptions(options);
  if (auto options = attr.getUnrollAndJam())
    convertLoopOptions(options);
  if (auto options = attr.getLicm())
    convertLoopOptions(options);
  if (auto options = attr.getDistribute())
    convertLoopOptions(options);
  if (auto options = attr.getPipeline())
    convertLoopOptions(options);
  if (auto options = attr.getPeeled())
    convertLoopOptions(options);
  if (auto options = attr.getUnswitch())
    convertLoopOptions(options);

  ArrayRef<AccessGroupAttr> parallelAccessGroups = attr.getParallelAccesses();
  if (!parallelAccessGroups.empty()) {
    SmallVector<toolchain::Metadata *> parallelAccess;
    parallelAccess.push_back(
        toolchain::MDString::get(ctx, "toolchain.loop.parallel_accesses"));
    for (AccessGroupAttr accessGroupAttr : parallelAccessGroups)
      parallelAccess.push_back(
          loopAnnotationTranslation.getAccessGroup(accessGroupAttr));
    metadataNodes.push_back(toolchain::MDNode::get(ctx, parallelAccess));
  }

  // Create loop options and set the first operand to itself.
  toolchain::MDNode *loopMD = toolchain::MDNode::get(ctx, metadataNodes);
  loopMD->replaceOperandWith(0, loopMD);

  return loopMD;
}

toolchain::MDNode *
LoopAnnotationTranslation::translateLoopAnnotation(LoopAnnotationAttr attr,
                                                   Operation *op) {
  if (!attr)
    return nullptr;

  toolchain::MDNode *loopMD = lookupLoopMetadata(attr);
  if (loopMD)
    return loopMD;

  loopMD =
      LoopAnnotationConversion(attr, op, *this, this->llvmModule.getContext())
          .convert();
  // Store a map from this Attribute to the LLVM metadata in case we
  // encounter it again.
  mapLoopMetadata(attr, loopMD);
  return loopMD;
}

toolchain::MDNode *
LoopAnnotationTranslation::getAccessGroup(AccessGroupAttr accessGroupAttr) {
  auto [result, inserted] =
      accessGroupMetadataMapping.try_emplace(accessGroupAttr);
  if (inserted)
    result->second = toolchain::MDNode::getDistinct(llvmModule.getContext(), {});
  return result->second;
}

toolchain::MDNode *
LoopAnnotationTranslation::getAccessGroups(AccessGroupOpInterface op) {
  ArrayAttr accessGroups = op.getAccessGroupsOrNull();
  if (!accessGroups || accessGroups.empty())
    return nullptr;

  SmallVector<toolchain::Metadata *> groupMDs;
  for (AccessGroupAttr group : accessGroups.getAsRange<AccessGroupAttr>())
    groupMDs.push_back(getAccessGroup(group));
  if (groupMDs.size() == 1)
    return toolchain::cast<toolchain::MDNode>(groupMDs.front());
  return toolchain::MDNode::get(llvmModule.getContext(), groupMDs);
}
