//===--- PassManager.h  - Codira Pass Manager --------------------*- C++ -*-===//
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

#include "language/SIL/Notifications.h"
#include "language/SIL/InstructionUtils.h"
#include "language/SIL/BasicBlockBits.h"
#include "language/SIL/NodeBits.h"
#include "language/SIL/OperandBits.h"
#include "language/SILOptimizer/Analysis/Analysis.h"
#include "language/SILOptimizer/PassManager/PassPipeline.h"
#include "language/SILOptimizer/PassManager/Passes.h"
#include "language/SILOptimizer/Utils/SILSSAUpdater.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/Support/Casting.h"
#include "toolchain/Support/Chrono.h"
#include "toolchain/Support/ErrorHandling.h"
#include <vector>
#include <bitset>

#ifndef LANGUAGE_SILOPTIMIZER_PASSMANAGER_PASSMANAGER_H
#define LANGUAGE_SILOPTIMIZER_PASSMANAGER_PASSMANAGER_H

namespace language {

class SILFunction;
class SILFunctionTransform;
class SILModule;
class SILModuleTransform;
class SILOptions;
class SILTransform;
class SILPassManager;
class SILCombiner;

namespace irgen {
class IRGenModule;
class IRGenerator;
}

/// The main entrypoint for executing a pipeline pass on a SIL module.
void executePassPipelinePlan(SILModule *SM, const SILPassPipelinePlan &plan,
                             bool isMandatory = false,
                             irgen::IRGenModule *IRMod = nullptr);

/// Utility class to invoke Codira passes.
class CodiraPassInvocation {
  /// Backlink to the pass manager.
  SILPassManager *passManager;

  /// The current transform.
  SILTransform *transform = nullptr;

  /// The currently optimized function.
  SILFunction *function = nullptr;

  /// Non-null if this is an instruction pass, invoked from SILCombine.
  SILCombiner *silCombiner = nullptr;

  /// Change notifications, collected during a pass run.
  SILAnalysis::InvalidationKind changeNotifications =
      SILAnalysis::InvalidationKind::Nothing;

  bool functionTablesChanged = false;

  /// All slabs, allocated by the pass.
  SILModule::SlabList allocatedSlabs;

  SILSSAUpdater *ssaUpdater = nullptr;
  SmallVector<SILPhiArgument *, 4> insertedPhisBySSAUpdater;

  CodiraPassInvocation *nestedCodiraPassInvocation = nullptr;

  static constexpr int BlockSetCapacity = SILBasicBlock::numCustomBits;
  char blockSetStorage[sizeof(BasicBlockSet) * BlockSetCapacity];
  bool aliveBlockSets[BlockSetCapacity];
  int numBlockSetsAllocated = 0;

  static constexpr int NodeSetCapacity = SILNode::numCustomBits;
  char nodeSetStorage[sizeof(NodeSet) * NodeSetCapacity];
  bool aliveNodeSets[NodeSetCapacity];
  int numNodeSetsAllocated = 0;

  static constexpr int OperandSetCapacity = Operand::numCustomBits;
  char operandSetStorage[sizeof(OperandSet) * OperandSetCapacity];
  bool aliveOperandSets[OperandSetCapacity];
  int numOperandSetsAllocated = 0;

  int numClonersAllocated = 0;

  bool needFixStackNesting = false;

  void endPass();

public:
  CodiraPassInvocation(SILPassManager *passManager, SILFunction *function,
                         SILCombiner *silCombiner) :
    passManager(passManager), function(function), silCombiner(silCombiner) {}

  CodiraPassInvocation(SILPassManager *passManager, SILTransform *transform,
                      SILFunction *function) :
    passManager(passManager), transform(transform), function(function) {}

  CodiraPassInvocation(SILPassManager *passManager) :
    passManager(passManager) {}

  ~CodiraPassInvocation();

  SILPassManager *getPassManager() const { return passManager; }
  
  SILTransform *getTransform() const { return transform; }

  SILFunction *getFunction() const { return function; }

  irgen::IRGenModule *getIRGenModule();

  FixedSizeSlab *allocSlab(FixedSizeSlab *afterSlab);

  FixedSizeSlab *freeSlab(FixedSizeSlab *slab);

  BasicBlockSet *allocBlockSet();

  void freeBlockSet(BasicBlockSet *set);

  NodeSet *allocNodeSet();

  void freeNodeSet(NodeSet *set);

  OperandSet *allocOperandSet();

  void freeOperandSet(OperandSet *set);

  /// The top-level API to erase an instruction, called from the Codira pass.
  void eraseInstruction(SILInstruction *inst, bool salvageDebugInfo);

