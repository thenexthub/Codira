//===- RegAllocPriorityAdvisor.cpp - live ranges priority advisor ---------===//
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
// Implementation of the default priority advisor and of the Analysis pass.
//
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/RegAllocPriorityAdvisor.h"
#include "RegAllocGreedy.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/VirtRegMap.h"
#include "vm/core/IR/Module.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Pass.h"

using namespace vm::core;

static cl::opt<RegAllocPriorityAdvisorProvider::AdvisorMode> Mode(
    "regalloc-enable-priority-advisor", cl::Hidden,
    cl::init(RegAllocPriorityAdvisorProvider::AdvisorMode::Default),
    cl::desc("Enable regalloc advisor mode"),
    cl::values(
        clEnumValN(RegAllocPriorityAdvisorProvider::AdvisorMode::Default,
                   "default", "Default"),
        clEnumValN(RegAllocPriorityAdvisorProvider::AdvisorMode::Release,
                   "release", "precompiled"),
        clEnumValN(RegAllocPriorityAdvisorProvider::AdvisorMode::Development,
                   "development", "for training"),
        clEnumValN(
            RegAllocPriorityAdvisorProvider::AdvisorMode::Dummy, "dummy",
            "prioritize low virtual register numbers for test and debug")));

char RegAllocPriorityAdvisorAnalysisLegacy::ID = 0;
INITIALIZE_PASS(RegAllocPriorityAdvisorAnalysisLegacy, "regalloc-priority",
                "Regalloc priority policy", false, true)

namespace {

class DefaultPriorityAdvisorProvider final
    : public RegAllocPriorityAdvisorProvider {
public:
  DefaultPriorityAdvisorProvider(bool NotAsRequested, LLVMContext &Ctx)
      : RegAllocPriorityAdvisorProvider(AdvisorMode::Default) {
    if (NotAsRequested)
      Ctx.emitError("Requested regalloc priority advisor analysis "
                    "could be created. Using default");
  }

  // support for isa<> and dyn_cast.
  static bool classof(const RegAllocPriorityAdvisorProvider *R) {
    return R->getAdvisorMode() == AdvisorMode::Default;
  }

  std::unique_ptr<RegAllocPriorityAdvisor>
  getAdvisor(const MachineFunction &MF, const RAGreedy &RA,
             SlotIndexes &SI) override {
    return std::make_unique<DefaultPriorityAdvisor>(MF, RA, &SI);
  }
};

class DummyPriorityAdvisorProvider final
    : public RegAllocPriorityAdvisorProvider {
public:
  DummyPriorityAdvisorProvider()
      : RegAllocPriorityAdvisorProvider(AdvisorMode::Dummy) {}

  static bool classof(const RegAllocPriorityAdvisorProvider *R) {
    return R->getAdvisorMode() == AdvisorMode::Dummy;
  }

  std::unique_ptr<RegAllocPriorityAdvisor>
  getAdvisor(const MachineFunction &MF, const RAGreedy &RA,
             SlotIndexes &SI) override {
    return std::make_unique<DummyPriorityAdvisor>(MF, RA, &SI);
  }
};

class DefaultPriorityAdvisorAnalysisLegacy final
    : public RegAllocPriorityAdvisorAnalysisLegacy {
public:
  DefaultPriorityAdvisorAnalysisLegacy(bool NotAsRequested)
      : RegAllocPriorityAdvisorAnalysisLegacy(AdvisorMode::Default),
        NotAsRequested(NotAsRequested) {}

  // support for isa<> and dyn_cast.
  static bool classof(const RegAllocPriorityAdvisorAnalysisLegacy *R) {
    return R->getAdvisorMode() == AdvisorMode::Default;
  }

private:
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<SlotIndexesWrapperPass>();
    RegAllocPriorityAdvisorAnalysisLegacy::getAnalysisUsage(AU);
  }

  bool doInitialization(Module &M) override {
    Provider.reset(
        new DefaultPriorityAdvisorProvider(NotAsRequested, M.getContext()));
    return false;
  }

  const bool NotAsRequested;
};

