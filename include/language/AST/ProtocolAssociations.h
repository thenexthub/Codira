//===--- ProtocolAssociations.h - Associated types/conformances -*- C++ -*-===//
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
// This file defines types for representing types and conformances
// associated with a protocol.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_AST_PROTOCOLASSOCIATIONS_H
#define LANGUAGE_AST_PROTOCOLASSOCIATIONS_H

#include "language/AST/Decl.h"
#include "toolchain/ADT/DenseMapInfo.h"

namespace language {

/// A base conformance of a protocol.
class BaseConformance {
  ProtocolDecl *Source;
  ProtocolDecl *Requirement;

public:
  explicit BaseConformance(ProtocolDecl *source,
                                 ProtocolDecl *requirement)
      : Source(source), Requirement(requirement) {
    assert(source && requirement);
  }

  ProtocolDecl *getSourceProtocol() const {
    return Source;
  }

  ProtocolDecl *getBaseRequirement() const {
    return Requirement;
  }
};

/// A conformance associated with a protocol.
class AssociatedConformance {
  ProtocolDecl *Source;
  CanType Association;
  ProtocolDecl *Requirement;

  using SourceInfo = toolchain::DenseMapInfo<ProtocolDecl*>;

  explicit AssociatedConformance(ProtocolDecl *specialValue)
      : Source(specialValue), Association(CanType()), Requirement(nullptr) {}

public:
  explicit AssociatedConformance(ProtocolDecl *source, CanType association,
                                 ProtocolDecl *requirement)
      : Source(source), Association(association), Requirement(requirement) {
    assert(source && association && requirement);
  }

  ProtocolDecl *getSourceProtocol() const {
    return Source;
  }

  CanType getAssociation() const {
    return Association;
  }

  ProtocolDecl *getAssociatedRequirement() const {
    return Requirement;
  }

  friend bool operator==(const AssociatedConformance &lhs,
                         const AssociatedConformance &rhs) {
    return lhs.Source == rhs.Source &&
           lhs.Association == rhs.Association &&
           lhs.Requirement == rhs.Requirement;
  }
  friend bool operator!=(const AssociatedConformance &lhs,
                         const AssociatedConformance &rhs) {
    return !(lhs == rhs);
  }

  unsigned getHashValue() const {
    return hash_value(toolchain::hash_combine(Source,
                                         Association.getPointer(),
                                         Requirement));
  }

  static AssociatedConformance getEmptyKey() {
    return AssociatedConformance(SourceInfo::getEmptyKey());
  }
  static AssociatedConformance getTombstoneKey() {
    return AssociatedConformance(SourceInfo::getTombstoneKey());
  }
};

} // end namespace language

namespace toolchain {
  template <> struct DenseMapInfo<language::AssociatedConformance> {
    static inline language::AssociatedConformance getEmptyKey() {
      return language::AssociatedConformance::getEmptyKey();
    }

    static inline language::AssociatedConformance getTombstoneKey() {
      return language::AssociatedConformance::getTombstoneKey();
    }

    static unsigned getHashValue(language::AssociatedConformance val) {
      return val.getHashValue();
    }

    static bool isEqual(language::AssociatedConformance lhs,
                        language::AssociatedConformance rhs) {
      return lhs == rhs;
    }
  };
}

#endif
