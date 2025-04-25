//===--- SupportedFeatures.cpp - Supported features printing --------------===//
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

#include <array>
#include <vector>

#include "language/Basic/Feature.h"
#include "language/Frontend/Frontend.h"

#include "llvm/Support/raw_ostream.h"

using namespace language;

namespace language {
namespace features {
/// Print information about what features upcoming/experimental are
/// supported by the compiler.
/// The information includes whether a feature is adoptable and for
/// upcoming features - what is the first mode it's introduced.
void printSupportedFeatures(llvm::raw_ostream &out) {
  std::array upcoming{
#define LANGUAGE_FEATURE(FeatureName, SENumber, Description)
#define UPCOMING_FEATURE(FeatureName, SENumber, Version) Feature::FeatureName,
#include "language/Basic/Features.def"
  };

  std::vector<swift::Feature> experimental{{
#define LANGUAGE_FEATURE(FeatureName, SENumber, Description)
#define EXPERIMENTAL_FEATURE(FeatureName, AvailableInProd) Feature::FeatureName,
#include "language/Basic/Features.def"
  }};

  // Include only experimental features that are available in production.
  llvm::erase_if(experimental, [](auto &feature) {
    return feature.isAvailableInProduction();
  });

  out << "{\n";
  auto printFeature = [&out](const Feature &feature) {
    out << "      ";
    out << "{ \"name\": \"" << feature.getName() << "\"";
    if (feature.isAdoptable()) {
      out << ", \"migratable\": true";
    }
    if (auto version = feature.getLanguageVersion()) {
      out << ", \"enabled_in\": \"" << *version << "\"";
    }
    out << " }";
  };

  out << "  \"features\": {\n";
  out << "    \"upcoming\": [\n";
  llvm::interleave(upcoming, printFeature, [&out] { out << ",\n"; });
  out << "\n    ],\n";

  out << "    \"experimental\": [\n";
  llvm::interleave(experimental, printFeature, [&out] { out << ",\n"; });
  out << "\n    ]\n";

  out << "  }\n";
  out << "}\n";
}

} // end namespace features
} // end namespace language
