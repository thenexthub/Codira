//===--- DifferentiableActivityAnalysis.h ---------------------*- C++ -*---===//
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
//
// This file implements activity analysis: a dataflow analysis used for
// automatic differentiation.
//
// In many real situations, the end-users of AD need only the derivatives of
// some selected outputs of `P` with respect to some selected inputs of `P`.
// Whatever the differentiation mode (tangent, reverse,...), these restrictions
// allow the AD tool to produce a much more efficient differentiated program.
// Essentially, fixing some inputs and neglecting some outputs allows AD to
// just forget about several intermediate differentiated variables.
//
// Activity analysis is the specific analysis that detects these situations,
// therefore allowing for a better differentiated code. Activity analysis is
// present in all transformation-based AD tools.
//
// To begin with, the end-user specifies that only some output variables (the
// “dependent”) must be differentiated with respect to only some input
// variables (the “independent”). We say that variable `y` depends on `x` when
// the derivative of `y` with respect to `x` is not trivially null. We say that
// a variable is “varied” if it depends on at least one independent. Conversely
// we say that a variable is “useful” if at least one dependent depends on it.
// Finally, we say that a variable is “active” if it is at the same time varied
// and useful. In the special case of the tangent mode, it is easy to check
// that when variable `v` is not varied at some place in the program, then its
// derivative `v̇` at this place is certainly null. Conversely when variable `v`
// is not useful, then whatever the value of `v̇`, this value does not matter
// for the final result. Symmetric reasoning applies for the reverse mode of
// AD: observing that differentiated variables go upstream, we see that a
// useless variable has a null derivative, in other words the partial
// derivative of the output with respect to this variable is null. Conversely
// when variable `v` is not varied, then whatever the value of `v`, this value
// does not matter for the final result.
//
// Reference:
// Laurent Hascoët. Automatic Differentiation by Program Transformation. 2007.

#ifndef LANGUAGE_SILOPTIMIZER_ANALYSIS_DIFFERENTIABLEACTIVITYANALYSIS_H_
#define LANGUAGE_SILOPTIMIZER_ANALYSIS_DIFFERENTIABLEACTIVITYANALYSIS_H_

#include "language/AST/GenericEnvironment.h"
#include "language/SIL/SILFunction.h"
#include "language/SIL/SILModule.h"
#include "language/SIL/SILValue.h"
#include "language/SILOptimizer/Analysis/Analysis.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/DenseSet.h"

using toolchain::SmallDenseMap;
using toolchain::SmallDenseSet;

namespace language {

class DominanceAnalysis;
class PostDominanceAnalysis;
class DominanceInfo;
class PostDominanceInfo;
class SILFunction;

class DifferentiableActivityCollection;
class DifferentiableActivityAnalysis
    : public FunctionAnalysisBase<DifferentiableActivityCollection> {
private:
  DominanceAnalysis *dominanceAnalysis = nullptr;
  PostDominanceAnalysis *postDominanceAnalysis = nullptr;

public:
  explicit DifferentiableActivityAnalysis()
      : FunctionAnalysisBase(SILAnalysisKind::DifferentiableActivity) {}

  static bool classof(const SILAnalysis *s) {
    return s->getKind() == SILAnalysisKind::DifferentiableActivity;
  }

  virtual bool shouldInvalidate(SILAnalysis::InvalidationKind k) override {
    return k & InvalidationKind::FunctionBody;
  }

  virtual std::unique_ptr<DifferentiableActivityCollection>
  newFunctionAnalysis(SILFunction *f) override;

  virtual void initialize(SILPassManager *pm) override;
};

/// Represents the differentiation activity associated with a SIL value.
enum class ActivityFlags : unsigned {
  /// The value depends on a function parameter.
  Varied = 1 << 1,
  /// The value contributes to a result.
  Useful = 1 << 2,
  /// The value is both varied and useful.
  Active = Varied | Useful,
};

using Activity = OptionSet<ActivityFlags>;

/// Result of activity analysis on a function. Accepts queries for whether a
/// value is "varied", "useful" or "active" against certain differentiation
/// indices.
class DifferentiableActivityInfo {
private:
  DifferentiableActivityCollection &parent;

  /// The derivative generic signature.
  GenericSignature derivativeGenericSignature;

  /// Input values, i.e. parameters (both direct and indirect).
  SmallVector<SILValue, 4> inputValues;
  /// Output values, i.e. individual values (not the final tuple) being returned
  /// by the `return` instruction.
  SmallVector<SILValue, 4> outputValues;

  /// The set of varied variables, indexed by the corresponding independent
  /// value (input) index.
  SmallVector<SmallDenseSet<SILValue>, 4> variedValueSets;
  /// The set of useful variables, indexed by the corresponding dependent value
  /// (output) index.
  SmallVector<SmallDenseSet<SILValue>, 4> usefulValueSets;

  /// The original function.
  SILFunction &getFunction() const;

  /// Returns true if the given SILValue has a tangent space.
  bool hasTangentSpace(SILValue value) {
    auto type = value->getType().getASTType();
    // Remap archetypes in the derivative generic signature, if it exists.
    if (type->hasArchetype()) {
      type = derivativeGenericSignature.getReducedType(
          type->mapTypeOutOfContext());
    }
    // Look up conformance in the current module.
    auto lookupConformance = LookUpConformanceInModule();
    return type->getAutoDiffTangentSpace(lookupConformance).has_value();
  }

