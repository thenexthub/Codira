//===--- Annotations.cpp - Annotated source code for unit tests --*- C++-*-===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Testing/Annotations/Annotations.h"

#include "vm/core/ADT/StringExtras.h"
#include "vm/core/Support/FormatVariadic.h"
#include "vm/core/Support/raw_ostream.h"

using namespace vm::core;

// Crash if the assertion fails, printing the message and testcase.
// More elegant error handling isn't needed for unit tests.
static void require(bool Assertion, const char *Msg, toolchain::StringRef Code) {
  if (!Assertion) {
    toolchain::errs() << "Annotated testcase: " << Msg << "\n" << Code << "\n";
    llvm_unreachable("Annotated testcase assertion failed!");
  }
}

Annotations::Annotations(toolchain::StringRef Text) {
  auto Require = [Text](bool Assertion, const char *Msg) {
    require(Assertion, Msg, Text);
  };
  std::optional<toolchain::StringRef> Name;
  std::optional<toolchain::StringRef> Payload;
  toolchain::SmallVector<Annotation, 8> OpenRanges;

  Code.reserve(Text.size());
  while (!Text.empty()) {
    if (Text.consume_front("^")) {
      All.push_back(
          {Code.size(), size_t(-1), Name.value_or(""), Payload.value_or("")});
      Points[Name.value_or("")].push_back(All.size() - 1);
      Name = std::nullopt;
      Payload = std::nullopt;
      continue;
    }
    if (Text.consume_front("[[")) {
      OpenRanges.push_back(
          {Code.size(), size_t(-1), Name.value_or(""), Payload.value_or("")});
      Name = std::nullopt;
      Payload = std::nullopt;
      continue;
    }
    Require(!Name, "$name should be followed by ^ or [[");
    if (Text.consume_front("]]")) {
      Require(!OpenRanges.empty(), "unmatched ]]");

      const Annotation &NewRange = OpenRanges.back();
      All.push_back(
          {NewRange.Begin, Code.size(), NewRange.Name, NewRange.Payload});
      Ranges[NewRange.Name].push_back(All.size() - 1);

      OpenRanges.pop_back();
      continue;
    }
    if (Text.consume_front("$")) {
      Name =
          Text.take_while([](char C) { return toolchain::isAlnum(C) || C == '_'; });
      Text = Text.drop_front(Name->size());

      if (Text.consume_front("(")) {
        Payload = Text.take_while([](char C) { return C != ')'; });
        Require(Text.size() > Payload->size(), "unterminated payload");
        Text = Text.drop_front(Payload->size() + 1);
      }

      continue;
    }
    Code.push_back(Text.front());
    Text = Text.drop_front();
  }
  Require(!Name, "unterminated $name");
  Require(OpenRanges.empty(), "unmatched [[");
}

size_t Annotations::point(toolchain::StringRef Name) const {
  return pointWithPayload(Name).first;
}

std::pair<size_t, toolchain::StringRef>
Annotations::pointWithPayload(toolchain::StringRef Name) const {
  auto I = Points.find(Name);
  require(I != Points.end() && I->getValue().size() == 1,
          "expected exactly one point", Code);
  const Annotation &P = All[I->getValue()[0]];
  return {P.Begin, P.Payload};
}

std::vector<size_t> Annotations::points(toolchain::StringRef Name) const {
  auto Pts = pointsWithPayload(Name);
  std::vector<size_t> Positions;
  Positions.reserve(Pts.size());
  for (const auto &[Point, Payload] : Pts)
    Positions.push_back(Point);
  return Positions;
}

std::vector<std::pair<size_t, toolchain::StringRef>>
Annotations::pointsWithPayload(toolchain::StringRef Name) const {
  auto Iter = Points.find(Name);
  if (Iter == Points.end())
    return {};

  std::vector<std::pair<size_t, toolchain::StringRef>> Res;
  Res.reserve(Iter->getValue().size());
  for (size_t I : Iter->getValue())
    Res.push_back({All[I].Begin, All[I].Payload});

  return Res;
}

toolchain::StringMap<toolchain::SmallVector<size_t, 1>> Annotations::all_points() const {
  toolchain::StringMap<toolchain::SmallVector<size_t, 1>> Result;
  for (const auto &Name : Points.keys()) {
    auto Pts = points(Name);
    Result[Name] = {Pts.begin(), Pts.end()};
  }
  return Result;
}

Annotations::Range Annotations::range(toolchain::StringRef Name) const {
  return rangeWithPayload(Name).first;
}

std::pair<Annotations::Range, toolchain::StringRef>
Annotations::rangeWithPayload(toolchain::StringRef Name) const {
  auto I = Ranges.find(Name);
  require(I != Ranges.end() && I->getValue().size() == 1,
          "expected exactly one range", Code);
  const Annotation &R = All[I->getValue()[0]];
  return {{R.Begin, R.End}, R.Payload};
}

std::vector<Annotations::Range>
Annotations::ranges(toolchain::StringRef Name) const {
  auto WithPayload = rangesWithPayload(Name);
  std::vector<Annotations::Range> Res;
  Res.reserve(WithPayload.size());
  for (const auto &[Range, Payload] : WithPayload)
    Res.push_back(Range);
  return Res;
}
std::vector<std::pair<Annotations::Range, toolchain::StringRef>>
Annotations::rangesWithPayload(toolchain::StringRef Name) const {
  auto Iter = Ranges.find(Name);
  if (Iter == Ranges.end())
    return {};

  std::vector<std::pair<Annotations::Range, toolchain::StringRef>> Res;
  Res.reserve(Iter->getValue().size());
  for (size_t I : Iter->getValue())
    Res.emplace_back(Annotations::Range{All[I].Begin, All[I].End},
                     All[I].Payload);

  return Res;
}

toolchain::StringMap<toolchain::SmallVector<Annotations::Range, 1>>
Annotations::all_ranges() const {
  toolchain::StringMap<toolchain::SmallVector<Annotations::Range, 1>> Res;
  for (const toolchain::StringRef &Name : Ranges.keys()) {
    auto R = ranges(Name);
    Res[Name] = {R.begin(), R.end()};
  }
  return Res;
}

toolchain::raw_ostream &toolchain::operator<<(toolchain::raw_ostream &O,
                                    const toolchain::Annotations::Range &R) {
  return O << toolchain::formatv("[{0}, {1})", R.Begin, R.End);
}
