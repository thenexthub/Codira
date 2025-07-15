//===-- ProtocolConformanceAnalysis.cpp - Protocol Conformance Analysis ---===//
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
// This analysis collects a set of nominal types (classes, structs, and enums)
// that conform to a protocol during whole module compilation. We only track
// protocols that are non-public.

#include "language/SILOptimizer/Analysis/ProtocolConformanceAnalysis.h"
#include "language/AST/ASTContext.h"
#include "language/AST/ASTWalker.h"
#include "language/AST/Module.h"
#include "language/AST/ProtocolConformance.h"
#include "language/SIL/SILInstruction.h"
#include "language/SIL/SILModule.h"
#include "language/SIL/SILValue.h"

using namespace language;

namespace {
/// A helper class to collect all nominal type declarations.
class NominalTypeWalker : public ASTWalker {
  ProtocolConformanceAnalysis::ProtocolConformanceMap &ProtocolConformanceCache;

public:
  NominalTypeWalker(ProtocolConformanceAnalysis::ProtocolConformanceMap
                    &ProtocolConformanceCache)
      : ProtocolConformanceCache(ProtocolConformanceCache) {}

  /// Walk everything in a macro
  MacroWalking getMacroWalkingBehavior() const override {
    return MacroWalking::ArgumentsAndExpansion;
  }

  PreWalkAction walkToDeclPre(Decl *D) override {
    /// (1) Walk over all NominalTypeDecls to determine conformances.
    if (auto *NTD = dyn_cast<NominalTypeDecl>(D)) {
      if (!isa<ProtocolDecl>(NTD)) {
        auto Protocols = NTD->getAllProtocols();
        for (auto &Protocol : Protocols) {
          if (Protocol->getEffectiveAccess() <= AccessLevel::Internal) {
            ProtocolConformanceCache[Protocol].push_back(NTD);
          }
        }
      }
    }
    /// (2) Walk over all ExtensionDecls to determine conformances.
    if (auto *e = dyn_cast<ExtensionDecl>(D)) {
      auto *ntd = e->getExtendedNominal();
      if (ntd && !isa<ProtocolDecl>(ntd)) {
        for (auto *conformance : e->getLocalConformances()) {
          if (isa<NormalProtocolConformance>(conformance)) {
            auto *proto = conformance->getProtocol();
            if (proto->getEffectiveAccess() <= AccessLevel::Internal) {
              ProtocolConformanceCache[proto].push_back(ntd);
            }
          }
        }
      }
    }
    return Action::Continue();
  }
};
} // end anonymous namespace

// FIXME: This could be implemented as a request.
void ProtocolConformanceAnalysis::populateConformanceCacheIfNecessary() {
  if (ProtocolConformanceCache.has_value())
    return;

  ProtocolConformanceCache.emplace();

  // We only do this in Whole-Module compilation mode.
  if (!M->isWholeModule())
    return;

  // Process all types implementing protocols.
  SmallVector<Decl *, 32> Decls;

  // Find all top level declarations.
  M->getCodiraModule()->getTopLevelDecls(Decls);

  /// This operation is quadratic and should only be performed
  /// in whole module compilation!
  NominalTypeWalker Walker(*ProtocolConformanceCache);
  for (auto *D : Decls) {
    D->walk(Walker);
  }
}

/// Recursively traverse the conformance lists to determine sole conforming
/// class, struct or enum type.
NominalTypeDecl *
ProtocolConformanceAnalysis::findSoleConformingType(ProtocolDecl *Protocol) {

  /// First check in the SoleConformingTypeCache.
  auto SoleConformingTypeIt = SoleConformingTypeCache.find(Protocol);
  if (SoleConformingTypeIt != SoleConformingTypeCache.end())
    return SoleConformingTypeIt->second;

  SmallVector<ProtocolDecl *, 8> PDWorkList;
  SmallPtrSet<ProtocolDecl *, 8> VisitedPDs;
  NominalTypeDecl *SoleConformingNTD = nullptr;
  PDWorkList.push_back(Protocol);
  while (!PDWorkList.empty()) {
    auto *PD = PDWorkList.pop_back_val();
    // Protocols must have internal or lower access.
    if (PD->getEffectiveAccess() > AccessLevel::Internal) {
      return nullptr;
    }
    VisitedPDs.insert(PD);
    auto NTDList = getConformances(PD);
    for (auto *ConformingNTD : NTDList) {
      // Recurse on protocol types.
      if (auto *Proto = dyn_cast<ProtocolDecl>(ConformingNTD)) {
        // Ignore visited protocol decls.
        if (!VisitedPDs.count(Proto))
          PDWorkList.push_back(Proto);
      } else { // Classes, Structs and Enums are added here.
        // Bail if more than one conforming types were found.
        if (SoleConformingNTD && ConformingNTD != SoleConformingNTD) {
          return nullptr;
        } else {
          SoleConformingNTD = ConformingNTD;
        }
      }
    }
  }

  // Bail if we did not find a sole conforming type.
  if (!SoleConformingNTD)
    return nullptr;

  // Generic declarations are ignored.
  if (SoleConformingNTD->isGenericContext()) {
    return nullptr;
  }

  // Populate SoleConformingTypeCache.
  SoleConformingTypeCache.insert(std::pair<ProtocolDecl *, NominalTypeDecl *>(
      Protocol, SoleConformingNTD));

  // Return SoleConformingNTD.
  return SoleConformingNTD;
}

// Wrapper function to findSoleConformingType that checks for additional
// constraints for classes using ClassHierarchyAnalysis.
bool ProtocolConformanceAnalysis::getSoleConformingType(
    ProtocolDecl *Protocol, ClassHierarchyAnalysis *CHA, CanType &ConcreteType) {
  // Determine the sole conforming type.
  auto *NTD = findSoleConformingType(Protocol);
  if (!NTD)
    return false;

  // Sole conforming class should not be open access or have any derived class.
  ClassDecl *CD;
  if ((CD = dyn_cast<ClassDecl>(NTD)) &&
      (CD->getEffectiveAccess() == AccessLevel::Open ||
       CHA->hasKnownDirectSubclasses(CD))) {
    return false;
  }

  // Save the concrete type.
  ConcreteType = NTD->getDeclaredType()->getCanonicalType();
  return true;
}

ProtocolConformanceAnalysis::~ProtocolConformanceAnalysis() {}
