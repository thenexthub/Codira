//===--- ReferenceBindingTransform.cpp ------------------------------------===//
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

#define DEBUG_TYPE "sil-reference-binding-transform"

#include "language/AST/DiagnosticsSIL.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/Defer.h"
#include "language/SIL/BasicBlockDatastructures.h"
#include "language/SIL/FieldSensitivePrunedLiveness.h"
#include "language/SIL/PrunedLiveness.h"
#include "language/SIL/SILBuilder.h"
#include "language/SILOptimizer/PassManager/Transforms.h"

using namespace language;

static toolchain::cl::opt<bool> SilentlyEmitDiagnostics(
    "sil-reference-binding-diagnostics-silently-emit-diagnostics",
    toolchain::cl::desc(
        "For testing purposes, emit the diagnostic silently so we can "
        "filecheck the result of emitting an error from the move checkers"),
    toolchain::cl::init(false));

//===----------------------------------------------------------------------===//
//                          MARK: Diagnosis Helpers
//===----------------------------------------------------------------------===//

template <typename... T, typename... U>
static void diagnose(SILInstruction *inst, Diag<T...> diag, U &&...args) {
  // If we asked to actually emit diagnostics, skip it. This lets us when
  // testing to write FileCheck tests for tests without an error along side
  // other tests where we want to use -verify.
  if (SilentlyEmitDiagnostics)
    return;

  // See if the consuming use is an owned moveonly_to_copyable whose only
  // user is a return. In that case, use the return loc instead. We do this
  // b/c it is illegal to put a return value location on a non-return value
  // instruction... so we have to hack around this slightly.
  auto loc = inst->getLoc();
  if (auto *mtc = dyn_cast<MoveOnlyWrapperToCopyableValueInst>(inst)) {
    if (auto *ri = mtc->getSingleUserOfType<ReturnInst>()) {
      loc = ri->getLoc();
    }
  }

  auto &context = inst->getModule().getASTContext();
  context.Diags.diagnose(loc.getSourceLoc(), diag, std::forward<U>(args)...);
}

//===----------------------------------------------------------------------===//
//                              MARK: Utilities
//===----------------------------------------------------------------------===//

namespace {

struct RAIILLVMDebug {
  StringRef str;

  RAIILLVMDebug(StringRef str) : str(str) {
    TOOLCHAIN_DEBUG(toolchain::dbgs() << "===>>> Starting " << str << '\n');
  }

  RAIILLVMDebug(StringRef str, SILInstruction *u) : str(str) {
    TOOLCHAIN_DEBUG(toolchain::dbgs() << "===>>> Starting " << str << ":" << *u);
  }

  ~RAIILLVMDebug() {
    TOOLCHAIN_DEBUG(toolchain::dbgs() << "===<<< Completed " << str << '\n');
  }
};

} // namespace

//===----------------------------------------------------------------------===//
//                         MARK: Gather Address Uses
//===----------------------------------------------------------------------===//

namespace {

struct ValidateAllUsesWithinLiveness : public AccessUseVisitor {
  SSAPrunedLiveness &liveness;
  SILInstruction *markInst;
  SILInstruction *initInst;
  bool emittedDiagnostic = false;

  ValidateAllUsesWithinLiveness(SSAPrunedLiveness &gatherUsesLiveness,
                                SILInstruction *markInst,
                                SILInstruction *initInst)
      : AccessUseVisitor(AccessUseType::Overlapping,
                         NestedAccessType::IgnoreAccessBegin),
        liveness(gatherUsesLiveness), markInst(markInst), initInst(initInst) {}

  bool visitUse(Operand *op, AccessUseType useTy) override {
    auto *user = op->getUser();

    // Skip our initInst which is going to be within the lifetime.
    if (user == initInst)
      return true;

    // Skip end_access. We care about the end_access uses.
    if (isa<EndAccessInst>(user))
      return true;

    if (liveness.isWithinBoundary(user, /*deadEndBlocks=*/nullptr)) {
      TOOLCHAIN_DEBUG(toolchain::dbgs() << "User in boundary: " << *user);
      diagnose(op->getUser(),
               diag::sil_referencebinding_src_used_within_inout_scope);
      diagnose(markInst, diag::sil_referencebinding_inout_binding_here);
      emittedDiagnostic = true;
    }

    return true;
  }
};

} // end anonymous namespace

