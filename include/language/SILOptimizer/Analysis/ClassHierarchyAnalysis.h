//===--- ClassHierarchyAnalysis.h - Analysis of Class Hierarchy -*- C++ -*-===//
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

#ifndef SWIFT_SILOPTIMIZER_ANALYSIS_CLASSHIERARCHY_H
#define SWIFT_SILOPTIMIZER_ANALYSIS_CLASSHIERARCHY_H

#include "language/SILOptimizer/Analysis/Analysis.h"
#include "language/SIL/SILArgument.h"
#include "language/SIL/SILValue.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/Support/Debug.h"

namespace language {

class SILModule;
class ClassDecl;
class ClassHierarchyAnalysis : public SILAnalysis {
public:
  typedef SmallVector<ClassDecl *, 8> ClassList;
  typedef llvm::DenseMap<ClassDecl *, ClassList> ClassListMap;

  ClassHierarchyAnalysis(SILModule *Mod)
      : SILAnalysis(SILAnalysisKind::ClassHierarchy), M(Mod) {}

  ~ClassHierarchyAnalysis();

  static bool classof(const SILAnalysis *S) {
    return S->getKind() == SILAnalysisKind::ClassHierarchy;
  }

  /// Invalidate all information in this analysis.
  virtual void invalidate() override {
    // Nothing can invalidate, because types are static and cannot be changed
    // during the SIL pass pipeline.
  }

  /// Invalidate all of the information for a specific function.
  virtual void invalidate(SILFunction *F, InvalidationKind K) override { }

  /// Notify the analysis about a newly created function.
  virtual void notifyAddedOrModifiedFunction(SILFunction *F) override {}

  /// Notify the analysis about a function which will be deleted from the
  /// module.
  virtual void notifyWillDeleteFunction(SILFunction *F) override {}

  /// Notify the analysis about changed witness or vtables.
  virtual void invalidateFunctionTables() override { }

  /// Returns a list of the known direct subclasses of a class \p C in
  /// the current module.
  const ClassList &getDirectSubClasses(ClassDecl *C) {
    populateDirectSubclassesCacheIfNecessary();
    return (*DirectSubclassesCache)[C];
  }

  /// Returns a list of the known indirect subclasses of a class \p C in
  /// the current module.
  const ClassList &getIndirectSubClasses(ClassDecl *C) {
    if (!IndirectSubclassesCache.count(C)) {
      // Lazy initialization
      auto &K = IndirectSubclassesCache[C];
      getIndirectSubClasses(C, K);
    }
    return IndirectSubclassesCache[C];
  }

  /// Returns true if the class is inherited by another class in this module.
  bool hasKnownDirectSubclasses(ClassDecl *C) {
    populateDirectSubclassesCacheIfNecessary();
    return DirectSubclassesCache->count(C);
  }

  /// Returns true if the class is indirectly inherited by another class
  /// in this module.
  bool hasKnownIndirectSubclasses(ClassDecl *C) {
    return IndirectSubclassesCache.count(C) &&
           !IndirectSubclassesCache[C].empty();
  }

private:
  void getIndirectSubClasses(ClassDecl *Base,
                             ClassList &IndirectSubs);
  /// The module
  SILModule *M;

  /// A cache that maps a class to all of its known direct subclasses.
  std::optional<ClassListMap> DirectSubclassesCache;

  /// A cache that maps a class to all of its known indirect subclasses.
  ClassListMap IndirectSubclassesCache;

  /// Populates `DirectSubclassesCache` if necessary.
  void populateDirectSubclassesCacheIfNecessary();
};

}
#endif
