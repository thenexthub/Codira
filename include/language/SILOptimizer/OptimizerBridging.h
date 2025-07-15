//===--- OptimizerBridging.h - header for the OptimizerBridging module ----===//
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

#ifndef LANGUAGE_SILOPTIMIZER_OPTIMIZERBRIDGING_H
#define LANGUAGE_SILOPTIMIZER_OPTIMIZERBRIDGING_H

// Do not add other C++/toolchain/language header files here!
// Function implementations should be placed into OptimizerBridgingImpl.h or PassManager.cpp
// (under OptimizerBridging) andrequired header files should be added there.
//
// Pure bridging mode does not permit including any C++/toolchain/language headers.
// See also the comments for `BRIDGING_MODE` in the top-level CMakeLists.txt file.
//
#include "language/AST/ASTBridging.h"
#include "language/SIL/SILBridging.h"

#ifndef USED_IN_CPP_SOURCE

// Pure bridging mode does not permit including any C++/toolchain/language headers.
// See also the comments for `BRIDGING_MODE` in the top-level CMakeLists.txt file.
#ifdef LANGUAGE_SIL_SILVALUE_H
#error "should not include language headers into bridging header"
#endif
#ifdef TOOLCHAIN_SUPPORT_COMPILER_H
#error "should not include toolchain headers into bridging header"
#endif

#endif // USED_IN_CPP_SOURCE

LANGUAGE_BEGIN_NULLABILITY_ANNOTATIONS

namespace language {
class AliasAnalysis;
class BasicCalleeAnalysis;
class CalleeList;
class DeadEndBlocks;
class DominanceInfo;
class PostDominanceInfo;
class BasicBlockSet;
class NodeSet;
class OperandSet;
class ClonerWithFixedLocation;
class CodiraPassInvocation;
class FixedSizeSlabPayload;
class FixedSizeSlab;
class SILVTable;
class SpecializationCloner;
}

struct BridgedPassContext;

struct BridgedAliasAnalysis {
  language::AliasAnalysis * _Nonnull aa;

  // Workaround for a compiler bug.
  // When this unused function is removed, the compiler gives an error.
  BRIDGED_INLINE bool unused(BridgedValue address1, BridgedValue address2) const;

  typedef void (* _Nonnull InitFn)(BridgedAliasAnalysis aliasAnalysis, CodiraInt size);
  typedef void (* _Nonnull DestroyFn)(BridgedAliasAnalysis aliasAnalysis);
  typedef BridgedMemoryBehavior (* _Nonnull GetMemEffectFn)(
        BridgedPassContext context, BridgedAliasAnalysis aliasAnalysis,
        BridgedValue, BridgedInstruction);
  typedef bool (* _Nonnull Escaping2InstFn)(
        BridgedPassContext context, BridgedAliasAnalysis aliasAnalysis, BridgedValue, BridgedInstruction);
  typedef bool (* _Nonnull Escaping2ValIntFn)(
        BridgedPassContext context, BridgedAliasAnalysis aliasAnalysis, BridgedValue, BridgedValue);
  typedef bool (* _Nonnull MayAliasFn)(
        BridgedPassContext context, BridgedAliasAnalysis aliasAnalysis, BridgedValue, BridgedValue);

  static void registerAnalysis(InitFn initFn,
                               DestroyFn destroyFn,
                               GetMemEffectFn getMemEffectsFn,
                               Escaping2InstFn isObjReleasedFn,
                               Escaping2ValIntFn isAddrVisibleFromObjFn,
                               MayAliasFn mayAliasFn);
};

struct BridgedCalleeAnalysis {
  language::BasicCalleeAnalysis * _Nonnull ca;

  struct CalleeList {
    uint64_t storage[3];

    BRIDGED_INLINE CalleeList(language::CalleeList list);
    BRIDGED_INLINE language::CalleeList unbridged() const;

    BRIDGED_INLINE bool isIncomplete() const;
    BRIDGED_INLINE CodiraInt getCount() const;
    LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedFunction getCallee(CodiraInt index) const;
  };

  LANGUAGE_IMPORT_UNSAFE CalleeList getCallees(BridgedValue callee) const;
  LANGUAGE_IMPORT_UNSAFE CalleeList getDestructors(BridgedType type, bool isExactType) const;

  typedef bool (* _Nonnull IsDeinitBarrierFn)(BridgedInstruction, BridgedCalleeAnalysis bca);
  typedef BridgedMemoryBehavior (* _Nonnull GetMemBehvaiorFn)(
        BridgedInstruction apply, bool observeRetains, BridgedCalleeAnalysis bca);

