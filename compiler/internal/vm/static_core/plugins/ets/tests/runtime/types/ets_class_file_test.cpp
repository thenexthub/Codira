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

#include <gtest/gtest.h>

#include "get_test_class.h"
#include "ets_coroutine.h"

#include "types/ets_class.h"
#include "types/ets_method.h"
#include "ets_utils.h"

// NOLINTBEGIN(readability-magic-numbers)

namespace ark::ets::test {

namespace {

void InSamePackagePrologue()
{
    {
        const char *source = R"(
            .language eTS
            .record Test {}
        )";

        EtsClass *klass = GetTestClass(source, "LTest;");
        ASSERT_NE(klass, nullptr);

        EtsArray *array1 = EtsObjectArray::Create(klass, 1000U);
        EtsArray *array2 = EtsFloatArray::Create(1000U);
        ASSERT_NE(array1, nullptr);
        ASSERT_NE(array2, nullptr);

        EtsClass *klass1 = array1->GetClass();
        EtsClass *klass2 = array2->GetClass();
        ASSERT_NE(klass1, nullptr);
        ASSERT_NE(klass2, nullptr);

        ASSERT_TRUE(klass1->IsInSamePackage(klass2));
    }
    {
        EtsArray *array1 = EtsFloatArray::Create(1000U);
        EtsArray *array2 = EtsIntArray::Create(1000U);
        ASSERT_NE(array1, nullptr);
        ASSERT_NE(array2, nullptr);

        EtsClass *klass1 = array1->GetClass();
        EtsClass *klass2 = array2->GetClass();
        ASSERT_NE(klass1, nullptr);
        ASSERT_NE(klass2, nullptr);

        ASSERT_TRUE(klass1->IsInSamePackage(klass2));
    }
}

}  // namespace

class EtsClassTest : public testing::Test {
public:
    EtsClassTest()
    {
        options_.SetShouldLoadBootPandaFiles(true);
        options_.SetShouldInitializeIntrinsics(false);
        options_.SetCompilerEnableJit(false);
        options_.SetGcType("epsilon");
        options_.SetLoadRuntimes({"ets"});

        auto stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to mock_stdlib.abc" << std::endl;
            std::abort();
        }
        options_.SetBootPandaFiles({stdlib});

        Runtime::Create(options_);
    }

    ~EtsClassTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(EtsClassTest);
    NO_MOVE_SEMANTIC(EtsClassTest);

    void SetUp() override
    {
        coroutine_ = EtsCoroutine::GetCurrent();
        coroutine_->ManagedCodeBegin();
    }

    void TearDown() override
    {
        coroutine_->ManagedCodeEnd();
    }

private:
    RuntimeOptions options_;
    EtsCoroutine *coroutine_ = nullptr;
};

TEST_F(EtsClassTest, GetBase)
{
    {
        const char *source = R"(
            .language eTS
            .record A {}
            .record B <ets.extends = A> {}
            .record C <ets.extends = B> {}
        )";

        EtsClass *klassA = GetTestClass(source, "LA;");
        EtsClass *klassB = GetTestClass(source, "LB;");
        EtsClass *klassC = GetTestClass(source, "LC;");
        ASSERT_NE(klassA, nullptr);
        ASSERT_NE(klassB, nullptr);
        ASSERT_NE(klassC, nullptr);

        ASSERT_EQ(klassC->GetBase(), klassB);
        ASSERT_EQ(klassB->GetBase(), klassA);
        ASSERT_STREQ(klassA->GetBase()->GetDescriptor(), "Lstd/core/Object;");
    }
    {
        const char *source = R"(
            .language eTS
            .record ITest <ets.abstract, ets.interface>{}
        )";

        EtsClass *klassItest = GetTestClass(source, "LITest;");

        ASSERT_NE(klassItest, nullptr);
        ASSERT_TRUE(klassItest->IsInterface());
        ASSERT_TRUE(klassItest->GetBase()->IsEtsObject());
    }
}

TEST_F(EtsClassTest, GetStaticFieldsNumber)
{
    {
        const char *source = R"(
            .language eTS
            .record A {
                f32 x <static>
                i32 y <static>
                f32 z <static>
            }
        )";

        EtsClass *klass = GetTestClass(source, "LA;");
        ASSERT_NE(klass, nullptr);
        ASSERT_EQ(klass->GetStaticFieldsNumber(), 3U);
        ASSERT_EQ(klass->GetInstanceFieldsNumber(), 0);
    }
    {
        const char *source = R"(
            .language eTS
            .record B {}
        )";

        EtsClass *klass = GetTestClass(source, "LB;");
        ASSERT_NE(klass, nullptr);
        ASSERT_EQ(klass->GetStaticFieldsNumber(), 0);
        ASSERT_EQ(klass->GetInstanceFieldsNumber(), 0);
    }
}

