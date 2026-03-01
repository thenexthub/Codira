//===- MLRegAllocPriorityAdvisor.cpp - ML priority advisor-----------------===//
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
// Implementation of the ML priority advisor and reward injection pass
//
//===----------------------------------------------------------------------===//

#include "AllocationOrder.h"
#include "RegAllocGreedy.h"
#include "vm/core/Analysis/AliasAnalysis.h"
#include "vm/core/Analysis/InteractiveModelRunner.h"
#include "vm/core/Analysis/MLModelRunner.h"
#include "vm/core/Analysis/ReleaseModeModelRunner.h"
#include "vm/core/Analysis/TensorSpec.h"
#include "vm/core/CodeGen/CalcSpillWeights.h"
#include "vm/core/CodeGen/LiveRegMatrix.h"
#include "vm/core/CodeGen/MachineBlockFrequencyInfo.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineLoopInfo.h"
#include "vm/core/CodeGen/MachineRegisterInfo.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/CodeGen/RegAllocPriorityAdvisor.h"
#include "vm/core/CodeGen/RegisterClassInfo.h"
#include "vm/core/CodeGen/SlotIndexes.h"
#include "vm/core/CodeGen/VirtRegMap.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Pass.h"
#include "vm/core/PassRegistry.h"
#include "vm/core/Support/CommandLine.h"

#if defined(LLVM_HAVE_TFLITE)
#include "vm/core/Analysis/ModelUnderTrainingRunner.h"
#include "vm/core/Analysis/NoInferenceModelRunner.h"
#include "vm/core/Analysis/Utils/TrainingLogger.h"
#include "vm/core/IR/Module.h"
#endif

using namespace vm::core;

static cl::opt<std::string> InteractiveChannelBaseName(
    "regalloc-priority-interactive-channel-base", cl::Hidden,
    cl::desc(
        "Base file path for the interactive mode. The incoming filename should "
        "have the name <regalloc-priority-interactive-channel-base>.in, while "
        "the outgoing name should be "
        "<regalloc-priority-interactive-channel-base>.out"));

using CompiledModelType = NoopSavedModelImpl;

// Options that only make sense in development mode
#ifdef LLVM_HAVE_TFLITE
#include "RegAllocScore.h"
#include "vm/core/Analysis/Utils/TFUtils.h"

static cl::opt<std::string> TrainingLog(
    "regalloc-priority-training-log", cl::Hidden,
    cl::desc("Training log for the register allocator priority model"));

static cl::opt<std::string> ModelUnderTraining(
    "regalloc-priority-model", cl::Hidden,
    cl::desc("The model being trained for register allocation priority"));

#endif // #ifdef LLVM_HAVE_TFLITE

namespace vm::core {

static const std::vector<int64_t> PerLiveRangeShape{1};

#define RA_PRIORITY_FEATURES_LIST(M)                                           \
  M(int64_t, li_size, PerLiveRangeShape, "size")                               \
  M(int64_t, stage, PerLiveRangeShape, "stage")                                \
  M(float, weight, PerLiveRangeShape, "weight")

#define DecisionName "priority"
static const TensorSpec DecisionSpec =
    TensorSpec::createSpec<float>(DecisionName, {1});


// Named features index.
enum FeatureIDs {
#define _FEATURE_IDX(_, name, __, ___) name,
  RA_PRIORITY_FEATURES_LIST(_FEATURE_IDX)
#undef _FEATURE_IDX
      FeatureCount
};

class MLPriorityAdvisor : public RegAllocPriorityAdvisor {
public:
  MLPriorityAdvisor(const MachineFunction &MF, const RAGreedy &RA,
                    SlotIndexes *const Indexes, MLModelRunner *Runner);

protected:
  const RegAllocPriorityAdvisor &getDefaultAdvisor() const {
    return static_cast<const RegAllocPriorityAdvisor &>(DefaultAdvisor);
  }

