//===- CalleeCache.h - Determine callees per call site ----------*- C++ -*-===//
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

#ifndef LANGUAGE_UTILS_CALLEECACHE_H
#define LANGUAGE_UTILS_CALLEECACHE_H

#include "language/SIL/ApplySite.h"
#include "language/SIL/SILDeclRef.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/PointerIntPair.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/Support/Allocator.h"

namespace language {

class SILFunction;
class SILModule;
class SILWitnessTable;
class ValueDecl;

/// Do we have enough information to determine all callees that could
/// be reached by calling the function represented by Decl?
bool calleesAreStaticallyKnowable(SILModule &module, SILDeclRef decl);

/// Do we have enough information to determine all callees that could
/// be reached by calling the function represented by Decl?
bool calleesAreStaticallyKnowable(SILModule &module, ValueDecl *vd);

/// CalleeList is a data structure representing the list of potential
/// callees at a particular apply site. It also has a query that
/// allows a client to determine whether the list is incomplete in the
/// sense that there may be unrepresented callees.
class CalleeList {
  friend class CalleeCache;

  using Callees = toolchain::SmallVector<SILFunction *, 16>;

  void *functionOrCallees;

  enum class Kind : uint8_t {
    empty,
    singleFunction,
    multipleCallees
  } kind;

  bool incomplete;

  CalleeList(void *ptr, Kind kind, bool isIncomplete) :
    functionOrCallees(ptr), kind(kind), incomplete(isIncomplete) {}

public:
  /// Constructor for when we know nothing about the callees and must
  /// assume the worst.
  CalleeList() : CalleeList(nullptr, Kind::empty, /*incomplete*/ true) {}

  /// Constructor for the case where we know an apply can target only
  /// a single function.
  CalleeList(SILFunction *F)
    : CalleeList(F, Kind::singleFunction, /*incomplete*/ false) {}

  /// Constructor for arbitrary lists of callees.
  CalleeList(Callees *callees, bool IsIncomplete)
    : CalleeList(callees, Kind::multipleCallees, IsIncomplete) {}

  static CalleeList fromOpaque(void *ptr, unsigned char kind, unsigned char isComplete) {
    return CalleeList(ptr, (Kind)kind, (bool)isComplete);
  }

  void *getOpaquePtr() const { return functionOrCallees; }
  unsigned char getOpaqueKind() const { return (unsigned char)kind; }

  LANGUAGE_DEBUG_DUMP;

  void print(toolchain::raw_ostream &os) const;

  /// Return an iterator for the beginning of the list.
  ArrayRef<SILFunction *>::iterator begin() const {
    switch (kind) {
      case Kind::empty:
        return nullptr;
      case Kind::singleFunction:
        return (SILFunction * const *)&functionOrCallees;
      case Kind::multipleCallees:
        return ((Callees *)functionOrCallees)->begin();
    }
  }

  /// Return an iterator for the end of the list.
  ArrayRef<SILFunction *>::iterator end() const {
    switch (kind) {
      case Kind::empty:
        return nullptr;
      case Kind::singleFunction:
        return (SILFunction * const *)&functionOrCallees + 1;
      case Kind::multipleCallees:
        return ((Callees *)functionOrCallees)->end();
    }
  }

  size_t getCount() const {
    switch (kind) {
      case Kind::empty:           return 0;
      case Kind::singleFunction:  return 1;
      case Kind::multipleCallees: return ((Callees *)functionOrCallees)->size();
    }
  }

  SILFunction *get(unsigned index) const {
    switch (kind) {
      case Kind::empty:           toolchain_unreachable("empty callee list");
      case Kind::singleFunction:  return (SILFunction *)functionOrCallees;
      case Kind::multipleCallees: return ((Callees *)functionOrCallees)->operator[](index);
    }
  }

  bool isIncomplete() const { return incomplete; }

  /// Returns true if all callees are known and not external.
  bool allCalleesVisible() const;
};

/// CalleeCache is a helper class that builds lists of potential
/// callees for class and witness method applications, and provides an
/// interface for retrieving a (possibly incomplete) CalleeList for
/// any function application site (including those that are simple
/// function_ref, thin_to_thick, or partial_apply callees).
class CalleeCache {
  using CalleesAndCanCallUnknown = toolchain::PointerIntPair<CalleeList::Callees *, 1>;
  using CacheType = toolchain::DenseMap<SILDeclRef, CalleesAndCanCallUnknown>;

  SILModule &M;

  // Allocator for the SmallVectors that we will be allocating.
  toolchain::SpecificBumpPtrAllocator<CalleeList::Callees> Allocator;

  // The cache of precomputed callee lists for function decls appearing
  // in class virtual dispatch tables and witness tables.
  CacheType TheCache;

public:
  CalleeCache(SILModule &M) : M(M) {
    computeMethodCallees();
    sortAndUniqueCallees();
  }

  ~CalleeCache() {
    Allocator.DestroyAll();
  }

  /// Return the list of callees that can potentially be called at the
  /// given apply site.
  CalleeList getCalleeList(FullApplySite FAS) const;

  CalleeList getCalleeListOfValue(SILValue callee) const {
    return getCalleeListForCalleeKind(callee);
  }

  /// Return the list of callees that can potentially be called at the
  /// given instruction. E.g. it could be destructors.
  CalleeList getDestructors(SILType type, bool isExactType) const;

  CalleeList getCalleeList(SILDeclRef Decl) const;

private:
  void enumerateFunctionsInModule();
  void sortAndUniqueCallees();
  CalleesAndCanCallUnknown &getOrCreateCalleesForMethod(SILDeclRef Decl);
  void computeClassMethodCallees();
  void computeWitnessMethodCalleesForWitnessTable(SILWitnessTable &WT);
  void computeMethodCallees();
  SILFunction *getSingleCalleeForWitnessMethod(WitnessMethodInst *WMI) const;
  CalleeList getCalleeList(WitnessMethodInst *WMI) const;
  CalleeList getCalleeList(ClassMethodInst *CMI) const;
  CalleeList getCalleeListForCalleeKind(SILValue Callee) const;
};

} // end namespace language

#endif