  static void registerAnalysis(IsDeinitBarrierFn isDeinitBarrierFn,
                               GetMemBehvaiorFn getEffectsFn);
};

struct BridgedDeadEndBlocksAnalysis {
  language::DeadEndBlocks * _Nonnull deb;

  BRIDGED_INLINE bool isDeadEnd(BridgedBasicBlock block) const;
};

struct BridgedDomTree {
  language::DominanceInfo * _Nonnull di;

  BRIDGED_INLINE bool dominates(BridgedBasicBlock dominating, BridgedBasicBlock dominated) const;
};

struct BridgedPostDomTree {
  language::PostDominanceInfo * _Nonnull pdi;

  BRIDGED_INLINE bool postDominates(BridgedBasicBlock dominating, BridgedBasicBlock dominated) const;
};

struct BridgedUtilities {
  typedef void (* _Nonnull VerifyFunctionFn)(BridgedPassContext, BridgedFunction);
  typedef void (* _Nonnull UpdateFunctionFn)(BridgedPassContext, BridgedFunction);
  typedef void (* _Nonnull UpdatePhisFn)(BridgedPassContext, BridgedArrayRef);

  static void registerVerifier(VerifyFunctionFn verifyFunctionFn);
  static void registerPhiUpdater(UpdateFunctionFn updateBorrowedFromFn,
                                 UpdatePhisFn updateBorrowedFromPhisFn,
                                 UpdatePhisFn replacePhisWithIncomingValuesFn);
};

struct BridgedBasicBlockSet {
  language::BasicBlockSet * _Nonnull set;

  BRIDGED_INLINE bool contains(BridgedBasicBlock block) const;
  BRIDGED_INLINE bool insert(BridgedBasicBlock block) const;
  BRIDGED_INLINE void erase(BridgedBasicBlock block) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedFunction getFunction() const;
};

struct BridgedNodeSet {
  language::NodeSet * _Nonnull set;

  BRIDGED_INLINE bool containsValue(BridgedValue value) const;
  BRIDGED_INLINE bool insertValue(BridgedValue value) const;
  BRIDGED_INLINE void eraseValue(BridgedValue value) const;
  BRIDGED_INLINE bool containsInstruction(BridgedInstruction inst) const;
  BRIDGED_INLINE bool insertInstruction(BridgedInstruction inst) const;
  BRIDGED_INLINE void eraseInstruction(BridgedInstruction inst) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedFunction getFunction() const;
};

struct BridgedOperandSet {
  language::OperandSet * _Nonnull set;

  BRIDGED_INLINE bool contains(BridgedOperand operand) const;
  BRIDGED_INLINE bool insert(BridgedOperand operand) const;
  BRIDGED_INLINE void erase(BridgedOperand operand) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedFunction getFunction() const;
};

struct BridgedCloner {
  language::ClonerWithFixedLocation * _Nonnull cloner;

  BridgedCloner(BridgedGlobalVar var, BridgedPassContext context);
  BridgedCloner(BridgedInstruction inst, BridgedPassContext context);
  void destroy(BridgedPassContext context);
  LANGUAGE_IMPORT_UNSAFE BridgedValue getClonedValue(BridgedValue v);
  bool isValueCloned(BridgedValue v) const;
  void clone(BridgedInstruction inst);
  void recordFoldedValue(BridgedValue origValue, BridgedValue mappedValue);
};

struct BridgedSpecializationCloner {
  language::SpecializationCloner * _Nonnull cloner;

  LANGUAGE_IMPORT_UNSAFE BridgedSpecializationCloner(BridgedFunction emptySpecializedFunction);
  LANGUAGE_IMPORT_UNSAFE BridgedFunction getCloned() const;
  LANGUAGE_IMPORT_UNSAFE BridgedBasicBlock getClonedBasicBlock(BridgedBasicBlock originalBasicBlock) const;
  void cloneFunctionBody(BridgedFunction originalFunction, BridgedBasicBlock clonedEntryBlock,
                         BridgedValueArray clonedEntryBlockArgs) const;
  void cloneFunctionBody(BridgedFunction originalFunction) const;
};

struct BridgedPassContext {
  language::CodiraPassInvocation * _Nonnull invocation;

  enum class SILStage {
    Raw,
    Canonical,
    Lowered
  };

