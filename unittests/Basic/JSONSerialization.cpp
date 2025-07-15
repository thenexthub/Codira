//===--- CacheTest.cpp ----------------------------------------------------===//
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

#include "language/Basic/JSONSerialization.h"
#include "gtest/gtest.h"

namespace {
struct Leaf {
  int Val;
};

struct Root {
  std::string Name;
  std::vector<Leaf *> Children;
  std::vector<Leaf *> Empty;
  Leaf *Side;
};
} // namespace

namespace language {
namespace json {

void *IncludeSpecialValueUserInfoKey = &IncludeSpecialValueUserInfoKey;

template <> struct ObjectTraits<Leaf> {
  static void mapping(Output &out, Leaf &value) {
    switch (value.Val) {
    case 0:
      break;
    case 1:
      out.mapRequired("Value", value.Val);
      break;
    case 2: {
      std::string two("two");
      out.mapRequired("Value", two);
      break;
    }
    default:
      break;
    }
  }
};

template <> struct NullableTraits<Leaf *> {
  static bool isNull(Leaf *&value) { return !value; }
  static Leaf &get(Leaf *&value) { return *value; }
};

template <> struct ArrayTraits<std::vector<Leaf *>> {

  using value_type = Leaf *;

  static size_t size(Output &out, std::vector<Leaf *> &seq) {
    return seq.size();
  }
  static Leaf *&element(Output &out, std::vector<Leaf *> &seq, size_t index) {
    return seq[index];
  }
};

template <> struct ObjectTraits<Root> {
  static void mapping(Output &out, Root &value) {
    out.mapRequired("Name", value.Name);
    out.mapRequired("Children", value.Children);
    out.mapRequired("Empty", value.Empty);
    out.mapRequired("Side", value.Side);
    if ((bool)out.getUserInfo()[IncludeSpecialValueUserInfoKey]) {
      std::string Value = "42";
      out.mapRequired("SpecialValue", Value);
    }
  }
};

} // namespace json
} // namespace language

TEST(JSONSerialization, basicCompact) {
  Leaf LeafObj0{0};
  Leaf LeafObj1{1};
  Leaf LeafObj2{2};
  Root RootObj{"foo", {&LeafObj0, &LeafObj1, nullptr, &LeafObj2}, {}, nullptr};
  std::string Buffer;
  toolchain::raw_string_ostream Stream(Buffer);
  language::json::Output Out(Stream, /*UserInfo=*/{}, /*PrettyPrint=*/false);

  Out << RootObj;
  Stream.flush();

  EXPECT_EQ(Buffer, "{\"Name\":\"foo\",\"Children\":[{},{\"Value\":1},null,{"
                    "\"Value\":\"two\"}],\"Empty\":[],\"Side\":null}");
}

TEST(JSONSerialization, basicPretty) {
  Leaf LeafObj0{0};
  Leaf LeafObj1{1};
  Leaf LeafObj2{2};
  Root RootObj{"foo", {&LeafObj0, &LeafObj1, nullptr, &LeafObj2}, {}, nullptr};
  std::string Buffer;
  toolchain::raw_string_ostream Stream(Buffer);
  language::json::Output Out(Stream);

  Out << RootObj;
  Stream.flush();

  EXPECT_EQ(Buffer, R"""({
  "Name": "foo",
  "Children": [
    {},
    {
      "Value": 1
    },
    null,
    {
      "Value": "two"
    }
  ],
  "Empty": [],
  "Side": null
})""");
}

TEST(JSONSerialization, basicUserInfo) {
  Root RootObj{"foo", {}, {}, nullptr};
  std::string Buffer;
  toolchain::raw_string_ostream Stream(Buffer);

  language::json::Output::UserInfoMap UserInfo;
  UserInfo[language::json::IncludeSpecialValueUserInfoKey] = (void *)true;
  language::json::Output Out(Stream, UserInfo);

  Out << RootObj;
  Stream.flush();

  EXPECT_EQ(Buffer, R"""({
  "Name": "foo",
  "Children": [],
  "Empty": [],
  "Side": null,
  "SpecialValue": "42"
})""");
}