  // The assumption is that if the Runner could not be constructed, we emit-ed
  // error, and we shouldn't be asking for it here.
  const MLModelRunner &getRunner() const { return *Runner; }
  float getPriorityImpl(const LiveInterval &LI) const;
  unsigned getPriority(const LiveInterval &LI) const override;

private:
  const DefaultPriorityAdvisor DefaultAdvisor;
  MLModelRunner *const Runner;
};

#define _DECL_FEATURES(type, name, shape, _)                                   \
  TensorSpec::createSpec<type>(#name, shape),

static const std::vector<TensorSpec> InputFeatures{
    {RA_PRIORITY_FEATURES_LIST(_DECL_FEATURES)},
};
#undef _DECL_FEATURES

// ===================================
// Release (AOT) - specifics
// ===================================
class ReleaseModePriorityAdvisorProvider final
    : public RegAllocPriorityAdvisorProvider {
public:
  ReleaseModePriorityAdvisorProvider()
      : RegAllocPriorityAdvisorProvider(AdvisorMode::Release) {}
  std::unique_ptr<RegAllocPriorityAdvisor>
  getAdvisor(const MachineFunction &MF, const RAGreedy &RA,
             SlotIndexes &SI) override {
    if (!Runner) {
      if (InteractiveChannelBaseName.empty())
        Runner = std::make_unique<ReleaseModeModelRunner<CompiledModelType>>(
            MF.getFunction().getContext(), InputFeatures, DecisionName);
      else
        Runner = std::make_unique<InteractiveModelRunner>(
            MF.getFunction().getContext(), InputFeatures, DecisionSpec,
            InteractiveChannelBaseName + ".out",
            InteractiveChannelBaseName + ".in");
    }
    return std::make_unique<MLPriorityAdvisor>(MF, RA, &SI, Runner.get());
  }

private:
  std::unique_ptr<MLModelRunner> Runner;
};

class ReleaseModePriorityAdvisorAnalysisLegacy final
    : public RegAllocPriorityAdvisorAnalysisLegacy {
public:
  ReleaseModePriorityAdvisorAnalysisLegacy()
      : RegAllocPriorityAdvisorAnalysisLegacy(AdvisorMode::Release) {}
  // support for isa<> and dyn_cast.
  static bool classof(const RegAllocPriorityAdvisorAnalysisLegacy *R) {
    return R->getAdvisorMode() == AdvisorMode::Release;
  }

private:
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
    AU.addRequired<SlotIndexesWrapperPass>();
    RegAllocPriorityAdvisorAnalysisLegacy::getAnalysisUsage(AU);
  }

  bool doInitialization(Module &M) override {
    Provider = std::make_unique<ReleaseModePriorityAdvisorProvider>();
    return false;
  }
};

// ===================================
// Development mode-specifics
// ===================================
//
// Features we log
#ifdef LLVM_HAVE_TFLITE
static const TensorSpec Reward = TensorSpec::createSpec<float>("reward", {1});

#define _DECL_TRAIN_FEATURES(type, name, shape, _)                             \
  TensorSpec::createSpec<type>(std::string("action_") + #name, shape),

static const std::vector<TensorSpec> TrainingInputFeatures{
    {RA_PRIORITY_FEATURES_LIST(_DECL_TRAIN_FEATURES)
         TensorSpec::createSpec<float>("action_discount", {1}),
     TensorSpec::createSpec<int32_t>("action_step_type", {1}),
     TensorSpec::createSpec<float>("action_reward", {1})}};
#undef _DECL_TRAIN_FEATURES

class DevelopmentModePriorityAdvisor : public MLPriorityAdvisor {
public:
  DevelopmentModePriorityAdvisor(const MachineFunction &MF, const RAGreedy &RA,
                                 SlotIndexes *const Indexes,
                                 MLModelRunner *Runner, Logger *Log)
      : MLPriorityAdvisor(MF, RA, Indexes, Runner), Log(Log) {}

private:
  unsigned getPriority(const LiveInterval &LI) const override;
  Logger *const Log;
};

class DevelopmentModePriorityAdvisorProvider final
    : public RegAllocPriorityAdvisorProvider {

public:
  // Save all the logs (when requested).
  DevelopmentModePriorityAdvisorProvider(LLVMContext &Ctx)
      : RegAllocPriorityAdvisorProvider(AdvisorMode::Development) {
    if (ModelUnderTraining.empty() && TrainingLog.empty()) {
      Ctx.emitError("Regalloc development mode should be requested with at "
                    "least logging enabled and/or a training model");
      return;
    }
    if (ModelUnderTraining.empty())
      Runner = std::make_unique<NoInferenceModelRunner>(Ctx, InputFeatures);
    else
      Runner = ModelUnderTrainingRunner::createAndEnsureValid(
          Ctx, ModelUnderTraining, DecisionName, TrainingInputFeatures);
    if (!Runner) {
      Ctx.emitError("Regalloc: could not set up the model runner");
      return;
    }
    if (TrainingLog.empty())
      return;
    std::error_code EC;
    auto OS = std::make_unique<raw_fd_ostream>(TrainingLog, EC);
    if (EC) {
      Ctx.emitError(EC.message() + ":" + TrainingLog);
      return;
    }
    std::vector<TensorSpec> LFS = InputFeatures;
    if (auto *MUTR = dyn_cast<ModelUnderTrainingRunner>(Runner.get()))
      append_range(LFS, MUTR->extraOutputsForLoggingSpecs());
    // We always log the output; in particular, if we're not evaluating, we
    // don't have an output spec json file. That's why we handle the
    // 'normal' output separately.
    LFS.push_back(DecisionSpec);

    Log = std::make_unique<Logger>(std::move(OS), LFS, Reward,
                                   /*IncludeReward*/ true);
  }

  void logRewardIfNeeded(const MachineFunction &MF,
                         toolchain::function_ref<float()> GetReward) override {
    if (!Log || !Log->hasAnyObservationForContext(MF.getName()))
      return;
    // The function pass manager would run all the function passes for a
    // function, so we assume the last context belongs to this function. If
    // this invariant ever changes, we can implement at that time switching
    // contexts. At this point, it'd be an error
    if (Log->currentContext() != MF.getName()) {
      MF.getFunction().getContext().emitError(
          "The training log context shouldn't have had changed.");
    }
    if (Log->hasObservationInProgress())
      Log->logReward<float>(GetReward());
  }

  std::unique_ptr<RegAllocPriorityAdvisor>
  getAdvisor(const MachineFunction &MF, const RAGreedy &RA,
             SlotIndexes &SI) override {
    if (!Runner)
      return nullptr;
    if (Log) {
      Log->switchContext(MF.getName());
    }
    return std::make_unique<DevelopmentModePriorityAdvisor>(
        MF, RA, &SI, Runner.get(), Log.get());
  }

  std::unique_ptr<MLModelRunner> Runner;
  std::unique_ptr<Logger> Log;
};

class DevelopmentModePriorityAdvisorAnalysisLegacy final
    : public RegAllocPriorityAdvisorAnalysisLegacy {
public:
  DevelopmentModePriorityAdvisorAnalysisLegacy()
      : RegAllocPriorityAdvisorAnalysisLegacy(AdvisorMode::Development) {}

  // support for isa<> and dyn_cast.
  static bool classof(const RegAllocPriorityAdvisorAnalysisLegacy *R) {
    return R->getAdvisorMode() == AdvisorMode::Development;
  }

  void logRewardIfNeeded(const MachineFunction &MF,
                         toolchain::function_ref<float()> GetReward) override {
    Provider->logRewardIfNeeded(MF, GetReward);
  }

private:
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
    AU.addRequired<SlotIndexesWrapperPass>();
    RegAllocPriorityAdvisorAnalysisLegacy::getAnalysisUsage(AU);
  }

  // Save all the logs (when requested).
  bool doInitialization(Module &M) override {
    Provider = std::make_unique<DevelopmentModePriorityAdvisorProvider>(
        M.getContext());
    return false;
    ;
  }
};
#endif //#ifdef LLVM_HAVE_TFLITE

} // namespace vm::core

