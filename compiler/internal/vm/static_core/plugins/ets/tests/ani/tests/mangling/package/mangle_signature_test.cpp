/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <string_view>
#include "ani.h"
#include "ani_gtest.h"
#include "plugins/ets/runtime/ani/ani_mangle.h"
#include "plugins/ets/runtime/types/ets_array.h"

namespace ark::ets::ani::testing {

class MangleSignatureTest : public AniTest {};

// type F = (u: number | string | FixedArray<char>) => (E | B) | Partial<A>
static constexpr std::string_view FOO_UNION_SIGNATURE =
    "X{A{c}C{std.core.Double}C{std.core.String}}:X{C{msig.B}E{msig.E}P{msig.A}}";
static constexpr std::string_view NAMESPACE_FOO_UNION_SIGNATURE =
    "X{A{c}C{std.core.Double}C{std.core.String}}:X{C{msig.rls.B}E{msig.rls.E}P{msig.A}}";
// type F = <T extends A, V extends T | string>(u: T | V | A | number): FixedArray<T> | null | V
static constexpr std::string_view FOO1_UNION_SIGNATURE =
    "X{C{msig.A}C{std.core.String}C{std.core.Double}}:X{A{C{msig.A}}C{msig.A}C{std.core.String}C{std.core.Null}}";
static constexpr std::string_view NAMESPACE_FOO1_UNION_SIGNATURE =
    "X{C{msig.rls.A}C{std.core.String}C{std.core.Double}}:X{A{C{msig.rls.A}}C{msig.rls.A}C{std.core.String}C{std.core."
    "Null}}";

TEST_F(MangleSignatureTest, FormatVoid_NewToOld)
{
    std::optional<PandaString> desc;

    desc = Mangle::ConvertSignature(":");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), ":V");
}

TEST_F(MangleSignatureTest, FormatVoid_OldToOld)
{
    std::optional<PandaString> desc;

    desc = Mangle::ConvertSignature(":V");
    ASSERT_FALSE(desc.has_value());
}

// CC-OFFNXT(huge_method[C++], G.FUD.05) long test with solid logic
TEST_F(MangleSignatureTest, FormatPrimitives_NewToOld)
{
    std::optional<PandaString> desc;

    // Check 'boolean'
    desc = Mangle::ConvertSignature("z:");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), "Z:V");
    desc = Mangle::ConvertSignature(":z");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), ":Z");

    // Check 'char'
    desc = Mangle::ConvertSignature("c:");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), "C:V");
    desc = Mangle::ConvertSignature(":c");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), ":C");

    // Check 'byte'
    desc = Mangle::ConvertSignature("b:");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), "B:V");
    desc = Mangle::ConvertSignature(":b");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), ":B");

    // Check 'short'
    desc = Mangle::ConvertSignature("s:");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), "S:V");
    desc = Mangle::ConvertSignature(":s");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), ":S");

    // Check 'int'
    desc = Mangle::ConvertSignature("i:");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), "I:V");
    desc = Mangle::ConvertSignature(":i");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), ":I");

    // Check 'long'
    desc = Mangle::ConvertSignature("l:");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), "J:V");
    desc = Mangle::ConvertSignature(":l");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), ":J");

    // Check 'float'
    desc = Mangle::ConvertSignature("f:");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), "F:V");
    desc = Mangle::ConvertSignature(":f");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), ":F");

    // Check 'double'
    desc = Mangle::ConvertSignature("d:");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), "D:V");
    desc = Mangle::ConvertSignature(":d");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), ":D");

    // Check 'any'
    desc = Mangle::ConvertSignature("Y:");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), "Lstd/core/Object;:V");
    desc = Mangle::ConvertSignature(":Y");
    EXPECT_STREQ(desc.value().c_str(), ":Lstd/core/Object;");

    // Check 'never'
    desc = Mangle::ConvertSignature("N:");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), "Lstd/core/Object;:V");
    desc = Mangle::ConvertSignature(":N");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), ":Lstd/core/Object;");

    // Check mixed primitives
    desc = Mangle::ConvertSignature("zcbsilfdY:z");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), "ZCBSIJFDLstd/core/Object;:Z");

    desc = Mangle::ConvertSignature("zcbsilfdY:N");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), "ZCBSIJFDLstd/core/Object;:Lstd/core/Object;");
}

TEST_F(MangleSignatureTest, FormatPrimitives_OldToOld)
{
    std::optional<PandaString> desc;

    // Check 'boolean'
    desc = Mangle::ConvertSignature("Z:V");
    ASSERT_FALSE(desc.has_value());
    desc = Mangle::ConvertSignature(":Z");
    ASSERT_FALSE(desc.has_value());

    // Check 'char'
    desc = Mangle::ConvertSignature("C:V");
    ASSERT_FALSE(desc.has_value());
    desc = Mangle::ConvertSignature(":C");
    ASSERT_FALSE(desc.has_value());

    // Check 'byte'
    desc = Mangle::ConvertSignature("B:V");
    ASSERT_FALSE(desc.has_value());
    desc = Mangle::ConvertSignature(":B");
    ASSERT_FALSE(desc.has_value());

    // Check 'short'
    desc = Mangle::ConvertSignature("S:V");
    ASSERT_FALSE(desc.has_value());
    desc = Mangle::ConvertSignature(":S");
    ASSERT_FALSE(desc.has_value());

    // Check 'int'
    desc = Mangle::ConvertSignature("I:V");
    ASSERT_FALSE(desc.has_value());
    desc = Mangle::ConvertSignature(":I");
    ASSERT_FALSE(desc.has_value());

    // Check 'long'
    desc = Mangle::ConvertSignature("J:V");
    ASSERT_FALSE(desc.has_value());
    desc = Mangle::ConvertSignature(":J");
    ASSERT_FALSE(desc.has_value());

    // Check 'float'
    desc = Mangle::ConvertSignature("F:V");
    ASSERT_FALSE(desc.has_value());
    desc = Mangle::ConvertSignature(":F");
    ASSERT_FALSE(desc.has_value());

    // Check 'double'
    desc = Mangle::ConvertSignature("D:V");
    ASSERT_FALSE(desc.has_value());
    desc = Mangle::ConvertSignature(":D");
    ASSERT_FALSE(desc.has_value());

    // Check mixed primitives
    desc = Mangle::ConvertSignature("ZCBSIJFD:Z");
    ASSERT_FALSE(desc.has_value());
}

