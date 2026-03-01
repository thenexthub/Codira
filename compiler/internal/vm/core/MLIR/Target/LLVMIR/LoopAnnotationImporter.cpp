//===- LoopAnnotationImporter.cpp - Loop annotation import ----------------===//
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

#include "LoopAnnotationImporter.h"
#include "vm/core/IR/Constants.h"

using namespace mlir;
using namespace mlir::LLVM;
using namespace mlir::LLVM::detail;

namespace {
/// Helper class that keeps the state of one metadata to attribute conversion.
struct LoopMetadataConversion {
  LoopMetadataConversion(const toolchain::MDNode *node, Location loc,
                         LoopAnnotationImporter &loopAnnotationImporter)
      : node(node), loc(loc), loopAnnotationImporter(loopAnnotationImporter),
        ctx(loc->getContext()){};
  /// Converts this structs loop metadata node into a LoopAnnotationAttr.
  LoopAnnotationAttr convert();

  /// Initializes the shared state for the conversion member functions.
  LogicalResult initConversionState();

  /// Helper function to get and erase a property.
  const toolchain::MDNode *lookupAndEraseProperty(StringRef name);

  /// Helper functions to lookup and convert MDNodes into a specifc attribute
  /// kind. These functions return null-attributes if there is no node with the
  /// specified name, or failure, if the node is ill-formatted.
  FailureOr<BoolAttr> lookupUnitNode(StringRef name);
  FailureOr<BoolAttr> lookupBoolNode(StringRef name, bool negated = false);
  FailureOr<BoolAttr> lookupIntNodeAsBoolAttr(StringRef name);
  FailureOr<IntegerAttr> lookupIntNode(StringRef name);
  FailureOr<toolchain::MDNode *> lookupMDNode(StringRef name);
  FailureOr<SmallVector<toolchain::MDNode *>> lookupMDNodes(StringRef name);
  FailureOr<LoopAnnotationAttr> lookupFollowupNode(StringRef name);
  FailureOr<BoolAttr> lookupBooleanUnitNode(StringRef enableName,
                                            StringRef disableName,
                                            bool negated = false);

  /// Conversion functions for sub-attributes.
  FailureOr<LoopVectorizeAttr> convertVectorizeAttr();
  FailureOr<LoopInterleaveAttr> convertInterleaveAttr();
  FailureOr<LoopUnrollAttr> convertUnrollAttr();
  FailureOr<LoopUnrollAndJamAttr> convertUnrollAndJamAttr();
  FailureOr<LoopLICMAttr> convertLICMAttr();
  FailureOr<LoopDistributeAttr> convertDistributeAttr();
  FailureOr<LoopPipelineAttr> convertPipelineAttr();
  FailureOr<LoopPeeledAttr> convertPeeledAttr();
  FailureOr<LoopUnswitchAttr> convertUnswitchAttr();
  FailureOr<SmallVector<AccessGroupAttr>> convertParallelAccesses();
  FusedLoc convertStartLoc();
  FailureOr<FusedLoc> convertEndLoc();

  toolchain::SmallVector<toolchain::DILocation *, 2> locations;
  toolchain::StringMap<const toolchain::MDNode *> propertyMap;
  const toolchain::MDNode *node;
  Location loc;
  LoopAnnotationImporter &loopAnnotationImporter;
  MLIRContext *ctx;
};
} // namespace

