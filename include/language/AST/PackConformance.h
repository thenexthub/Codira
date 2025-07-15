//===--- PackConformance.h - Variadic Protocol Conformance ------*- C++ -*-===//
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
// This file defines the PackConformance structure, which describes the
// conformance of a type pack parameter to a protocol.
//
//===----------------------------------------------------------------------===//
#ifndef LANGUAGE_AST_PACKCONFORMANCE_H
#define LANGUAGE_AST_PACKCONFORMANCE_H

#include "language/AST/ASTAllocated.h"
#include "language/AST/ProtocolConformanceRef.h"
#include "language/AST/Type.h"
#include "language/AST/TypeAlignments.h"
#include "language/Basic/Compiler.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/FoldingSet.h"
#include "toolchain/Support/TrailingObjects.h"

namespace language {

class PackType;

class alignas(1 << DeclAlignInBits) PackConformance final
  : public ASTAllocated<PackConformance>,
    public toolchain::FoldingSetNode,
    private toolchain::TrailingObjects<PackConformance, ProtocolConformanceRef> {
  friend class ASTContext;
  friend TrailingObjects;

  /// The pack type conforming to the protocol.
  PackType *ConformingType;

  /// The conformed-to protocol.
  ProtocolDecl *Protocol;

public:
  void Profile(toolchain::FoldingSetNodeID &ID) const;
  static void Profile(toolchain::FoldingSetNodeID &ID,
                      PackType *conformingType,
                      ProtocolDecl *protocol,
                      ArrayRef<ProtocolConformanceRef> conformances);

private:
  PackConformance(PackType *conformingType,
                  ProtocolDecl *protocol,
                  ArrayRef<ProtocolConformanceRef> conformances);

  size_t numTrailingObjects(OverloadToken<ProtocolConformanceRef>) const;

public:
  static PackConformance *get(PackType *conformingType,
                              ProtocolDecl *protocol,
                              ArrayRef<ProtocolConformanceRef> conformances);

  PackType *getType() const { return ConformingType; }

  ProtocolDecl *getProtocol() const { return Protocol; }

  ArrayRef<ProtocolConformanceRef> getPatternConformances() const;

  bool isInvalid() const;

  bool isCanonical() const;

  PackConformance *getCanonicalConformance() const;

  PackType *getTypeWitness(AssociatedTypeDecl *assocType,
                           SubstOptions options=std::nullopt) const;

  PackConformance *
  getAssociatedConformance(Type assocType, ProtocolDecl *protocol) const;

  /// The ProtocolConformanceRef either stores a pack conformance, or
  /// it is invalid in the case of substitution failure.
  ProtocolConformanceRef subst(SubstitutionMap subMap,
                               SubstOptions options = std::nullopt) const;

  /// The ProtocolConformanceRef either stores a pack conformance, or
  /// it is invalid in the case of substitution failure.
  ProtocolConformanceRef subst(TypeSubstitutionFn subs,
                               LookupConformanceFn conformances,
                               SubstOptions options = std::nullopt) const;

  /// Apply an in-flight substitution to the conformances in this
  /// protocol conformance ref.
  ///
  /// This function should generally not be used outside of the
  /// substitution subsystem.
  ProtocolConformanceRef subst(InFlightSubstitution &IFS) const;

  LANGUAGE_DEBUG_DUMP;
  void dump(toolchain::raw_ostream &out, unsigned indent = 0) const;
};

void simple_display(toolchain::raw_ostream &out, PackConformance *conformance);

} // end namespace language

#endif // LANGUAGE_AST_PACKCONFORMANCE_H
