//===--- PruneVTables.cpp - Prune unnecessary vtable entries --------------===//
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
// Mark sil_vtable entries as [nonoverridden] when possible, so that we know
// at IRGen time they can be elided from runtime vtables.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "prune-vtables"

#include "language/SIL/CalleeCache.h"
#include "language/SILOptimizer/PassManager/Transforms.h"
#include "language/SILOptimizer/Utils/InstOptUtils.h"

STATISTIC(NumNonoverriddenVTableEntries,
          "# of vtable entries marked non-overridden");

using namespace language;

namespace {
class PruneVTables : public SILModuleTransform {
  void runOnVTable(SILModule *M, SILVTable *vtable) {
    TOOLCHAIN_DEBUG(toolchain::dbgs() << "PruneVTables inspecting table:\n";
               vtable->print(toolchain::dbgs()));
    if (!M->isWholeModule() &&
        vtable->getClass()->getEffectiveAccess() >= AccessLevel::FilePrivate) {
      TOOLCHAIN_DEBUG(toolchain::dbgs() << "Ignoring visible table: ";
                 vtable->print(toolchain::dbgs()));
      return;
    }

    for (auto &entry : vtable->getMutableEntries()) {
      
      // We don't need to worry about entries that are overridden,
      // or have already been found to have no overrides.
      if (entry.isNonOverridden()) {
        TOOLCHAIN_DEBUG(toolchain::dbgs() << "-- entry for ";
                   entry.getMethod().print(toolchain::dbgs());
                   toolchain::dbgs() << " is already nonoverridden\n");
        continue;
      }
      
      switch (entry.getKind()) {
      case SILVTable::Entry::Normal:
      case SILVTable::Entry::Inherited:
        break;
          
      case SILVTable::Entry::Override:
        TOOLCHAIN_DEBUG(toolchain::dbgs() << "-- entry for ";
                   entry.getMethod().print(toolchain::dbgs());
                   toolchain::dbgs() << " is an override\n");
        continue;
      }

      // The destructor entry must remain.
      if (entry.getMethod().kind == SILDeclRef::Kind::Deallocator) {
        TOOLCHAIN_DEBUG(toolchain::dbgs() << "-- entry for ";
                   entry.getMethod().print(toolchain::dbgs());
                   toolchain::dbgs() << " is a destructor\n");
        continue;
      }

      auto methodDecl = entry.getMethod().getAbstractFunctionDecl();
      if (!methodDecl) {
        TOOLCHAIN_DEBUG(toolchain::dbgs() << "-- entry for ";
                   entry.getMethod().print(toolchain::dbgs());
                   toolchain::dbgs() << " is not a function decl\n");
        continue;
      }

      // Is the method declared final?
      if (!methodDecl->isFinal()) {
        // Are callees of this entry statically knowable?
        if (!calleesAreStaticallyKnowable(*M, entry.getMethod())) {
          TOOLCHAIN_DEBUG(toolchain::dbgs() << "-- entry for ";
                     entry.getMethod().print(toolchain::dbgs());
                     toolchain::dbgs() << " does not have statically-knowable callees\n");
          continue;
        }
        
        // Does the method have any overrides in this module?
        if (methodDecl->isOverridden()) {
          TOOLCHAIN_DEBUG(toolchain::dbgs() << "-- entry for ";
                     entry.getMethod().print(toolchain::dbgs());
                     toolchain::dbgs() << " has overrides\n");
          continue;
        }
      }
      TOOLCHAIN_DEBUG(toolchain::dbgs() << "++ entry for ";
                 entry.getMethod().print(toolchain::dbgs());
                 toolchain::dbgs() << " can be marked non-overridden!\n");
      ++NumNonoverriddenVTableEntries;
      entry.setNonOverridden(true);
      vtable->updateVTableCache(entry);
    }
  }
  
  void run() override {
    SILModule *M = getModule();
    
    for (auto &vtable : M->getVTables()) {
      runOnVTable(M, vtable);
    }
  }
};
}

SILTransform *language::createPruneVTables() {
  return new PruneVTables();
}
