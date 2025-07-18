//===--- CallerAnalysis.cpp - Determine callsites to a function  ----------===//
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

#define DEBUG_TYPE "sil-caller-analysis"
#include "language/SILOptimizer/Analysis/CallerAnalysis.h"
#include "language/Basic/Assertions.h"
#include "language/SIL/InstructionUtils.h"
#include "language/SIL/SILModule.h"
#include "language/SIL/SILVisitor.h"
#include "language/SILOptimizer/Utils/InstOptUtils.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/YAMLTraits.h"

using namespace language;

namespace {
using FunctionInfo = CallerAnalysis::FunctionInfo;
} // end anonymous namespace

//===----------------------------------------------------------------------===//
//                        CallerAnalysis::FunctionInfo
//===----------------------------------------------------------------------===//

CallerAnalysis::FunctionInfo::FunctionInfo(SILFunction *f)
    : callerStates(),
      // TODO: Make this more aggressive by considering
      // final/visibility/etc.
      mayHaveIndirectCallers(
          f->getDynamicallyReplacedFunction() ||
          f->getReferencedAdHocRequirementWitnessFunction() ||
          canBeCalledIndirectly(f->getRepresentation())),
      mayHaveExternalCallers(f->isPossiblyUsedExternally() ||
                             f->isAvailableExternally()) {}

//===----------------------------------------------------------------------===//
//                   CallerAnalysis::ApplySiteFinderVisitor
//===----------------------------------------------------------------------===//

struct CallerAnalysis::ApplySiteFinderVisitor
    : SILInstructionVisitor<ApplySiteFinderVisitor, bool> {
  CallerAnalysis *analysis;
  SILFunction *callerFn;

#ifndef NDEBUG
  SmallPtrSet<SILInstruction *, 8> visitedCallSites;
  toolchain::SmallSetVector<SILInstruction *, 8> callSitesThatMustBeVisited;
#endif

  ApplySiteFinderVisitor(CallerAnalysis *analysis, SILFunction *callerFn)
      : analysis(analysis), callerFn(callerFn) {}
  ~ApplySiteFinderVisitor();

  bool visitSILInstruction(SILInstruction *) { return false; }
  bool visitFunctionRefInst(FunctionRefInst *fri) {
    return visitFunctionRefBaseInst(fri);
  }
  bool visitDynamicFunctionRefInst(DynamicFunctionRefInst *fri) {
    return visitFunctionRefBaseInst(fri);
  }
  bool
  visitPreviousDynamicFunctionRefInst(PreviousDynamicFunctionRefInst *fri) {
    return visitFunctionRefBaseInst(fri);
  }
  bool visitFunctionRefBaseInst(FunctionRefBaseInst *fri);

  void process();
  void processApplySites(ArrayRef<ApplySite> applySites);
  void processApplySites(ArrayRef<FullApplySite> applySites);
  void checkCallSiteInvariants(SILInstruction &i);
};

void CallerAnalysis::ApplySiteFinderVisitor::processApplySites(
    ArrayRef<ApplySite> applySites) {
  // For now we just verify our invariants. If we need to update other
  // non-NDEBUG state related to apply sites, this should be updated.
#ifndef NDEBUG
  for (auto applySite : applySites) {
    visitedCallSites.insert(applySite.getInstruction());
    callSitesThatMustBeVisited.remove(applySite.getInstruction());
  }
#endif
}

void CallerAnalysis::ApplySiteFinderVisitor::processApplySites(
    ArrayRef<FullApplySite> applySites) {
  // For now we just verify our invariants. If we need to update other
  // non-NDEBUG state related to apply sites, this should be updated.
#ifndef NDEBUG
  for (auto applySite : applySites) {
    visitedCallSites.insert(applySite.getInstruction());
    callSitesThatMustBeVisited.remove(applySite.getInstruction());
  }
#endif
}

