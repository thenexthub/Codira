//===--- Feature.cpp --------------------------------------------*- C++ -*-===//
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

#include "language/Basic/Feature.h"
#include "toolchain/ADT/StringSwitch.h"
#include "toolchain/Support/ErrorHandling.h"

using namespace language;

bool Feature::isAvailableInProduction() const {
  switch (kind) {
#define LANGUAGE_FEATURE(FeatureName, SENumber, Description)                   \
  case Feature::FeatureName:                                                   \
    return true;
#define EXPERIMENTAL_FEATURE(FeatureName, AvailableInProd)                     \
  case Feature::FeatureName:                                                   \
    return AvailableInProd;
#include "language/Basic/Features.def"
  }
  toolchain_unreachable("covered switch");
}

toolchain::StringRef Feature::getName() const {
  switch (kind) {
#define LANGUAGE_FEATURE(FeatureName, SENumber, Description)                   \
  case Feature::FeatureName:                                                   \
    return #FeatureName;
#include "language/Basic/Features.def"
  }
  toolchain_unreachable("covered switch");
}

std::optional<Feature> Feature::getUpcomingFeature(toolchain::StringRef name) {
  return toolchain::StringSwitch<std::optional<Feature>>(name)
#define LANGUAGE_FEATURE(FeatureName, SENumber, Description)
#define UPCOMING_FEATURE(FeatureName, SENumber, Version)                       \
  .Case(#FeatureName, Feature::FeatureName)
#include "language/Basic/Features.def"
      .Default(std::nullopt);
}

std::optional<Feature> Feature::getExperimentalFeature(toolchain::StringRef name) {
  return toolchain::StringSwitch<std::optional<Feature>>(name)
#define LANGUAGE_FEATURE(FeatureName, SENumber, Description)
#define EXPERIMENTAL_FEATURE(FeatureName, AvailableInProd)                     \
  .Case(#FeatureName, Feature::FeatureName)
#include "language/Basic/Features.def"
      .Default(std::nullopt);
}

std::optional<unsigned> Feature::getLanguageVersion() const {
  switch (kind) {
#define LANGUAGE_FEATURE(FeatureName, SENumber, Description)
#define UPCOMING_FEATURE(FeatureName, SENumber, Version)                       \
  case Feature::FeatureName:                                                   \
    return Version;
#include "language/Basic/Features.def"
  default:
    return std::nullopt;
  }
}

bool Feature::isMigratable() const {
  switch (kind) {
#define MIGRATABLE_UPCOMING_FEATURE(FeatureName, SENumber, Version)
#define MIGRATABLE_EXPERIMENTAL_FEATURE(FeatureName, AvailableInProd)
#define MIGRATABLE_OPTIONAL_LANGUAGE_FEATURE(FeatureName, SENumber, Name)
#define LANGUAGE_FEATURE(FeatureName, SENumber, Description)                   \
  case Feature::FeatureName:
#include "language/Basic/Features.def"
    return false;
#define LANGUAGE_FEATURE(FeatureName, SENumber, Description)
#define MIGRATABLE_UPCOMING_FEATURE(FeatureName, SENumber, Version)            \
  case Feature::FeatureName:
#define MIGRATABLE_EXPERIMENTAL_FEATURE(FeatureName, AvailableInProd)          \
  case Feature::FeatureName:
#define MIGRATABLE_OPTIONAL_LANGUAGE_FEATURE(FeatureName, SENumber, Name)      \
  case Feature::FeatureName:
#include "language/Basic/Features.def"
    return true;
  }
}

bool Feature::includeInModuleInterface() const {
  switch (kind) {
#define LANGUAGE_FEATURE(FeatureName, SENumber, Description)                   \
  case Feature::FeatureName:                                                   \
    return true;
#define EXPERIMENTAL_FEATURE_EXCLUDED_FROM_MODULE_INTERFACE(FeatureName,       \
                                                            AvailableInProd)   \
  case Feature::FeatureName:                                                   \
    return false;
#include "language/Basic/Features.def"
  }
  toolchain_unreachable("covered switch");
}