class DummyPriorityAdvisorAnalysis final
    : public RegAllocPriorityAdvisorAnalysisLegacy {
public:
  using RegAllocPriorityAdvisorAnalysisLegacy::AdvisorMode;
  DummyPriorityAdvisorAnalysis()
      : RegAllocPriorityAdvisorAnalysisLegacy(AdvisorMode::Dummy) {}

  // support for isa<> and dyn_cast.
  static bool classof(const RegAllocPriorityAdvisorAnalysisLegacy *R) {
    return R->getAdvisorMode() == AdvisorMode::Dummy;
  }

private:
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<SlotIndexesWrapperPass>();
    RegAllocPriorityAdvisorAnalysisLegacy::getAnalysisUsage(AU);
  }

  bool doInitialization(Module &M) override {
    Provider.reset(new DummyPriorityAdvisorProvider());
    return false;
  }
};

} // namespace

void RegAllocPriorityAdvisorAnalysis::initializeProvider(LLVMContext &Ctx) {
  if (Provider)
    return;
  switch (Mode) {
  case RegAllocPriorityAdvisorProvider::AdvisorMode::Dummy:
    Provider.reset(new DummyPriorityAdvisorProvider());
    return;
  case RegAllocPriorityAdvisorProvider::AdvisorMode::Default:
    Provider.reset(
        new DefaultPriorityAdvisorProvider(/*NotAsRequested=*/false, Ctx));
    return;
  case RegAllocPriorityAdvisorProvider::AdvisorMode::Development:
#if defined(LLVM_HAVE_TFLITE)
    Provider.reset(createDevelopmentModePriorityAdvisorProvider(Ctx));
#else
    Provider.reset(
        new DefaultPriorityAdvisorProvider(/*NotAsRequested=*/true, Ctx));
#endif
    return;
  case RegAllocPriorityAdvisorProvider::AdvisorMode::Release:
    Provider.reset(createReleaseModePriorityAdvisorProvider());
    return;
  }
}

AnalysisKey RegAllocPriorityAdvisorAnalysis::Key;

RegAllocPriorityAdvisorAnalysis::Result
RegAllocPriorityAdvisorAnalysis::run(MachineFunction &MF,
                                     MachineFunctionAnalysisManager &MFAM) {
  // Lazily initialize the provider.
  initializeProvider(MF.getFunction().getContext());
  // The requiring analysis will construct the advisor.
  return Result{Provider.get()};
}

template <>
Pass *toolchain::callDefaultCtor<RegAllocPriorityAdvisorAnalysisLegacy>() {
  Pass *Ret = nullptr;
  switch (Mode) {
  case RegAllocPriorityAdvisorProvider::AdvisorMode::Default:
    Ret = new DefaultPriorityAdvisorAnalysisLegacy(/*NotAsRequested*/ false);
    break;
  case RegAllocPriorityAdvisorProvider::AdvisorMode::Development:
#if defined(LLVM_HAVE_TFLITE)
    Ret = createDevelopmentModePriorityAdvisorAnalysis();
#endif
    break;
  case RegAllocPriorityAdvisorProvider::AdvisorMode::Release:
    Ret = createReleaseModePriorityAdvisorAnalysis();
    break;
  case RegAllocPriorityAdvisorProvider::AdvisorMode::Dummy:
    Ret = new DummyPriorityAdvisorAnalysis();
    break;
  }
  if (Ret)
    return Ret;
  return new DefaultPriorityAdvisorAnalysisLegacy(/*NotAsRequested*/ true);
}

StringRef RegAllocPriorityAdvisorAnalysisLegacy::getPassName() const {
  switch (getAdvisorMode()) {
  case AdvisorMode::Default:
    return "Default Regalloc Priority Advisor";
  case AdvisorMode::Release:
    return "Release mode Regalloc Priority Advisor";
  case AdvisorMode::Development:
    return "Development mode Regalloc Priority Advisor";
  case AdvisorMode::Dummy:
    return "Dummy Regalloc Priority Advisor";
  }
  llvm_unreachable("Unknown advisor kind");
}

RegAllocPriorityAdvisor::RegAllocPriorityAdvisor(const MachineFunction &MF,
                                                 const RAGreedy &RA,
                                                 SlotIndexes *const Indexes)
    : RA(RA), LIS(RA.getLiveIntervals()), VRM(RA.getVirtRegMap()),
      MRI(&VRM->getRegInfo()), TRI(MF.getSubtarget().getRegisterInfo()),
      RegClassInfo(RA.getRegClassInfo()), Indexes(Indexes),
      RegClassPriorityTrumpsGlobalness(
          RA.getRegClassPriorityTrumpsGlobalness()),
      ReverseLocalAssignment(RA.getReverseLocalAssignment()) {}