TEST_F(MangleSignatureTest, FormatNullAndUndefined_NewToOld)
{
    std::optional<PandaString> desc;

    desc = Mangle::ConvertSignature("U:");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), "Lstd/core/Object;:V");
    desc = Mangle::ConvertSignature(":U");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), ":Lstd/core/Object;");
}

TEST_F(MangleSignatureTest, FormatReferences_NewToOld)
{
    std::optional<PandaString> desc;

    // Check 'class'
    desc = Mangle::ConvertSignature("C{std.core.Object}:");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), "Lstd/core/Object;:V");
    desc = Mangle::ConvertSignature(":C{std.core.Object}");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), ":Lstd/core/Object;");

    // Check 'enume'
    desc = Mangle::ConvertSignature("E{a.b.c.Color}:");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), "La/b/c/Color;:V");
    desc = Mangle::ConvertSignature(":E{a.b.c.Color}");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), ":La/b/c/Color;");

    // Check 'Partial<T>'
    desc = Mangle::ConvertSignature("P{a.b.c.X}:");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), "La/b/c/%%partial-X;:V");
    desc = Mangle::ConvertSignature(":P{a.b.c.X}");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), ":La/b/c/%%partial-X;");
}

// CC-OFFNXT(huge_method[C++], G.FUD.05) long test with solid logic
TEST_F(MangleSignatureTest, FormatPrimitivesFixedArray_NewToOld)
{
    std::optional<PandaString> desc;

    // Check 'FixedArray<boolean>'
    desc = Mangle::ConvertSignature("A{z}:");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), "[Z:V");
    desc = Mangle::ConvertSignature(":A{z}");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), ":[Z");

    // Check 'FixedArray<char>'
    desc = Mangle::ConvertSignature("A{c}:");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), "[C:V");
    desc = Mangle::ConvertSignature(":A{c}");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), ":[C");

    // Check 'FixedArray<byte>'
    desc = Mangle::ConvertSignature("A{b}:");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), "[B:V");
    desc = Mangle::ConvertSignature(":A{b}");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), ":[B");

    // Check 'FixedArray<short>'
    desc = Mangle::ConvertSignature("A{s}:");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), "[S:V");
    desc = Mangle::ConvertSignature(":A{s}");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), ":[S");

    // Check 'FixedArray<int>'
    desc = Mangle::ConvertSignature("i:");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), "I:V");
    desc = Mangle::ConvertSignature(":i");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), ":I");

    // Check 'FixedArray<long>'
    desc = Mangle::ConvertSignature("A{l}:");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), "[J:V");
    desc = Mangle::ConvertSignature(":A{l}");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), ":[J");

    // Check 'FixedArray<float>'
    desc = Mangle::ConvertSignature("A{f}:");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), "[F:V");
    desc = Mangle::ConvertSignature(":A{f}");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), ":[F");

    // Check 'FixedArray<double>'
    desc = Mangle::ConvertSignature("A{d}:");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), "[D:V");
    desc = Mangle::ConvertSignature(":A{d}");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), ":[D");

    // Check 'FixedArray<any>'
    desc = Mangle::ConvertSignature("A{Y}:");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), "[Lstd/core/Object;:V");
    desc = Mangle::ConvertSignature(":A{Y}");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), ":[Lstd/core/Object;");

    // Check 'FixedArray<never>'
    desc = Mangle::ConvertSignature("A{N}:");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), "[Lstd/core/Object;:V");
    desc = Mangle::ConvertSignature(":A{N}");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), ":[Lstd/core/Object;");

    // Check mixed 'FixedArray' types
    desc = Mangle::ConvertSignature("A{z}A{c}A{b}A{s}A{i}A{l}A{f}A{d}A{Y}:A{b}");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), "[Z[C[B[S[I[J[F[D[Lstd/core/Object;:[B");

    // Check nested 'FixedArray' types
    desc = Mangle::ConvertSignature("A{A{A{A{A{c}}}}}:A{b}");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), "[[[[[C:[B");
}