//===----------------------------------------------------------------------===//
//                              MARK: Transform
//===----------------------------------------------------------------------===//

namespace {

struct DiagnosticEmitter {
  unsigned numDiagnostics = 0;

  void diagnoseUnknownPattern(MarkUnresolvedReferenceBindingInst *mark) {
    diagnose(mark, diag::sil_referencebinding_unknown_pattern);
    ++numDiagnostics;
  }
};

} // namespace

namespace {

class ReferenceBindingProcessor {
  MarkUnresolvedReferenceBindingInst *mark;
  DiagnosticEmitter &diagnosticEmitter;

public:
  ReferenceBindingProcessor(MarkUnresolvedReferenceBindingInst *mark,
                            DiagnosticEmitter &diagnosticEmitter)
      : mark(mark), diagnosticEmitter(diagnosticEmitter) {}

  bool process() &&;

private:
  CopyAddrInst *findInit();
};

} // namespace

CopyAddrInst *ReferenceBindingProcessor::findInit() {
  RAIILLVMDebug toolchainDebug("Find initialization");

  // We rely on the following semantics to find our initialization:
  //
  // 1. SILGen always initializes values completely.
  // 2. Boxes always have operations guarded /except/ for their
  //    initialization.
  // 3. We force inout to always be initialized once.
  //
  // Thus to find our initialization, we need to find a project_box use of our
  // target that directly initializes the value.
  CopyAddrInst *initInst = nullptr;
  InstructionWorklist worklist(mark->getFunction());
  for (auto *user : mark->getUsers()) {
    worklist.push(user);
  }
  while (auto *user = worklist.pop()) {
    TOOLCHAIN_DEBUG(toolchain::dbgs() << "Visiting use: " << *user);

    auto *bbi = dyn_cast<BeginBorrowInst>(user);
    if (bbi && bbi->isFromVarDecl()) {
      for (auto *user : bbi->getUsers()) {
        worklist.push(user);
      }
      continue;
    }
    auto *pbi = dyn_cast<ProjectBoxInst>(user);
    if (!pbi)
      continue;
    TOOLCHAIN_DEBUG(toolchain::dbgs() << "    Found project_box! Visiting pbi uses!\n");

    for (auto *pbiUse : pbi->getUses()) {
      TOOLCHAIN_DEBUG(toolchain::dbgs() << "    Pbi Use: " << *pbiUse->getUser());
      auto *cai = dyn_cast<CopyAddrInst>(pbiUse->getUser());
      if (!cai || cai->getDest() != pbi) {
        TOOLCHAIN_DEBUG(toolchain::dbgs() << "    Either not a copy_addr or dest is "
                                   "not the project_box! Skipping!\n");
        continue;
      }

      if (initInst || !cai->isInitializationOfDest() || cai->isTakeOfSrc()) {
        TOOLCHAIN_DEBUG(
            toolchain::dbgs()
            << "    Either already found an init inst or is an assign of a "
               "dest or a take of src... emitting unknown pattern!\n");
        diagnosticEmitter.diagnoseUnknownPattern(mark);
        return nullptr;
      }
      assert(!initInst && "Init twice?!");
      assert(!cai->isTakeOfSrc());
      initInst = cai;
      TOOLCHAIN_DEBUG(toolchain::dbgs()
                 << "    Found our init! Checking for other inits!\n");
    }
  }
  TOOLCHAIN_DEBUG(toolchain::dbgs() << "Final Init: " << *initInst);
  return initInst;
}

