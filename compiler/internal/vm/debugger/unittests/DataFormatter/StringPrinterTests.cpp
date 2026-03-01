//===-- StringPrinterTests.cpp --------------------------------------------===//
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

#include "lldb/DataFormatters/StringPrinter.h"
#include "lldb/Utility/DataExtractor.h"
#include "lldb/Utility/Endian.h"
#include "lldb/Utility/StreamString.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"
#include "gtest/gtest.h"
#include <optional>
#include <string>

using namespace lldb;
using namespace lldb_private;
using lldb_private::formatters::StringPrinter;
using llvm::StringRef;

#define QUOTE(x) std::string("\"" x "\"")

/// Format \p input according to the specified string encoding and special char
/// escape style.
template <StringPrinter::StringElementType elem_ty>
static std::optional<std::string>
format(StringRef input, StringPrinter::EscapeStyle escape_style) {
  StreamString out;
  StringPrinter::ReadBufferAndDumpToStreamOptions opts;
  opts.SetStream(&out);
  opts.SetSourceSize(input.size());
  opts.SetNeedsZeroTermination(true);
  opts.SetEscapeNonPrintables(true);
  opts.SetIgnoreMaxLength(false);
  opts.SetEscapeStyle(escape_style);
  opts.SetData(DataExtractor(input.data(), input.size(),
                             endian::InlHostByteOrder(), sizeof(void *)));
  const bool success = StringPrinter::ReadBufferAndDumpToStream<elem_ty>(opts);
  if (!success)
    return std::nullopt;
  return out.GetString().str();
}

// Test ASCII formatting for C++. This behaves exactly like UTF8 formatting for
// C++, although that's questionable (see FIXME in StringPrinter.cpp).
TEST(StringPrinterTests, CxxASCII) {
  auto fmt = [](StringRef str) {
    return format<StringPrinter::StringElementType::ASCII>(
        str, StringPrinter::EscapeStyle::CXX);
  };

  // Special escapes.
  EXPECT_EQ(fmt({"\0", 1}), QUOTE(""));
  EXPECT_EQ(fmt("\a"), QUOTE(R"(\a)"));
  EXPECT_EQ(fmt("\b"), QUOTE(R"(\b)"));
  EXPECT_EQ(fmt("\f"), QUOTE(R"(\f)"));
  EXPECT_EQ(fmt("\n"), QUOTE(R"(\n)"));
  EXPECT_EQ(fmt("\r"), QUOTE(R"(\r)"));
  EXPECT_EQ(fmt("\t"), QUOTE(R"(\t)"));
  EXPECT_EQ(fmt("\v"), QUOTE(R"(\v)"));
  EXPECT_EQ(fmt("\""), QUOTE(R"(\")"));
  EXPECT_EQ(fmt("\'"), QUOTE(R"(')"));
  EXPECT_EQ(fmt("\\"), QUOTE(R"(\\)"));

  // Printable characters.
  EXPECT_EQ(fmt("'"), QUOTE("'"));
  EXPECT_EQ(fmt("a"), QUOTE("a"));
  EXPECT_EQ(fmt("Z"), QUOTE("Z"));
  EXPECT_EQ(fmt("🥑"), QUOTE("🥑"));

  // Octal (\nnn), hex (\xnn), extended octal (\unnnn or \Unnnnnnnn).
  EXPECT_EQ(fmt("\uD55C"), QUOTE("\uD55C"));
  EXPECT_EQ(fmt("\U00010348"), QUOTE("\U00010348"));

  EXPECT_EQ(fmt("\376"), QUOTE(R"(\xfe)")); // \376 is 254 in decimal.
  EXPECT_EQ(fmt("\xfe"), QUOTE(R"(\xfe)")); // \xfe is 254 in decimal.
}

