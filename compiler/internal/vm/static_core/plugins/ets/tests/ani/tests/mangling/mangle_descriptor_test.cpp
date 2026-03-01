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

#include "ani_gtest.h"
#include "plugins/ets/runtime/ani/ani_mangle.h"

namespace ark::ets::ani::testing {

class MangleDescriptorTest : public AniTest {};

TEST_F(MangleDescriptorTest, Format_NewToOld)
{
    std::optional<PandaString> desc;

    desc = Mangle::ConvertDescriptor("a");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), "La;");

    desc = Mangle::ConvertDescriptor("a.b");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), "La/b;");

    desc = Mangle::ConvertDescriptor("a.b.c");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), "La/b/c;");

    desc = Mangle::ConvertDescriptor("A{C{a.b.c}}");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), "LA{C{a/b/c}};");

    desc = Mangle::ConvertDescriptor("A{A{C{a.b.c}}}");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), "LA{A{C{a/b/c}}};");

    desc = Mangle::ConvertDescriptor("A{A{E{a.b.c}}}");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), "LA{A{E{a/b/c}}};");

    desc = Mangle::ConvertDescriptor("A{A{A{C{a.b.c}}}}");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), "LA{A{A{C{a/b/c}}}};");

    desc = Mangle::ConvertDescriptor("A{C{a.b.c}}", true);
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), "[La/b/c;");

    desc = Mangle::ConvertDescriptor("A{A{C{a.b.c}}}", true);
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), "[[La/b/c;");

    desc = Mangle::ConvertDescriptor("A{A{E{a.b.c}}}", true);
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), "[[La/b/c;");

    desc = Mangle::ConvertDescriptor("A{A{A{C{a.b.c}}}}", true);
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), "[[[La/b/c;");

    desc = Mangle::ConvertDescriptor("C{a.b.c}");
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), "LC{a/b/c};");

    desc = Mangle::ConvertDescriptor("C{a.b.c}", true);
    ASSERT_TRUE(desc.has_value());
    EXPECT_STREQ(desc.value().c_str(), "LC{a/b/c};");
}

TEST_F(MangleDescriptorTest, Format_OldToOld)
{
    std::optional<PandaString> desc;

    desc = Mangle::ConvertDescriptor("La;");
    ASSERT_FALSE(desc.has_value());

    desc = Mangle::ConvertDescriptor("La/b;");
    ASSERT_FALSE(desc.has_value());

    desc = Mangle::ConvertDescriptor("La/b/c;");
    ASSERT_FALSE(desc.has_value());
}

TEST_F(MangleDescriptorTest, Format_Wrong)
{
    std::optional<PandaString> desc;

    desc = Mangle::ConvertDescriptor("");
    ASSERT_FALSE(desc.has_value());

    desc = Mangle::ConvertDescriptor("a;");
    ASSERT_FALSE(desc.has_value());

    desc = Mangle::ConvertDescriptor("a/b");
    ASSERT_FALSE(desc.has_value());

    desc = Mangle::ConvertDescriptor("a/b/c");
    ASSERT_FALSE(desc.has_value());
}

TEST_F(MangleDescriptorTest, FindModule)
{
    ani_module mod {};
    EXPECT_EQ(env_->FindModule("mm.mangle_descriptor_test", &mod), ANI_OK);
}

TEST_F(MangleDescriptorTest, FindModule_OldFormat)
{
    ani_module mod {};
    EXPECT_EQ(env_->FindModule("Lmm/mangle_descriptor_test;", &mod), ANI_INVALID_DESCRIPTOR);
}

TEST_F(MangleDescriptorTest, FindNamespace)
{
    ani_namespace ns {};
    EXPECT_EQ(env_->FindNamespace("mm.mangle_descriptor_test.rls", &ns), ANI_OK);
    EXPECT_EQ(env_->FindNamespace("mm.mangle_descriptor_test.rls.ns", &ns), ANI_OK);
}

