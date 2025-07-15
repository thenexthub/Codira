//===--- PackConformance.cpp - Variadic Protocol Conformance --------------===//
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
// This file implements the PackConformance structure, which describes the
// conformance of a type pack parameter to a protocol.
//
//===----------------------------------------------------------------------===//

#include "language/AST/PackConformance.h"
#include "language/AST/ASTContext.h"
#include "language/AST/Decl.h"
#include "language/AST/InFlightSubstitution.h"
#include "language/AST/Module.h"
#include "language/AST/Types.h"
#include "language/Basic/Assertions.h"

#define DEBUG_TYPE "AST"

using namespace language;

void PackConformance::Profile(toolchain::FoldingSetNodeID &ID) const {
  Profile(ID, ConformingType, Protocol, getPatternConformances());
}

void PackConformance::Profile(toolchain::FoldingSetNodeID &ID,
                              PackType *conformingType,
                              ProtocolDecl *protocol,
                              ArrayRef<ProtocolConformanceRef> conformances) {
  ID.AddPointer(conformingType);
  ID.AddPointer(protocol);
  for (auto conformance : conformances)
    ID.AddPointer(conformance.getOpaqueValue());
}

PackConformance::PackConformance(PackType *conformingType,
                                 ProtocolDecl *protocol,
                                 ArrayRef<ProtocolConformanceRef> conformances)
  : ConformingType(conformingType), Protocol(protocol) {

  assert(ConformingType->getNumElements() == conformances.size());
  std::uninitialized_copy(conformances.begin(), conformances.end(),
                          getTrailingObjects<ProtocolConformanceRef>());
}

size_t PackConformance::numTrailingObjects(
    OverloadToken<ProtocolConformanceRef>) const {
  return ConformingType->getNumElements();
}

bool PackConformance::isInvalid() const {
  return toolchain::any_of(getPatternConformances(),
                      [&](const auto ref) { return ref.isInvalid(); });
}

ArrayRef<ProtocolConformanceRef>
PackConformance::getPatternConformances() const {
  return {getTrailingObjects<ProtocolConformanceRef>(),
          ConformingType->getNumElements()};
}

bool PackConformance::isCanonical() const {
  if (!ConformingType->isCanonical())
    return false;

  for (auto conformance : getPatternConformances())
    if (!conformance.isCanonical())
      return false;

  return true;
}

PackConformance *PackConformance::getCanonicalConformance() const {
  if (isCanonical())
    return const_cast<PackConformance *>(this);

  SmallVector<ProtocolConformanceRef, 4> conformances;
  for (auto conformance : getPatternConformances())
    conformances.push_back(conformance.getCanonicalConformanceRef());

  auto canonical = PackConformance::get(
      cast<PackType>(ConformingType->getCanonicalType()),
      Protocol, conformances);

  assert(canonical->isCanonical());
  return canonical;
}

/// Project the corresponding associated type from each pack element
/// of the conforming type, collecting the results into a new pack type
/// that has the same pack expansion structure as the conforming type.
PackType *PackConformance::getTypeWitness(
    AssociatedTypeDecl *assocType,
    SubstOptions options) const {
  SmallVector<Type, 4> packElements;

  auto conformances = getPatternConformances();
  for (unsigned i : indices(conformances)) {
    auto packElement = ConformingType->getElementType(i);

    // If the pack element is a pack expansion, build a new pack expansion
    // with the same count type as the original element, and the pattern
    // type replaced with the associated type witness from the pattern
    // conformance.
    if (auto *packExpansion = packElement->getAs<PackExpansionType>()) {
      auto assocTypePattern =
        conformances[i].getTypeWitness(assocType, options);

      packElements.push_back(PackExpansionType::get(
          assocTypePattern, packExpansion->getCountType()));

    // If the pack element is a scalar type, replace the scalar type with
    // the associated type witness from the pattern conformance.
    } else {
      auto assocTypeScalar = conformances[i].getTypeWitness(assocType, options);
      packElements.push_back(assocTypeScalar);
    }
  }

  return PackType::get(Protocol->getASTContext(), packElements);
}

/// Project the corresponding associated conformance from each pack element
/// of the conforming type, collecting the results into a new pack conformnace
/// whose conforming type has the same pack expansion structure as our
/// conforming type.
PackConformance *PackConformance::getAssociatedConformance(
    Type assocType, ProtocolDecl *protocol) const {
  SmallVector<Type, 4> packElements;
  SmallVector<ProtocolConformanceRef, 4> packConformances;

  auto conformances = getPatternConformances();
  for (unsigned i : indices(conformances)) {
    auto packElement = ConformingType->getElementType(i);

    if (auto *packExpansion = packElement->getAs<PackExpansionType>()) {
      auto assocConformancePattern =
        conformances[i].getAssociatedConformance(assocType, protocol);
      packConformances.push_back(assocConformancePattern);

      auto assocTypePattern = assocConformancePattern.getType();
      packElements.push_back(PackExpansionType::get(
          assocTypePattern, packExpansion->getCountType()));
    } else {
      auto assocConformanceScalar =
        conformances[i].getAssociatedConformance(assocType, protocol);
      packConformances.push_back(assocConformanceScalar);

      auto assocTypeScalar = assocConformanceScalar.getType();
      packElements.push_back(assocTypeScalar);
    }
  }

  auto conformingType = PackType::get(Protocol->getASTContext(), packElements);
  return PackConformance::get(conformingType, protocol, packConformances);
}

ProtocolConformanceRef PackConformance::subst(SubstitutionMap subMap,
                                              SubstOptions options) const {
  InFlightSubstitutionViaSubMap IFS(subMap, options);
  return subst(IFS);
}

ProtocolConformanceRef PackConformance::subst(TypeSubstitutionFn subs,
                                              LookupConformanceFn conformances,
                                              SubstOptions options) const {
  InFlightSubstitution IFS(subs, conformances, options);
  return subst(IFS);
}

ProtocolConformanceRef
PackConformance::subst(InFlightSubstitution &IFS) const {
  // Results built up by the expansion.
  SmallVector<Type, 4> substElementTypes;
  SmallVector<ProtocolConformanceRef, 4> substConformances;

  auto origConformances = getPatternConformances();
  assert(ConformingType->getNumElements() == origConformances.size());

  for (auto i : range(ConformingType->getNumElements())) {
    auto origElementType = ConformingType->getElementType(i);
    if (auto *origExpansion = origElementType->getAs<PackExpansionType>()) {
      // Substitute and expand an expansion element of the original pack.
      IFS.expandPackExpansionType(origExpansion,
                                  [&](Type substComponentType) {
        substElementTypes.push_back(substComponentType);

        // Just substitute the conformance.  We don't directly represent
        // pack expansion conformances here; it's sort of implicit in the
        // corresponding pack element type.
        substConformances.push_back(origConformances[i].subst(IFS));
      });
    } else {
      // Substitute a scalar element of the original pack.
      substElementTypes.push_back(origElementType.subst(IFS));
      substConformances.push_back(origConformances[i].subst(IFS));
    }
  }

  auto &ctx = Protocol->getASTContext();
  auto *substConformingType = PackType::get(ctx, substElementTypes);

  auto substConformance = PackConformance::get(substConformingType, Protocol,
                                               substConformances);
  return ProtocolConformanceRef(substConformance);
}

void language::simple_display(toolchain::raw_ostream &out, PackConformance *conformance) {
  out << conformance->getType() << " : {";
  bool first = true;
  for (auto patternConformance : conformance->getPatternConformances()) {
    if (first) {
      out << ", ";
      first = false;
    }
    simple_display(out, patternConformance);
  }
  out << "}";
}