// Test UTF8 formatting for C++.
TEST(StringPrinterTests, CxxUTF8) {
  auto fmt = [](StringRef str) {
    return format<StringPrinter::StringElementType::UTF8>(
        str, StringPrinter::EscapeStyle::CXX);
  };

  // Special escapes.
  EXPECT_EQ(fmt({"\0", 1}), QUOTE(""));
  EXPECT_EQ(fmt("\a"), QUOTE(R"(\a)"));
  EXPECT_EQ(fmt("\b"), QUOTE(R"(\b)"));
  EXPECT_EQ(fmt("\f"), QUOTE(R"(\f)"));
  EXPECT_EQ(fmt("\n"), QUOTE(R"(\n)"));
  EXPECT_EQ(fmt("\r"), QUOTE(R"(\r)"));
  EXPECT_EQ(fmt("\t"), QUOTE(R"(\t)"));
  EXPECT_EQ(fmt("\v"), QUOTE(R"(\v)"));
  EXPECT_EQ(fmt("\""), QUOTE(R"(\")"));
  EXPECT_EQ(fmt("\'"), QUOTE(R"(')"));
  EXPECT_EQ(fmt("\\"), QUOTE(R"(\\)"));

  // Printable characters.
  EXPECT_EQ(fmt("'"), QUOTE("'"));
  EXPECT_EQ(fmt("a"), QUOTE("a"));
  EXPECT_EQ(fmt("Z"), QUOTE("Z"));
  EXPECT_EQ(fmt("🥑"), QUOTE("🥑"));

  // Octal (\nnn), hex (\xnn), extended octal (\unnnn or \Unnnnnnnn).
  EXPECT_EQ(fmt("\uD55C"), QUOTE("\uD55C"));
  EXPECT_EQ(fmt("\U00010348"), QUOTE("\U00010348"));

  EXPECT_EQ(fmt("\376"), QUOTE(R"(\xfe)")); // \376 is 254 in decimal.
  EXPECT_EQ(fmt("\xfe"), QUOTE(R"(\xfe)")); // \xfe is 254 in decimal.
}

// Test UTF8 formatting for Swift.
TEST(StringPrinterTests, SwiftUTF8) {
  auto fmt = [](StringRef str) {
    return format<StringPrinter::StringElementType::UTF8>(
        str, StringPrinter::EscapeStyle::Swift);
  };

  // Special escapes.
  EXPECT_EQ(fmt({"\0", 1}), QUOTE(""));
  EXPECT_EQ(fmt("\a"), QUOTE(R"(\a)"));
  EXPECT_EQ(fmt("\b"), QUOTE(R"(\u{8})"));
  EXPECT_EQ(fmt("\f"), QUOTE(R"(\u{c})"));
  EXPECT_EQ(fmt("\n"), QUOTE(R"(\n)"));
  EXPECT_EQ(fmt("\r"), QUOTE(R"(\r)"));
  EXPECT_EQ(fmt("\t"), QUOTE(R"(\t)"));
  EXPECT_EQ(fmt("\v"), QUOTE(R"(\u{b})"));
  EXPECT_EQ(fmt("\""), QUOTE(R"(\")"));
  EXPECT_EQ(fmt("\'"), QUOTE(R"(\')"));
  EXPECT_EQ(fmt("\\"), QUOTE(R"(\\)"));

  // Printable characters.
  EXPECT_EQ(fmt("'"), QUOTE(R"(\')"));
  EXPECT_EQ(fmt("a"), QUOTE("a"));
  EXPECT_EQ(fmt("Z"), QUOTE("Z"));
  EXPECT_EQ(fmt("🥑"), QUOTE("🥑"));

  // Octal (\nnn), hex (\xnn), extended octal (\unnnn or \Unnnnnnnn).
  EXPECT_EQ(fmt("\uD55C"), QUOTE("\uD55C"));
  EXPECT_EQ(fmt("\U00010348"), QUOTE("\U00010348"));

  EXPECT_EQ(fmt("\376"), QUOTE(R"(\u{fe})")); // \376 is 254 in decimal.
  EXPECT_EQ(fmt("\xfe"), QUOTE(R"(\u{fe})")); // \xfe is 254 in decimal.
}
