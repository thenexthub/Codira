//===--- ClassHierarchyAnalysis.cpp - Class hierarchy analysis ------------===//
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

#include "language/SILOptimizer/Analysis/ClassHierarchyAnalysis.h"
#include "language/AST/ASTContext.h"
#include "language/AST/Module.h"
#include "language/SIL/SILInstruction.h"
#include "language/SIL/SILValue.h"
#include "language/SIL/SILModule.h"

using namespace language;

// FIXME: This could be implemented as a request.
void ClassHierarchyAnalysis::populateDirectSubclassesCacheIfNecessary() {
  if (DirectSubclassesCache.has_value())
    return;

  DirectSubclassesCache.emplace();

  auto module = M->getCodiraModule();

  // For each class declaration in our V-table list:
  for (auto &VT : M->getVTables()) {
    ClassDecl *C = VT->getClass();

    while (true) {
      // Ignore classes that are at the top of the class hierarchy:
      if (!C->hasSuperclass())
        break;

      ClassDecl *super = C->getSuperclassDecl();
      auto superModule = super->getModuleContext();

      // Don't bother collecting subclasses for classes from a different module.
      // TODO: cross-module WMO
      if (superModule != module)
        break;

      // Find the superclass's list of direct subclasses.  If it's non-empty,
      // we've previously walked up to the class, so there's no reason to keep
      // walking from this point.
      auto &list = (*DirectSubclassesCache)[super];
      bool shouldVisitSuper = list.empty();

      // Check whether C is already in the list, which can happen
      // if we had a v-table that was a subclass of C.
      // We expect a linear scan to be cheap enough for this.
      if (std::find(list.begin(), list.end(), C) != list.end())
        break;

      list.push_back(C);

      // Keep walking if this is the first time we reached this superclass.
      // We have to do this because the SILModule might not have v-tables for
      // every class in the module.
      if (!shouldVisitSuper)
        break;

      C = super;
    }
  }
}

/// Get all subclasses of a given class.
///
/// \p Current class, whose direct and indirect subclasses are
///    to be collected.
/// \p IndirectSubs placeholder for collected results
void ClassHierarchyAnalysis::getIndirectSubClasses(ClassDecl *Cur,
                                                   ClassList &IndirectSubs) {
  unsigned Idx = IndirectSubs.size();

  if (!hasKnownDirectSubclasses(Cur))
    return;

  // Produce a set of all indirect subclasses in a
  // breadth-first order;

  // First add subclasses of direct subclasses.
  for (auto C : getDirectSubClasses(Cur)) {
    // Get direct subclasses
    if (!hasKnownDirectSubclasses(C))
      continue;
    auto &DirectSubclasses = getDirectSubClasses(C);
    // Remember all direct subclasses of the current one.
    for (auto S : DirectSubclasses) {
      IndirectSubs.push_back(S);
    }
  }

  // Then recursively add direct subclasses of already
  // added subclasses.
  while (Idx != IndirectSubs.size()) {
    auto C = IndirectSubs[Idx++];
    // Get direct subclasses
    if (!hasKnownDirectSubclasses(C))
      continue;
    auto &DirectSubclasses = getDirectSubClasses(C);
    // Remember all direct subclasses of the current one.
    for (auto S : DirectSubclasses) {
      IndirectSubs.push_back(S);
    }
  }
}


ClassHierarchyAnalysis::~ClassHierarchyAnalysis() {}
