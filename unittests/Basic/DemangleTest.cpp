//===--- DemangleTest.cpp -------------------------------------------------===//
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

#include "language/Demangling/Demangle.h"
#include "language/Demangling/Demangler.h"
#include "gtest/gtest.h"

using namespace language::Demangle;

TEST(Demangle, DemangleWrappers) {
  EXPECT_EQ("", demangleSymbolAsString(toolchain::StringRef("")));
  std::string MangledName = "_TtV1a1b\\\t\n\r\"\'\x1f\x20\x7e\x7f";
  MangledName += '\0';
  EXPECT_EQ("a.b with unmangled suffix \"\\\\\\t\\n\\r\\\"'\\x1F ~\\x7F\\0\"",
      demangleSymbolAsString(MangledName));
}

TEST(Demangle, IsObjCSymbol) {
  EXPECT_EQ("type metadata accessor for __C.NSNumber",
            demangleSymbolAsString(toolchain::StringRef("_$sSo8NSNumberCMa")));
  EXPECT_EQ(true, isObjCSymbol(toolchain::StringRef("_$sSo8NSNumberCMa")));
  EXPECT_EQ(false,
            isObjCSymbol(toolchain::StringRef("_$s3pat7inlinedSo8NSNumberCvp")));
  EXPECT_EQ(true, isObjCSymbol(toolchain::StringRef("_$sSC3fooyS2d_SdtFTO")));
}

TEST(Demangle, CustomGenericParameterNames) {
  std::string SymbolName = "_$s1a1gyq_q__xt_tr0_lF";
  std::string DemangledName = "a.g<Q, U>((U, Q)) -> U";

  DemangleOptions Options;
  Options.GenericParameterName = [](uint64_t depth, uint64_t index) {
    return index ? "U" : "Q";
  };
  std::string Result = demangleSymbolAsString(SymbolName, Options);
  EXPECT_STREQ(DemangledName.c_str(), Result.c_str());
}

TEST(Demangle, DeepEquals) {
  static std::string Symbols[]{
#define SYMBOL(Mangled, Demangled) Mangled,
#include "ManglingTestData.def"
  };
  for (const auto &Symbol : Symbols) {
    Demangler D1;
    Demangler D2;
    auto tree1 = D1.demangleSymbol(Symbol);
    auto tree2 = D2.demangleSymbol(Symbol);
    EXPECT_TRUE(tree1->isDeepEqualTo(tree2)) << "Failing symbol: " << Symbol;
  }
}