TEST_F(MangleSignatureTest, FormatPrimitivesFixedArray_OldToOld)
{
    std::optional<PandaString> desc;

    // Check 'FixedArray<boolean>'
    desc = Mangle::ConvertSignature("[Z:V");
    ASSERT_FALSE(desc.has_value());
    desc = Mangle::ConvertSignature(":[Z");
    ASSERT_FALSE(desc.has_value());

    // Check 'FixedArray<char>'
    desc = Mangle::ConvertSignature("[C:V");
    ASSERT_FALSE(desc.has_value());
    desc = Mangle::ConvertSignature(":[C");
    ASSERT_FALSE(desc.has_value());

    // Check 'FixedArray<byte>'
    desc = Mangle::ConvertSignature("[B:V");
    ASSERT_FALSE(desc.has_value());
    desc = Mangle::ConvertSignature(":[B");
    ASSERT_FALSE(desc.has_value());
    // Check 'FixedArray<short>'
    desc = Mangle::ConvertSignature("[S:V");
    ASSERT_FALSE(desc.has_value());
    desc = Mangle::ConvertSignature(":[S");
    ASSERT_FALSE(desc.has_value());

    // Check 'FixedArray<int>'
    desc = Mangle::ConvertSignature("I:V");
    ASSERT_FALSE(desc.has_value());
    desc = Mangle::ConvertSignature(":I");
    ASSERT_FALSE(desc.has_value());

    // Check 'FixedArray<long>'
    desc = Mangle::ConvertSignature("[J:V");
    ASSERT_FALSE(desc.has_value());
    desc = Mangle::ConvertSignature(":[J");
    ASSERT_FALSE(desc.has_value());

    // Check 'FixedArray<float>'
    desc = Mangle::ConvertSignature("[F:V");
    ASSERT_FALSE(desc.has_value());
    desc = Mangle::ConvertSignature(":[F");
    ASSERT_FALSE(desc.has_value());

    // Check 'FixedArray<double>'
    desc = Mangle::ConvertSignature("[D:V");
    ASSERT_FALSE(desc.has_value());
    desc = Mangle::ConvertSignature(":[D");
    ASSERT_FALSE(desc.has_value());

    // Check mixed 'FixedArray' types
    desc = Mangle::ConvertSignature("[Z[C[B[S[I[J[F[D:[B");
    ASSERT_FALSE(desc.has_value());

    // Check nested 'FixedArray' types
    desc = Mangle::ConvertSignature("[[[[[C:[B");
    ASSERT_FALSE(desc.has_value());
}

TEST_F(MangleSignatureTest, FormatReferencesFixedArray_NewToOld)
{
    std::optional<PandaString> desc;

    desc = Mangle::ConvertSignature("A{C{std.core.String}}dA{A{E{a.b.Color}}}:A{P{a.b.X}}");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), "[Lstd/core/String;D[[La/b/Color;:[La/b/%%partial-X;");
}

TEST_F(MangleSignatureTest, FormatReferencesFixedArray_OldToOld)
{
    std::optional<PandaString> desc;

    desc = Mangle::ConvertSignature("[Lstd/core/String;D[[La/b/Color;:[La/b/%%partial-X;");
    ASSERT_FALSE(desc.has_value());
}

TEST_F(MangleSignatureTest, FormatUnion_NewToRuntime)
{
    std::optional<PandaString> desc;

    // type F = (u: a.b | double | FixedArray<int>) => null | e
    desc = Mangle::ConvertSignature("X{C{a.b}C{std.core.Double}A{i}}:X{C{std.core.Null}E{e}}");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), "{ULa/b;[ILstd/core/Double;}:{ULe;Lstd/core/Null;}");

    // type F = (u: (e | double) | FixedArray<string[] | FunctionR1>) => void
    desc = Mangle::ConvertSignature("X{X{E{e}C{std.core.Double}}A{X{A{C{std.core.String}}C{std.core.FunctionR1}}}}:");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), "{U{ULe;Lstd/core/Double;}[{ULstd/core/FunctionR1;[Lstd/core/String;}}:V");

    // type F = (u: number | string | FixedArray<char>) => (msig.E | msig.B) | Partial<msig.A>
    desc = Mangle::ConvertSignature(FOO_UNION_SIGNATURE);
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), "{ULstd/core/Double;Lstd/core/String;[C}:{ULmsig/%%partial-A;Lmsig/B;Lmsig/E;}");

    // type F = <T extends A, V extends T | string>(u: T | V | A | number): FixedArray<T> | null | V
    desc = Mangle::ConvertSignature(FOO1_UNION_SIGNATURE);
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(),
                 "{ULmsig/A;Lstd/core/Double;Lstd/core/String;}:{ULmsig/A;[Lmsig/A;Lstd/core/Null;Lstd/core/String;}");
}

TEST_F(MangleSignatureTest, Format_Wrong)
{
    std::optional<PandaString> desc;

    desc = Mangle::ConvertSignature("");
    ASSERT_FALSE(desc.has_value());

    desc = Mangle::ConvertSignature("b:A{A{c}");
    ASSERT_FALSE(desc.has_value());

    desc = Mangle::ConvertSignature("A{A{c}}}:z");
    ASSERT_FALSE(desc.has_value());

    desc = Mangle::ConvertSignature("A{A{c}}");
    ASSERT_FALSE(desc.has_value());

    desc = Mangle::ConvertSignature("C{a:b}:f");
    ASSERT_FALSE(desc.has_value());

    desc = Mangle::ConvertSignature("C{a/b}:");
    ASSERT_FALSE(desc.has_value());

    desc = Mangle::ConvertSignature("A{C{a/b}}:");
    ASSERT_FALSE(desc.has_value());

    desc = Mangle::ConvertSignature("C{a.b}:C{a/b}");
    ASSERT_FALSE(desc.has_value());

    desc = Mangle::ConvertSignature(":X{C{a.b}C{c/d}}");
    ASSERT_FALSE(desc.has_value());
}

