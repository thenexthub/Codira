//===-- FeatureParsingTest.h ------------------------------------*- C++ -*-===//
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

#ifndef FEATURE_PARSING_TEST_H
#define FEATURE_PARSING_TEST_H

#include "ArgParsingTest.h"
#include "language/Basic/Feature.h"

struct FeatureParsingTest : public ArgParsingTest {
  static const std::string defaultLangMode;

  FeatureParsingTest();
};

struct FeatureWrapper final {
  language::Feature id;
  std::string name;
  std::string langMode;

  FeatureWrapper(language::Feature id);

  operator language::Feature() const { return id; }
};

// MARK: - Printers

// Google Test requires custom printers to be declared in the same namespace as
// the type they take.
namespace language {
void PrintTo(const StrictConcurrency &, std::ostream *);
void PrintTo(const LangOptions::FeatureState &, std::ostream *);
void PrintTo(const LangOptions::FeatureState::Kind &, std::ostream *);
} // end namespace language

#endif // FEATURE_PARSING_TEST_H