CallerAnalysis::ApplySiteFinderVisitor::~ApplySiteFinderVisitor() {
#ifndef NDEBUG
  if (callSitesThatMustBeVisited.empty())
    return;
  toolchain::errs() << "Found unhandled call sites!\n";
  while (callSitesThatMustBeVisited.size()) {
    auto *i = callSitesThatMustBeVisited.pop_back_val();
    toolchain::errs() << "Inst: " << *i;
  }
  assert(false && "Unhandled call site?!");
#endif
}

bool CallerAnalysis::ApplySiteFinderVisitor::visitFunctionRefBaseInst(
    FunctionRefBaseInst *fri) {
  auto optResult = findLocalApplySites(fri);
  auto *calleeFn = fri->getInitiallyReferencedFunction();

  // First make an edge from our callerInfo to our calleeState for invalidation
  // purposes.
  analysis->getOrInsertFunctionInfo(callerFn).calleeStates.insert(calleeFn);

  // Then grab our callee state and update it with state for this caller.
  auto iter = analysis->getOrInsertFunctionInfo(calleeFn).callerStates.
                  insert({callerFn, {}});
  // If we succeeded in inserting a new value, put in an optimistic
  // value for escaping.
  if (iter.second) {
    iter.first->second.isDirectCallerSetComplete = true;
  }

  // Then check if we found any information at all from our result. If we
  // didn't, then mark this as escaping and bail.
  if (!optResult.has_value()) {
    iter.first->second.isDirectCallerSetComplete = false;
    return true;
  }

  auto &result = optResult.value();

  // Ok. We know that we have some sort of information. Merge that information
  // into our information.
  iter.first->second.isDirectCallerSetComplete &= !result.isEscaping();

  if (result.fullApplySites.size()) {
    iter.first->second.hasFullApply = true;
    processApplySites(toolchain::ArrayRef(result.fullApplySites));
  }

  if (result.partialApplySites.size()) {
    auto optMin = iter.first->second.getNumPartiallyAppliedArguments();
    unsigned min = optMin.value_or(UINT_MAX);
    for (ApplySite partialSite : result.partialApplySites) {
      min = std::min(min, partialSite.getNumArguments());
    }
    iter.first->second.setNumPartiallyAppliedArguments(min);
    processApplySites(result.partialApplySites);
  }

  return true;
}

void CallerAnalysis::ApplySiteFinderVisitor::checkCallSiteInvariants(
    SILInstruction &i) {
#ifndef NDEBUG
  if (auto apply = FullApplySite::isa(&i)) {
    if (apply.getCalleeFunction() && !visitedCallSites.count(&i)) {
      callSitesThatMustBeVisited.insert(&i);
    }
    return;
  }

  // Make sure that we are in sync with looking for partial apply callees.
  if (auto *pai = dyn_cast<PartialApplyInst>(&i)) {
    if (pai->getCalleeFunction() && !visitedCallSites.count(&i)) {
      callSitesThatMustBeVisited.insert(pai);
    }
    return;
  }
#endif
}

void CallerAnalysis::ApplySiteFinderVisitor::process() {
  for (auto &block : *callerFn) {
    for (auto &i : block) {
#ifndef NDEBUG
      // If this is a call site that we visited as part of seeing a different
      // function_ref, skip it. We know that it has been processed correctly.
      //
      // NOTE: This is only used in NDEBUG builds since we only use this as part
      // of the verification that we can find all callees going forward along
      // def-use edges that FullApplySite is able to track backwards along
      // def-use edges.
      if (visitedCallSites.count(&i))
        continue;
#endif

      // Try to find the apply sites for this specific FRI.
      if (visit(&i))
        continue;

#ifndef NDEBUG
      checkCallSiteInvariants(i);
#endif
    }
  }
}

//===----------------------------------------------------------------------===//
//                               CallerAnalysis
//===----------------------------------------------------------------------===//