TEST_F(MangleSignatureTest, Module_FindFunction)
{
    std::optional<PandaString> desc;

    ani_module m {};
    ASSERT_EQ(env_->FindModule("msig", &m), ANI_OK);

    ani_function fn {};
    EXPECT_EQ(env_->Module_FindFunction(m, "foo", ":", &fn), ANI_OK);

    // Check primitives
    EXPECT_EQ(env_->Module_FindFunction(m, "foo", "z:", &fn), ANI_OK);
    EXPECT_EQ(env_->Module_FindFunction(m, "foo", "c:", &fn), ANI_OK);
    EXPECT_EQ(env_->Module_FindFunction(m, "foo", "s:", &fn), ANI_OK);
    EXPECT_EQ(env_->Module_FindFunction(m, "foo", "i:", &fn), ANI_OK);
    EXPECT_EQ(env_->Module_FindFunction(m, "foo", "l:", &fn), ANI_OK);
    EXPECT_EQ(env_->Module_FindFunction(m, "foo", "f:", &fn), ANI_OK);
    EXPECT_EQ(env_->Module_FindFunction(m, "foo", "d:", &fn), ANI_OK);

    // Check never
    EXPECT_EQ(env_->Module_FindFunction(m, "fooNever", "N:", &fn), ANI_OK);
    EXPECT_EQ(env_->Module_FindFunction(m, "fooNever", ":N", &fn), ANI_OK);

    // Check any
    EXPECT_EQ(env_->Module_FindFunction(m, "fooAny", "Y:", &fn), ANI_OK);
    EXPECT_EQ(env_->Module_FindFunction(m, "fooAny", ":Y", &fn), ANI_OK);

    // Check generics
    EXPECT_EQ(env_->Module_FindFunction(m, "foo1", "Y:", &fn), ANI_OK);
    EXPECT_EQ(env_->Module_FindFunction(m, "foo1", ":Y", &fn), ANI_OK);
    EXPECT_EQ(env_->Module_FindFunction(m, "foo1", "Y:N", &fn), ANI_OK);
    EXPECT_EQ(env_->Module_FindFunction(m, "fooFixedArray", "A{Y}:", &fn), ANI_OK);
    EXPECT_EQ(env_->Module_FindFunction(m, "fooFixedArray", ":A{Y}", &fn), ANI_OK);

    // Check references
    EXPECT_EQ(env_->Module_FindFunction(m, "foo", "dC{msig.A}C{msig.B}:E{msig.E}", &fn), ANI_OK);
    EXPECT_EQ(env_->Module_FindFunction(m, "foo", "P{msig.A}C{std.core.Array}:", &fn), ANI_OK);
    EXPECT_EQ(env_->Module_FindFunction(m, "foo", FOO_UNION_SIGNATURE.data(), &fn), ANI_OK);
    EXPECT_EQ(env_->Module_FindFunction(m, "foo1", FOO1_UNION_SIGNATURE.data(), &fn), ANI_OK);
}

TEST_F(MangleSignatureTest, Module_FindFunction_OldFormat)
{
    std::optional<PandaString> desc;

    ani_module m {};
    ASSERT_EQ(env_->FindModule("msig", &m), ANI_OK);

    ani_function fn {};
    EXPECT_EQ(env_->Module_FindFunction(m, "foo", ":V", &fn), ANI_INVALID_DESCRIPTOR);

    // Check primitives
    EXPECT_EQ(env_->Module_FindFunction(m, "foo", "Z:V", &fn), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Module_FindFunction(m, "foo", "C:V", &fn), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Module_FindFunction(m, "foo", "S:V", &fn), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Module_FindFunction(m, "foo", "I:V", &fn), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Module_FindFunction(m, "foo", "J:V", &fn), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Module_FindFunction(m, "foo", "F:V", &fn), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Module_FindFunction(m, "foo", "D:V", &fn), ANI_INVALID_DESCRIPTOR);

    // Check never
    EXPECT_EQ(env_->Module_FindFunction(m, "fooNever", "Lstd/core/Object;:V", &fn), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Module_FindFunction(m, "fooNever", ":Lstd/core/Object;", &fn), ANI_INVALID_DESCRIPTOR);

    // Check any
    EXPECT_EQ(env_->Module_FindFunction(m, "fooAny", "Lstd/core/Object;:V", &fn), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Module_FindFunction(m, "fooAny", ":Lstd/core/Object;", &fn), ANI_INVALID_DESCRIPTOR);

    // Check generics
    EXPECT_EQ(env_->Module_FindFunction(m, "foo1", "Lstd/core/Object;:V", &fn), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Module_FindFunction(m, "foo1", ":Lstd/core/Object;", &fn), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Module_FindFunction(m, "foo1", "Lstd/core/Object;:Lstd/core/Object;", &fn), ANI_INVALID_DESCRIPTOR);

    // Check references
    EXPECT_EQ(env_->Module_FindFunction(m, "foo", "DLmsig/A;Lmsig/B;:Lmsig/E;", &fn), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Module_FindFunction(m, "foo", "Lmsig/%%partial-A;Lstd/core/Array;:V", &fn), ANI_INVALID_DESCRIPTOR);
}

TEST_F(MangleSignatureTest, Namespace_FindFunction)
{
    std::optional<PandaString> desc;

    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("msig.rls", &ns), ANI_OK);

    ani_function fn {};
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "foo", ":", &fn), ANI_OK);

    // Check primitives
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "foo", "z:", &fn), ANI_OK);
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "foo", "c:", &fn), ANI_OK);
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "foo", "s:", &fn), ANI_OK);
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "foo", "i:", &fn), ANI_OK);
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "foo", "l:", &fn), ANI_OK);
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "foo", "f:", &fn), ANI_OK);
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "foo", "d:", &fn), ANI_OK);

    // Check never
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "fooNever", ":N", &fn), ANI_OK);
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "fooNever", "N:", &fn), ANI_OK);

    // Check any
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "fooAny", "Y:", &fn), ANI_OK);
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "fooAny", ":Y", &fn), ANI_OK);

    // Check generics
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "foo1", "Y:", &fn), ANI_OK);
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "foo1", ":Y", &fn), ANI_OK);
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "foo1", "Y:N", &fn), ANI_OK);
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "fooFixedArray", "A{Y}:", &fn), ANI_OK);
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "fooFixedArray", ":A{Y}", &fn), ANI_OK);

    // Check references
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "foo", "dC{msig.rls.A}C{msig.rls.B}:E{msig.rls.E}", &fn), ANI_OK);
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "foo", "P{msig.A}C{std.core.Array}:", &fn), ANI_OK);
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "foo", NAMESPACE_FOO_UNION_SIGNATURE.data(), &fn), ANI_OK);
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "foo1", NAMESPACE_FOO1_UNION_SIGNATURE.data(), &fn), ANI_OK);
}

