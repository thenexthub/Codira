//===--- TypeLookupError.cpp - TypeLookupError Tests ----------------------===//
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

#include "language/Demangling/TypeLookupError.h"
#include "gtest/gtest.h"
#include <vector>

using namespace language;

TEST(TypeLookupError, ConstantString) {
  auto error = TypeLookupError("testing testing");
  char *str = error.copyErrorString();
  ASSERT_STREQ(str, "testing testing");
  error.freeErrorString(str);
}

TEST(TypeLookupError, FormatString) {
  auto error = TYPE_LOOKUP_ERROR_FMT("%d %d %d %d %d %d %d %d %d %d", 0, 1, 2,
                                     3, 4, 5, 6, 7, 8, 9);
  char *str = error.copyErrorString();
  ASSERT_STREQ(str, "0 1 2 3 4 5 6 7 8 9");
  error.freeErrorString(str);
}

TEST(TypeLookupError, Copying) {
  std::vector<TypeLookupError> vec;

  {
    auto originalError = TYPE_LOOKUP_ERROR_FMT("%d %d %d %d %d %d %d %d %d %d",
                                               0, 1, 2, 3, 4, 5, 6, 7, 8, 9);
    for (int i = 0; i < 5; i++)
      vec.push_back(originalError);
  }

  for (auto &error : vec) {
    char *str = error.copyErrorString();
    ASSERT_STREQ(str, "0 1 2 3 4 5 6 7 8 9");
    error.freeErrorString(str);
  }

  auto extractedError = vec[4];
  vec.clear();
  char *str = extractedError.copyErrorString();
  ASSERT_STREQ(str, "0 1 2 3 4 5 6 7 8 9");
  extractedError.freeErrorString(str);
}
