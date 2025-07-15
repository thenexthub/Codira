//===--- EditorPlaceholderTest.cpp ----------------------------------------===//
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

#include "language/Basic/EditorPlaceholder.h"
#include "gtest/gtest.h"
#include <optional>

using namespace language;

TEST(EditorPlaceholder, EditorPlaceholders) {
  const char *Text = "<#this is simple#>";
  EditorPlaceholderData Data = *parseEditorPlaceholder(Text);
  EXPECT_EQ(EditorPlaceholderKind::Basic, Data.Kind);
  EXPECT_EQ("this is simple", Data.Display);
  EXPECT_TRUE(Data.Type.empty());
  EXPECT_TRUE(Data.TypeForExpansion.empty());

  Text = "<#T##x: Int##Int#>";
  Data = *parseEditorPlaceholder(Text);
  EXPECT_EQ(EditorPlaceholderKind::Typed, Data.Kind);
  EXPECT_EQ("x: Int", Data.Display);
  EXPECT_EQ("Int", Data.Type);
  EXPECT_EQ("Int", Data.TypeForExpansion);

  Text = "<#T##x: Int##Blah##()->Int#>";
  Data = *parseEditorPlaceholder(Text);
  EXPECT_EQ(EditorPlaceholderKind::Typed, Data.Kind);
  EXPECT_EQ("x: Int", Data.Display);
  EXPECT_EQ("Blah", Data.Type);
  EXPECT_EQ("()->Int", Data.TypeForExpansion);

  Text = "<#T##Int#>";
  Data = *parseEditorPlaceholder(Text);
  EXPECT_EQ(EditorPlaceholderKind::Typed, Data.Kind);
  EXPECT_EQ("Int", Data.Display);
  EXPECT_EQ("Int", Data.Type);
  EXPECT_EQ("Int", Data.TypeForExpansion);
}

TEST(EditorPlaceholder, InvalidEditorPlaceholders) {
  const char *Text = "<#foo";
  std::optional<EditorPlaceholderData> DataOpt = parseEditorPlaceholder(Text);
  EXPECT_FALSE(DataOpt.has_value());

  Text = "foo#>";
  DataOpt = parseEditorPlaceholder(Text);
  EXPECT_FALSE(DataOpt.has_value());

  Text = "#foo#>";
  DataOpt = parseEditorPlaceholder(Text);
  EXPECT_FALSE(DataOpt.has_value());

  Text = " <#foo#>";
  DataOpt = parseEditorPlaceholder(Text);
  EXPECT_FALSE(DataOpt.has_value());
}