TEST_F(EtsClassTest, GetInstanceFieldsNumber)
{
    {
        const char *source = R"(
            .language eTS
            .record TestObject {}
            .record A {
                TestObject x
                TestObject y
                TestObject z
                TestObject k
            }
        )";

        EtsClass *klass = GetTestClass(source, "LA;");
        ASSERT_NE(klass, nullptr);
        ASSERT_EQ(klass->GetStaticFieldsNumber(), 0);
        ASSERT_EQ(klass->GetInstanceFieldsNumber(), 4U);
    }
    {
        const char *source = R"(
            .language eTS
            .record B {
                f32 x <static>
            }
        )";

        EtsClass *klass = GetTestClass(source, "LB;");
        ASSERT_NE(klass, nullptr);
        ASSERT_EQ(klass->GetStaticFieldsNumber(), 1);
        ASSERT_EQ(klass->GetInstanceFieldsNumber(), 0);
    }
}

TEST_F(EtsClassTest, GetFieldIndexByName)
{
    const char *source = R"(
            .language eTS
            .record TestObject {}
            .record A {
                TestObject obj_x
                TestObject obj_y
                TestObject obj_z
                TestObject obj_k
            }
            .record B <ets.extends = A> {
                TestObject obj_l
            }
        )";

    EtsClass *klass = GetTestClass(source, "LB;");
    ASSERT_NE(klass, nullptr);

    std::unordered_set<std::uint32_t> descSetIndex = {0, 1, 2, 3};
    ASSERT_TRUE(descSetIndex.count(klass->GetFieldIndexByName("obj_x")) > 0);
    descSetIndex.erase(klass->GetFieldIndexByName("obj_x"));
    ASSERT_TRUE(descSetIndex.count(klass->GetFieldIndexByName("obj_k")) > 0);
    descSetIndex.erase(klass->GetFieldIndexByName("obj_k"));
    ASSERT_TRUE(descSetIndex.count(klass->GetFieldIndexByName("obj_p")) == 0);
}

TEST_F(EtsClassTest, GetFieldIDByName)
{
    const char *source = R"(
        .language eTS
        .record TestObject {}
        .record Test {
            TestObject x
            i32 y
        }
    )";

    EtsClass *klass = GetTestClass(source, "LTest;");
    ASSERT_NE(klass, nullptr);

    ASSERT_EQ(klass->GetFieldIDByName("x", "F"), nullptr);
    ASSERT_NE(klass->GetFieldIDByName("x", "LTestObject;"), nullptr);
    ASSERT_EQ(klass->GetFieldIDByName("y", "Z"), nullptr);
    ASSERT_NE(klass->GetFieldIDByName("y", "I"), nullptr);

    EtsField *fieldX = klass->GetFieldIDByName("x");
    EtsField *fieldY = klass->GetFieldIDByName("y");
    ASSERT_NE(fieldX, nullptr);
    ASSERT_NE(fieldY, nullptr);
    ASSERT_FALSE(fieldX->IsStatic());
    ASSERT_FALSE(fieldY->IsStatic());

    ASSERT_EQ(fieldX, klass->GetFieldIDByOffset(fieldX->GetOffset()));
    ASSERT_EQ(fieldY, klass->GetFieldIDByOffset(fieldY->GetOffset()));
}

TEST_F(EtsClassTest, GetDeclaredFieldIDByName)
{
    const char *source = R"(
        .language eTS
        .record Ball {
            f32 BALL_RADIUS
            i32 BALL_SPEED
            f32 vx
            i32 vy
        }
    )";

    EtsClass *klass = GetTestClass(source, "LBall;");
    ASSERT_NE(klass, nullptr);

    EtsField *field1 = klass->GetDeclaredFieldIDByName("BALL_RADIUS");
    ASSERT_NE(field1, nullptr);
    ASSERT_TRUE(!strcmp(field1->GetName(), "BALL_RADIUS"));

    EtsField *field2 = klass->GetDeclaredFieldIDByName("BALL_NUM");
    ASSERT_EQ(field2, nullptr);

    EtsField *field3 = klass->GetDeclaredFieldIDByName("vx");
    ASSERT_NE(field3, nullptr);
    ASSERT_TRUE(!strcmp(field3->GetName(), "vx"));

    ASSERT_EQ(field1, klass->GetFieldIDByOffset(field1->GetOffset()));
    ASSERT_EQ(field3, klass->GetFieldIDByOffset(field3->GetOffset()));
}

