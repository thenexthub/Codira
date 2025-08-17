//===---- CGLoopInfo.h - LLVM CodeGen for loop metadata -*- C++ -*---------===//
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
//
// This is the internal state used for toolchain translation for loop statement
// metadata.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_CODEGEN_CGLOOPINFO_H
#define LANGUAGE_CORE_LIB_CODEGEN_CGLOOPINFO_H

#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/IR/DebugLoc.h"
#include "toolchain/IR/Value.h"
#include "toolchain/Support/Compiler.h"

namespace toolchain {
class BasicBlock;
class Instruction;
class MDNode;
} // end namespace toolchain

namespace language::Core {
class Attr;
class ASTContext;
class CodeGenOptions;
namespace CodeGen {

/// Attributes that may be specified on loops.
struct LoopAttributes {
  explicit LoopAttributes(bool IsParallel = false);
  void clear();

  /// Generate toolchain.loop.parallel metadata for loads and stores.
  bool IsParallel;

  /// State of loop vectorization or unrolling.
  enum LVEnableState { Unspecified, Enable, Disable, Full };

  /// Value for toolchain.loop.vectorize.enable metadata.
  LVEnableState VectorizeEnable;

  /// Value for toolchain.loop.unroll.* metadata (enable, disable, or full).
  LVEnableState UnrollEnable;

  /// Value for toolchain.loop.unroll_and_jam.* metadata (enable, disable, or full).
  LVEnableState UnrollAndJamEnable;

  /// Value for toolchain.loop.vectorize.predicate metadata
  LVEnableState VectorizePredicateEnable;

  /// Value for toolchain.loop.vectorize.width metadata.
  unsigned VectorizeWidth;

  // Value for toolchain.loop.vectorize.scalable.enable
  LVEnableState VectorizeScalable;

  /// Value for toolchain.loop.interleave.count metadata.
  unsigned InterleaveCount;

  /// toolchain.unroll.
  unsigned UnrollCount;

  /// toolchain.unroll.
  unsigned UnrollAndJamCount;

  /// Value for toolchain.loop.distribute.enable metadata.
  LVEnableState DistributeEnable;

  /// Value for toolchain.loop.pipeline.disable metadata.
  bool PipelineDisabled;

  /// Value for toolchain.loop.pipeline.iicount metadata.
  unsigned PipelineInitiationInterval;

  /// Value for 'toolchain.loop.align' metadata.
  unsigned CodeAlign;

  /// Value for whether the loop is required to make progress.
  bool MustProgress;
};

/// Information used when generating a structured loop.
class LoopInfo {
public:
  /// Construct a new LoopInfo for the loop with entry Header.
  LoopInfo(toolchain::BasicBlock *Header, const LoopAttributes &Attrs,
           const toolchain::DebugLoc &StartLoc, const toolchain::DebugLoc &EndLoc,
           LoopInfo *Parent);

  /// Get the loop id metadata for this loop.
  toolchain::MDNode *getLoopID() const { return TempLoopID.get(); }

  /// Get the header block of this loop.
  toolchain::BasicBlock *getHeader() const { return Header; }

  /// Get the set of attributes active for this loop.
  const LoopAttributes &getAttributes() const { return Attrs; }

  /// Return this loop's access group or nullptr if it does not have one.
  toolchain::MDNode *getAccessGroup() const { return AccGroup; }

  /// Create the loop's metadata. Must be called after its nested loops have
  /// been processed.
  void finish();

  /// Returns the first outer loop containing this loop if any, nullptr
  /// otherwise.
  const LoopInfo *getParent() const { return Parent; }

private:
  /// Loop ID metadata.
  toolchain::TempMDTuple TempLoopID;
  /// Header block of this loop.
  toolchain::BasicBlock *Header;
  /// The attributes for this loop.
  LoopAttributes Attrs;
  /// The access group for memory accesses parallel to this loop.
  toolchain::MDNode *AccGroup = nullptr;
  /// Start location of this loop.
  toolchain::DebugLoc StartLoc;
  /// End location of this loop.
  toolchain::DebugLoc EndLoc;
  /// The next outer loop, or nullptr if this is the outermost loop.
  LoopInfo *Parent;
  /// If this loop has unroll-and-jam metadata, this can be set by the inner
  /// loop's LoopInfo to set the toolchain.loop.unroll_and_jam.followup_inner
  /// metadata.
  std::optional<toolchain::SmallVector<toolchain::Metadata *, 4>>
      UnrollAndJamInnerFollowup;

