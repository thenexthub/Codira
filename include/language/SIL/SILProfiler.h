//===--- SILProfiler.h - Instrumentation based profiling ===-----*- C++ -*-===//
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
// This file defines SILProfiler, which contains the profiling state for one
// function. It's used to drive PGO and generate code coverage reports.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SIL_PROFILER_H
#define LANGUAGE_SIL_PROFILER_H

#include "language/AST/ASTNode.h"
#include "language/Basic/ProfileCounter.h"
#include "language/SIL/SILAllocated.h"
#include "language/SIL/SILDeclRef.h"
#include "toolchain/ADT/DenseMap.h"

namespace language {

class AbstractFunctionDecl;
class SILCoverageMap;
class SILFunction;
class SILModule;

/// A reference to a given profiler counter.
class ProfileCounterRef final {
public:
  enum class Kind : uint8_t {
    /// References an ASTNode.
    Node,

    /// References the error branch for an apply or access.
    ErrorBranch
  };

private:
  friend struct toolchain::DenseMapInfo<ProfileCounterRef>;

  ASTNode Node;
  Kind RefKind;

  ProfileCounterRef(ASTNode node, Kind kind) : Node(node), RefKind(kind) {}

public:
  /// A profile counter that is associated with a given ASTNode.
  static ProfileCounterRef node(ASTNode node) {
    assert(node && "Must have non-null ASTNode");
    return ProfileCounterRef(node, Kind::Node);
  }

  /// A profile counter that is associated with the error branch of a particular
  /// error-throwing AST node.
  static ProfileCounterRef errorBranchOf(ASTNode node) {
    return ProfileCounterRef(node, Kind::ErrorBranch);
  }

  /// Retrieve the corresponding location of the counter.
  SILLocation getLocation() const;

  LANGUAGE_DEBUG_DUMP;
  void dump(raw_ostream &OS) const;

  /// A simpler dump output for inline printing.
  void dumpSimple(raw_ostream &OS) const;

  friend bool operator==(const ProfileCounterRef &lhs,
                         const ProfileCounterRef &rhs) {
    return lhs.Node == rhs.Node && lhs.RefKind == rhs.RefKind;
  }
  friend bool operator!=(const ProfileCounterRef &lhs,
                         const ProfileCounterRef &rhs) {
    return !(lhs == rhs);
  }
  friend toolchain::hash_code hash_value(const ProfileCounterRef &ref) {
    return toolchain::hash_combine(ref.Node, ref.RefKind);
  }
};

/// SILProfiler - Maps AST nodes to profile counters.
class SILProfiler : public SILAllocated<SILProfiler> {
private:
  SILModule &M;

  SILDeclRef forDecl;

  bool EmitCoverageMapping;

  SILCoverageMap *CovMap = nullptr;

  StringRef CurrentFileName;

  std::string PGOFuncName;

  uint64_t PGOFuncHash = 0;

  unsigned NumRegionCounters = 0;

  toolchain::DenseMap<ProfileCounterRef, unsigned> RegionCounterMap;

  toolchain::DenseMap<ProfileCounterRef, ProfileCounter> RegionLoadedCounterMap;

  toolchain::DenseMap<ASTNode, ASTNode> RegionCondToParentMap;

  std::vector<std::tuple<std::string, uint64_t, std::string>> CoverageData;

  SILProfiler(SILModule &M, SILDeclRef forDecl, bool EmitCoverageMapping)
      : M(M), forDecl(forDecl), EmitCoverageMapping(EmitCoverageMapping) {}

public:
  static SILProfiler *create(SILModule &M, SILDeclRef Ref);

  /// Check if the function is set up for profiling.
  bool hasRegionCounters() const { return NumRegionCounters != 0; }

  /// Get the execution count corresponding to \p Ref from a profile, if one
  /// is available.
  ProfileCounter getExecutionCount(ProfileCounterRef Ref);

  /// Get the execution count corresponding to \p Node from a profile, if one
  /// is available.
  ProfileCounter getExecutionCount(ASTNode Node);

  /// Get the node's parent ASTNode (e.g to get the parent IfStmt or IfCond of
  /// a condition), if one is available.
  std::optional<ASTNode> getPGOParent(ASTNode Node);

  /// Get the function name mangled for use with PGO.
  StringRef getPGOFuncName() const { return PGOFuncName; }

  /// Get the function hash.
  uint64_t getPGOFuncHash() const { return PGOFuncHash; }

  /// Get the number of region counters.
  unsigned getNumRegionCounters() const { return NumRegionCounters; }

  /// Retrieve the counter index for a given counter reference, asserting that
  /// it is present.
  unsigned getCounterIndexFor(ProfileCounterRef ref);

  /// Whether a counter has been recorded for the given counter reference.
  bool hasCounterFor(ProfileCounterRef ref) {
    return RegionCounterMap.contains(ref);
  }

private:
  /// Map counters to ASTNodes and set them up for profiling the function.
  void assignRegionCounters();
};

} // end namespace language

namespace toolchain {
using language::ProfileCounterRef;
using language::ASTNode;

template <> struct DenseMapInfo<ProfileCounterRef> {
  static inline ProfileCounterRef getEmptyKey() {
    return ProfileCounterRef(DenseMapInfo<ASTNode>::getEmptyKey(),
                             ProfileCounterRef::Kind::Node);
  }
  static inline ProfileCounterRef getTombstoneKey() {
    return ProfileCounterRef(DenseMapInfo<ASTNode>::getTombstoneKey(),
                             ProfileCounterRef::Kind::Node);
  }
  static unsigned getHashValue(const ProfileCounterRef &ref) {
    return hash_value(ref);
  }
  static bool isEqual(const ProfileCounterRef &lhs,
                      const ProfileCounterRef &rhs) {
    return lhs == rhs;
  }
};
} // end namespace toolchain

#endif // LANGUAGE_SIL_PROFILER_H