// NOTE: This is only meant to be used by external users of CallerAnalysis since
// it recomputes our invalidated results. For internal uses, please instead use
// getOrInsertFunctionInfo or unsafeGetFunctionInfo.
const FunctionInfo &CallerAnalysis::getFunctionInfo(SILFunction *f) const {
  // Recompute every function in the invalidated function list and empty the
  // list.
  auto &self = const_cast<CallerAnalysis &>(*this);
  if (funcInfos.find(f) == funcInfos.end()) {
    (void)self.getOrInsertFunctionInfo(f);
    self.recomputeFunctionList.insert(f);
  }
  self.processRecomputeFunctionList();
  return self.unsafeGetFunctionInfo(f);
}

// Private only version of this function for mutable callers that tries to
// initialize a new f.
FunctionInfo &CallerAnalysis::getOrInsertFunctionInfo(SILFunction *f) {
  TOOLCHAIN_DEBUG(toolchain::dbgs() << "CallerAnalysis: Creating caller info for: "
                          << f->getName() << "\n");
  return funcInfos.try_emplace(f, f).first->second;
}

FunctionInfo &CallerAnalysis::unsafeGetFunctionInfo(SILFunction *f) {
  auto r = funcInfos.find(f);
  assert(r != funcInfos.end() && "Function does not have functionInfo!");
  return r->second;
}

const FunctionInfo &
CallerAnalysis::unsafeGetFunctionInfo(SILFunction *f) const {
  auto r = funcInfos.find(f);
  assert(r != funcInfos.end() && "Function does not have functionInfo!");
  return r->second;
}

CallerAnalysis::CallerAnalysis(SILModule *m)
    : SILAnalysis(SILAnalysisKind::Caller), mod(*m) {
  // When we start we create a function info for each f and add all f to the
  // recompute function list.
  for (auto &f : mod) {
    getOrInsertFunctionInfo(&f);
    recomputeFunctionList.insert(&f);
  }
}

void CallerAnalysis::processFunctionCallSites(SILFunction *callerFn) {
  ApplySiteFinderVisitor visitor(this, callerFn);
  visitor.process();
}

void CallerAnalysis::invalidateAllInfo(SILFunction *f, FunctionInfo &fInfo) {
  // Then we first eliminate any callees that we point at.
  invalidateKnownCallees(f, fInfo);

  // And then eliminate any caller edges that we need.
  while (fInfo.callerStates.size()) {
    auto back = fInfo.callerStates.back();
    SILFunction *caller = back.first;
    auto &callerInfo = unsafeGetFunctionInfo(caller);
    TOOLCHAIN_DEBUG(toolchain::dbgs()
               << "    caller-backedge: " << caller->getName() << "\n");
    bool foundF = callerInfo.calleeStates.remove(f);
    (void)foundF;
    assert(foundF && "Bad caller edge pointing at f?");
    fInfo.callerStates.pop_back();
  }
}

void CallerAnalysis::invalidateKnownCallees(SILFunction *caller,
                                            FunctionInfo &callerInfo) {
  TOOLCHAIN_DEBUG(toolchain::dbgs() << "Invalidating caller: " << caller->getName()
                          << "\n");
  while (callerInfo.calleeStates.size()) {
    auto *callee = callerInfo.calleeStates.pop_back_val();
    FunctionInfo &calleeInfo = unsafeGetFunctionInfo(callee);
    TOOLCHAIN_DEBUG(toolchain::dbgs() << "    callee: " << callee->getName() << "\n");
    assert(calleeInfo.callerStates.count(caller) &&
           "Referenced callee is not fully/partially applied in the caller?!");

    // Then remove the caller from this specific callee's info struct
    // and to be conservative mark the callee as potentially having an
    // escaping use that we do not understand.
    calleeInfo.callerStates.erase(caller);
  }
}

void CallerAnalysis::invalidateKnownCallees(SILFunction *caller) {
  auto iter = funcInfos.find(caller);
  if (iter != funcInfos.end()) {
    // Look up the callees that our caller refers to and invalidate any
    // values that point back at the caller.
    invalidateKnownCallees(caller, iter->second);
  }
}