TEST_F(MangleSignatureTest, Namespace_FindFunction_OldFormat)
{
    std::optional<PandaString> desc;

    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("msig.rls", &ns), ANI_OK);

    ani_function fn {};
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "foo", ":V", &fn), ANI_INVALID_DESCRIPTOR);

    // Check primitives
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "foo", "Z:V", &fn), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "foo", "C:V", &fn), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "foo", "S:V", &fn), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "foo", "I:V", &fn), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "foo", "J:V", &fn), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "foo", "F:V", &fn), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "foo", "D:V", &fn), ANI_INVALID_DESCRIPTOR);

    // Check never
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "fooNever", "Lstd/core/Object;:V", &fn), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "fooNever", ":Lstd/core/Object;", &fn), ANI_INVALID_DESCRIPTOR);

    // Check any
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "fooAny", "Lstd/core/Object;:V", &fn), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "fooAny", ":Lstd/core/Object;", &fn), ANI_INVALID_DESCRIPTOR);

    // Check generics
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "foo1", "Lstd/core/Object;:V", &fn), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "foo1", ":Lstd/core/Object;", &fn), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "foo1", "Lstd/core/Object;:Lstd/core/Object;", &fn),
              ANI_INVALID_DESCRIPTOR);

    // Check references
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "foo", "DLmsig/rls/A;Lmsig/rls/B;:Lmsig/rls/E;", &fn),
              ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "foo", "Lmsig/%%partial-A;Lstd/core/Array;:V", &fn),
              ANI_INVALID_DESCRIPTOR);
}

TEST_F(MangleSignatureTest, Class_FindMethod)
{
    std::optional<PandaString> desc;

    ani_class cls {};
    ASSERT_EQ(env_->FindClass("msig.F", &cls), ANI_OK);

    ani_method method {};
    EXPECT_EQ(env_->Class_FindMethod(cls, "foo", ":", &method), ANI_OK);

    // Check primitives
    EXPECT_EQ(env_->Class_FindMethod(cls, "foo", "z:", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindMethod(cls, "foo", "c:", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindMethod(cls, "foo", "s:", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindMethod(cls, "foo", "i:", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindMethod(cls, "foo", "l:", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindMethod(cls, "foo", "f:", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindMethod(cls, "foo", "d:", &method), ANI_OK);

    // Check never
    EXPECT_EQ(env_->Class_FindMethod(cls, "fooNever", "N:", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindMethod(cls, "fooNever", ":N", &method), ANI_OK);

    // Check any
    EXPECT_EQ(env_->Class_FindMethod(cls, "fooAny", "Y:", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindMethod(cls, "fooAny", ":Y", &method), ANI_OK);

    // Check generics
    EXPECT_EQ(env_->Class_FindMethod(cls, "foo1", "Y:", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindMethod(cls, "foo1", ":Y", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindMethod(cls, "foo1", "Y:N", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindMethod(cls, "fooFixedArray", "A{Y}:", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindMethod(cls, "fooFixedArray", ":A{Y}", &method), ANI_OK);

    // Check references
    EXPECT_EQ(env_->Class_FindMethod(cls, "foo", "dC{msig.A}C{msig.B}:E{msig.E}", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindMethod(cls, "foo", "P{msig.A}C{std.core.Array}:", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindMethod(cls, "foo", FOO_UNION_SIGNATURE.data(), &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindMethod(cls, "foo1", FOO1_UNION_SIGNATURE.data(), &method), ANI_OK);
}

TEST_F(MangleSignatureTest, Class_FindMethod_OldFormat)
{
    std::optional<PandaString> desc;

    ani_class cls {};
    ASSERT_EQ(env_->FindClass("msig.F", &cls), ANI_OK);

    ani_method method {};
    EXPECT_EQ(env_->Class_FindMethod(cls, "foo", ":V", &method), ANI_INVALID_DESCRIPTOR);

    // Check primitives
    EXPECT_EQ(env_->Class_FindMethod(cls, "foo", "Z:V", &method), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Class_FindMethod(cls, "foo", "C:V", &method), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Class_FindMethod(cls, "foo", "S:V", &method), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Class_FindMethod(cls, "foo", "I:V", &method), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Class_FindMethod(cls, "foo", "J:V", &method), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Class_FindMethod(cls, "foo", "F:V", &method), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Class_FindMethod(cls, "foo", "D:V", &method), ANI_INVALID_DESCRIPTOR);

    // Check never
    EXPECT_EQ(env_->Class_FindMethod(cls, "fooNever", "Lstd/core/Object;:V", &method), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Class_FindMethod(cls, "fooNever", ":Lstd/core/Object;", &method), ANI_INVALID_DESCRIPTOR);

    // Check any
    EXPECT_EQ(env_->Class_FindMethod(cls, "fooAny", "Lstd/core/Object;:V", &method), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Class_FindMethod(cls, "fooAny", ":Lstd/core/Object;", &method), ANI_INVALID_DESCRIPTOR);

    // Check generics
    EXPECT_EQ(env_->Class_FindMethod(cls, "foo1", "Lstd/core/Object;:V", &method), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Class_FindMethod(cls, "foo1", ":Lstd/core/Object;", &method), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Class_FindMethod(cls, "foo1", "Lstd/core/Object;:Lstd/core/Object;", &method),
              ANI_INVALID_DESCRIPTOR);

    // Check references
    EXPECT_EQ(env_->Class_FindMethod(cls, "foo", "DLmsig/A;Lmsig/B;:Lmsig/E;", &method), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Class_FindMethod(cls, "foo", "Lmsig/%%partial-A;Lstd/core/Array;:V", &method),
              ANI_INVALID_DESCRIPTOR);
}

TEST_F(MangleSignatureTest, Class_FindStaticMethod)
{
    std::optional<PandaString> desc;

    ani_class cls {};
    ASSERT_EQ(env_->FindClass("msig.G", &cls), ANI_OK);

    ani_static_method method {};
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "foo", ":", &method), ANI_OK);

    // Check primitives
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "foo", "z:", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "foo", "c:", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "foo", "s:", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "foo", "i:", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "foo", "l:", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "foo", "f:", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "foo", "d:", &method), ANI_OK);

    // Check never
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "fooNever", "N:", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "fooNever", ":N", &method), ANI_OK);

    // Check any
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "fooAny", "Y:", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "fooAny", ":Y", &method), ANI_OK);

    // Check generics
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "foo1", "Y:", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "foo1", ":Y", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "foo1", "Y:N", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "fooFixedArray", "A{Y}:", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "fooFixedArray", ":A{Y}", &method), ANI_OK);

    // Check references
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "foo", "dC{msig.A}C{msig.B}:E{msig.E}", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "foo", "P{msig.A}C{std.core.Array}:", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "foo", FOO_UNION_SIGNATURE.data(), &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "foo1", FOO1_UNION_SIGNATURE.data(), &method), ANI_OK);
}

TEST_F(MangleSignatureTest, Class_FindStaticMethod_OldFormat)
{
    std::optional<PandaString> desc;

    ani_class cls {};
    ASSERT_EQ(env_->FindClass("msig.G", &cls), ANI_OK);

    ani_static_method method {};
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "foo", ":V", &method), ANI_INVALID_DESCRIPTOR);

    // Check primitives
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "foo", "Z:V", &method), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "foo", "C:V", &method), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "foo", "S:V", &method), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "foo", "I:V", &method), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "foo", "J:V", &method), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "foo", "F:V", &method), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "foo", "D:V", &method), ANI_INVALID_DESCRIPTOR);

    // Check never
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "fooNever", "Lstd/core/Object;:V", &method), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "fooNever", ":Lstd/core/Object;", &method), ANI_INVALID_DESCRIPTOR);

    // Check any
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "fooAny", "Lstd/core/Object;:V", &method), ANI_INVALID_DESCRIPTOR);

    // Check generics
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "foo1", "Lstd/core/Object;:V", &method), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "foo1", ":Lstd/core/Object;", &method), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "foo1", "Lstd/core/Object;:Lstd/core/Object;", &method),
              ANI_INVALID_DESCRIPTOR);

    // Check references
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "foo", "DLmsig/A;Lmsig/B;:Lmsig/E;", &method), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "foo", "Lmsig/%%partial-A;Lstd/core/Array;:V", &method),
              ANI_INVALID_DESCRIPTOR);
}

