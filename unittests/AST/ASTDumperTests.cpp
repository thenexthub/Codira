//===--- ASTDumperTests.cpp - Tests for ASTDumper -----------------===//
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

#include "TestContext.h"
#include "language/AST/ASTContext.h"
#include "language/AST/GenericSignature.h"
#include "language/AST/Types.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/raw_ostream.h"
#include "gtest/gtest.h"
#include <string>

using namespace language;
using namespace language::unittest;

TEST(ASTDumper, ArchetypeType) {
  TestContext C;
  auto &ctx = C.Ctx;

  auto sig = buildGenericSignature(ctx, nullptr, {ctx.TheSelfType}, {},
                                   /*allowInverses=*/true);

  TypeBase *archetype = nullptr;
  {
    toolchain::SmallVector<ProtocolDecl *> protocols;
    archetype = PrimaryArchetypeType::getNew(ctx, sig.getGenericEnvironment(),
                                             ctx.TheSelfType, protocols, Type(),
                                             nullptr);
  }

  std::string fullStr;
  {
    toolchain::raw_string_ostream os(fullStr);
    archetype->dump(os);
  }

  toolchain::StringRef str(fullStr);
  EXPECT_TRUE(str.consume_front("(primary_archetype_type address=0x"));
  {
    intptr_t integer;
    EXPECT_FALSE(str.consumeInteger(16, integer));
  }

  EXPECT_EQ(str,
            " name=\"τ_0_0\"\n"
            "  (interface_type=generic_type_param_type depth=0 index=0 param_kind=type))\n");
}