TEST_F(EtsClassTest, GetStaticFieldIDByName)
{
    const char *source = R"(
        .language eTS
        .record TestObject {}
        .record Test {
            TestObject x <static>
            i32 y <static>
        }
    )";

    EtsClass *klass = GetTestClass(source, "LTest;");
    ASSERT_NE(klass, nullptr);

    ASSERT_EQ(klass->GetStaticFieldIDByName("x", "F"), nullptr);
    ASSERT_NE(klass->GetStaticFieldIDByName("x", "LTestObject;"), nullptr);
    ASSERT_EQ(klass->GetStaticFieldIDByName("y", "Z"), nullptr);
    ASSERT_NE(klass->GetStaticFieldIDByName("y", "I"), nullptr);

    EtsField *fieldX = klass->GetStaticFieldIDByName("x");
    EtsField *fieldY = klass->GetStaticFieldIDByName("y");
    ASSERT_NE(fieldX, nullptr);
    ASSERT_NE(fieldY, nullptr);
    ASSERT_TRUE(fieldX->IsStatic());
    ASSERT_TRUE(fieldY->IsStatic());

    ASSERT_EQ(fieldX, klass->GetStaticFieldIDByOffset(fieldX->GetOffset()));
    ASSERT_EQ(fieldY, klass->GetStaticFieldIDByOffset(fieldY->GetOffset()));
}

TEST_F(EtsClassTest, SetAndGetStaticFieldPrimitive)
{
    const char *source = R"(
        .language eTS
        .record Test {
            i32 x <static>
            f32 y <static>
        }
    )";

    EtsClass *klass = GetTestClass(source, "LTest;");
    ASSERT_NE(klass, nullptr);

    EtsField *fieldX = klass->GetStaticFieldIDByName("x");
    EtsField *fieldY = klass->GetStaticFieldIDByName("y");
    ASSERT_NE(fieldX, nullptr);
    ASSERT_NE(fieldY, nullptr);

    int32_t nmb1 = 53;
    float nmb2 = 123.90;

    klass->SetStaticFieldPrimitive<int32_t>(fieldX, nmb1);
    klass->SetStaticFieldPrimitive<float>(fieldY, nmb2);

    ASSERT_EQ(klass->GetStaticFieldPrimitive<int32_t>(fieldX), nmb1);
    ASSERT_EQ(klass->GetStaticFieldPrimitive<float>(fieldY), nmb2);
}

TEST_F(EtsClassTest, SetAndGetStaticFieldPrimitiveByOffset)
{
    const char *source = R"(
        .language eTS
        .record Test {
            i32 x <static>
            f32 y <static>
        }
    )";

    EtsClass *klass = GetTestClass(source, "LTest;");
    ASSERT_NE(klass, nullptr);

    std::size_t fieldXOffset = klass->GetStaticFieldIDByName("x")->GetOffset();
    std::size_t fieldYOffset = klass->GetStaticFieldIDByName("y")->GetOffset();

    int32_t nmb1 = 53;
    float nmb2 = 123.90;

    klass->SetStaticFieldPrimitive<int32_t>(fieldXOffset, false, nmb1);
    klass->SetStaticFieldPrimitive<float>(fieldYOffset, false, nmb2);

    ASSERT_EQ(klass->GetStaticFieldPrimitive<int32_t>(fieldXOffset, false), nmb1);
    ASSERT_EQ(klass->GetStaticFieldPrimitive<float>(fieldYOffset, false), nmb2);
}

TEST_F(EtsClassTest, SetAndGetFieldObject)
{
    const char *source = R"(
        .language eTS
        .record Foo {}
        .record Bar {
            Foo x <static>
            Foo y <static>
        }
    )";

    EtsClass *barKlass = GetTestClass(source, "LBar;");
    EtsClass *fooKlass = GetTestClass(source, "LFoo;");
    ASSERT_NE(barKlass, nullptr);
    ASSERT_NE(fooKlass, nullptr);

    EtsObject *fooObj1 = EtsObject::Create(fooKlass);
    EtsObject *fooObj2 = EtsObject::Create(fooKlass);
    ASSERT_NE(fooObj1, nullptr);
    ASSERT_NE(fooObj2, nullptr);

    EtsField *fieldX = barKlass->GetStaticFieldIDByName("x");
    EtsField *fieldY = barKlass->GetStaticFieldIDByName("y");
    ASSERT_NE(fieldX, nullptr);
    ASSERT_NE(fieldY, nullptr);

    barKlass->SetStaticFieldObject(fieldX, fooObj1);
    barKlass->SetStaticFieldObject(fieldY, fooObj2);
    ASSERT_EQ(barKlass->GetStaticFieldObject(fieldX), fooObj1);
    ASSERT_EQ(barKlass->GetStaticFieldObject(fieldY), fooObj2);

    barKlass->SetStaticFieldObject(fieldX, fooObj2);
    barKlass->SetStaticFieldObject(fieldY, fooObj1);
    ASSERT_EQ(barKlass->GetStaticFieldObject(fieldX), fooObj2);
    ASSERT_EQ(barKlass->GetStaticFieldObject(fieldY), fooObj1);
}