TEST_F(MangleSignatureTest, Class_FindIndexableGetter)
{
    std::optional<PandaString> desc;

    ani_class cls {};
    ASSERT_EQ(env_->FindClass("msig.F", &cls), ANI_OK);

    ani_method method {};
    EXPECT_EQ(env_->Class_FindIndexableGetter(cls, "d:i", &method), ANI_OK);
}

TEST_F(MangleSignatureTest, Class_FindIndexableGetter_OldFormat)
{
    std::optional<PandaString> desc;

    ani_class cls {};
    ASSERT_EQ(env_->FindClass("msig.F", &cls), ANI_OK);

    ani_method method {};
    EXPECT_EQ(env_->Class_FindIndexableGetter(cls, "D:I", &method), ANI_INVALID_DESCRIPTOR);
}

TEST_F(MangleSignatureTest, Class_FindIndexableSetter)
{
    std::optional<PandaString> desc;

    ani_class cls {};
    ASSERT_EQ(env_->FindClass("msig.F", &cls), ANI_OK);

    ani_method method {};
    // Check primitives
    EXPECT_EQ(env_->Class_FindIndexableSetter(cls, "dz:", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindIndexableSetter(cls, "dc:", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindIndexableSetter(cls, "ds:", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindIndexableSetter(cls, "di:", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindIndexableSetter(cls, "dl:", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindIndexableSetter(cls, "df:", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindIndexableSetter(cls, "dd:", &method), ANI_OK);

    // Check Any
    EXPECT_EQ(env_->Class_FindIndexableSetter(cls, "dY:", &method), ANI_OK);

    // Check never
    EXPECT_EQ(env_->Class_FindIndexableSetter(cls, "dN:", &method), ANI_OK);

    // Check references
    EXPECT_EQ(env_->Class_FindIndexableSetter(cls, "dC{std.core.String}:", &method), ANI_OK);
}

TEST_F(MangleSignatureTest, Class_FindIndexableSetter_OldFormat)
{
    std::optional<PandaString> desc;

    ani_class cls {};
    ASSERT_EQ(env_->FindClass("msig.F", &cls), ANI_OK);

    ani_method method {};

    // Check primitives
    EXPECT_EQ(env_->Class_FindIndexableSetter(cls, "DZ:V", &method), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Class_FindIndexableSetter(cls, "DC:V", &method), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Class_FindIndexableSetter(cls, "DS:V", &method), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Class_FindIndexableSetter(cls, "DI:V", &method), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Class_FindIndexableSetter(cls, "DJ:V", &method), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Class_FindIndexableSetter(cls, "DF:V", &method), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->Class_FindIndexableSetter(cls, "DD:V", &method), ANI_INVALID_DESCRIPTOR);

    // Check Any
    EXPECT_EQ(env_->Class_FindIndexableSetter(cls, "DLstd/core/Object;:V", &method), ANI_INVALID_DESCRIPTOR);

    // Check never
    EXPECT_EQ(env_->Class_FindIndexableSetter(cls, "DLstd/core/Object;:V", &method), ANI_INVALID_DESCRIPTOR);

    // Check references
    EXPECT_EQ(env_->Class_FindIndexableSetter(cls, "DLstd/core/String;:V", &method), ANI_INVALID_DESCRIPTOR);
}

TEST_F(MangleSignatureTest, Class_CallStaticMethodByName)
{
    std::optional<PandaString> desc;

    ani_class cls {};
    ASSERT_EQ(env_->FindClass("msig.G", &cls), ANI_OK);

    ani_int result {};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(cls, "foo", "iii:i", &result, 1, 2U, 3U), ANI_OK);
    ASSERT_EQ(result, 1 + 2U + 3U);

    // Check correct union type is returned from `Class_CallStaticMethodByName_Ref`
    static constexpr std::string_view SAMPLE_STRING = "sample";
    ani_string sampleStr {};
    ASSERT_EQ(env_->String_NewUTF8(SAMPLE_STRING.data(), SAMPLE_STRING.size(), &sampleStr), ANI_OK);
    ani_ref unionResult {};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Ref(cls, "foo", FOO_UNION_SIGNATURE.data(), &unionResult, sampleStr),
              ANI_OK);
    // Check returned object is of correct type
    ani_boolean booleanResult = ANI_FALSE;
    ASSERT_EQ(env_->Reference_IsUndefined(unionResult, &booleanResult), ANI_OK);
    ASSERT_EQ(booleanResult, ANI_FALSE);
    ani_enum enumClass {};
    ASSERT_EQ(env_->FindEnum("msig.E", &enumClass), ANI_OK);
    ASSERT_EQ(env_->Object_InstanceOf(static_cast<ani_object>(unionResult), enumClass, &booleanResult), ANI_OK);
    ASSERT_EQ(booleanResult, ANI_TRUE);

    // Check never
    ani_ref res;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Ref(cls, "fooNever", ":N", &res), ANI_PENDING_ERROR);

    ani_boolean hasError = ANI_FALSE;
    ASSERT_EQ(env_->ExistUnhandledError(&hasError), ANI_OK);
    ASSERT_EQ(hasError, ANI_TRUE);
    ASSERT_EQ(env_->ResetError(), ANI_OK);

    // Check any
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Ref(cls, "fooAny", ":Y", &res), ANI_OK);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Void(cls, "fooAny", "Y:", res), ANI_OK);
}

TEST_F(MangleSignatureTest, Class_CallStaticMethodByName_OldFormat)
{
    std::optional<PandaString> desc;

    ani_class cls {};
    ASSERT_EQ(env_->FindClass("msig.G", &cls), ANI_OK);

    ani_int result {};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(cls, "foo", "III:I", &result, 1, 2U, 3U), ANI_INVALID_DESCRIPTOR);
}

