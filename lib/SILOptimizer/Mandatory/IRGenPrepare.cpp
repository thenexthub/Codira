//===--- IRGenPrepare.cpp -------------------------------------------------===//
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
///
/// \file
///
/// Cleanup SIL to make it suitable for IRGen.
///
/// We perform the following canonicalizations:
///
/// 1. We remove calls to Builtin.poundAssert() and Builtin.staticReport(),
///    which are not needed post SIL.
///
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "sil-cleanup"
#include "language/SIL/SILBuilder.h"
#include "language/SIL/SILFunction.h"
#include "language/SIL/SILInstruction.h"
#include "language/SIL/SILModule.h"
#include "language/SILOptimizer/PassManager/Passes.h"
#include "language/SILOptimizer/PassManager/Transforms.h"
#include "language/SILOptimizer/Utils/InstructionDeleter.h"
#include "language/Strings.h"

using namespace language;

// Print the message string of encountered `cond_fail` instructions the first
// time the message string is encountered.
static toolchain::cl::opt<bool> PrintCondFailMessages(
  "print-cond-fail-messages", toolchain::cl::init(false),
  toolchain::cl::desc("print cond_fail messages"));
static toolchain::cl::opt<bool> IncludeCondFailMessagesFunction(
  "print-cond-fail-messages-include-function-name", toolchain::cl::init(false),
  toolchain::cl::desc("when printing cond_fail messages include"
                 "the current SIL function name"));

static toolchain::DenseSet<StringRef> CondFailMessages;

static bool cleanFunction(SILFunction &fn) {
  bool madeChange = false;

  for (auto &bb : fn) {
    for (auto i = bb.begin(), e = bb.end(); i != e;) {
      // Make sure there is no iterator invalidation if the inspected
      // instruction gets removed from the block.
      SILInstruction *inst = &*i;
      ++i;

      // Print cond_fail messages the first time a specific cond_fail message
      // string is encountered.
      //   Run the language-frontend in a mode that will generate LLVM IR adding
      //   the option `-print-cond-fail-messages` will dump all cond_fail
      //   message strings encountered in the SIL.
      //   ```
      //     % language-frontend -Xtoolchain -print-cond-fail-messages -emit-ir/-c ...
      //     ...
      //     cond_fail message encountered: Range out of bounds
      //     cond_fail message encountered: Array index is out of range
      //     ...
      //   ```
      if (PrintCondFailMessages) {
        if (auto CFI = dyn_cast<CondFailInst>(inst)) {
          auto msg = CFI->getMessage().str();
          if (IncludeCondFailMessagesFunction) {
            msg.append(" in ");
            msg.append(fn.getName().str());
          }
          if (CondFailMessages.insert(msg).second)
            toolchain::dbgs() << "cond_fail message encountered: " << msg << "\n";
        }
      }

      // Remove calls to Builtin.poundAssert() and Builtin.staticReport().
      auto *bi = dyn_cast<BuiltinInst>(inst);
      if (!bi) {
        continue;
      }

      switch (bi->getBuiltinInfo().ID) {
        case BuiltinValueKind::CondFailMessage: {
          SILBuilderWithScope Builder(bi);
          Builder.createCondFail(bi->getLoc(), bi->getOperand(0),
            "unknown program error");
          TOOLCHAIN_FALLTHROUGH;
        }
        case BuiltinValueKind::PoundAssert:
        case BuiltinValueKind::StaticReport: {
          // The call to the builtin should get removed before we reach
          // IRGen.
          InstructionDeleter deleter;
          deleter.forceDelete(bi);
          // StaticReport only takes trivial operands, and therefore doesn't
          // require fixing the lifetime of its operands.
          deleter.cleanupDeadInstructions();
          madeChange = true;
          break;
        }
        default:
          break;
      }
    }
  }

  return madeChange;
}

//===----------------------------------------------------------------------===//
//                            Top Level Entrypoint
//===----------------------------------------------------------------------===//

namespace {

class IRGenPrepare : public SILFunctionTransform {
  void run() override {
    SILFunction *F = getFunction();

    if (getOptions().EmbeddedCodira) {
      // In embedded language all the code is generated in the top-level module.
      // Even de-serialized functions must be code-gen'd.
      SILLinkage linkage = F->getLinkage();
      if (isAvailableExternally(linkage)) {
        F->setLinkage(SILLinkage::Hidden);
      }
    }

    bool shouldInvalidate = cleanFunction(*F);

    if (shouldInvalidate)
      invalidateAnalysis(SILAnalysis::InvalidationKind::Instructions);
  }
};

} // end anonymous namespace


SILTransform *language::createIRGenPrepare() {
  return new IRGenPrepare();
}
