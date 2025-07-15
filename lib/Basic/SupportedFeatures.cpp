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

#include "language/AST/DiagnosticGroups.h"
#include "language/Basic/Feature.h"
#include "language/Frontend/Frontend.h"

#include "toolchain/Support/raw_ostream.h"

using namespace language;

namespace language {
namespace features {

/// The subset of diagnostic groups (called categories by the diagnostic machinery) whose diagnostics should be
/// considered to be part of the migration for this feature.
///
///  When making a feature migratable, ensure that all of the warnings that are used to drive the migration are
///  part of a diagnostic group, and put that diagnostic group into the list for that feature here.
static std::vector<DiagGroupID> migratableCategories(Feature feature) {
  switch (feature) {
    case Feature::InnerKind::ExistentialAny:
      return { DiagGroupID::ExistentialAny };
    case Feature::InnerKind::InferIsolatedConformances:
      return { DiagGroupID::IsolatedConformances };
    case Feature::InnerKind::MemberImportVisibility:
      return { DiagGroupID::MemberImportVisibility };
    case Feature::InnerKind::NonisolatedNonsendingByDefault:
      return { DiagGroupID::NonisolatedNonsendingByDefault };
    case Feature::InnerKind::StrictMemorySafety:
      return { DiagGroupID::StrictMemorySafety };

    // Provide unreachable cases for all of the non-migratable features.
#define LANGUAGE_FEATURE(FeatureName, SENumber, Description) case Feature::FeatureName:
#define MIGRATABLE_UPCOMING_FEATURE(FeatureName, SENumber, Version)
#define MIGRATABLE_EXPERIMENTAL_FEATURE(FeatureName, AvailableInProd)
#define MIGRATABLE_OPTIONAL_LANGUAGE_FEATURE(FeatureName, SENumber, Name)
#include "language/Basic/Features.def"
    toolchain_unreachable("Not a migratable feature");
  }
}

/// For optional language features, return the flag name used by the compiler to enable the feature. For all others,
/// returns an empty optional.
static std::optional<std::string_view> optionalFlagName(Feature feature) {
  switch (feature) {
  case Feature::StrictMemorySafety:
    return "-strict-memory-safety";

#define LANGUAGE_FEATURE(FeatureName, SENumber, Description) case Feature::FeatureName:
#define OPTIONAL_LANGUAGE_FEATURE(FeatureName, SENumber, Description)
#include "language/Basic/Features.def"
    return std::nullopt;
  }
}

/// Print information about what features upcoming/experimental are
/// supported by the compiler.
/// The information includes whether a feature is adoptable and for
/// upcoming features - what is the first mode it's introduced.
void printSupportedFeatures(toolchain::raw_ostream &out) {
  std::array optional{
#define LANGUAGE_FEATURE(FeatureName, SENumber, Description)
#define OPTIONAL_LANGUAGE_FEATURE(FeatureName, SENumber, Description) Feature::FeatureName,
#include "language/Basic/Features.def"
  };

  std::array upcoming{
#define LANGUAGE_FEATURE(FeatureName, SENumber, Description)
#define UPCOMING_FEATURE(FeatureName, SENumber, Version) Feature::FeatureName,
#include "language/Basic/Features.def"
  };

  std::vector<language::Feature> experimental{{
#define LANGUAGE_FEATURE(FeatureName, SENumber, Description)
#define EXPERIMENTAL_FEATURE(FeatureName, AvailableInProd) Feature::FeatureName,
#include "language/Basic/Features.def"
  }};

  // Include only experimental features that are available in production.
  toolchain::erase_if(experimental, [](auto &feature) {
    return feature.isAvailableInProduction();
  });

  out << "{\n";
  auto printFeature = [&out](const Feature &feature) {
    out << "      ";
    out << "{ \"name\": \"" << feature.getName() << "\"";
    if (feature.isMigratable()) {
      out << ", \"migratable\": true";

      auto categories = migratableCategories(feature);
      out << ", \"categories\": [";
      toolchain::interleave(categories, [&out](DiagGroupID diagGroupID) {
        out << "\"" << getDiagGroupInfoByID(diagGroupID).name << "\"";
      }, [&out] {
        out << ", ";
      });
      out << "]";
    }
    if (auto version = feature.getLanguageVersion()) {
      out << ", \"enabled_in\": \"" << *version << "\"";
    }

    if (auto flagName = optionalFlagName(feature)) {
      out << ", \"flag_name\": \"" << *flagName << "\"";
    }

    out << " }";
  };

  out << "  \"features\": {\n";
  out << "    \"optional\": [\n";
  toolchain::interleave(optional, printFeature, [&out] { out << ",\n"; });
  out << "\n    ],\n";

  out << "    \"upcoming\": [\n";
  toolchain::interleave(upcoming, printFeature, [&out] { out << ",\n"; });
  out << "\n    ],\n";

  out << "    \"experimental\": [\n";
  toolchain::interleave(experimental, printFeature, [&out] { out << ",\n"; });
  out << "\n    ]\n";

  out << "  }\n";
  out << "}\n";
}

} // end namespace features
} // end namespace language
