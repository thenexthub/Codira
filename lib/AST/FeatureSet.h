//===--- FeatureSet.h - Language feature support ----------------*- C++ -*-===//
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

#ifndef LANGUAGE_AST_FEATURES_H
#define LANGUAGE_AST_FEATURES_H

#include "language/AST/Decl.h"
#include "language/Basic/Feature.h"
#include "language/Basic/FixedBitSet.h"

namespace language {

using BasicFeatureSet =
    FixedBitSet<Feature::getNumFeatures(), Feature::InnerKind>;

class FeatureSet {
  BasicFeatureSet required;

  // Stored inverted: index i actually represents
  // Feature(numFeatures() - i)
  //
  // This is the easiest way of letting us iterate from largest to
  // smallest, i.e. from the newest to the oldest feature, which is
  // the order in which we need to emit #if clauses.
  using SuppressibleFeatureSet = FixedBitSet<Feature::getNumFeatures(), size_t>;
  SuppressibleFeatureSet suppressible;

public:
  class SuppressibleGenerator {
    SuppressibleFeatureSet::iterator i, e;
    friend class FeatureSet;
    SuppressibleGenerator(const SuppressibleFeatureSet &set)
        : i(set.begin()), e(set.end()) {}

  public:
    bool empty() const { return i == e; }
    Feature next() { return Feature(Feature::getNumFeatures() - *i++); }
  };

  bool empty() const { return required.empty() && suppressible.empty(); }

  bool hasAnyRequired() const { return !required.empty(); }
  const BasicFeatureSet &requiredFeatures() const { return required; }

  bool hasAnySuppressible() const { return !suppressible.empty(); }
  SuppressibleGenerator generateSuppressibleFeatures() const {
    return SuppressibleGenerator(suppressible);
  }

  enum InsertOrRemove : bool { Insert = true, Remove = false };

  void collectFeaturesUsed(Decl *decl, InsertOrRemove operation);

private:
  void collectRequiredFeature(Feature feature, InsertOrRemove operation);
  void collectSuppressibleFeature(Feature feature, InsertOrRemove operation);
};

/// Get the set of features that are uniquely used by this declaration, and are
/// not part of the enclosing context.
FeatureSet getUniqueFeaturesUsed(Decl *decl);

bool usesFeatureIsolatedDeinit(const Decl *decl);

} // end namespace language

#endif /* LANGUAGE_AST_FEATURES_H */