TEST_F(EtsClassTest, SetAndGetFieldObjectByOffset)
{
    const char *source = R"(
        .language eTS
        .record Foo {}
        .record Bar {
            Foo x <static>
            Foo y <static>
        }
    )";

    EtsClass *barKlass = GetTestClass(source, "LBar;");
    EtsClass *fooKlass = GetTestClass(source, "LFoo;");
    ASSERT_NE(barKlass, nullptr);
    ASSERT_NE(fooKlass, nullptr);

    EtsObject *fooObj1 = EtsObject::Create(fooKlass);
    EtsObject *fooObj2 = EtsObject::Create(fooKlass);
    ASSERT_NE(fooObj1, nullptr);
    ASSERT_NE(fooObj2, nullptr);

    EtsField *fieldX = barKlass->GetStaticFieldIDByName("x");
    EtsField *fieldY = barKlass->GetStaticFieldIDByName("y");
    ASSERT_NE(fieldX, nullptr);
    ASSERT_NE(fieldY, nullptr);

    barKlass->SetStaticFieldObject(fieldX->GetOffset(), false, fooObj1);
    barKlass->SetStaticFieldObject(fieldY->GetOffset(), false, fooObj2);
    ASSERT_EQ(barKlass->GetStaticFieldObject(fieldX->GetOffset(), false), fooObj1);
    ASSERT_EQ(barKlass->GetStaticFieldObject(fieldY->GetOffset(), false), fooObj2);

    barKlass->SetStaticFieldObject(fieldX, fooObj2);
    barKlass->SetStaticFieldObject(fieldY, fooObj1);
    ASSERT_EQ(barKlass->GetStaticFieldObject(fieldX), fooObj2);
    ASSERT_EQ(barKlass->GetStaticFieldObject(fieldY), fooObj1);
}

TEST_F(EtsClassTest, GetDirectMethod)
{
    const char *source = R"(
        .language eTS
        .record A {}
        .record B <ets.extends = A> {}
        .record TestObject {}

        .function void A.foo1() {
            return.void
        }
        .function TestObject B.foo2(i32 a0, i32 a1, f32 a2, f64 a3, f32 a4) {
            newobj v0, TestObject
            lda.obj v0
            return.obj
        }
    )";

    EtsClass *klassA = GetTestClass(source, "LA;");
    EtsClass *klassB = GetTestClass(source, "LB;");
    ASSERT_NE(klassA, nullptr);
    ASSERT_NE(klassB, nullptr);

    EtsMethod *methodFoo1 = klassA->GetDirectMethod("foo1", ":V");
    ASSERT_NE(methodFoo1, nullptr);
    ASSERT_TRUE(!strcmp(methodFoo1->GetName(), "foo1"));

    EtsMethod *methodFoo2 = klassB->GetDirectMethod("foo2", "IIFDF:LTestObject;");
    ASSERT_NE(methodFoo2, nullptr);
    ASSERT_TRUE(!strcmp(methodFoo2->GetName(), "foo2"));

    // GetDirectMethod can't find method from base class
    EtsMethod *methodFoo1FromKlassB = klassB->GetDirectMethod("foo1", ":V");
    ASSERT_EQ(methodFoo1FromKlassB, nullptr);
}

TEST_F(EtsClassTest, GetMethod)
{
    const char *source = R"(
        .language eTS
        .record A {}
        .record B <ets.extends = A> {}
        .record TestObject {}

        .function void A.foo1() {
            return.void
        }
        .function TestObject B.foo2(i32 a0, i32 a1, f32 a2, f64 a3, f32 a4) {
            newobj v0, TestObject
            lda.obj v0
            return.obj
        }
    )";

    EtsClass *klassA = GetTestClass(source, "LA;");
    EtsClass *klassB = GetTestClass(source, "LB;");
    ASSERT_NE(klassA, nullptr);
    ASSERT_NE(klassB, nullptr);

    EtsMethod *methodFoo1 = klassA->GetStaticMethod("foo1", ":V");
    ASSERT_NE(methodFoo1, nullptr);
    ASSERT_TRUE(!strcmp(methodFoo1->GetName(), "foo1"));

    EtsMethod *methodFoo2 = klassB->GetStaticMethod("foo2", "IIFDF:LTestObject;");
    ASSERT_NE(methodFoo2, nullptr);
    ASSERT_TRUE(!strcmp(methodFoo2->GetName(), "foo2"));

    // GetMethod can find method from base class
    EtsMethod *methodFoo1FromKlassB = klassB->GetStaticMethod("foo1", nullptr);
    ASSERT_NE(methodFoo1FromKlassB, nullptr);
    ASSERT_TRUE(!strcmp(methodFoo1FromKlassB->GetName(), "foo1"));
}