TEST_F(MangleSignatureTest, Object_CallMethodByName)
{
    std::optional<PandaString> desc;

    ani_class cls {};
    ASSERT_EQ(env_->FindClass("msig.F", &cls), ANI_OK);

    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", ":", &ctor), ANI_OK);

    ani_object object {};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Object_New(cls, ctor, &object), ANI_OK);

    ani_int result {};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Object_CallMethodByName_Int(object, "foo", "ii:i", &result, 1, 2U), ANI_OK);
    ASSERT_EQ(result, 1 + 2U);

    // Check correct union type is returned from `Object_CallMethodByName_Ref`
    static constexpr std::string_view SAMPLE_STRING = "sample";
    ani_string sampleStr {};
    ASSERT_EQ(env_->String_NewUTF8(SAMPLE_STRING.data(), SAMPLE_STRING.size(), &sampleStr), ANI_OK);
    ani_ref unionResult {};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Object_CallMethodByName_Ref(object, "foo", FOO_UNION_SIGNATURE.data(), &unionResult, sampleStr),
              ANI_OK);
    // Check returned object is of correct type
    ani_boolean booleanResult = ANI_FALSE;
    ASSERT_EQ(env_->Reference_IsUndefined(unionResult, &booleanResult), ANI_OK);
    ASSERT_EQ(booleanResult, ANI_FALSE);
    ani_enum enumClass {};
    ASSERT_EQ(env_->FindEnum("msig.E", &enumClass), ANI_OK);
    ASSERT_EQ(env_->Object_InstanceOf(static_cast<ani_object>(unionResult), enumClass, &booleanResult), ANI_OK);
    ASSERT_EQ(booleanResult, ANI_TRUE);

    // Check never
    ani_ref res;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Object_CallMethodByName_Ref(object, "fooNever", ":N", &res), ANI_PENDING_ERROR);

    ani_boolean hasError = ANI_FALSE;
    ASSERT_EQ(env_->ExistUnhandledError(&hasError), ANI_OK);
    ASSERT_EQ(hasError, ANI_TRUE);
    ASSERT_EQ(env_->ResetError(), ANI_OK);

    // Check any
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Object_CallMethodByName_Ref(object, "fooAny", ":Y", &res), ANI_OK);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Object_CallMethodByName_Void(object, "fooAny", "Y:", res), ANI_OK);
}