LogicalResult LoopMetadataConversion::initConversionState() {
  // Check if it's a valid node.
  if (node->getNumOperands() == 0 ||
      dyn_cast<toolchain::MDNode>(node->getOperand(0)) != node)
    return emitWarning(loc) << "invalid loop node";

  for (const toolchain::MDOperand &operand : toolchain::drop_begin(node->operands())) {
    if (auto *diLoc = dyn_cast<toolchain::DILocation>(operand)) {
      locations.push_back(diLoc);
      continue;
    }

    auto *property = dyn_cast<toolchain::MDNode>(operand);
    if (!property)
      return emitWarning(loc) << "expected all loop properties to be either "
                                 "debug locations or metadata nodes";

    if (property->getNumOperands() == 0)
      return emitWarning(loc) << "cannot import empty loop property";

    auto *nameNode = dyn_cast<toolchain::MDString>(property->getOperand(0));
    if (!nameNode)
      return emitWarning(loc) << "cannot import loop property without a name";
    StringRef name = nameNode->getString();

    bool succ = propertyMap.try_emplace(name, property).second;
    if (!succ)
      return emitWarning(loc)
             << "cannot import loop properties with duplicated names " << name;
  }

  return success();
}

const toolchain::MDNode *
LoopMetadataConversion::lookupAndEraseProperty(StringRef name) {
  auto it = propertyMap.find(name);
  if (it == propertyMap.end())
    return nullptr;
  const toolchain::MDNode *property = it->getValue();
  propertyMap.erase(it);
  return property;
}

FailureOr<BoolAttr> LoopMetadataConversion::lookupUnitNode(StringRef name) {
  const toolchain::MDNode *property = lookupAndEraseProperty(name);
  if (!property)
    return BoolAttr(nullptr);

  if (property->getNumOperands() != 1)
    return emitWarning(loc)
           << "expected metadata node " << name << " to hold no value";

  return BoolAttr::get(ctx, true);
}

FailureOr<BoolAttr> LoopMetadataConversion::lookupBooleanUnitNode(
    StringRef enableName, StringRef disableName, bool negated) {
  auto enable = lookupUnitNode(enableName);
  auto disable = lookupUnitNode(disableName);
  if (failed(enable) || failed(disable))
    return failure();

  if (*enable && *disable)
    return emitWarning(loc)
           << "expected metadata nodes " << enableName << " and " << disableName
           << " to be mutually exclusive.";

  if (*enable)
    return BoolAttr::get(ctx, !negated);

  if (*disable)
    return BoolAttr::get(ctx, negated);
  return BoolAttr(nullptr);
}

FailureOr<BoolAttr> LoopMetadataConversion::lookupBoolNode(StringRef name,
                                                           bool negated) {
  const toolchain::MDNode *property = lookupAndEraseProperty(name);
  if (!property)
    return BoolAttr(nullptr);

  auto emitNodeWarning = [&]() {
    return emitWarning(loc)
           << "expected metadata node " << name << " to hold a boolean value";
  };

  if (property->getNumOperands() != 2)
    return emitNodeWarning();
  toolchain::ConstantInt *val =
      toolchain::mdconst::dyn_extract<toolchain::ConstantInt>(property->getOperand(1));
  if (!val || val->getBitWidth() != 1)
    return emitNodeWarning();

  return BoolAttr::get(ctx, val->getValue().getLimitedValue(1) ^ negated);
}

FailureOr<BoolAttr>
LoopMetadataConversion::lookupIntNodeAsBoolAttr(StringRef name) {
  const toolchain::MDNode *property = lookupAndEraseProperty(name);
  if (!property)
    return BoolAttr(nullptr);

  auto emitNodeWarning = [&]() {
    return emitWarning(loc)
           << "expected metadata node " << name << " to hold an integer value";
  };

  if (property->getNumOperands() != 2)
    return emitNodeWarning();
  toolchain::ConstantInt *val =
      toolchain::mdconst::dyn_extract<toolchain::ConstantInt>(property->getOperand(1));
  if (!val || val->getBitWidth() != 32)
    return emitNodeWarning();

  return BoolAttr::get(ctx, val->getValue().getLimitedValue(1));
}

