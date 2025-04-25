//===--- AbstractConformance.h - Abstract conformance storage ---*- C++ -*-===//
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
// This file defines the AbstractConformance class, which represents
// the conformance of a type parameter or archetype to a protocol.
// These are usually stashed inside a ProtocolConformanceRef.
//
//===----------------------------------------------------------------------===//
#ifndef SWIFT_AST_ABSTRACT_CONFORMANCE_H
#define SWIFT_AST_ABSTRACT_CONFORMANCE_H

#include "language/AST/Type.h"
#include "llvm/ADT/FoldingSet.h"

namespace language {
class AbstractConformance final : public llvm::FoldingSetNode {
  Type conformingType;
  ProtocolDecl *requirement;

public:
  AbstractConformance(Type conformingType, ProtocolDecl *requirement)
    : conformingType(conformingType), requirement(requirement) { }

  Type getType() const { return conformingType; }
  ProtocolDecl *getProtocol() const { return requirement; }

  void Profile(llvm::FoldingSetNodeID &id) const {
    Profile(id, getType(), getProtocol());
  }

  /// Profile the storage for this conformance, for use with LLVM's FoldingSet.
  static void Profile(llvm::FoldingSetNodeID &id,
                      Type conformingType,
                      ProtocolDecl *requirement) {
    id.AddPointer(conformingType.getPointer());
    id.AddPointer(requirement);
  }
};

}

#endif // SWIFT_AST_ABSTRACT_CONFORMANCE_H

