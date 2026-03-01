/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

#include "bytecodeopt_peepholes.h"
#include "common.h"
#include "compiler/optimizer/optimizations/cleanup.h"

namespace ark::bytecodeopt::test {

static inline size_t HasInitObject(compiler::Graph *g)
{
    for (auto bb : g->GetBlocksRPO()) {
        for (auto inst : bb->AllInsts()) {
            if (inst->GetOpcode() == compiler::Opcode::InitObject) {
                return true;
            }
        }
    }
    return false;
}

TEST_F(IrBuilderTest, NewObjectAndCtorEliminatedInitObjAppears)
{
    auto source = R"(
        .record R { u1 f }
        .function void R.ctor(R a0) <ctor> { return.void }
        .function void main() {
            newobj v0, R
            call.short R.ctor, v0
            return.void
        }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    EXPECT_EQ(HasInitObject(GetGraph()), false);
    EXPECT_TRUE(GetGraph()->RunPass<BytecodeOptPeepholes>());
    EXPECT_EQ(HasInitObject(GetGraph()), true);
}

TEST_F(IrBuilderTest, TryBlockWithGapNoInitObj)
{
    auto source = R"(
        .record E {}
        .record R { u1 f }
        .function void R.ctor(R a0) <ctor> { return.void }
        .function void main() {
        try_begin:
            newobj v0, R
            movi v1, 0x1
            call.short R.ctor, v0
        try_end:
            return.void
        catch_all:
            return.void
        .catchall try_begin, try_end, catch_all
        }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    EXPECT_EQ(HasInitObject(GetGraph()), false);
    EXPECT_FALSE(GetGraph()->RunPass<BytecodeOptPeepholes>());
    EXPECT_EQ(HasInitObject(GetGraph()), false);
}

TEST_F(IrBuilderTest, SafeArithmeticBetweenNewAndCtor)
{
    auto src = R"(
        .record R { u1 f }
        .function void R.ctor(R a0) <ctor> { return.void }
        .function void main() {
            movi v1, 0x2
            movi v2, 0x3
            newobj v0, R
            add v1, v2
            call.short R.ctor, v0
            return.void
        }
    )";
    ASSERT_TRUE(ParseToGraph(src, "main"));
    EXPECT_EQ(HasInitObject(GetGraph()), false);
    EXPECT_TRUE(GetGraph()->RunPass<BytecodeOptPeepholes>());
    EXPECT_EQ(HasInitObject(GetGraph()), true);
}

TEST_F(IrBuilderTest, SafeMovBetweenNewAndCtor)
{
    auto src = R"(
        .record R { u1 f }
        .function void R.ctor(R a0) <ctor> { return.void }
        .function void main() {
            newobj v0, R
            movi v1, 0x1
            call.short R.ctor, v0
            return.void
        }
    )";
    ASSERT_TRUE(ParseToGraph(src, "main"));
    EXPECT_EQ(HasInitObject(GetGraph()), false);
    EXPECT_TRUE(GetGraph()->RunPass<BytecodeOptPeepholes>());
    EXPECT_EQ(HasInitObject(GetGraph()), true);
}

TEST_F(IrBuilderTest, SafeMovLdaBetweenNewAndCtor)
{
    auto source = R"(
        .record R {}
        .function void R.ctor(R a0) <ctor> { return.void }
        .function i32 main() {
            newobj v0, R
            movi v1, 0x2a
            lda v1
            call.short R.ctor, v0
            ldai 0x0
            return
        }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    EXPECT_EQ(HasInitObject(GetGraph()), false);
    EXPECT_TRUE(GetGraph()->RunPass<BytecodeOptPeepholes>());
    EXPECT_EQ(HasInitObject(GetGraph()), true);
}

TEST_F(IrBuilderTest, UnrelatedCallBetweenNewAndCtor)
{
    auto source = R"(
        .record R {}
        .record H {}
        .function void R.ctor(R a0) <ctor> { return.void }
        .function void H.helper() { return.void }
        .function i32 main() {
            newobj v0, R
            call.short H.helper
            call.short R.ctor, v0
            ldai 0x0
            return
        }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    EXPECT_EQ(HasInitObject(GetGraph()), false);
    EXPECT_FALSE(GetGraph()->RunPass<BytecodeOptPeepholes>());
    EXPECT_EQ(HasInitObject(GetGraph()), false);
}

TEST_F(IrBuilderTest, SafeInstUsedAsCtorArg)
{
    auto source = R"(
        .record R {}
        .function void R.ctor(R a0, i32 a1) <ctor> { return.void }
        .function void main(i32 a0) {
            newobj v0, R
            addiv v1, a0, 0xa
            call.short R.ctor, v0, v1
            return.void
        }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    EXPECT_EQ(HasInitObject(GetGraph()), false);
    EXPECT_TRUE(GetGraph()->RunPass<BytecodeOptPeepholes>());
    EXPECT_EQ(HasInitObject(GetGraph()), true);
}
}  // namespace ark::bytecodeopt::test
