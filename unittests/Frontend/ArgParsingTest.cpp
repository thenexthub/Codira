//===-- ArgParsingTest.cpp --------------------------------------*- C++ -*-===//
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

#include "ArgParsingTest.h"

using namespace language;

ArgParsingTest::ArgParsingTest() : diags(sourceMgr) {}

void ArgParsingTest::parseArgs(const Args &args) {
  std::vector<const char *> adjustedArgs;

  if (this->langMode) {
    adjustedArgs.reserve(args.size() + 2);
    adjustedArgs.push_back("-language-version");
    adjustedArgs.push_back(this->langMode->data());
  } else {
    adjustedArgs.reserve(args.size());
  }

  for (auto &arg : args) {
    adjustedArgs.push_back(arg.data());
  }

  this->invocation.parseArgs(adjustedArgs, this->diags);
}

LangOptions &ArgParsingTest::getLangOptions() {
  return this->invocation.getLangOptions();
}

void PrintTo(const Args &value, std::ostream *os) {
  *os << '"';

  if (!value.empty()) {
    const auto lastIdx = value.size() - 1;
    for (size_t idx = 0; idx != lastIdx; ++idx) {
      *os << value[idx] << ' ';
    }
    *os << value[lastIdx];
  }

  *os << '"';
}