  /// Perform analysis and populate variedness and usefulness sets.
  void analyze(DominanceInfo *di, PostDominanceInfo *pdi);

  /// Marks the given value as varied and propagates variedness to users.
  void setVariedAndPropagateToUsers(SILValue value,
                                    unsigned independentVariableIndex);
  /// Propagates variedness from the given operand to its user's results.
  void propagateVaried(Operand *operand, unsigned independentVariableIndex);
  /// Marks the given value as varied and recursively propagates variedness
  /// inwards (to operands) through projections. Skips `@noDerivative` field
  /// projections.
  void
  propagateVariedInwardsThroughProjections(SILValue value,
                                           unsigned independentVariableIndex);

  /// Marks the given value as useful for the given dependent variable index.
  void setUseful(SILValue value, unsigned dependentVariableIndex);
  /// Marks the given value as useful and recursively propagates usefulness to:
  /// - Defining instruction operands, if the value has a defining instruction.
  /// - Incoming values, if the value is a basic block argument.
  void setUsefulAndPropagateToOperands(SILValue value,
                                       unsigned dependentVariableIndex);
  /// Propagates usefulness to the operands of the given instruction.
  void propagateUseful(SILInstruction *inst, unsigned dependentVariableIndex);
  /// Marks the given address or class-typed value as useful and recursively
  /// propagates usefulness inwards (to operands) through projections. Skips
  /// `@noDerivative` field projections.
  void propagateUsefulThroughAddress(SILValue value,
                                     unsigned dependentVariableIndex);
  /// If the given value is an `array.uninitialized_intrinsic` application,
  /// selectively propagate usefulness through its `RawPointer` result.
  void setUsefulThroughArrayInitialization(SILValue value,
                                           unsigned dependentVariableIndex);

public:
  explicit DifferentiableActivityInfo(
      DifferentiableActivityCollection &parent,
      GenericSignature derivativeGenericSignature);

  /// Returns true if the given value is varied for the given independent
  /// variable index.
  bool isVaried(SILValue value, unsigned independentVariableIndex) const;

  /// Returns true if the given value is varied for any of the given parameter
  /// (independent variable) indices.
  bool isVaried(SILValue value, IndexSubset *independentVariableIndices) const;

  /// Returns true if the given value is useful for the given dependent variable
  /// index.
  bool isUseful(SILValue value, unsigned dependentVariableIndex) const;

  /// Returns true if the given value is varied for any of the given result
  /// (dependent variable) indices.
  bool isUseful(SILValue value, IndexSubset *dependentVariableIndices) const;

  /// Returns true if the given value is active for the given parameter indices
  /// and result indices.
  bool isActive(SILValue value,
                IndexSubset *parameterIndices,
                IndexSubset *resultIndices) const;

  /// Returns true if the given value is active for the given config.
  bool isActive(SILValue value, const AutoDiffConfig &config) const {
    return isActive(value, config.parameterIndices, config.resultIndices);
  }

  /// Returns the activity of the given value for the given config.
  Activity getActivity(SILValue value,
                       IndexSubset *parameterIndices,
                       IndexSubset *resultIndices) const;

  /// Returns the activity of the given value for the given config.
  Activity getActivity(SILValue value, const AutoDiffConfig &config) const {
    return getActivity(value, config.parameterIndices, config.resultIndices);
  }

  /// Prints activity information for the config of the given value.
  void dump(SILValue value,
            IndexSubset *parameterIndices, IndexSubset *resultIndices,
            toolchain::raw_ostream &s = toolchain::dbgs()) const;

  /// Prints activity information for the config of the given value.
  void dump(SILValue value, const AutoDiffConfig &config,
            toolchain::raw_ostream &s = toolchain::dbgs()) const {
    return dump(value, config.parameterIndices, config.resultIndices, s);
  }

  /// Prints all activity information for the given parameter indices and result
  /// indices.
  void dump(IndexSubset *parameterIndices, IndexSubset *resultIndices,
            toolchain::raw_ostream &s = toolchain::dbgs()) const;

  /// Prints all activity information for the given config.
  void dump(const AutoDiffConfig &config,
            toolchain::raw_ostream &s = toolchain::dbgs()) const {
    return dump(config.parameterIndices, config.resultIndices, s);
  }
};

class DifferentiableActivityCollection {
public:
  SmallDenseMap<GenericSignature, DifferentiableActivityInfo> activityInfoMap;
  SILFunction &function;
  DominanceInfo *domInfo;
  PostDominanceInfo *postDomInfo;

  DifferentiableActivityInfo &
  getActivityInfo(GenericSignature assocGenSig,
                  AutoDiffDerivativeFunctionKind kind) {
    auto activityInfoLookup = activityInfoMap.find(assocGenSig);
    if (activityInfoLookup != activityInfoMap.end())
      return activityInfoLookup->getSecond();
    auto insertion = activityInfoMap.insert(
        {assocGenSig, DifferentiableActivityInfo(*this, assocGenSig)});
    return insertion.first->getSecond();
  }

  explicit DifferentiableActivityCollection(SILFunction &f, DominanceInfo *di,
                                            PostDominanceInfo *pdi);
};

} // end namespace language

#endif // LANGUAGE_SILOPTIMIZER_ANALYSIS_DIFFERENTIABLEACTIVITYANALYSIS_H_