  BridgedOwnedString getModuleDescription() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedChangeNotificationHandler asNotificationHandler() const;
  BRIDGED_INLINE void notifyDependencyOnBodyOf(BridgedFunction otherFunction) const;
  BRIDGED_INLINE SILStage getSILStage() const;
  BRIDGED_INLINE bool hadError() const;
  BRIDGED_INLINE bool moduleIsSerialized() const;
  BRIDGED_INLINE bool moduleHasLoweredAddresses() const;
  BRIDGED_INLINE bool isTransforming(BridgedFunction function) const;

  // Analysis

  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedAliasAnalysis getAliasAnalysis() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedCalleeAnalysis getCalleeAnalysis() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedDeadEndBlocksAnalysis getDeadEndBlocksAnalysis() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedDomTree getDomTree() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedPostDomTree getPostDomTree() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedDeclObj getCodiraArrayDecl() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedDeclObj getCodiraMutableSpanDecl() const;

  // AST

  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE
  BridgedDiagnosticEngine getDiagnosticEngine() const;

  // SIL modifications

  struct DevirtResult {
    OptionalBridgedInstruction newApply;
    bool cfgChanged;
  };

  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedBasicBlock splitBlockBefore(BridgedInstruction bridgedInst) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedBasicBlock splitBlockAfter(BridgedInstruction bridgedInst) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedBasicBlock createBlockAfter(BridgedBasicBlock bridgedBlock) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedBasicBlock appendBlock(BridgedFunction bridgedFunction) const;
  BRIDGED_INLINE void eraseInstruction(BridgedInstruction inst, bool salvageDebugInfo) const;
  BRIDGED_INLINE void eraseBlock(BridgedBasicBlock block) const;
  static BRIDGED_INLINE void moveInstructionBefore(BridgedInstruction inst, BridgedInstruction beforeInst);
  bool tryOptimizeApplyOfPartialApply(BridgedInstruction closure) const;
  bool tryDeleteDeadClosure(BridgedInstruction closure, bool needKeepArgsAlive) const;
  LANGUAGE_IMPORT_UNSAFE DevirtResult tryDevirtualizeApply(BridgedInstruction apply, bool isMandatory) const;
  bool tryOptimizeKeypath(BridgedInstruction apply) const;
  LANGUAGE_IMPORT_UNSAFE OptionalBridgedValue constantFoldBuiltin(BridgedInstruction builtin) const;
  LANGUAGE_IMPORT_UNSAFE OptionalBridgedFunction specializeFunction(BridgedFunction function,
                                                                 BridgedSubstitutionMap substitutions) const;
  void deserializeAllCallees(BridgedFunction function, bool deserializeAll) const;
  bool specializeClassMethodInst(BridgedInstruction cm) const;
  bool specializeWitnessMethodInst(BridgedInstruction wm) const;
  bool specializeAppliesInFunction(BridgedFunction function, bool isMandatory) const;
  BridgedOwnedString mangleOutlinedVariable(BridgedFunction function) const;
  BridgedOwnedString mangleAsyncRemoved(BridgedFunction function) const;
  BridgedOwnedString mangleWithDeadArgs(BridgedArrayRef bridgedDeadArgIndices, BridgedFunction function) const;
  BridgedOwnedString mangleWithClosureArgs(BridgedValueArray closureArgs,
                                                               BridgedArrayRef closureArgIndices,
                                                               BridgedFunction applySiteCallee) const;
  BridgedOwnedString mangleWithBoxToStackPromotedArgs(BridgedArrayRef bridgedPromotedArgIndices,
                                                      BridgedFunction bridgedOriginalFunction) const;

  LANGUAGE_IMPORT_UNSAFE BridgedGlobalVar createGlobalVariable(BridgedStringRef name, BridgedType type,
                                                            BridgedLinkage linkage, bool isLet) const;
  void inlineFunction(BridgedInstruction apply, bool mandatoryInline) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedValue getSILUndef(BridgedType type) const;
  BRIDGED_INLINE bool eliminateDeadAllocations(BridgedFunction f) const;
  void eraseFunction(BridgedFunction function) const;

  BRIDGED_INLINE bool shouldExpand(BridgedType type) const;

  // IRGen

  CodiraInt getStaticSize(BridgedType type) const;
  CodiraInt getStaticAlignment(BridgedType type) const;
  CodiraInt getStaticStride(BridgedType type) const;
  bool canMakeStaticObjectReadOnly(BridgedType type) const;