RegAllocPriorityAdvisorAnalysisLegacy *
toolchain::createReleaseModePriorityAdvisorAnalysis() {
  return toolchain::isEmbeddedModelEvaluatorValid<CompiledModelType>() ||
                 !InteractiveChannelBaseName.empty()
             ? new ReleaseModePriorityAdvisorAnalysisLegacy()
             : nullptr;
}

MLPriorityAdvisor::MLPriorityAdvisor(const MachineFunction &MF,
                                     const RAGreedy &RA,
                                     SlotIndexes *const Indexes,
                                     MLModelRunner *Runner)
    : RegAllocPriorityAdvisor(MF, RA, Indexes), DefaultAdvisor(MF, RA, Indexes),
      Runner(std::move(Runner)) {
  assert(this->Runner);
  Runner->switchContext(MF.getName());
}

float MLPriorityAdvisor::getPriorityImpl(const LiveInterval &LI) const {
  const unsigned Size = LI.getSize();
  LiveRangeStage Stage = RA.getExtraInfo().getStage(LI);

  *Runner->getTensor<int64_t>(0) = static_cast<int64_t>(Size);
  *Runner->getTensor<int64_t>(1) = static_cast<int64_t>(Stage);
  *Runner->getTensor<float>(2) = static_cast<float>(LI.weight());

  return Runner->evaluate<float>();
}