  /// Create a followup MDNode that has @p LoopProperties as its attributes.
  toolchain::MDNode *
  createFollowupMetadata(const char *FollowupName,
                         toolchain::ArrayRef<toolchain::Metadata *> LoopProperties);

  /// Create a metadata list for transformations.
  ///
  /// The methods call each other in case multiple transformations are applied
  /// to a loop. The transformation first to be applied will use metadata list
  /// of the next transformation in its followup attribute.
  ///
  /// @param Attrs             The loop's transformations.
  /// @param LoopProperties    Non-transformation properties such as debug
  ///                          location, parallel accesses and disabled
  ///                          transformations. These are added to the returned
  ///                          LoopID.
  /// @param HasUserTransforms [out] Set to true if the returned MDNode encodes
  ///                          at least one transformation.
  ///
  /// @return A metadata list that can be used for the toolchain.loop annotation or
  ///         followup-attribute.
  /// @{
  toolchain::SmallVector<toolchain::Metadata *, 4>
  createPipeliningMetadata(const LoopAttributes &Attrs,
                           toolchain::ArrayRef<toolchain::Metadata *> LoopProperties,
                           bool &HasUserTransforms);
  toolchain::SmallVector<toolchain::Metadata *, 4>
  createPartialUnrollMetadata(const LoopAttributes &Attrs,
                              toolchain::ArrayRef<toolchain::Metadata *> LoopProperties,
                              bool &HasUserTransforms);
  toolchain::SmallVector<toolchain::Metadata *, 4>
  createUnrollAndJamMetadata(const LoopAttributes &Attrs,
                             toolchain::ArrayRef<toolchain::Metadata *> LoopProperties,
                             bool &HasUserTransforms);
  toolchain::SmallVector<toolchain::Metadata *, 4>
  createLoopVectorizeMetadata(const LoopAttributes &Attrs,
                              toolchain::ArrayRef<toolchain::Metadata *> LoopProperties,
                              bool &HasUserTransforms);
  toolchain::SmallVector<toolchain::Metadata *, 4>
  createLoopDistributeMetadata(const LoopAttributes &Attrs,
                               toolchain::ArrayRef<toolchain::Metadata *> LoopProperties,
                               bool &HasUserTransforms);
  toolchain::SmallVector<toolchain::Metadata *, 4>
  createFullUnrollMetadata(const LoopAttributes &Attrs,
                           toolchain::ArrayRef<toolchain::Metadata *> LoopProperties,
                           bool &HasUserTransforms);

  /// @}

  /// Create a metadata list for this loop, including transformation-unspecific
  /// metadata such as debug location.
  ///
  /// @param Attrs             This loop's attributes and transformations.
  /// @param LoopProperties    Additional non-transformation properties to add
  ///                          to the LoopID, such as transformation-specific
  ///                          metadata that are not covered by @p Attrs.
  /// @param HasUserTransforms [out] Set to true if the returned MDNode encodes
  ///                          at least one transformation.
  ///
  /// @return A metadata list that can be used for the toolchain.loop annotation.
  toolchain::SmallVector<toolchain::Metadata *, 4>
  createMetadata(const LoopAttributes &Attrs,
                 toolchain::ArrayRef<toolchain::Metadata *> LoopProperties,
                 bool &HasUserTransforms);
};

/// A stack of loop information corresponding to loop nesting levels.
/// This stack can be used to prepare attributes which are applied when a loop
/// is emitted.
class LoopInfoStack {
  LoopInfoStack(const LoopInfoStack &) = delete;
  void operator=(const LoopInfoStack &) = delete;

public:
  LoopInfoStack() {}

  /// Begin a new structured loop. The set of staged attributes will be
  /// applied to the loop and then cleared.
  void push(toolchain::BasicBlock *Header, const toolchain::DebugLoc &StartLoc,
            const toolchain::DebugLoc &EndLoc);

