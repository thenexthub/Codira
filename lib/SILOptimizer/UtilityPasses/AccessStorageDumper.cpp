//===--- AccessStorageDumper.cpp - Dump accessed storage ----------------===//
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

#define DEBUG_TYPE "sil-accessed-storage-dumper"
#include "language/Basic/Assertions.h"
#include "language/SIL/MemAccessUtils.h"
#include "language/SIL/PrettyStackTrace.h"
#include "language/SIL/SILFunction.h"
#include "language/SIL/SILInstruction.h"
#include "language/SIL/SILValue.h"
#include "language/SILOptimizer/PassManager/Passes.h"
#include "language/SILOptimizer/PassManager/Transforms.h"
#include "toolchain/Support/CommandLine.h"
#include "toolchain/Support/Debug.h"

using namespace language;

static toolchain::cl::opt<bool> EnableDumpUses(
    "enable-access-storage-dump-uses", toolchain::cl::init(false),
    toolchain::cl::desc("With --sil-access-storage-dumper, dump all uses"));

namespace {

/// Dumps sorage information for each access.
class AccessStorageDumper : public SILModuleTransform {
  toolchain::SmallVector<Operand *, 32> uses;

  void dumpAccessStorage(Operand *operand) {
    SILFunction *function = operand->getParentFunction();
    // Print storage itself first, for comparison against AccessPath. They can
    // differ in rare cases of unidentified storage with phis.
    AccessStorage::compute(operand->get()).print(toolchain::outs());
    // Now print the access path and base.
    auto pathAndBase = AccessPathWithBase::compute(operand->get());
    pathAndBase.print(toolchain::outs());
    // If enable-accessed-storage-dump-uses is set, dump all types of uses.
    auto accessPath = pathAndBase.accessPath;
    if (!accessPath.isValid() || !EnableDumpUses)
      return;

    uses.clear();
    accessPath.collectUses(uses, AccessUseType::Exact, function);
    toolchain::outs() << "Exact Uses {\n";
    for (auto *useOperand : uses) {
      toolchain::outs() << *useOperand->getUser() << "  ";
      auto usePath = AccessPath::compute(useOperand->get());
      usePath.printPath(toolchain::outs());
      assert(accessPath == usePath
             && "access path does not match use access path");
    }
    toolchain::outs() << "}\n";
    uses.clear();
    accessPath.collectUses(uses, AccessUseType::Inner, function);
    toolchain::outs() << "Inner Uses {\n";
    for (auto *useOperand : uses) {
      toolchain::outs() << *useOperand->getUser() << "  ";
      auto usePath = AccessPath::compute(useOperand->get());
      usePath.printPath(toolchain::outs());
      assert(accessPath.contains(usePath)
             && "access path does not contain use access path");
    }
    toolchain::outs() << "}\n";
    uses.clear();
    accessPath.collectUses(uses, AccessUseType::Overlapping, function);
    toolchain::outs() << "Overlapping Uses {\n";
    for (auto *useOperand : uses) {
      toolchain::outs() << *useOperand->getUser() << "  ";
      auto usePath = AccessPath::compute(useOperand->get());
      usePath.printPath(toolchain::outs());
      assert(accessPath.mayOverlap(usePath)
             && "access path does not overlap with use access path");
    }
    toolchain::outs() << "}\n";
  }

  void run() override {
    for (auto &fn : *getModule()) {
      toolchain::outs() << "@" << fn.getName() << "\n";
      if (fn.empty()) {
        toolchain::outs() << "<unknown>\n";
        continue;
      }
      PrettyStackTraceSILFunction functionDumper("...", &fn);
      for (auto &bb : fn) {
        for (auto &inst : bb) {
          if (inst.mayReadOrWriteMemory()) {
            toolchain::outs() << "###For MemOp: " << inst;
            visitAccessedAddress(&inst, [this](Operand *operand) {
              dumpAccessStorage(operand);
            });
          }
        }
      }
    }
  }
};

} // end anonymous namespace

SILTransform *language::createAccessStorageDumper() {
  return new AccessStorageDumper();
}