unsigned MLPriorityAdvisor::getPriority(const LiveInterval &LI) const {
  return static_cast<unsigned>(getPriorityImpl(LI));
}

#ifdef LLVM_HAVE_TFLITE
RegAllocPriorityAdvisorAnalysisLegacy *
toolchain::createDevelopmentModePriorityAdvisorAnalysis() {
  return new DevelopmentModePriorityAdvisorAnalysisLegacy();
}

unsigned
DevelopmentModePriorityAdvisor::getPriority(const LiveInterval &LI) const {
  double Prio = 0;

  if (isa<ModelUnderTrainingRunner>(getRunner())) {
    Prio = MLPriorityAdvisor::getPriorityImpl(LI);
  } else {
    Prio = getDefaultAdvisor().getPriority(LI);
  }

  if (TrainingLog.empty())
    return Prio;

  // TODO(mtrofin): when we support optional rewards, this can go away. In the
  // meantime, we log the "pretend" reward (0) for the previous observation
  // before starting a new one.
  if (Log->hasObservationInProgress())
    Log->logReward<float>(0.0);

  Log->startObservation();
  size_t CurrentFeature = 0;
  for (; CurrentFeature < InputFeatures.size(); ++CurrentFeature) {
    Log->logTensorValue(CurrentFeature,
                        reinterpret_cast<const char *>(
                            getRunner().getTensorUntyped(CurrentFeature)));
  }

  if (auto *MUTR = dyn_cast<ModelUnderTrainingRunner>(&getRunner())) {
    for (size_t I = 0; I < MUTR->extraOutputsForLoggingSpecs().size();
         ++I, ++CurrentFeature)
      Log->logTensorValue(
          CurrentFeature,
          reinterpret_cast<const char *>(MUTR->getUntypedExtraOutputValue(I)));
  }

  float Ret = static_cast<float>(Prio);
  Log->logTensorValue(CurrentFeature, reinterpret_cast<const char *>(&Ret));
  Log->endObservation();

  return static_cast<unsigned>(Prio);
}

RegAllocPriorityAdvisorProvider *
toolchain::createDevelopmentModePriorityAdvisorProvider(LLVMContext &Ctx) {
  return new DevelopmentModePriorityAdvisorProvider(Ctx);
}

#endif // #ifdef LLVM_HAVE_TFLITE

RegAllocPriorityAdvisorProvider *
toolchain::createReleaseModePriorityAdvisorProvider() {
  return new ReleaseModePriorityAdvisorProvider();
}