  /// Begin a new structured loop. Stage attributes from the Attrs list.
  /// The staged attributes are applied to the loop and then cleared.
  void push(toolchain::BasicBlock *Header, language::Core::ASTContext &Ctx,
            const language::Core::CodeGenOptions &CGOpts,
            toolchain::ArrayRef<const Attr *> Attrs, const toolchain::DebugLoc &StartLoc,
            const toolchain::DebugLoc &EndLoc, bool MustProgress = false);

  /// End the current loop.
  void pop();

  /// Return the top loop id metadata.
  toolchain::MDNode *getCurLoopID() const { return getInfo().getLoopID(); }

  /// Return true if the top loop is parallel.
  bool getCurLoopParallel() const {
    return hasInfo() ? getInfo().getAttributes().IsParallel : false;
  }

  /// Function called by the CodeGenFunction when an instruction is
  /// created.
  void InsertHelper(toolchain::Instruction *I) const;

  /// Set the next pushed loop as parallel.
  void setParallel(bool Enable = true) { StagedAttrs.IsParallel = Enable; }

  /// Set the next pushed loop 'vectorize.enable'
  void setVectorizeEnable(bool Enable = true) {
    StagedAttrs.VectorizeEnable =
        Enable ? LoopAttributes::Enable : LoopAttributes::Disable;
  }

  /// Set the next pushed loop as a distribution candidate.
  void setDistributeState(bool Enable = true) {
    StagedAttrs.DistributeEnable =
        Enable ? LoopAttributes::Enable : LoopAttributes::Disable;
  }

  /// Set the next pushed loop unroll state.
  void setUnrollState(const LoopAttributes::LVEnableState &State) {
    StagedAttrs.UnrollEnable = State;
  }

  /// Set the next pushed vectorize predicate state.
  void setVectorizePredicateState(const LoopAttributes::LVEnableState &State) {
    StagedAttrs.VectorizePredicateEnable = State;
  }

  /// Set the next pushed loop unroll_and_jam state.
  void setUnrollAndJamState(const LoopAttributes::LVEnableState &State) {
    StagedAttrs.UnrollAndJamEnable = State;
  }

  /// Set the vectorize width for the next loop pushed.
  void setVectorizeWidth(unsigned W) { StagedAttrs.VectorizeWidth = W; }

  void setVectorizeScalable(const LoopAttributes::LVEnableState &State) {
    StagedAttrs.VectorizeScalable = State;
  }

  /// Set the interleave count for the next loop pushed.
  void setInterleaveCount(unsigned C) { StagedAttrs.InterleaveCount = C; }

  /// Set the unroll count for the next loop pushed.
  void setUnrollCount(unsigned C) { StagedAttrs.UnrollCount = C; }

  /// \brief Set the unroll count for the next loop pushed.
  void setUnrollAndJamCount(unsigned C) { StagedAttrs.UnrollAndJamCount = C; }

  /// Set the pipeline disabled state.
  void setPipelineDisabled(bool S) { StagedAttrs.PipelineDisabled = S; }

  /// Set the pipeline initiation interval.
  void setPipelineInitiationInterval(unsigned C) {
    StagedAttrs.PipelineInitiationInterval = C;
  }

  /// Set value of code align for the next loop pushed.
  void setCodeAlign(unsigned C) { StagedAttrs.CodeAlign = C; }

  /// Set no progress for the next loop pushed.
  void setMustProgress(bool P) { StagedAttrs.MustProgress = P; }

  /// Returns true if there is LoopInfo on the stack.
  bool hasInfo() const { return !Active.empty(); }
  /// Return the LoopInfo for the current loop. HasInfo should be called
  /// first to ensure LoopInfo is present.
  const LoopInfo &getInfo() const { return *Active.back(); }

private:
  /// The set of attributes that will be applied to the next pushed loop.
  LoopAttributes StagedAttrs;
  /// Stack of active loops.
  toolchain::SmallVector<std::unique_ptr<LoopInfo>, 4> Active;
};

} // end namespace CodeGen
} // end namespace language::Core

#endif