TEST_F(EtsClassTest, GetMethodByIndex)
{
    const char *source = R"(
        .language eTS
        .record NN {}
        .record A {}
        .record B <ets.extends = A> {}
        .record TestObject {}

        .function void A.foo1() {
            return.void
        }
        .function void A.foo2() {
            return.void
        }
        .function TestObject B.foo3(i32 a0, i32 a1, f32 a2, f64 a3, f32 a4) {
            newobj v0, TestObject
            lda.obj v0
            return.obj
        }
    )";

    EtsClass *klassA = GetTestClass(source, "LA;");
    EtsClass *klassB = GetTestClass(source, "LB;");
    ASSERT_NE(klassA, nullptr);
    ASSERT_NE(klassB, nullptr);

    // test Method:GetMethodsNum
    std::size_t klassAMethodSize = 2;
    std::size_t klassBMethodSize = 3;
    ASSERT_EQ(klassAMethodSize, klassA->GetMethodsNum());
    ASSERT_EQ(klassBMethodSize, klassB->GetMethodsNum());

    EtsClass *klassN = GetTestClass(source, "LNN;");
    ASSERT_NE(klassN, nullptr);
    ASSERT_FALSE(klassN->GetMethodsNum());

    std::unordered_set<std::string> descSetA = {"foo1", "foo2"};
    for (std::size_t i = 0; i < klassAMethodSize; ++i) {
        ASSERT_TRUE(descSetA.count(klassA->GetMethodByIndex(i)->GetName()) > 0);
        descSetA.erase(klassA->GetMethodByIndex(i)->GetName());
    }

    std::unordered_set<std::string> descSetB = {"foo1", "foo2", "foo3"};
    for (std::size_t i = 0; i < klassBMethodSize; ++i) {
        ASSERT_TRUE(descSetB.count(klassB->GetMethodByIndex(i)->GetName()) > 0);
        descSetB.erase(klassB->GetMethodByIndex(i)->GetName());
    }
}

TEST_F(EtsClassTest, GetMethods)
{
    const char *source = R"(
        .language eTS
        .record NN {}
        .record A {}
        .record B <ets.extends = A> {}
        .record TestObject {}

        .function void A.foo1() {
            return.void
        }
        .function void A.foo2() {
            return.void
        }
        .function TestObject B.foo3(i32 a0, i32 a1, f32 a2, f64 a3, f32 a4) {
            newobj v0, TestObject
            lda.obj v0
            return.obj
        }
    )";

    EtsClass *klassA = GetTestClass(source, "LA;");
    EtsClass *klassB = GetTestClass(source, "LB;");
    ASSERT_NE(klassA, nullptr);
    ASSERT_NE(klassB, nullptr);

    std::unordered_set<std::string> descSetA = {"foo1", "foo2"};
    auto etsMethodsA = PandaVector<EtsMethod *>();
    etsMethodsA = klassA->GetMethods();
    std::size_t etsMethodsASize = etsMethodsA.size();
    for (std::size_t i = 0; i < etsMethodsASize; ++i) {
        ASSERT_TRUE(descSetA.count(etsMethodsA.at(i)->GetName()) > 0);
        descSetA.erase(etsMethodsA.at(i)->GetName());
    }

    std::unordered_set<std::string> descSetB = {"foo1", "foo2", "foo3"};
    auto etsMethodsB = PandaVector<EtsMethod *>();
    etsMethodsB = klassB->GetMethods();
    std::size_t etsMethodsBSize = etsMethodsB.size();
    for (std::size_t i = 0; i < etsMethodsBSize; ++i) {
        ASSERT_TRUE(descSetB.count(etsMethodsB.at(i)->GetName()) > 0);
        descSetB.erase(etsMethodsB.at(i)->GetName());
    }

    EtsClass *klassN = GetTestClass(source, "LNN;");
    ASSERT_NE(klassN, nullptr);
    auto etsMethodsC = PandaVector<EtsMethod *>();
    etsMethodsC = klassN->GetMethods();
    ASSERT_TRUE(etsMethodsC.empty());
}