FailureOr<IntegerAttr> LoopMetadataConversion::lookupIntNode(StringRef name) {
  const toolchain::MDNode *property = lookupAndEraseProperty(name);
  if (!property)
    return IntegerAttr(nullptr);

  auto emitNodeWarning = [&]() {
    return emitWarning(loc)
           << "expected metadata node " << name << " to hold an i32 value";
  };

  if (property->getNumOperands() != 2)
    return emitNodeWarning();

  toolchain::ConstantInt *val =
      toolchain::mdconst::dyn_extract<toolchain::ConstantInt>(property->getOperand(1));
  if (!val || val->getBitWidth() != 32)
    return emitNodeWarning();

  return IntegerAttr::get(IntegerType::get(ctx, 32),
                          val->getValue().getLimitedValue());
}

FailureOr<toolchain::MDNode *> LoopMetadataConversion::lookupMDNode(StringRef name) {
  const toolchain::MDNode *property = lookupAndEraseProperty(name);
  if (!property)
    return nullptr;

  auto emitNodeWarning = [&]() {
    return emitWarning(loc)
           << "expected metadata node " << name << " to hold an MDNode";
  };

  if (property->getNumOperands() != 2)
    return emitNodeWarning();

  auto *node = dyn_cast<toolchain::MDNode>(property->getOperand(1));
  if (!node)
    return emitNodeWarning();

  return node;
}

FailureOr<SmallVector<toolchain::MDNode *>>
LoopMetadataConversion::lookupMDNodes(StringRef name) {
  const toolchain::MDNode *property = lookupAndEraseProperty(name);
  SmallVector<toolchain::MDNode *> res;
  if (!property)
    return res;

  auto emitNodeWarning = [&]() {
    return emitWarning(loc) << "expected metadata node " << name
                            << " to hold one or multiple MDNodes";
  };

  if (property->getNumOperands() < 2)
    return emitNodeWarning();

  for (unsigned i = 1, e = property->getNumOperands(); i < e; ++i) {
    auto *node = dyn_cast<toolchain::MDNode>(property->getOperand(i));
    if (!node)
      return emitNodeWarning();
    res.push_back(node);
  }

  return res;
}

FailureOr<LoopAnnotationAttr>
LoopMetadataConversion::lookupFollowupNode(StringRef name) {
  auto node = lookupMDNode(name);
  if (failed(node))
    return failure();
  if (*node == nullptr)
    return LoopAnnotationAttr(nullptr);

  return loopAnnotationImporter.translateLoopAnnotation(*node, loc);
}

static bool isEmptyOrNull(const Attribute attr) { return !attr; }

template <typename T>
static bool isEmptyOrNull(const SmallVectorImpl<T> &vec) {
  return vec.empty();
}

/// Helper function that only creates and attribute of type T if all argument
/// conversion were successfull and at least one of them holds a non-null value.
template <typename T, typename... P>
static T createIfNonNull(MLIRContext *ctx, const P &...args) {
  bool anyFailed = (failed(args) || ...);
  if (anyFailed)
    return {};

  bool allEmpty = (isEmptyOrNull(*args) && ...);
  if (allEmpty)
    return {};

  return T::get(ctx, *args...);
}

FailureOr<LoopVectorizeAttr> LoopMetadataConversion::convertVectorizeAttr() {
  FailureOr<BoolAttr> enable =
      lookupBoolNode("toolchain.loop.vectorize.enable", true);
  FailureOr<BoolAttr> predicateEnable =
      lookupBoolNode("toolchain.loop.vectorize.predicate.enable");
  FailureOr<BoolAttr> scalableEnable =
      lookupBoolNode("toolchain.loop.vectorize.scalable.enable");
  FailureOr<IntegerAttr> width = lookupIntNode("toolchain.loop.vectorize.width");
  FailureOr<LoopAnnotationAttr> followupVec =
      lookupFollowupNode("toolchain.loop.vectorize.followup_vectorized");
  FailureOr<LoopAnnotationAttr> followupEpi =
      lookupFollowupNode("toolchain.loop.vectorize.followup_epilogue");
  FailureOr<LoopAnnotationAttr> followupAll =
      lookupFollowupNode("toolchain.loop.vectorize.followup_all");

  return createIfNonNull<LoopVectorizeAttr>(ctx, enable, predicateEnable,
                                            scalableEnable, width, followupVec,
                                            followupEpi, followupAll);
}

