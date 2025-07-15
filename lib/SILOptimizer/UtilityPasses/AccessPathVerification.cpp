//===--- AccessPathVerification.cpp - verify access paths and storage -----===//
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
/// Verify AccessPath computation. For the address of every memory operation in
/// the module, compute the access path, compute all the users of that path,
/// then verify that all users have the same access path.
///
/// This is potentially expensive, so there is a fast mode that limits the
/// number of uses visited per path.
///
/// During full verification, also check that all addresses that share an
/// AccessPath are covered when computed the use list of that AccessPath. This
/// is important because optimizations may assume that the use list is
/// complete.
///
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "access-path-verification"
#include "language/Basic/Assertions.h"
#include "language/SIL/MemAccessUtils.h"
#include "language/SIL/PrettyStackTrace.h"
#include "language/SIL/SILFunction.h"
#include "language/SIL/SILInstruction.h"
#include "language/SIL/SILValue.h"
#include "language/SILOptimizer/PassManager/Passes.h"
#include "language/SILOptimizer/PassManager/Transforms.h"
#include "toolchain/Support/Debug.h"

using namespace language;

namespace {

/// Verify access path and uses of each access.
class AccessPathVerification : public SILModuleTransform {
  toolchain::DenseMap<Operand *, AccessPath> useToPathMap;

  // Transient uses
  toolchain::SmallVector<Operand *, 64> uses;

public:
  void verifyAccessPath(Operand *operand) {
    auto accessPath = AccessPath::compute(operand->get());
    if (!accessPath.isValid())
      return;

    auto iterAndInserted = useToPathMap.try_emplace(operand, accessPath);
    // If this use was already computed from a previously visited path, make
    // sure the path we just computed matches.
    if (!iterAndInserted.second) {
      auto collectedFromPath = iterAndInserted.first->second;
      if (collectedFromPath != accessPath) {
        toolchain::errs() << "Address use: " << *operand->getUser()
                     << "  collected from path\n  ";
        collectedFromPath.print(toolchain::errs());
        toolchain::errs() << "  has different path\n  ";
        accessPath.print(toolchain::errs());
        operand->getUser()->getFunction()->print(toolchain::errs());
        assert(false && "computed path does not match collected path");
      }
      return;
    }
    // This is a new path, so map all its uses.
    assert(uses.empty());
    accessPath.collectUses(uses, AccessUseType::Exact,
                           operand->getParentFunction());
    bool foundOperandUse = false;
    for (Operand *use : uses) {
      if (use == operand) {
        foundOperandUse = true;
        continue;
      }
      auto iterAndInserted = useToPathMap.try_emplace(use, accessPath);
      if (!iterAndInserted.second) {
        toolchain::errs() << "Address use: " << *operand->getUser()
                     << "  with path...\n";
        accessPath.print(toolchain::errs());
        toolchain::errs() << "  was not collected for: " << *use->getUser();
        toolchain::errs() << "  with path...\n";
        auto computedPath = iterAndInserted.first->second;
        computedPath.print(toolchain::errs());
        use->getUser()->getFunction()->print(toolchain::errs());
        assert(false && "missing collected use");
      }
    }
    if (!foundOperandUse && !accessPath.hasUnknownOffset()) {
      toolchain::errs() << "Address use: " << *operand->getUser()
                   << "  is not a use of path\n  ";
      accessPath.print(toolchain::errs());
      assert(false && "not a user of its own computed path ");
    }
    uses.clear();
  }

  void run() override {
    for (auto &fn : *getModule()) {
      if (fn.empty())
        continue;

      PrettyStackTraceSILFunction functionDumper("...", &fn);
      for (auto &bb : fn) {
        for (auto &inst : bb) {
          if (inst.mayReadOrWriteMemory())
            visitAccessedAddress(&inst, [this](Operand *operand) {
              return verifyAccessPath(operand);
            });
        }
      }
      useToPathMap.clear();
    }
  }
};

} // end anonymous namespace

SILTransform *language::createAccessPathVerification() {
  return new AccessPathVerification();
}