TEST_F(EtsClassTest, SetAndGetName)
{
    const char *source = R"(
        .language eTS
        .record A {}
        .record AB {}
        .record ABC {}
    )";

    EtsClass *klass = GetTestClass(source, "LA;");
    ASSERT_NE(klass, nullptr);
    ASSERT_TRUE(klass->GetName()->StringsAreEqual(EtsString::CreateFromMUtf8("A")->AsObject()));

    klass = GetTestClass(source, "LAB;");
    ASSERT_NE(klass, nullptr);
    ASSERT_TRUE(klass->GetName()->StringsAreEqual(EtsString::CreateFromMUtf8("AB")->AsObject()));

    klass = GetTestClass(source, "LABC;");
    ASSERT_NE(klass, nullptr);
    ASSERT_TRUE(klass->GetName()->StringsAreEqual(EtsString::CreateFromMUtf8("ABC")->AsObject()));

    EtsString *name = EtsString::CreateFromMUtf8("TestNameA");
    klass->SetName(name);
    ASSERT_TRUE(klass->GetName()->StringsAreEqual(name->AsObject()));

    name = EtsString::CreateFromMUtf8("TestNameB");
    klass->SetName(name);
    ASSERT_TRUE(klass->GetName()->StringsAreEqual(name->AsObject()));

    name = EtsString::CreateFromMUtf8("TestNameC");
    klass->SetName(name);
    ASSERT_TRUE(klass->GetName()->StringsAreEqual(name->AsObject()));
}

TEST_F(EtsClassTest, CompareAndSetName)
{
    const char *source = R"(
        .language eTS
        .record Test {}
    )";

    EtsClass *klass = GetTestClass(source, "LTest;");
    ASSERT_NE(klass, nullptr);

    EtsString *oldName = EtsString::CreateFromMUtf8("TestOldName");
    EtsString *newName = EtsString::CreateFromMUtf8("TestNewName");
    ASSERT_NE(oldName, nullptr);
    ASSERT_NE(newName, nullptr);

    klass->SetName(oldName);

    ASSERT_TRUE(klass->CompareAndSetName(oldName, newName));
    ASSERT_TRUE(klass->GetName()->StringsAreEqual(newName->AsObject()));

    ASSERT_TRUE(klass->CompareAndSetName(newName, oldName));
    ASSERT_TRUE(klass->GetName()->StringsAreEqual(oldName->AsObject()));
}

TEST_F(EtsClassTest, IsInSamePackage)
{
    ASSERT_TRUE(EtsClass::IsInSamePackage("LA;", "LA;"));
    ASSERT_TRUE(EtsClass::IsInSamePackage("LA/B;", "LA/B;"));
    ASSERT_TRUE(EtsClass::IsInSamePackage("LA/B/C;", "LA/B/C;"));

    ASSERT_FALSE(EtsClass::IsInSamePackage("LA/B;", "LA/B/C;"));
    ASSERT_FALSE(EtsClass::IsInSamePackage("LA/B/C;", "LA/B;"));

    InSamePackagePrologue();

    {
        const char *source = R"(
            .language eTS
            .record Test.A {}
            .record Test.B {}
            .record Fake.A {}
        )";

        EtsClass *klassTestA = GetTestClass(source, "LTest/A;");
        EtsClass *klassTestB = GetTestClass(source, "LTest/B;");
        EtsClass *klassFakeA = GetTestClass(source, "LFake/A;");
        ASSERT_NE(klassTestA, nullptr);
        ASSERT_NE(klassTestB, nullptr);
        ASSERT_NE(klassFakeA, nullptr);

        ASSERT_TRUE(klassTestA->IsInSamePackage(klassTestA));
        ASSERT_TRUE(klassTestB->IsInSamePackage(klassTestB));
        ASSERT_TRUE(klassFakeA->IsInSamePackage(klassFakeA));

        ASSERT_TRUE(klassTestA->IsInSamePackage(klassTestB));
        ASSERT_TRUE(klassTestB->IsInSamePackage(klassTestA));

        ASSERT_FALSE(klassTestA->IsInSamePackage(klassFakeA));
        ASSERT_FALSE(klassFakeA->IsInSamePackage(klassTestA));
    }
    {
        const char *source = R"(
            .language eTS
            .record A.B.C {}
            .record A.B {}
        )";

        EtsClass *klassAbc = GetTestClass(source, "LA/B/C;");
        EtsClass *klassAb = GetTestClass(source, "LA/B;");
        ASSERT_NE(klassAbc, nullptr);
        ASSERT_NE(klassAb, nullptr);

        ASSERT_FALSE(klassAbc->IsInSamePackage(klassAb));
        ASSERT_FALSE(klassAb->IsInSamePackage(klassAbc));
    }
}

TEST_F(EtsClassTest, SetAndGetSuperClass)
{
    const char *sources = R"(
        .language eTS
        .record A {}
        .record B {}
    )";

    EtsClass *klass = GetTestClass(sources, "LA;");
    EtsClass *superKlass = GetTestClass(sources, "LB;");
    ASSERT_NE(klass, nullptr);
    ASSERT_NE(superKlass, nullptr);

    ASSERT_EQ(klass->GetSuperClass(), PandaEtsVM::GetCurrent()->GetClassLinker()->GetClassRoot(EtsClassRoot::OBJECT));
    klass->SetSuperClass(superKlass);
    ASSERT_EQ(klass->GetSuperClass(), superKlass);
}