bool ReferenceBindingProcessor::process() && {
  // Find a single initialization of our inout parameter. See helper function
  // for the information about the invariants we rely upon.
  auto *initInst = findInit();
  if (!initInst)
    return false;

  // Ok, we have our initialization. Now gather our destroys of our value and
  // initialize an SSAPrunedLiveness, using our initInst as our def.
  auto *fn = mark->getFunction();

  SmallVector<SILBasicBlock *, 8> discoveredBlocks;
  SSAPrunedLiveness liveness(fn, &discoveredBlocks);
  StackList<DestroyValueInst *> destroyValueInst(fn);
  {
    RAIILLVMDebug toolchainDebug("Initializing liveness!");

    liveness.initializeDef(mark);
    for (auto *consume : mark->getConsumingUses()) {
      // Make sure that the destroy_value is not within the boundary.
      auto *dai = dyn_cast<DestroyValueInst>(consume->getUser());
      if (!dai) {
        TOOLCHAIN_DEBUG(toolchain::dbgs() << "    Found non destroy value consuming use! "
                                   "Emitting unknown pattern: "
                                << *dai);
        diagnosticEmitter.diagnoseUnknownPattern(mark);
        return false;
      }

      TOOLCHAIN_DEBUG(toolchain::dbgs() << "    Found destroy_value use: " << *dai);
      liveness.updateForUse(dai, true /*is lifetime ending*/);
      destroyValueInst.push_back(dai);
    }
  }

  // Now make sure that our source address doesn't have any uses within the
  // lifetime of our box. Or emit an error. Since sema always ensures that we
  // have an lvalue, we should always find an access scope.
  BeginAccessInst *bai = nullptr;
  StackList<SILInstruction *> endAccesses(fn);
  {
    auto accessPathWithBase =
        AccessPathWithBase::computeInScope(initInst->getSrc());
    assert(accessPathWithBase.base);

    // Then search up for a single begin_access from src to find our
    // begin_access we need to fix. We know that it will always be directly on
    // the type since we only allow for inout today to be on decl_ref expr. This
    // will change in the future. Once we find that begin_access, we need to
    // convert it to a modify and expand it.
    bai = cast<BeginAccessInst>(accessPathWithBase.base);
    assert(bai->getAccessKind() == SILAccessKind::Read);
    for (auto *eai : bai->getEndAccesses())
      endAccesses.push_back(eai);

    {
      ValidateAllUsesWithinLiveness initGatherer(liveness, mark, initInst);
      if (!visitAccessPathBaseUses(initGatherer, accessPathWithBase, fn)) {
        diagnosticEmitter.diagnoseUnknownPattern(mark);
        return false;
      }
    }
  }

  initInst->setIsTakeOfSrc(IsTake);
  bai->setAccessKind(SILAccessKind::Modify);

  while (!endAccesses.empty())
    endAccesses.pop_back_val()->eraseFromParent();

  while (!destroyValueInst.empty()) {
    auto *consume = destroyValueInst.pop_back_val();
    SILBuilderWithScope builder(consume);
    auto *pbi = builder.createProjectBox(consume->getLoc(), mark, 0);
    builder.createCopyAddr(consume->getLoc(), pbi, initInst->getSrc(), IsTake,
                           IsInitialization);
    builder.createEndAccess(consume->getLoc(), bai, false /*aborted*/);
    builder.createDeallocBox(consume->getLoc(), mark);
    consume->eraseFromParent();
  }

  return true;
}

static bool runTransform(SILFunction *fn) {
  bool madeChange = false;

  // First go through and find all of our mark_unresolved_reference_binding.
  StackList<MarkUnresolvedReferenceBindingInst *> targets(fn);
  for (auto &block : *fn) {
    for (auto &ii : block) {
      if (auto *murbi = dyn_cast<MarkUnresolvedReferenceBindingInst>(&ii))
        targets.push_back(murbi);
    }
  }

  DiagnosticEmitter emitter;
  while (!targets.empty()) {
    auto *mark = targets.pop_back_val();

    TOOLCHAIN_DEBUG(toolchain::dbgs() << "===> Visiting mark: " << *mark);
    ReferenceBindingProcessor processor{mark, emitter};
    madeChange |= std::move(processor).process();

    mark->replaceAllUsesWith(mark->getOperand());
    mark->eraseFromParent();
    madeChange = true;
  }

  return madeChange;
}

//===----------------------------------------------------------------------===//
//                         MARK: Top Level Entrypoint
//===----------------------------------------------------------------------===//

namespace {

struct ReferenceBindingTransformPass : SILFunctionTransform {
  void run() override {
    auto *fn = getFunction();

    // Only run this if the reference binding feature is enabled.
    if (!fn->getASTContext().LangOpts.hasFeature(Feature::ReferenceBindings))
      return;

    TOOLCHAIN_DEBUG(toolchain::dbgs()
               << "!!! === RefBindingTransform: " << fn->getName() << '\n');
    if (runTransform(fn)) {
      invalidateAnalysis(SILAnalysis::Instructions);
    }
  }
};

} // namespace

SILTransform *language::createReferenceBindingTransform() {
  return new ReferenceBindingTransformPass();
}