TEST_F(MangleDescriptorTest, FindNamespace_OldFormat)
{
    ani_namespace ns {};
    EXPECT_EQ(env_->FindNamespace("Lmm/mangle_descriptor_test/rls;", &ns), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->FindNamespace("Lmm/mangle_descriptor_test/rls/ns;", &ns), ANI_INVALID_DESCRIPTOR);
}

TEST_F(MangleDescriptorTest, FindClass)
{
    ani_class cls {};
    EXPECT_EQ(env_->FindClass("mm.mangle_descriptor_test.A", &cls), ANI_OK);
    EXPECT_EQ(env_->FindClass("mm.mangle_descriptor_test.B", &cls), ANI_OK);
    EXPECT_EQ(env_->FindClass("mm.mangle_descriptor_test.rls.A", &cls), ANI_OK);
    EXPECT_EQ(env_->FindClass("mm.mangle_descriptor_test.rls.B", &cls), ANI_OK);
    EXPECT_EQ(env_->FindClass("mm.mangle_descriptor_test.rls.ns.A", &cls), ANI_OK);
    EXPECT_EQ(env_->FindClass("mm.mangle_descriptor_test.rls.ns.B", &cls), ANI_OK);
    EXPECT_EQ(env_->FindClass("C{mm.mangle_descriptor_test.rls.ns.A}", &cls), ANI_NOT_FOUND);
    EXPECT_EQ(env_->FindClass("C{mm.mangle_descriptor_test.rls.ns.B}", &cls), ANI_NOT_FOUND);
    EXPECT_EQ(env_->FindClass("A{E{mm.mangle_descriptor_test.E}}", &cls), ANI_OK);
    EXPECT_EQ(env_->FindClass("A{C{mm.mangle_descriptor_test.rls.ns.A}}", &cls), ANI_OK);
    EXPECT_EQ(env_->FindClass("A{C{mm.mangle_descriptor_test.rls.ns.B}}", &cls), ANI_OK);
}

TEST_F(MangleDescriptorTest, FindClass_OldFormat)
{
    ani_class cls {};
    EXPECT_EQ(env_->FindClass("Lmm/mangle_descriptor_test/A;", &cls), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->FindClass("Lmm/mangle_descriptor_test/B;", &cls), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->FindClass("Lmm/mangle_descriptor_test/rls/A;", &cls), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->FindClass("Lmm/mangle_descriptor_test/rls/B;", &cls), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->FindClass("Lmm/mangle_descriptor_test/rls/ns/A;", &cls), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->FindClass("Lmm/mangle_descriptor_test/rls/ns/B;", &cls), ANI_INVALID_DESCRIPTOR);
}

TEST_F(MangleDescriptorTest, FindEnum)
{
    ani_enum enm {};
    EXPECT_EQ(env_->FindEnum("mm.mangle_descriptor_test.E", &enm), ANI_OK);
    EXPECT_EQ(env_->FindEnum("mm.mangle_descriptor_test.rls.E", &enm), ANI_OK);
    EXPECT_EQ(env_->FindEnum("mm.mangle_descriptor_test.rls.ns.E", &enm), ANI_OK);
    EXPECT_EQ(env_->FindEnum("E{mm.mangle_descriptor_test.E}", &enm), ANI_NOT_FOUND);
}

TEST_F(MangleDescriptorTest, FindEnum_OldFormat)
{
    ani_enum enm {};
    EXPECT_EQ(env_->FindEnum("Lmm/mangle_descriptor_test/E;", &enm), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->FindEnum("Lmm/mangle_descriptor_test/rls/E;", &enm), ANI_INVALID_DESCRIPTOR);
    EXPECT_EQ(env_->FindEnum("Lmm/mangle_descriptor_test/rls/ns/E;", &enm), ANI_INVALID_DESCRIPTOR);
}

}  // namespace ark::ets::ani::testing