TEST_F(EtsClassTest, IsSubClass)
{
    const char *source = R"(
        .language eTS
        .record A {}
        .record B <ets.extends = A> {}
    )";
    EtsClass *klassA = GetTestClass(source, "LA;");
    EtsClass *klassB = GetTestClass(source, "LB;");
    ASSERT_NE(klassA, nullptr);
    ASSERT_NE(klassB, nullptr);

    ASSERT_TRUE(klassB->IsSubClass(klassA));

    ASSERT_TRUE(klassA->IsSubClass(klassA));
    ASSERT_TRUE(klassB->IsSubClass(klassB));
}

TEST_F(EtsClassTest, SetAndGetFlags)
{
    const char *source = R"(
        .language eTS
        .record Test {}
    )";

    EtsClass *klass = GetTestClass(source, "LTest;");
    ASSERT_NE(klass, nullptr);

    uint32_t flags = 0U | 1U << 17U | 1U << 18U;

    klass->SetFlags(flags);
    ASSERT_EQ(klass->GetFlags(), flags);
    ASSERT_TRUE(klass->IsReference());
    ASSERT_TRUE(klass->IsWeakReference());
    ASSERT_TRUE(klass->IsFinalizerReference());
}

TEST_F(EtsClassTest, SetAndGetComponentType)
{
    const char *source = R"(
        .language eTS
        .record Test {}
    )";

    EtsClass *klass = GetTestClass(source, "LTest;");
    ASSERT_NE(klass, nullptr);

    uint32_t arrayLength = 100;
    auto *array1 = EtsObjectArray::Create(klass, arrayLength);
    auto *array2 = EtsObjectArray::Create(klass, arrayLength);

    ASSERT_NE(array1, nullptr);
    ASSERT_NE(array2, nullptr);
    ASSERT_EQ(array1->GetClass()->GetComponentType(), array2->GetClass()->GetComponentType());

    source = R"(
        .language eTS
        .record TestObject {}
    )";

    EtsClass *componentType = GetTestClass(source, "LTestObject;");
    ASSERT_NE(componentType, nullptr);

    klass->SetComponentType(componentType);
    ASSERT_EQ(klass->GetComponentType(), componentType);

    klass->SetComponentType(nullptr);
    ASSERT_EQ(klass->GetComponentType(), nullptr);
}

TEST_F(EtsClassTest, EnumerateMethods)
{
    const char *source = R"(
        .language eTS
        .record Test {}
        .function void Test.foo1() {
            return.void
        }
        .function void Test.foo2() {
            return.void
        }
        .function void Test.foo3() {
            return.void
        }
        .function void Test.foo4() {
            return.void
        }
        .function void Test.foo5() {
            return.void
        }
    )";

    EtsClass *klass = GetTestClass(source, "LTest;");
    ASSERT_NE(klass, nullptr);

    std::size_t methodsVectorSize = 5;
    std::vector<EtsMethod *> methods(methodsVectorSize);
    std::vector<EtsMethod *> enumerateMethods(methodsVectorSize);

    methods.push_back(klass->GetInstanceMethod("foo1", nullptr));
    methods.push_back(klass->GetInstanceMethod("foo2", nullptr));
    methods.push_back(klass->GetInstanceMethod("foo3", nullptr));
    methods.push_back(klass->GetInstanceMethod("foo4", nullptr));
    methods.push_back(klass->GetInstanceMethod("foo5", nullptr));

    klass->EnumerateDirectMethods([&enumerateMethods, &methodsVectorSize](EtsMethod *method) {
        enumerateMethods.push_back(method);
        return enumerateMethods.size() == methodsVectorSize;
    });

    for (std::size_t i = 0; i < methodsVectorSize; ++i) {
        ASSERT_EQ(methods[i], enumerateMethods[i]);
    }
}