TEST_F(MangleSignatureTest, Object_CallMethodByName_OldFormat)
{
    std::optional<PandaString> desc;

    ani_class cls {};
    ASSERT_EQ(env_->FindClass("msig.F", &cls), ANI_OK);

    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", ":", &ctor), ANI_OK);

    ani_object object {};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Object_New(cls, ctor, &object), ANI_OK);

    ani_int result {};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Object_CallMethodByName_Int(object, "foo", "II:I", &result, 1, 2U), ANI_INVALID_DESCRIPTOR);
}

static EtsArray *ClassBindNativeFunctionsFoo1(EtsArray *data)
{
    return data;
}

static void ClassBindNativeFunctionsFoo2([[maybe_unused]] EtsArray *data, [[maybe_unused]] EtsObject *fn) {}

TEST_F(MangleSignatureTest, Class_BindNativeMethods)
{
    auto *foo1 = reinterpret_cast<void *>(ClassBindNativeFunctionsFoo1);
    auto *foo2 = reinterpret_cast<void *>(ClassBindNativeFunctionsFoo2);

    std::array functions = {
        ani_native_function {"foo", "C{std.core.Array}:C{std.core.Array}", foo1},
        ani_native_function {"foo", "C{std.core.Array}C{std.core.Function1}:", foo2},
    };

    ani_class cls {};
    ASSERT_EQ(env_->FindClass("msig.F", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_BindNativeMethods(cls, functions.data(), functions.size()), ANI_OK);
}

TEST_F(MangleSignatureTest, Class_BindNativeMethods_OldFormat)
{
    auto *foo1 = reinterpret_cast<void *>(ClassBindNativeFunctionsFoo1);
    auto *foo2 = reinterpret_cast<void *>(ClassBindNativeFunctionsFoo2);

    std::array functions = {
        ani_native_function {"foo", "Lstd/core/Array;:Lstd/core/Array;", foo1},
        ani_native_function {"foo", "Lstd/core/Array;Lstd/core/Function1;:V", foo2},
    };

    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lmsig/F;", &cls), ANI_INVALID_DESCRIPTOR);
    ASSERT_EQ(env_->FindClass("msig.F", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_BindNativeMethods(cls, functions.data(), functions.size()), ANI_INVALID_DESCRIPTOR);
}

TEST_F(MangleSignatureTest, Namespace_BindNativeFunctions)
{
    auto *foo1 = reinterpret_cast<void *>(ClassBindNativeFunctionsFoo1);
    auto *foo2 = reinterpret_cast<void *>(ClassBindNativeFunctionsFoo2);

    std::array functions = {
        ani_native_function {"foo", "C{std.core.Array}:C{std.core.Array}", foo1},
        ani_native_function {"foo", "C{std.core.Array}C{std.core.Function1}:", foo2},
    };

    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("msig.rls", &ns), ANI_OK);
    ASSERT_EQ(env_->Namespace_BindNativeFunctions(ns, functions.data(), functions.size()), ANI_OK);
}

TEST_F(MangleSignatureTest, Namespace_BindNativeFunctions_OldFormat)
{
    auto *foo1 = reinterpret_cast<void *>(ClassBindNativeFunctionsFoo1);
    auto *foo2 = reinterpret_cast<void *>(ClassBindNativeFunctionsFoo2);

    std::array functions = {
        ani_native_function {"foo", "Lstd/core/Array;:Lstd/core/Array;", foo1},
        ani_native_function {"foo", "Lstd/core/Array;Lstd/core/Function1;:V", foo2},
    };

    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lmsig/rls;", &ns), ANI_INVALID_DESCRIPTOR);
    ASSERT_EQ(env_->FindNamespace("msig.rls", &ns), ANI_OK);
    ASSERT_EQ(env_->Namespace_BindNativeFunctions(ns, functions.data(), functions.size()), ANI_INVALID_DESCRIPTOR);
}

TEST_F(MangleSignatureTest, Module_BindNativeFunctions)
{
    auto *foo1 = reinterpret_cast<void *>(ClassBindNativeFunctionsFoo1);
    auto *foo2 = reinterpret_cast<void *>(ClassBindNativeFunctionsFoo2);

    std::array functions = {
        ani_native_function {"foo", "C{std.core.Array}:C{std.core.Array}", foo1},
        ani_native_function {"foo", "C{std.core.Array}C{std.core.Function1}:", foo2},
    };

    ani_module m {};
    ASSERT_EQ(env_->FindModule("msig", &m), ANI_OK);
    ASSERT_EQ(env_->Module_BindNativeFunctions(m, functions.data(), functions.size()), ANI_OK);
}

TEST_F(MangleSignatureTest, Module_BindNativeFunctions_OldFormat)
{
    auto *foo1 = reinterpret_cast<void *>(ClassBindNativeFunctionsFoo1);
    auto *foo2 = reinterpret_cast<void *>(ClassBindNativeFunctionsFoo2);

    std::array functions = {
        ani_native_function {"foo", "Lstd/core/Array;:Lstd/core/Array;", foo1},
        ani_native_function {"foo", "Lstd/core/Array;Lstd/core/Function1;:V", foo2},
    };

    ani_module m {};
    ASSERT_EQ(env_->FindModule("Lmsig;", &m), ANI_INVALID_DESCRIPTOR);
    ASSERT_EQ(env_->FindModule("msig", &m), ANI_OK);
    ASSERT_EQ(env_->Module_BindNativeFunctions(m, functions.data(), functions.size()), ANI_INVALID_DESCRIPTOR);
}

}  // namespace ark::ets::ani::testing