void CallerAnalysis::verify(SILFunction *caller) const {
#ifndef NDEBUG
  const FunctionInfo &callerInfo = unsafeGetFunctionInfo(caller);
  verify(caller, callerInfo);
#endif
}

void CallerAnalysis::verify(SILFunction *function,
                            const FunctionInfo &functionInfo) const {
#ifndef NDEBUG
  TOOLCHAIN_DEBUG(toolchain::dbgs() << "Validating function: " << function->getName()
                          << "\n");
  for (auto *callee : functionInfo.calleeStates) {
    TOOLCHAIN_DEBUG(toolchain::dbgs() << "    callee: " << callee->getName() << "\n");
    const FunctionInfo &calleeInfo = unsafeGetFunctionInfo(callee);
    assert(calleeInfo.callerStates.count(function) &&
           "Referenced callee is not fully/partially applied in the caller");
  }

  // Make sure all caller edges are valid.
  for (auto callerPair : functionInfo.callerStates) {
    auto *caller = callerPair.first;
    TOOLCHAIN_DEBUG(toolchain::dbgs() << "    caller: " << caller->getName() << "\n");
    const FunctionInfo &callerInfo = unsafeGetFunctionInfo(caller);
    assert(callerInfo.calleeStates.count(function) &&
           "Referencing caller does not have a callee edge for function");
  }
#endif
}

void CallerAnalysis::verify() const {
#ifndef NDEBUG
  std::vector<SILFunction *> seenFunctions;
  for (auto &fn : mod) {
    bool found = funcInfos.count(&fn);
    if (!found) {
      toolchain::errs() << "Missing notification for added function: '"
                   << fn.getName() << "'\n";
      toolchain_unreachable("standard error assertion");
    }
    seenFunctions.push_back(&fn);
  }

  sortUnique(seenFunctions);
  for (auto &pair : funcInfos) {
    bool found = std::binary_search(seenFunctions.begin(), seenFunctions.end(),
                                    pair.first);
    if (!found) {
      toolchain::errs() << "Notification not sent for deleted function: '"
                   << pair.first->getName() << "'.";
      toolchain_unreachable("standard error assertion");
    }
    verify(pair.first, pair.second);
  }
#endif
}

void CallerAnalysis::invalidate() {
  for (auto &f : mod) {
    // Since we are going over all functions in the module
    // invalidateKnownCallees should be sufficient.
    invalidateKnownCallees(&f);
    // We do not need to clear recompute function list since we know that it can
    // at most contain a subset of the functions in the module so the SetVector
    // will unique for us.
    recomputeFunctionList.insert(&f);
  }
}

void CallerAnalysis::notifyWillDeleteFunction(SILFunction *f) {
  auto iter = funcInfos.find(f);
  if (iter == funcInfos.end())
    return;
    
  invalidateAllInfo(f, iter->second);
  recomputeFunctionList.remove(f);
  // Now that we have invalidated all references to the function, delete it.
  funcInfos.erase(iter);
}

//===----------------------------------------------------------------------===//
//                         CallerAnalysis YAML Dumper
//===----------------------------------------------------------------------===//

namespace {

using toolchain::yaml::IO;
using toolchain::yaml::MappingTraits;
using toolchain::yaml::Output;
using toolchain::yaml::ScalarEnumerationTraits;
using toolchain::yaml::SequenceTraits;

/// A special struct that marshals call graph state into a form that
/// is easy for toolchain's yaml i/o to dump. Its structure is meant to
/// correspond to how the data should be shown by the printer, so
/// naturally it is slightly redundant.
struct YAMLCallGraphNode {
  StringRef calleeName;
  bool hasCaller;
  unsigned minPartialAppliedArgs;
  bool hasOnlyCompleteDirectCallerSets;
  bool hasAllCallers;
  std::vector<StringRef> partialAppliers;
  std::vector<StringRef> fullAppliers;