TEST_F(EtsClassTest, EnumerateInterfaces)
{
    const char *source = R"(
        .language eTS
        .record A <ets.abstract, ets.interface> {}
        .record B <ets.abstract, ets.interface> {}
        .record C <ets.abstract, ets.interface> {}
        .record D <ets.abstract, ets.interface> {}
        .record E <ets.abstract, ets.interface> {}
        .record Test <ets.implements=A, ets.implements=B, ets.implements=C, ets.implements=D, ets.implements=E> {}
    )";

    EtsClass *klass = GetTestClass(source, "LTest;");
    ASSERT_NE(klass, nullptr);

    std::size_t interfaceVectorSize = 5;
    std::vector<EtsClass *> interfaces(interfaceVectorSize);
    std::vector<EtsClass *> enumerateInterfaces(interfaceVectorSize);

    interfaces.push_back(GetTestClass(source, "LA;"));
    interfaces.push_back(GetTestClass(source, "LB;"));
    interfaces.push_back(GetTestClass(source, "LC;"));
    interfaces.push_back(GetTestClass(source, "LD;"));
    interfaces.push_back(GetTestClass(source, "LE;"));

    klass->EnumerateInterfaces([&enumerateInterfaces, &interfaceVectorSize](EtsClass *interface) {
        enumerateInterfaces.push_back(interface);
        return enumerateInterfaces.size() == interfaceVectorSize;
    });

    for (std::size_t i = 0; i < interfaceVectorSize; ++i) {
        ASSERT_EQ(interfaces[i], enumerateInterfaces[i]);
    }
}

TEST_F(EtsClassTest, EnumerateVTable)
{
    const char *source = R"(
        .language eTS
        .record Test {}
        .function void Test.foo1() {
            return.void
        }
        .function void Test.foo2() {
            return.void
        }
        .function void Test.foo3() {
            return.void
        }
        .function void Test.foo4() {
            return.void
        }
        .function void Test.foo5() {
            return.void
        }
    )";

    EtsClass *klass = GetTestClass(source, "LTest;");
    ASSERT_NE(klass, nullptr);

    std::size_t methodsVectorSize = 5;
    std::vector<EtsMethod *> methods(methodsVectorSize);
    std::vector<EtsMethod *> enumerateMethods(methodsVectorSize);

    methods.push_back(klass->GetInstanceMethod("foo1", nullptr));
    methods.push_back(klass->GetInstanceMethod("foo2", nullptr));
    methods.push_back(klass->GetInstanceMethod("foo3", nullptr));
    methods.push_back(klass->GetInstanceMethod("foo4", nullptr));
    methods.push_back(klass->GetInstanceMethod("foo5", nullptr));

    klass->EnumerateVtable([&enumerateMethods, &methodsVectorSize](Method *method) {
        enumerateMethods.push_back(EtsMethod::FromRuntimeMethod(method));
        return enumerateMethods.size() == methodsVectorSize;
    });

    for (std::size_t i = 0; i < methodsVectorSize; ++i) {
        ASSERT_EQ(methods[i], enumerateMethods[i]);
    }
}

TEST_F(EtsClassTest, NameFromDescriptor)
{
    ark::ets::RuntimeDescriptorParser nameParser("[Lstd/core/Object;");
    ASSERT_EQ(nameParser.Resolve(), "std.core.Object[]");

    nameParser = RuntimeDescriptorParser("{U[I[J[Lstd/core/String;}");
    ASSERT_EQ(nameParser.Resolve(), "{Ui32[],i64[],std.core.String[]}");

    nameParser = RuntimeDescriptorParser("{ULLLL/L;LLLL/N;}");
    ASSERT_EQ(nameParser.Resolve(), "{ULLL.L,LLL.N}");

    nameParser = RuntimeDescriptorParser("[[{U[I[J[Lstd/core/String;}");
    ASSERT_EQ(nameParser.Resolve(), "{Ui32[],i64[],std.core.String[]}[][]");

    nameParser = RuntimeDescriptorParser("{ULstd/core/String;[{ULstd/core/String;[Lstd/core/String;}}");
    ASSERT_EQ(nameParser.Resolve(), "{Ustd.core.String,{Ustd.core.String,std.core.String[]}[]}");
}

TEST_F(EtsClassTest, NameToDescriptor)
{
    ark::ets::ClassPublicNameParser tdParser("std.core.Object[]");
    ASSERT_EQ(tdParser.Resolve(), "[Lstd/core/Object;");

    tdParser = ClassPublicNameParser("{Ui32[],i64[],std.core.String[]}");
    ASSERT_EQ(tdParser.Resolve(), "{U[I[J[Lstd/core/String;}");

    tdParser = ClassPublicNameParser("{ULLL.L,LLL.N}");
    ASSERT_EQ(tdParser.Resolve(), "{ULLLL/L;LLLL/N;}");

    tdParser = ClassPublicNameParser("{Ui32[],i64[],std.core.String[]}[][]");
    ASSERT_EQ(tdParser.Resolve(), "[[{U[I[J[Lstd/core/String;}");

    tdParser = ClassPublicNameParser("{Ustd.core.String,{Ustd.core.String,std.core.String[]}[]}");
    ASSERT_EQ(tdParser.Resolve(), "{ULstd/core/String;[{ULstd/core/String;[Lstd/core/String;}}");
}

}  // namespace ark::ets::test

// NOLINTEND(readability-magic-numbers)