  /// Called by the pass when changes are made to the SIL.
  void notifyChanges(SILAnalysis::InvalidationKind invalidationKind);

  void notifyFunctionTablesChanged();

  /// Called by the pass manager before the pass starts running.
  void startModulePassRun(SILModuleTransform *transform);

  /// Called by the pass manager before the pass starts running.
  void startFunctionPassRun(SILFunctionTransform *transform);

  /// Called by the SILCombiner before the instruction pass starts running.
  void startInstructionPassRun(SILInstruction *inst);

  /// Called by the pass manager when the pass has finished.
  void finishedModulePassRun();

  /// Called by the pass manager when the pass has finished.
  void finishedFunctionPassRun();

  /// Called by the SILCombiner when the instruction pass has finished.
  void finishedInstructionPassRun();
  
  void beginTransformFunction(SILFunction *function);

  void endTransformFunction();

  void beginVerifyFunction(SILFunction *function);
  void endVerifyFunction();

  void notifyNewCloner() { numClonersAllocated++; }
  void notifyClonerDestroyed() { numClonersAllocated--; }

  void setNeedFixStackNesting(bool newValue) { needFixStackNesting = newValue; }
  bool getNeedFixStackNesting() const { return needFixStackNesting; }

  void initializeSSAUpdater(SILFunction *fn, SILType type,
                            ValueOwnershipKind ownership) {
    insertedPhisBySSAUpdater.clear();
    if (!ssaUpdater)
      ssaUpdater = new SILSSAUpdater(&insertedPhisBySSAUpdater);
    ssaUpdater->initialize(fn, type, ownership);
  }

  SILSSAUpdater *getSSAUpdater() const {
    assert(ssaUpdater && "SSAUpdater not initialized");
    return ssaUpdater;
  }

  ArrayRef<SILPhiArgument *> getInsertedPhisBySSAUpdater() { return insertedPhisBySSAUpdater; }

  CodiraPassInvocation *initializeNestedCodiraPassInvocation(SILFunction *newFunction) {
    assert(!nestedCodiraPassInvocation && "Nested Codira pass invocation already initialized");
    nestedCodiraPassInvocation = new CodiraPassInvocation(passManager, transform, newFunction);
    return nestedCodiraPassInvocation;
  }

  void deinitializeNestedCodiraPassInvocation() {
    assert(nestedCodiraPassInvocation && "Nested Codira pass invocation not initialized");
    nestedCodiraPassInvocation->endTransformFunction();
    delete nestedCodiraPassInvocation;
    nestedCodiraPassInvocation = nullptr;
  }
};

/// The SIL pass manager.
class SILPassManager {
  friend class ExecuteSILPipelineRequest;

  /// The module that the pass manager will transform.
  SILModule *Mod;

  /// An optional IRGenModule associated with this PassManager.
  irgen::IRGenModule *IRMod;
  irgen::IRGenerator *irgen;

  /// The list of transformations to run.
  toolchain::SmallVector<SILTransform *, 16> Transformations;

  /// A list of registered analysis.
  toolchain::SmallVector<SILAnalysis *, 16> Analyses;

  /// An entry in the FunctionWorkList.
  struct WorklistEntry {
    WorklistEntry(SILFunction *F) : F(F) { }

    SILFunction *F;

    /// The current position in the transform-list.
    unsigned PipelineIdx = 0;

    /// How many times the pipeline was restarted for the function.
    unsigned NumRestarts = 0;
  };

  /// The worklist of functions to be processed by function passes.
  std::vector<WorklistEntry> FunctionWorklist;

  // Name of the current optimization stage for diagnostics.
  std::string StageName;

  /// The number of passes run so far.
  unsigned NumPassesRun = 0;
  unsigned numSubpassesRun = 0;

  unsigned maxNumPassesToRun = UINT_MAX;
  unsigned maxNumSubpassesToRun = UINT_MAX;
  unsigned breakBeforePassCount = UINT_MAX;

  /// For invoking Codira passes.
  CodiraPassInvocation languagePassInvocation;

  /// A mask which has one bit for each pass. A one for a pass-bit means that
  /// the pass doesn't need to run, because nothing has changed since the
  /// previous run of that pass.
  typedef std::bitset<(size_t)PassKind::numPasses> CompletedPasses;
  
  /// A completed-passes mask for each function.
  toolchain::DenseMap<SILFunction *, CompletedPasses> CompletedPassesMap;

  /// Stores for each function the number of levels of specializations it is
  /// derived from an original function. E.g. if a function is a signature
  /// optimized specialization of a generic specialization, it has level 2.
  /// This is used to avoid an infinite amount of functions pushed on the
  /// worklist (e.g. caused by a bug in a specializing optimization).
  toolchain::DenseMap<SILFunction *, int> DerivationLevels;