  // Sets

  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedBasicBlockSet allocBasicBlockSet() const;
  BRIDGED_INLINE void freeBasicBlockSet(BridgedBasicBlockSet set) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedNodeSet allocNodeSet() const;
  BRIDGED_INLINE void freeNodeSet(BridgedNodeSet set) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedOperandSet allocOperandSet() const;
  BRIDGED_INLINE void freeOperandSet(BridgedOperandSet set) const;

  // Stack nesting

  BRIDGED_INLINE void notifyInvalidatedStackNesting() const;
  BRIDGED_INLINE bool getNeedFixStackNesting() const;
  void fixStackNesting(BridgedFunction function) const;

  // Slabs

  struct Slab {
    language::FixedSizeSlabPayload * _Nullable data = nullptr;

    BRIDGED_INLINE static CodiraInt getCapacity();
    BRIDGED_INLINE Slab(language::FixedSizeSlab * _Nullable slab);
    BRIDGED_INLINE language::FixedSizeSlab * _Nullable getSlab() const;
    LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE Slab getNext() const;
    LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE Slab getPrevious() const;
  };

  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE Slab allocSlab(Slab afterSlab) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE Slab freeSlab(Slab slab) const;

  // Access SIL module data structures

  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE OptionalBridgedFunction getFirstFunctionInModule() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE static OptionalBridgedFunction getNextFunctionInModule(BridgedFunction function);
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE OptionalBridgedGlobalVar getFirstGlobalInModule() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE static OptionalBridgedGlobalVar getNextGlobalInModule(BridgedGlobalVar global);
  BRIDGED_INLINE CodiraInt getNumVTables() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedVTable getVTable(CodiraInt index) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE OptionalBridgedWitnessTable getFirstWitnessTableInModule() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE static OptionalBridgedWitnessTable getNextWitnessTableInModule(
                                                                                  BridgedWitnessTable table);
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE OptionalBridgedDefaultWitnessTable getFirstDefaultWitnessTableInModule() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE static OptionalBridgedDefaultWitnessTable getNextDefaultWitnessTableInModule(
                                                                                  BridgedDefaultWitnessTable table);
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE OptionalBridgedFunction lookupFunction(BridgedStringRef name) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE OptionalBridgedFunction loadFunction(BridgedStringRef name,
                                                                          bool loadCalleesRecursively) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE
  OptionalBridgedVTable lookupVTable(BridgedDeclObj classDecl) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE
  OptionalBridgedVTable lookupSpecializedVTable(BridgedType classType) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE
  BridgedConformance getSpecializedConformance(BridgedConformance genericConformance,
                                                       BridgedASTType type,
                                                       BridgedSubstitutionMap substitutions) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE
  OptionalBridgedWitnessTable lookupWitnessTable(BridgedConformance conformance) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedWitnessTable createSpecializedWitnessTable(BridgedLinkage linkage,
                                                                            bool serialized,
                                                                            BridgedConformance conformance,
                                                                            BridgedArrayRef bridgedEntries) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedVTable createSpecializedVTable(BridgedType classType,
                                                                           bool serialized,
                                                                           BridgedArrayRef bridgedEntries) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE void loadFunction(BridgedFunction function, bool loadCalleesRecursively) const;
  LANGUAGE_IMPORT_UNSAFE OptionalBridgedFunction lookupStdlibFunction(BridgedStringRef name) const;
  LANGUAGE_IMPORT_UNSAFE OptionalBridgedFunction lookUpNominalDeinitFunction(BridgedDeclObj nominal) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedSubstitutionMap getContextSubstitutionMap(BridgedType type) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedType getBuiltinIntegerType(CodiraInt bitWidth) const;
  BRIDGED_INLINE bool calleesAreStaticallyKnowable(BridgedDeclRef method) const;
  LANGUAGE_IMPORT_UNSAFE BridgedFunction createEmptyFunction(BridgedStringRef name,
                                                          const BridgedParameterInfo * _Nullable bridgedParams,
                                                          CodiraInt paramCount,
                                                          bool hasSelfParam,
                                                          BridgedFunction fromFunc) const;
  void moveFunctionBody(BridgedFunction sourceFunc, BridgedFunction destFunc) const;

  // Passmanager housekeeping