FailureOr<LoopInterleaveAttr> LoopMetadataConversion::convertInterleaveAttr() {
  FailureOr<IntegerAttr> count = lookupIntNode("toolchain.loop.interleave.count");
  return createIfNonNull<LoopInterleaveAttr>(ctx, count);
}

FailureOr<LoopUnrollAttr> LoopMetadataConversion::convertUnrollAttr() {
  FailureOr<BoolAttr> disable = lookupBooleanUnitNode(
      "toolchain.loop.unroll.enable", "toolchain.loop.unroll.disable", /*negated=*/true);
  FailureOr<IntegerAttr> count = lookupIntNode("toolchain.loop.unroll.count");
  FailureOr<BoolAttr> runtimeDisable =
      lookupUnitNode("toolchain.loop.unroll.runtime.disable");
  FailureOr<BoolAttr> full = lookupUnitNode("toolchain.loop.unroll.full");
  FailureOr<LoopAnnotationAttr> followupUnrolled =
      lookupFollowupNode("toolchain.loop.unroll.followup_unrolled");
  FailureOr<LoopAnnotationAttr> followupRemainder =
      lookupFollowupNode("toolchain.loop.unroll.followup_remainder");
  FailureOr<LoopAnnotationAttr> followupAll =
      lookupFollowupNode("toolchain.loop.unroll.followup_all");

  return createIfNonNull<LoopUnrollAttr>(ctx, disable, count, runtimeDisable,
                                         full, followupUnrolled,
                                         followupRemainder, followupAll);
}

FailureOr<LoopUnrollAndJamAttr>
LoopMetadataConversion::convertUnrollAndJamAttr() {
  FailureOr<BoolAttr> disable = lookupBooleanUnitNode(
      "toolchain.loop.unroll_and_jam.enable", "toolchain.loop.unroll_and_jam.disable",
      /*negated=*/true);
  FailureOr<IntegerAttr> count =
      lookupIntNode("toolchain.loop.unroll_and_jam.count");
  FailureOr<LoopAnnotationAttr> followupOuter =
      lookupFollowupNode("toolchain.loop.unroll_and_jam.followup_outer");
  FailureOr<LoopAnnotationAttr> followupInner =
      lookupFollowupNode("toolchain.loop.unroll_and_jam.followup_inner");
  FailureOr<LoopAnnotationAttr> followupRemainderOuter =
      lookupFollowupNode("toolchain.loop.unroll_and_jam.followup_remainder_outer");
  FailureOr<LoopAnnotationAttr> followupRemainderInner =
      lookupFollowupNode("toolchain.loop.unroll_and_jam.followup_remainder_inner");
  FailureOr<LoopAnnotationAttr> followupAll =
      lookupFollowupNode("toolchain.loop.unroll_and_jam.followup_all");
  return createIfNonNull<LoopUnrollAndJamAttr>(
      ctx, disable, count, followupOuter, followupInner, followupRemainderOuter,
      followupRemainderInner, followupAll);
}

FailureOr<LoopLICMAttr> LoopMetadataConversion::convertLICMAttr() {
  FailureOr<BoolAttr> disable = lookupUnitNode("toolchain.licm.disable");
  FailureOr<BoolAttr> versioningDisable =
      lookupUnitNode("toolchain.loop.licm_versioning.disable");
  return createIfNonNull<LoopLICMAttr>(ctx, disable, versioningDisable);
}

