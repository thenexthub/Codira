//===--- Fulfillment.h - Deriving type/conformance metadata -----*- C++ -*-===//
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
// This file defines interfaces for deriving type metadata and protocol
// witness tables from various sources.
// 
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_IRGEN_FULFILLMENT_H
#define LANGUAGE_IRGEN_FULFILLMENT_H

#include "toolchain/ADT/MapVector.h"
#include "language/AST/GenericSignature.h"
#include "language/AST/Types.h"
#include "language/IRGen/GenericRequirement.h"
#include "MetadataPath.h"

namespace language {
namespace irgen {
  class IRGenModule;
  enum IsExact_t : bool;

/// The metadata value can be fulfilled by following the given metadata
/// path from the given source.
struct Fulfillment {
  Fulfillment() = default;
  Fulfillment(unsigned sourceIndex, MetadataPath &&path, MetadataState state)
    : SourceIndex(sourceIndex), State(unsigned(state)), Path(std::move(path)) {}

  /// The source index.
  uint64_t SourceIndex : 56;

  /// The state of the metadata at the fulfillment.
  uint64_t State : 8;

  /// The path from the source metadata.
  MetadataPath Path;

  MetadataState getState() const { return MetadataState(State); }
};

class FulfillmentMap {
  toolchain::MapVector<GenericRequirement, Fulfillment> Fulfillments;

public:
  struct InterestingKeysCallback {
    /// Is the given type something that we should add fulfillments for?
    virtual bool isInterestingType(CanType type) const = 0;

    /// Is the given type expressed in terms of types that we should add
    /// fulfillments for?
    ///
    /// It's okay to conservatively return true here.
    virtual bool hasInterestingType(CanType type) const = 0;

    /// Is the given pack expansion a simple expansion of an interesting
    /// type?
    virtual bool isInterestingPackExpansion(CanPackExpansionType type) const = 0;

    /// Are we only interested in a subset of the conformances for a
    /// given type?
    virtual bool hasLimitedInterestingConformances(CanType type) const = 0;

    /// Return the limited interesting conformances for an interesting type.
    virtual GenericSignature::RequiredProtocols
      getInterestingConformances(CanType type) const = 0;

    /// Return the limited interesting conformances for an interesting type.
    virtual CanType getSuperclassBound(CanType type) const = 0;

    virtual ~InterestingKeysCallback() = default;
  };

  FulfillmentMap() = default;

  using iterator = decltype(Fulfillments)::iterator;
  iterator begin() { return Fulfillments.begin(); }
  iterator end() { return Fulfillments.end(); }

  /// Is it even theoretically possible that we might find a fulfillment
  /// in the given type?
  static bool isInterestingTypeForFulfillments(CanType type) {
    // Some day, if we ever record fulfillments for concrete types, this
    // optimization will probably no longer be useful.
    return type->hasTypeParameter();
  }

  /// Search the given type metadata for useful fulfillments.
  ///
  /// \return true if any fulfillments were added by this search.
  bool searchTypeMetadata(IRGenModule &IGM, CanType type, IsExact_t isExact,
                          MetadataState metadataState,
                          unsigned sourceIndex, MetadataPath &&path,
                          const InterestingKeysCallback &interestingKeys);

  /// Metadata fulfillment in tuple conformance witness thunks.
  ///
  /// \return true if any fulfillments were added by this search.
  bool searchTupleTypeMetadata(IRGenModule &IGM, CanTupleType type, IsExact_t isExact,
                               MetadataState metadataState,
                               unsigned sourceIndex, MetadataPath &&path,
                               const InterestingKeysCallback &interestingKeys);

  /// Search the given type metadata pack for useful fulfillments.
  ///
  /// \return true if any fulfillments were added by this search.
  bool searchTypeMetadataPack(IRGenModule &IGM, CanPackType type,
                              IsExact_t isExact,
                              MetadataState metadataState,
                              unsigned sourceIndex, MetadataPath &&path,
                              const InterestingKeysCallback &interestingKeys);

  bool searchConformance(IRGenModule &IGM,
                         const ProtocolConformance *conformance,
                         unsigned sourceIndex, MetadataPath &&path,
                         const InterestingKeysCallback &interestingKeys);

  /// Search the given witness table for useful fulfillments.
  ///
  /// \return true if any fulfillments were added by this search.
  bool searchWitnessTable(IRGenModule &IGM, CanType type, ProtocolDecl *protocol,
                          unsigned sourceIndex, MetadataPath &&path,
                          const InterestingKeysCallback &interestingKeys);

  /// Consider a shape requirement for the given type.
  bool searchShapeRequirement(IRGenModule &IGM, CanType type,
                              unsigned sourceIndex, MetadataPath &&path);

  /// Register a fulfillment for the given key.
  ///
  /// \return true if the fulfillment was added, which won't happen if there's
  ///   already a fulfillment that was at least as good
  bool addFulfillment(GenericRequirement key, unsigned source,
                      MetadataPath &&path, MetadataState state);

  const Fulfillment *getFulfillment(GenericRequirement key) const {
    auto it = Fulfillments.find(key);
    if (it != Fulfillments.end()) {
      return &it->second;
    } else {
      return nullptr;
    }
  }

  const Fulfillment *getShape(CanType type) const {
    return getFulfillment(GenericRequirement::forShape(type));
  }

  const Fulfillment *getTypeMetadata(CanType type) const {
    return getFulfillment(GenericRequirement::forMetadata(type));
  }

  const Fulfillment *getWitnessTable(CanType type, ProtocolDecl *proto) const {
    return getFulfillment(GenericRequirement::forWitnessTable(type, proto));
  }

  void dump() const;
  void print(toolchain::raw_ostream &out) const;
  friend toolchain::raw_ostream &operator<<(toolchain::raw_ostream &out,
                                       const FulfillmentMap &map) {
    map.print(out);
    return out;
  }

private:
  bool searchNominalTypeMetadata(IRGenModule &IGM, CanType type,
                                 MetadataState metadataState, unsigned source,
                                 MetadataPath &&path,
                                 const InterestingKeysCallback &keys);

  /// Search the given witness table for useful fulfillments.
  ///
  /// \return true if any fulfillments were added by this search.
  bool searchWitnessTable(
      IRGenModule &IGM, CanType type, ProtocolDecl *protocol, unsigned source,
      MetadataPath &&path, const InterestingKeysCallback &keys,
      toolchain::SmallPtrSetImpl<ProtocolDecl *> *interestingConformances);
};

}
}

#endif
