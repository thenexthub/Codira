//===-- FeatureParsingTest.cpp ----------------------------------*- C++ -*-===//
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

#include "FeatureParsingTest.h"

using namespace language;

const std::string FeatureParsingTest::defaultLangMode = "5";

FeatureParsingTest::FeatureParsingTest() : ArgParsingTest() {
  this->langMode = defaultLangMode;
}

FeatureWrapper::FeatureWrapper(Feature id) : id(id), name(id.getName().data()) {
  auto langMode = id.getLanguageVersion();
  if (langMode) {
    this->langMode = std::to_string(*langMode);
  }
}

void language::PrintTo(const StrictConcurrency &value, std::ostream *os) {
  switch (value) {
  case StrictConcurrency::Minimal:
    *os << "Minimal";
    break;
  case StrictConcurrency::Targeted:
    *os << "Targeted";
    break;
  case StrictConcurrency::Complete:
    *os << "Complete";
    break;
  }
}

void language::PrintTo(const LangOptions::FeatureState &value, std::ostream *os) {
  PrintTo((const LangOptions::FeatureState::Kind &)value, os);
}

void language::PrintTo(const LangOptions::FeatureState::Kind &value,
                    std::ostream *os) {
  switch (value) {
  case LangOptions::FeatureState::Kind::Off:
    *os << "Off";
    break;
  case LangOptions::FeatureState::Kind::EnabledForMigration:
    *os << "EnabledForMigration";
    break;
  case LangOptions::FeatureState::Kind::Enabled:
    *os << "Enabled";
    break;
  }
}