FailureOr<LoopDistributeAttr> LoopMetadataConversion::convertDistributeAttr() {
  FailureOr<BoolAttr> disable =
      lookupBoolNode("toolchain.loop.distribute.enable", true);
  FailureOr<LoopAnnotationAttr> followupCoincident =
      lookupFollowupNode("toolchain.loop.distribute.followup_coincident");
  FailureOr<LoopAnnotationAttr> followupSequential =
      lookupFollowupNode("toolchain.loop.distribute.followup_sequential");
  FailureOr<LoopAnnotationAttr> followupFallback =
      lookupFollowupNode("toolchain.loop.distribute.followup_fallback");
  FailureOr<LoopAnnotationAttr> followupAll =
      lookupFollowupNode("toolchain.loop.distribute.followup_all");
  return createIfNonNull<LoopDistributeAttr>(ctx, disable, followupCoincident,
                                             followupSequential,
                                             followupFallback, followupAll);
}

FailureOr<LoopPipelineAttr> LoopMetadataConversion::convertPipelineAttr() {
  FailureOr<BoolAttr> disable = lookupBoolNode("toolchain.loop.pipeline.disable");
  FailureOr<IntegerAttr> initiationinterval =
      lookupIntNode("toolchain.loop.pipeline.initiationinterval");
  return createIfNonNull<LoopPipelineAttr>(ctx, disable, initiationinterval);
}

FailureOr<LoopPeeledAttr> LoopMetadataConversion::convertPeeledAttr() {
  FailureOr<IntegerAttr> count = lookupIntNode("toolchain.loop.peeled.count");
  return createIfNonNull<LoopPeeledAttr>(ctx, count);
}

FailureOr<LoopUnswitchAttr> LoopMetadataConversion::convertUnswitchAttr() {
  FailureOr<BoolAttr> partialDisable =
      lookupUnitNode("toolchain.loop.unswitch.partial.disable");
  return createIfNonNull<LoopUnswitchAttr>(ctx, partialDisable);
}

FailureOr<SmallVector<AccessGroupAttr>>
LoopMetadataConversion::convertParallelAccesses() {
  FailureOr<SmallVector<toolchain::MDNode *>> nodes =
      lookupMDNodes("toolchain.loop.parallel_accesses");
  if (failed(nodes))
    return failure();
  SmallVector<AccessGroupAttr> refs;
  for (toolchain::MDNode *node : *nodes) {
    FailureOr<SmallVector<AccessGroupAttr>> accessGroups =
        loopAnnotationImporter.lookupAccessGroupAttrs(node);
    if (failed(accessGroups)) {
      emitWarning(loc) << "could not lookup access group";
      continue;
    }
    toolchain::append_range(refs, *accessGroups);
  }
  return refs;
}

FusedLoc LoopMetadataConversion::convertStartLoc() {
  if (locations.empty())
    return {};
  return dyn_cast<FusedLoc>(
      loopAnnotationImporter.moduleImport.translateLoc(locations[0]));
}

FailureOr<FusedLoc> LoopMetadataConversion::convertEndLoc() {
  if (locations.size() < 2)
    return FusedLoc();
  if (locations.size() > 2)
    return emitError(loc)
           << "expected loop metadata to have at most two DILocations";
  return dyn_cast<FusedLoc>(
      loopAnnotationImporter.moduleImport.translateLoc(locations[1]));
}