  YAMLCallGraphNode() = delete;
  ~YAMLCallGraphNode() = default;

  YAMLCallGraphNode(const YAMLCallGraphNode &) = delete;
  YAMLCallGraphNode(YAMLCallGraphNode &&) = delete;
  YAMLCallGraphNode &operator=(const YAMLCallGraphNode &) = delete;
  YAMLCallGraphNode &operator=(YAMLCallGraphNode &&) = delete;

  YAMLCallGraphNode(StringRef calleeName, bool hasCaller,
                    unsigned minPartialAppliedArgs,
                    bool hasOnlyCompleteDirectCallerSets, bool hasAllCallers,
                    std::vector<StringRef> &&partialAppliers,
                    std::vector<StringRef> &&fullAppliers)
      : calleeName(calleeName), hasCaller(hasCaller),
        minPartialAppliedArgs(minPartialAppliedArgs),
        hasOnlyCompleteDirectCallerSets(hasOnlyCompleteDirectCallerSets),
        hasAllCallers(hasAllCallers),
        partialAppliers(std::move(partialAppliers)),
        fullAppliers(std::move(fullAppliers)) {}
};

} // end anonymous namespace

namespace toolchain {
namespace yaml {

template <> struct MappingTraits<YAMLCallGraphNode> {
  static void mapping(IO &io, YAMLCallGraphNode &fn) {
    io.mapRequired("calleeName", fn.calleeName);
    io.mapRequired("hasCaller", fn.hasCaller);
    io.mapRequired("minPartialAppliedArgs", fn.minPartialAppliedArgs);
    io.mapRequired("hasOnlyCompleteDirectCallerSets",
                   fn.hasOnlyCompleteDirectCallerSets);
    io.mapRequired("hasAllCallers", fn.hasAllCallers);
    io.mapRequired("partialAppliers", fn.partialAppliers);
    io.mapRequired("fullAppliers", fn.fullAppliers);
  }
};

} // namespace yaml
} // namespace toolchain

void CallerAnalysis::dump() const { print(toolchain::errs()); }

void CallerAnalysis::print(const char *filePath) const {
  using namespace toolchain::sys;
  std::error_code error;
  toolchain::raw_fd_ostream fileOutputStream(filePath, error, fs::OF_Text);
  if (error) {
    toolchain::errs() << "Failed to open path \"" << filePath << "\" for writing.!";
    toolchain_unreachable("default error handler");
  }
  print(fileOutputStream);
}

void CallerAnalysis::print(toolchain::raw_ostream &os) const {
  toolchain::yaml::Output yout(os);

  // NOTE: We purposely do not iterate over our internal state here to ensure
  // that we dump for all functions and that we dump the state we have stored
  // with the functions in module order.
  for (auto &f : mod) {
    const auto &fi = getFunctionInfo(&f);

    std::vector<StringRef> partialAppliers;
    std::vector<StringRef> fullAppliers;
    for (auto &apply : fi.getAllReferencingCallers()) {
      if (apply.second.hasFullApply) {
        fullAppliers.push_back(apply.first->getName());
      }
      if (apply.second.getNumPartiallyAppliedArguments().has_value()) {
        partialAppliers.push_back(apply.first->getName());
      }
    }

    YAMLCallGraphNode node(
        f.getName(), fi.hasDirectCaller(), fi.getMinPartialAppliedArgs(),
        fi.hasOnlyCompleteDirectCallerSets(), fi.foundAllCallers(),
        std::move(partialAppliers), std::move(fullAppliers));
    yout << node;
  }
}

//===----------------------------------------------------------------------===//
//                              Main Entry Point
//===----------------------------------------------------------------------===//

SILAnalysis *language::createCallerAnalysis(SILModule *mod) {
  return new CallerAnalysis(mod);
}