  /// Set to true when a pass invalidates an analysis.
  bool CurrentPassHasInvalidated = false;

  bool currentPassDependsOnCalleeBodies = false;

  /// True if we need to stop running passes and restart again on the
  /// same function.
  bool RestartPipeline = false;

  /// If true, passes are also run for functions which have
  /// OptimizationMode::NoOptimization.
  bool isMandatory = false;

  /// The notification handler for this specific SILPassManager.
  ///
  /// This is not owned by the pass manager, it is owned by the SILModule which
  /// is guaranteed to outlive any pass manager associated with it. We keep this
  /// bare pointer to ensure that we can deregister the notification after this
  /// pass manager is destroyed.
  DeserializationNotificationHandler *deserializationNotificationHandler;

  std::chrono::nanoseconds totalPassRuntime = std::chrono::nanoseconds(0);

public:
  /// C'tor. It creates and registers all analysis passes, which are defined
  /// in Analysis.def.
  SILPassManager(SILModule *M, bool isMandatory, irgen::IRGenModule *IRMod);

  const SILOptions &getOptions() const;

  /// Searches for an analysis of type T in the list of registered
  /// analysis. If the analysis is not found, the program terminates.
  template<typename T>
  T *getAnalysis() {
    for (SILAnalysis *A : Analyses)
      if (auto *R = toolchain::dyn_cast<T>(A))
        return R;

    toolchain_unreachable("Unable to find analysis for requested type.");
  }

  template<typename T>
  T *getAnalysis(SILFunction *f) {
    for (SILAnalysis *A : Analyses) {
      if (A->getKind() == T::getAnalysisKind())
        return static_cast<FunctionAnalysisBase<T> *>(A)->get(f);
    }
    toolchain_unreachable("Unable to find analysis for requested type.");
  }

  /// \returns the module that the pass manager owns.
  SILModule *getModule() { return Mod; }

  /// \returns the associated IGenModule or null if this is not an IRGen
  /// pass manager.
  irgen::IRGenModule *getIRGenModule();

  CodiraPassInvocation *getCodiraPassInvocation() {
    return &languagePassInvocation;
  }

  /// Restart the function pass pipeline on the same function
  /// that is currently being processed.
  void restartWithCurrentFunction(SILTransform *T);
  void clearRestartPipeline() { RestartPipeline = false; }
  bool shouldRestartPipeline() { return RestartPipeline; }

  /// Iterate over all analysis and invalidate them.
  void invalidateAllAnalysis() {
    // Invalidate the analysis (unless they are locked)
    for (auto AP : Analyses)
      if (!AP->isLocked())
        AP->invalidate();

    CurrentPassHasInvalidated = true;

    // Assume that all functions have changed. Clear all masks of all functions.
    CompletedPassesMap.clear();
  }

  /// Notify the pass manager of a newly create function for tracing.
  void notifyOfNewFunction(SILFunction *F, SILTransform *T);

  /// Add the function \p F to the function pass worklist.
  /// If not null, the function \p DerivedFrom is the function from which \p F
  /// is derived. This is used to avoid an infinite amount of functions pushed
  /// on the worklist (e.g. caused by a bug in a specializing optimization).
  void addFunctionToWorklist(SILFunction *F, SILFunction *DerivedFrom);

  /// Iterate over all analysis and notify them of the function.
  ///
  /// This function does not necessarily have to be newly created function. It
  /// is the job of the analysis to make sure no extra work is done if the
  /// particular analysis has been done on the function.
  void notifyAnalysisOfFunction(SILFunction *F) {
    for (auto AP : Analyses) {
      AP->notifyAddedOrModifiedFunction(F);
    }
  }

  /// Broadcast the invalidation of the function to all analysis.
  void invalidateAnalysis(SILFunction *F,
                          SILAnalysis::InvalidationKind K) {
    // Invalidate the analysis (unless they are locked)
    for (auto AP : Analyses)
      if (!AP->isLocked())
        AP->invalidate(F, K);
    
    CurrentPassHasInvalidated = true;
    // Any change let all passes run again.
    CompletedPassesMap[F].reset();
  }

  /// Iterate over all analysis and notify them of a change in witness-
  /// or vtables.
  void invalidateFunctionTables() {
    // Invalidate the analysis (unless they are locked)
    for (auto AP : Analyses)
      if (!AP->isLocked())
        AP->invalidateFunctionTables();

    CurrentPassHasInvalidated = true;

    // Assume that all functions have changed. Clear all masks of all functions.
    CompletedPassesMap.clear();
  }

