//===--- StringExtrasTest.cpp ---------------------------------------------===//
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

#include "language/Basic/StringExtras.h"
#include "toolchain/ADT/SmallString.h"
#include "gtest/gtest.h"
#include <algorithm>

using namespace language;

TEST(CamelCaseWordsTest, Iteration) {
  auto words = camel_case::getWords("URLByPrependingHTTPToURL");

  // Forward iteration count.
  EXPECT_EQ(6, std::distance(words.begin(), words.end()));

  // Reverse iteration count.
  EXPECT_EQ(6, std::distance(words.rbegin(), words.rend()));

  // Iteration contents.
  auto iter = words.begin();
  EXPECT_EQ(*iter, "URL");
  
  // Stepping forward.
  ++iter;
  EXPECT_EQ(*iter, "By");  

  // Immediately stepping back (fast path).
  --iter;
  EXPECT_EQ(*iter, "URL");  

  // Immediately stepping forward (fast path).
  ++iter;
  EXPECT_EQ(*iter, "By");

  // Stepping forward.
  ++iter;
  EXPECT_EQ(*iter, "Prepending");  

  // Stepping back twice (slow path).
  --iter;
  --iter;
  EXPECT_EQ(*iter, "URL");

  // Stepping forward to visit the remaining elements.
  ++iter;
  EXPECT_EQ(*iter, "By");
  ++iter;
  EXPECT_EQ(*iter, "Prepending");
  ++iter;
  EXPECT_EQ(*iter, "HTTP");
  ++iter;
  EXPECT_EQ(*iter, "To");
  ++iter;
  EXPECT_EQ(*iter, "URL");

  // We're done.
  ++iter;
  EXPECT_EQ(iter, words.end());
}

TEST(CamelCaseWordsTest, PluralAcronyms) {
  auto words = camel_case::getWords("CreateURLsFromPIXies");

  // Forward iteration count.
  EXPECT_EQ(4, std::distance(words.begin(), words.end()));

  // Reverse iteration count.
  EXPECT_EQ(4, std::distance(words.rbegin(), words.rend()));

  // Iteration contents.
  auto iter = words.begin();
  EXPECT_EQ(*iter, "Create");

  // Stepping forward.
  ++iter;
  EXPECT_EQ(*iter, "URLs");

  // Immediately stepping back.
  --iter;
  EXPECT_EQ(*iter, "Create");

  // Immediately stepping forward.
  ++iter;
  EXPECT_EQ(*iter, "URLs");

  // Stepping forward.
  ++iter;
  EXPECT_EQ(*iter, "From");

  // Stepping back twice (slow path).
  --iter;
  --iter;
  EXPECT_EQ(*iter, "Create");

  // Stepping forward to visit the remaining elements.
  ++iter;
  EXPECT_EQ(*iter, "URLs");
  ++iter;
  EXPECT_EQ(*iter, "From");
  ++iter;
  EXPECT_EQ(*iter, "PIXies");

  // We're done.
  ++iter;
  EXPECT_EQ(iter, words.end());
}

TEST(CamelCaseWordsTest, MorePluralAcronyms) {
  auto words = camel_case::getWords("inSameDayAsDate");

  // Forward iteration count.
  EXPECT_EQ(5, std::distance(words.begin(), words.end()));

  // Reverse iteration count.
  EXPECT_EQ(5, std::distance(words.rbegin(), words.rend()));

  // Iteration contents.
  auto iter = words.begin();
  EXPECT_EQ(*iter, "in");

  // Stepping forward.
  ++iter;
  EXPECT_EQ(*iter, "Same");

  // Immediately stepping back (fast path).
  --iter;
  EXPECT_EQ(*iter, "in");

  // Immediately stepping forward (fast path).
  ++iter;
  EXPECT_EQ(*iter, "Same");

  // Stepping forward.
  ++iter;
  EXPECT_EQ(*iter, "Day");

  // Stepping back twice (slow path).
  --iter;
  --iter;
  EXPECT_EQ(*iter, "in");

  // Stepping forward to visit the remaining elements.
  ++iter;
  EXPECT_EQ(*iter, "Same");
  ++iter;
  EXPECT_EQ(*iter, "Day");
  ++iter;
  EXPECT_EQ(*iter, "As");
  ++iter;
  EXPECT_EQ(*iter, "Date");

  // We're done.
  ++iter;
  EXPECT_EQ(iter, words.end());
}

TEST(CamelCaseWordsTest, WordsWithUnderscores) {
  auto words = camel_case::getWords("CF_Flags_789");
  EXPECT_EQ(5, std::distance(words.begin(), words.end()));

  auto iter = words.begin();
  EXPECT_EQ("CF", *iter++);
  EXPECT_EQ("_", *iter++);
  EXPECT_EQ("Flags", *iter++);
  EXPECT_EQ("_", *iter++);
  EXPECT_EQ("789", *iter++);
  EXPECT_EQ("789", *--iter);
  EXPECT_EQ("_", *--iter);
  EXPECT_EQ("Flags", *--iter);
  EXPECT_EQ("_", *--iter);
  EXPECT_EQ("CF", *--iter);

  auto manyUnderscores = camel_case::getWords("___ABC");
  EXPECT_EQ(4, std::distance(manyUnderscores.begin(), manyUnderscores.end()));

  iter = manyUnderscores.begin();
  EXPECT_EQ("_", *iter++);
  EXPECT_EQ("_", *iter++);
  EXPECT_EQ("_", *iter++);
  EXPECT_EQ("ABC", *iter++);
  EXPECT_EQ("ABC", *--iter);
  EXPECT_EQ("_", *--iter);
  EXPECT_EQ("_", *--iter);
  EXPECT_EQ("_", *--iter);
}

TEST(ToLowercaseTest, Words) {
  toolchain::SmallString<64> scratch;

  EXPECT_EQ(camel_case::toLowercaseWord("By", scratch), "by");
  EXPECT_EQ(camel_case::toLowercaseWord("and", scratch), "and");
  EXPECT_EQ(camel_case::toLowercaseWord("A", scratch), "a");
  EXPECT_EQ(camel_case::toLowercaseWord("URL", scratch), "URL");
  EXPECT_EQ(camel_case::toLowercaseWord("", scratch), "");
}

TEST(ToLowercaseInitialismsTest, Words) {
  toolchain::SmallString<64> scratch;

  EXPECT_EQ(camel_case::toLowercaseInitialisms("ToXML", scratch), "toXML");
  EXPECT_EQ(camel_case::toLowercaseInitialisms("URLsInHand", scratch),
            "urlsInHand");
}

TEST(ToSentencecaseTest, Words) {
  toolchain::SmallString<64> scratch;

  EXPECT_EQ(camel_case::toSentencecase("by", scratch), "By");
  EXPECT_EQ(camel_case::toSentencecase("a", scratch), "A");
  EXPECT_EQ(camel_case::toSentencecase("URL", scratch), "URL");
  EXPECT_EQ(camel_case::toSentencecase("", scratch), "");
}