LoopAnnotationAttr LoopMetadataConversion::convert() {
  if (failed(initConversionState()))
    return {};

  FailureOr<BoolAttr> disableNonForced =
      lookupUnitNode("toolchain.loop.disable_nonforced");
  FailureOr<LoopVectorizeAttr> vecAttr = convertVectorizeAttr();
  FailureOr<LoopInterleaveAttr> interleaveAttr = convertInterleaveAttr();
  FailureOr<LoopUnrollAttr> unrollAttr = convertUnrollAttr();
  FailureOr<LoopUnrollAndJamAttr> unrollAndJamAttr = convertUnrollAndJamAttr();
  FailureOr<LoopLICMAttr> licmAttr = convertLICMAttr();
  FailureOr<LoopDistributeAttr> distributeAttr = convertDistributeAttr();
  FailureOr<LoopPipelineAttr> pipelineAttr = convertPipelineAttr();
  FailureOr<LoopPeeledAttr> peeledAttr = convertPeeledAttr();
  FailureOr<LoopUnswitchAttr> unswitchAttr = convertUnswitchAttr();
  FailureOr<BoolAttr> mustProgress = lookupUnitNode("toolchain.loop.mustprogress");
  FailureOr<BoolAttr> isVectorized =
      lookupIntNodeAsBoolAttr("toolchain.loop.isvectorized");
  FailureOr<SmallVector<AccessGroupAttr>> parallelAccesses =
      convertParallelAccesses();

  // Drop the metadata if there are parts that cannot be imported.
  if (!propertyMap.empty()) {
    for (auto name : propertyMap.keys())
      emitWarning(loc) << "unknown loop annotation " << name;
    return {};
  }

  FailureOr<FusedLoc> startLoc = convertStartLoc();
  FailureOr<FusedLoc> endLoc = convertEndLoc();

  return createIfNonNull<LoopAnnotationAttr>(
      ctx, disableNonForced, vecAttr, interleaveAttr, unrollAttr,
      unrollAndJamAttr, licmAttr, distributeAttr, pipelineAttr, peeledAttr,
      unswitchAttr, mustProgress, isVectorized, startLoc, endLoc,
      parallelAccesses);
}

LoopAnnotationAttr
LoopAnnotationImporter::translateLoopAnnotation(const toolchain::MDNode *node,
                                                Location loc) {
  if (!node)
    return {};

  // Note: This check is necessary to distinguish between failed translations
  // and not yet attempted translations.
  auto it = loopMetadataMapping.find(node);
  if (it != loopMetadataMapping.end())
    return it->getSecond();

  LoopAnnotationAttr attr = LoopMetadataConversion(node, loc, *this).convert();

  mapLoopMetadata(node, attr);
  return attr;
}

LogicalResult
LoopAnnotationImporter::translateAccessGroup(const toolchain::MDNode *node,
                                             Location loc) {
  SmallVector<const toolchain::MDNode *> accessGroups;
  if (!node->getNumOperands())
    accessGroups.push_back(node);
  for (const toolchain::MDOperand &operand : node->operands()) {
    auto *childNode = dyn_cast<toolchain::MDNode>(operand);
    if (!childNode)
      return failure();
    accessGroups.push_back(cast<toolchain::MDNode>(operand.get()));
  }

  // Convert all entries of the access group list to access group operations.
  for (const toolchain::MDNode *accessGroup : accessGroups) {
    if (accessGroupMapping.count(accessGroup))
      continue;
    // Verify the access group node is distinct and empty.
    if (accessGroup->getNumOperands() != 0 || !accessGroup->isDistinct())
      return emitWarning(loc)
             << "expected an access group node to be empty and distinct";

    // Add a mapping from the access group node to the newly created attribute.
    accessGroupMapping[accessGroup] = builder.getAttr<AccessGroupAttr>();
  }
  return success();
}

FailureOr<SmallVector<AccessGroupAttr>>
LoopAnnotationImporter::lookupAccessGroupAttrs(const toolchain::MDNode *node) const {
  // An access group node is either a single access group or an access group
  // list.
  SmallVector<AccessGroupAttr> accessGroups;
  if (!node->getNumOperands())
    accessGroups.push_back(accessGroupMapping.lookup(node));
  for (const toolchain::MDOperand &operand : node->operands()) {
    auto *node = cast<toolchain::MDNode>(operand.get());
    accessGroups.push_back(accessGroupMapping.lookup(node));
  }
  // Exit if one of the access group node lookups failed.
  if (toolchain::is_contained(accessGroups, nullptr))
    return failure();
  return accessGroups;
}