  /// Iterate over all analysis and notify them of a deleted function.
  void notifyWillDeleteFunction(SILFunction *F) {
    // Invalidate the analysis (unless they are locked)
    for (auto AP : Analyses)
      if (!AP->isLocked())
        AP->notifyWillDeleteFunction(F);

    CurrentPassHasInvalidated = true;
    // Any change let all passes run again.
    CompletedPassesMap[F].reset();
  }

  void setDependingOnCalleeBodies() {
    currentPassDependsOnCalleeBodies = true;
  }

  /// Reset the state of the pass manager and remove all transformation
  /// owned by the pass manager. Analysis passes will be kept.
  void resetAndRemoveTransformations();

  /// Set the name of the current optimization stage.
  ///
  /// This is useful for debugging.
  void setStageName(toolchain::StringRef NextStage = "");

  /// Get the name of the current optimization stage.
  ///
  /// This is useful for debugging.
  StringRef getStageName() const;

  /// D'tor.
  ~SILPassManager();

  /// Verify all analyses.
  void verifyAnalyses() const;

  /// Precompute all analyses.
  void forcePrecomputeAnalyses(SILFunction *F) {
    for (auto *A : Analyses) {
      A->forcePrecompute(F);
    }
  }

  /// Verify all analyses, limiting the verification to just this one function
  /// if possible.
  ///
  /// Discussion: We leave it up to the analyses to decide how to implement
  /// this. If no override is provided the SILAnalysis should just call the
  /// normal verify method.
  void verifyAnalyses(SILFunction *F) const;

  void executePassPipelinePlan(const SILPassPipelinePlan &Plan);

  using Transformee = toolchain::PointerUnion<SILValue, SILInstruction *>;
  bool continueWithNextSubpassRun(std::optional<Transformee> forTransformee,
                                  SILFunction *function, SILTransform *trans);

  static bool isPassDisabled(StringRef passName);
  static bool isInstructionPassDisabled(StringRef instName);
  static bool isAnyPassDisabled();
  static bool disablePassesForFunction(SILFunction *function);

  /// Runs the SIL verifier which is implemented in the CodiraCompilerSources.
  void runCodiraFunctionVerification(SILFunction *f);

  /// Runs the SIL verifier which is implemented in the CodiraCompilerSources.
  void runCodiraModuleVerification();

private:
  void parsePassesToRunCount(StringRef countsStr);
  void parseBreakBeforePassCount(StringRef countsStr);

  bool doPrintBefore(SILTransform *T, SILFunction *F);

  bool doPrintAfter(SILTransform *T, SILFunction *F, bool PassChangedSIL);

  void execute();

  /// Add a pass of a specific kind.
  void addPass(PassKind Kind);

  /// Add a pass with a given name.
  void addPassForName(StringRef Name);

  /// Run the \p TransIdx'th SIL module transform over all the functions in
  /// the module.
  void runModulePass(unsigned TransIdx);

  /// Run the \p TransIdx'th pass on the function \p F.
  void runPassOnFunction(unsigned TransIdx, SILFunction *F);

  /// Run the passes in Transform from \p FromTransIdx to \p ToTransIdx.
  void runFunctionPasses(unsigned FromTransIdx, unsigned ToTransIdx);

  /// Helper function to check if the function pass should be run mandatorily
  /// All passes in mandatory pass pipeline and ownership model elimination are
  /// mandatory function passes.
  bool isMandatoryFunctionPass(SILFunctionTransform *);

  /// A helper function that returns (based on SIL stage and debug
  /// options) whether we should continue running passes.
  bool continueTransforming();

  /// Break before running a pass.
  bool breakBeforeRunning(StringRef fnName, SILFunctionTransform *SFT);

  /// Return true if all analyses are unlocked.
  bool analysesUnlocked();

  /// Dumps information about the pass with index \p TransIdx to toolchain::dbgs().
  void dumpPassInfo(const char *Title, SILTransform *Tr, SILFunction *F,
                    int passIdx = -1);

  /// Dumps information about the pass with index \p TransIdx to toolchain::dbgs().
  void dumpPassInfo(const char *Title, unsigned TransIdx,
                    SILFunction *F = nullptr);

  /// Displays the call graph in an external dot-viewer.
  /// This function is meant for use from the debugger.
  /// When asserts are disabled, this is a NoOp.
  void viewCallGraph();
};

inline void CodiraPassInvocation::
notifyChanges(SILAnalysis::InvalidationKind invalidationKind) {
    changeNotifications = (SILAnalysis::InvalidationKind)
        (changeNotifications | invalidationKind);
}
inline void CodiraPassInvocation::notifyFunctionTablesChanged() {
  functionTablesChanged = true;
}

} // end namespace language

#endif