  BRIDGED_INLINE void beginTransformFunction(BridgedFunction function) const;
  BRIDGED_INLINE void endTransformFunction() const;
  BRIDGED_INLINE bool continueWithNextSubpassRun(OptionalBridgedInstruction inst) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedPassContext initializeNestedPassContext(BridgedFunction newFunction) const;
  BRIDGED_INLINE void deinitializedNestedPassContext() const;
  BRIDGED_INLINE void
  addFunctionToPassManagerWorklist(BridgedFunction newFunction,
                                   BridgedFunction oldFunction) const;

  // SSAUpdater

  BRIDGED_INLINE void
  SSAUpdater_initialize(BridgedFunction function, BridgedType type,
                        BridgedValue::Ownership ownership) const;
  BRIDGED_INLINE void SSAUpdater_addAvailableValue(BridgedBasicBlock block, BridgedValue value) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedValue SSAUpdater_getValueAtEndOfBlock(BridgedBasicBlock block) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedValue SSAUpdater_getValueInMiddleOfBlock(BridgedBasicBlock block) const;
  BRIDGED_INLINE CodiraInt SSAUpdater_getNumInsertedPhis() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedValue SSAUpdater_getInsertedPhi(CodiraInt idx) const;

  // Options

  enum class AssertConfiguration {
    Debug = 0,
    Release = 1,
    Unchecked = 2
  };

  BRIDGED_INLINE bool useAggressiveReg2MemForCodeSize() const;
  BRIDGED_INLINE bool enableStackProtection() const;
  BRIDGED_INLINE bool enableMergeableTraps() const;
  BRIDGED_INLINE bool hasFeature(BridgedFeature feature) const;
  BRIDGED_INLINE bool enableMoveInoutStackProtection() const;
  BRIDGED_INLINE AssertConfiguration getAssertConfiguration() const;
  bool enableSimplificationFor(BridgedInstruction inst) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedDeclObj getCurrentModuleContext() const;
  BRIDGED_INLINE bool enableWMORequiredDiagnostics() const;
  BRIDGED_INLINE bool noAllocations() const;

  // Temporary for AddressableParameters Bootstrapping.
  BRIDGED_INLINE bool enableAddressDependencies() const;

  // Closure specializer
  LANGUAGE_IMPORT_UNSAFE BridgedFunction createSpecializedFunctionDeclaration(BridgedStringRef specializedName,
                                                        const BridgedParameterInfo * _Nullable specializedBridgedParams,
                                                        CodiraInt paramCount,
                                                        BridgedFunction bridgedOriginal,
                                                        bool makeThin,
                                                        bool makeBare) const;

  bool completeLifetime(BridgedValue value) const;
};

bool BeginApply_canInline(BridgedInstruction beginApply);

enum class BridgedDynamicCastResult {
  willSucceed,
  maySucceed,
  willFail
};

BridgedDynamicCastResult classifyDynamicCastBridged(BridgedCanType sourceTy, BridgedCanType destTy,
                                                    BridgedFunction function,
                                                    bool sourceTypeIsExact);

BridgedDynamicCastResult classifyDynamicCastBridged(BridgedInstruction inst);

void verifierError(BridgedStringRef message, OptionalBridgedInstruction atInstruction, OptionalBridgedArgument atArgument);

//===----------------------------------------------------------------------===//
//                          Pass registration
//===----------------------------------------------------------------------===//

struct BridgedFunctionPassCtxt {
  BridgedFunction function;
  BridgedPassContext passContext;
} ;

struct BridgedInstructionPassCtxt {
  BridgedInstruction instruction;
  BridgedPassContext passContext;
};

typedef void (* _Nonnull BridgedModulePassRunFn)(BridgedPassContext);
typedef void (* _Nonnull BridgedFunctionPassRunFn)(BridgedFunctionPassCtxt);
typedef void (* _Nonnull BridgedInstructionPassRunFn)(BridgedInstructionPassCtxt);

void SILPassManager_registerModulePass(BridgedStringRef name,
                                       BridgedModulePassRunFn runFn);
void SILPassManager_registerFunctionPass(BridgedStringRef name,
                                         BridgedFunctionPassRunFn runFn);
void SILCombine_registerInstructionPass(BridgedStringRef instClassName,
                                        BridgedInstructionPassRunFn runFn);

#ifndef PURE_BRIDGING_MODE
// In _not_ PURE_BRIDGING_MODE, briding functions are inlined and therefore inluded in the header file.
#include "OptimizerBridgingImpl.h"
#else
// For fflush and stdout
#include <stdio.h>
#endif

LANGUAGE_END_NULLABILITY_ANNOTATIONS

#endif
