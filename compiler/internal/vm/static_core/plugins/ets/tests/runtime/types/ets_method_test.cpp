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

#include "libarkbase/utils/utils.h"
#include "get_test_class.h"
#include "ets_coroutine.h"

#include "types/ets_method.h"
#include "plugins/ets/runtime/ani/scoped_objects_fix.h"

// NOLINTBEGIN(readability-magic-numbers)

namespace ark::ets::test {

class EtsMethodTest : public testing::Test {
public:
    EtsMethodTest()
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

    ~EtsMethodTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(EtsMethodTest);
    NO_MOVE_SEMANTIC(EtsMethodTest);

    void SetUp() override
    {
        coroutine_ = EtsCoroutine::GetCurrent();
        PandaEtsNapiEnv *env = coroutine_->GetEtsNapiEnv();

        s_ = new ani::ScopedManagedCodeFix(env);
    }

    void TearDown() override
    {
        delete s_;
    }

private:
    RuntimeOptions options_;
    EtsCoroutine *coroutine_ = nullptr;
    ani::ScopedManagedCodeFix *s_ = nullptr;

protected:
    ani::ScopedManagedCodeFix *GetScopedManagedCodeFix()
    {
        return s_;
    }
};

TEST_F(EtsMethodTest, Invoke)
{
    const char *source = R"(
        .language eTS
        .record Test {}

        .function i32 Test.foo() {
            ldai 111
            return
        }
        .function i32 Test.goo() {
            ldai 222
            return
        }
        .function i32 Test.sum() {
            call Test.foo
            sta v0
            call Test.goo
            add2 v0
            return
        }
    )";

    EtsClass *klass = GetTestClass(source, "LTest;");
    ASSERT(klass);

    EtsMethod *fooMethod = klass->GetStaticMethod("foo", nullptr);
    ASSERT(fooMethod);
    EtsMethod *gooMethod = klass->GetStaticMethod("goo", nullptr);
    ASSERT(gooMethod);
    EtsMethod *sumMethod = klass->GetStaticMethod("sum", nullptr);
    ASSERT(sumMethod);

    EtsValue res;
    ASSERT_EQ(fooMethod->Invoke(*GetScopedManagedCodeFix(), nullptr, &res), ANI_OK);
    ASSERT_EQ(res.GetAs<int32_t>(), 111_I);
    ASSERT_EQ(gooMethod->Invoke(*GetScopedManagedCodeFix(), nullptr, &res), ANI_OK);
    ASSERT_EQ(res.GetAs<int32_t>(), 222_I);
    ASSERT_EQ(sumMethod->Invoke(*GetScopedManagedCodeFix(), nullptr, &res), ANI_OK);
    ASSERT_EQ(res.GetAs<int32_t>(), 333_I);
}

TEST_F(EtsMethodTest, GetNumArgSlots)
{
    const char *source = R"(
        .language eTS
        .record Test {}

        .function i32 Test.foo1() <static, access.function=public> {
            ldai 0
            return
        }
        .function i32 Test.foo2(f32 a0) <static, access.function=private> {
            ldai 0
            return
        }
        .function i32 Test.foo3(i32 a0, i32 a1, f32 a2, f32 a3, f32 a4) <static, access.function=public> {
            ldai 0
            return
        }
    )";

    EtsClass *klass = GetTestClass(source, "LTest;");
    ASSERT(klass);

    EtsMethod *foo1Method = klass->GetStaticMethod("foo1", nullptr);
    ASSERT(foo1Method);
    EtsMethod *foo2Method = klass->GetStaticMethod("foo2", nullptr);
    ASSERT(foo2Method);
    EtsMethod *foo3Method = klass->GetStaticMethod("foo3", nullptr);
    ASSERT(foo3Method);

    ASSERT_TRUE(foo1Method->IsStatic());
    ASSERT_TRUE(foo2Method->IsStatic());
    ASSERT_TRUE(foo3Method->IsStatic());
    ASSERT_TRUE(foo1Method->IsPublic());
    ASSERT_FALSE(foo2Method->IsPublic());
    ASSERT_TRUE(foo3Method->IsPublic());

    ASSERT_EQ(foo1Method->GetNumArgSlots(), 0U);
    ASSERT_EQ(foo2Method->GetNumArgSlots(), 1U);
    ASSERT_EQ(foo3Method->GetNumArgSlots(), 5U);
}

TEST_F(EtsMethodTest, GetArgType)
{
    const char *source = R"(
        .language eTS
        .record Test {}

        .function i32 Test.foo(u1 a0, i8 a1, u16 a2, i16 a3, i32 a4, i64 a5, f32 a6, f64 a7) {
            ldai 0
            return
        }
    )";

    EtsClass *klass = GetTestClass(source, "LTest;");
    ASSERT(klass);

    EtsMethod *fooMethod = klass->GetStaticMethod("foo", nullptr);
    ASSERT(fooMethod);

    std::vector<EtsType> expectedArgTypes = {EtsType::BOOLEAN, EtsType::BYTE, EtsType::CHAR,  EtsType::SHORT,
                                             EtsType::INT,     EtsType::LONG, EtsType::FLOAT, EtsType::DOUBLE};
    EtsType argType;

    for (std::size_t i = 0; i < expectedArgTypes.size(); i++) {
        argType = fooMethod->GetArgType(i);
        ASSERT_EQ(argType, expectedArgTypes[i]);
    }
}

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP, G.FUD.05) solid logic
TEST_F(EtsMethodTest, GetReturnValueType)
{
    const char *source = R"(
        .language eTS
        .record Test {}
        .record TestObject {}

        .function u1 Test.foo0() {
            ldai 0x1
            return
        }
        .function i8 Test.foo1() {
            ldai 0x1
            return
        }
        .function u16 Test.foo2() {
            ldai 0x1
            return
        }
        .function i16 Test.foo3() {
            ldai 0x1
            return
        }
        .function i32 Test.foo4() {
            ldai 0x1
            return
        }
        .function i64 Test.foo5() {
            ldai.64 0x1
            return.64
        }
        .function f32 Test.foo6() {
            ldai 0x1
            return
        }
        .function f64 Test.foo7() {
            ldai.64 0x1
            return.64
        }
        .function TestObject Test.foo8() {
            newobj v0, TestObject
            lda.obj v0
            return.obj
        }
        .function void Test.foo9() { return.void }

    )";

    EtsClass *klass = GetTestClass(source, "LTest;");
    ASSERT(klass);

    std::vector<EtsType> returnValTypes = {EtsType::BOOLEAN, EtsType::BYTE, EtsType::CHAR,  EtsType::SHORT,
                                           EtsType::INT,     EtsType::LONG, EtsType::FLOAT, EtsType::DOUBLE,
                                           EtsType::OBJECT,  EtsType::VOID};
    std::vector<EtsMethod *> methods;
    EtsMethod *currentMethod = nullptr;

    for (std::size_t i = 0; i < 1; i++) {
        std::string fooName("foo");
        currentMethod = klass->GetStaticMethod((fooName + std::to_string(i)).data(), nullptr);
        ASSERT(currentMethod);
        methods.push_back(currentMethod);
    }
    for (std::size_t i = 0; i < 1; i++) {
        ASSERT_EQ(methods[i]->GetReturnValueType(), returnValTypes[i]);
    }
}

TEST_F(EtsMethodTest, GetMethodSignature)
{
    const char *source = R"(
        .language eTS
        .record Test {}
        .record TestObject {}

        .function TestObject Test.foo1(i32 a0) {
            newobj v0, TestObject
            lda.obj v0
            return.obj
        }
        .function i32 Test.foo2(i32 a0, f32 a1, f64 a2) {
            ldai 0
            return
        }
        .function u1 Test.foo3(i32 a0, i32 a1, f32 a2, f64 a3, f32 a4) {
            ldai 0
            return
        }
    )";

    EtsClass *klass = GetTestClass(source, "LTest;");
    ASSERT(klass);

    EtsMethod *foo1Method = klass->GetStaticMethod("foo1", nullptr);
    ASSERT(foo1Method);
    EtsMethod *foo2Method = klass->GetStaticMethod("foo2", nullptr);
    ASSERT(foo2Method);
    EtsMethod *foo3Method = klass->GetStaticMethod("foo3", nullptr);
    ASSERT(foo3Method);

    ASSERT_EQ(foo1Method->GetMethodSignature(), "I:LTestObject;");
    ASSERT_EQ(foo2Method->GetMethodSignature(), "IFD:I");
    ASSERT_EQ(foo3Method->GetMethodSignature(), "IIFDF:Z");
}

TEST_F(EtsMethodTest, GetLineNumFromBytecodeOffset)
{
    const char *source = R"(            # line 1
        .language eTS                   # line 2
        .record Test {}                 # line 3
        .function void Test.foo() {     # line 4
            movi v1, 256                # line 5, offset 0, size 4
            movi v25, 1                 # line 6, offset 4, size 3
            mov v0, v1                  # line 7, offset 7, size 2
            mov v0, v25                 # line 8, offset 9, size 3
            return.void                 # line 9, offset 12, size 1
        }
    )";

    EtsClass *klass = GetTestClass(source, "LTest;");
    ASSERT(klass);
    EtsMethod *fooMethod = klass->GetStaticMethod("foo", nullptr);
    ASSERT(fooMethod);

    ASSERT_EQ(fooMethod->GetLineNumFromBytecodeOffset(0U), 5_I);
    ASSERT_EQ(fooMethod->GetLineNumFromBytecodeOffset(1U), 5_I);
    ASSERT_EQ(fooMethod->GetLineNumFromBytecodeOffset(2U), 5_I);
    ASSERT_EQ(fooMethod->GetLineNumFromBytecodeOffset(3U), 5_I);

    ASSERT_EQ(fooMethod->GetLineNumFromBytecodeOffset(4U), 6_I);
    ASSERT_EQ(fooMethod->GetLineNumFromBytecodeOffset(5U), 6_I);
    ASSERT_EQ(fooMethod->GetLineNumFromBytecodeOffset(6U), 6_I);

    ASSERT_EQ(fooMethod->GetLineNumFromBytecodeOffset(7U), 7_I);
    ASSERT_EQ(fooMethod->GetLineNumFromBytecodeOffset(8U), 7_I);

    ASSERT_EQ(fooMethod->GetLineNumFromBytecodeOffset(9U), 8_I);
    ASSERT_EQ(fooMethod->GetLineNumFromBytecodeOffset(10U), 8_I);
    ASSERT_EQ(fooMethod->GetLineNumFromBytecodeOffset(11U), 8_I);

    ASSERT_EQ(fooMethod->GetLineNumFromBytecodeOffset(12U), 9_I);
}

TEST_F(EtsMethodTest, GetName)
{
    const char *source = R"(
        .language eTS
        .record Test {}

        .function i32 Test.foo1() {
            ldai 0
            return
        }
        .function i32 Test.foo2(f32 a0) {
            ldai 0
            return
        }
    )";

    EtsClass *klass = GetTestClass(source, "LTest;");
    ASSERT(klass);

    EtsMethod *foo1Method = klass->GetStaticMethod("foo1", nullptr);
    ASSERT(foo1Method);
    EtsMethod *foo2Method = klass->GetStaticMethod("foo2", nullptr);
    ASSERT(foo2Method);

    ASSERT_TRUE(!strcmp(foo1Method->GetName(), "foo1"));
    ASSERT_TRUE(!strcmp(foo2Method->GetName(), "foo2"));

    EtsString *str1 = foo1Method->GetNameString();
    EtsString *str2 = EtsString::CreateFromMUtf8("foo1");
    ASSERT_TRUE(str1->StringsAreEqual(reinterpret_cast<EtsObject *>(str2)));
    str1 = foo2Method->GetNameString();
    str2 = EtsString::CreateFromMUtf8("foo2");
    ASSERT_TRUE(str1->StringsAreEqual(reinterpret_cast<EtsObject *>(str2)));
}

TEST_F(EtsMethodTest, ResolveArgType)
{
    const char *source = R"(
        .language eTS
        .record Test {}
        .record TestObject {}

        .function i32 Test.foo1(TestObject a0, f32 a1) {
            ldai 0
            return
        }
        .function i32 Test.foo2(TestObject a0, TestObject a1) {
            ldai 0
            return
        }
        .function i32 Test.foo3(f32 a0, f32 a1) {
            ldai 0
            return
        }
    )";

    EtsClass *klass = GetTestClass(source, "LTest;");
    ASSERT(klass);

    EtsMethod *foo1Method = klass->GetStaticMethod("foo1", nullptr);
    ASSERT(foo1Method);
    EtsMethod *foo2Method = klass->GetStaticMethod("foo2", nullptr);
    ASSERT(foo2Method);
    EtsMethod *foo3Method = klass->GetStaticMethod("foo3", nullptr);
    ASSERT(foo3Method);

    ASSERT_EQ(foo1Method->ResolveArgType(0), foo2Method->ResolveArgType(0));
    ASSERT_EQ(foo2Method->ResolveArgType(0), foo2Method->ResolveArgType(1));

    ASSERT_EQ(foo1Method->ResolveArgType(1), foo3Method->ResolveArgType(0));
    ASSERT_EQ(foo3Method->ResolveArgType(0), foo3Method->ResolveArgType(1));
}

TEST_F(EtsMethodTest, ResolveReturnType)
{
    const char *source = R"(
        .language eTS
        .record Test {}
        .record TestObject {}

        .function TestObject Test.foo1() {
            newobj v0, TestObject
            lda.obj v0
            return.obj
        }
        .function TestObject Test.foo2() {
            newobj v0, TestObject
            lda.obj v0
            return.obj
        }
        .function i32 Test.foo3() {
            ldai 0
            return
        }
        .function i32 Test.foo4() {
            ldai 0
            return
        }
    )";

    EtsClass *klass = GetTestClass(source, "LTest;");
    ASSERT(klass);

    EtsMethod *foo1Method = klass->GetStaticMethod("foo1", nullptr);
    ASSERT(foo1Method);
    EtsMethod *foo2Method = klass->GetStaticMethod("foo2", nullptr);
    ASSERT(foo2Method);
    EtsMethod *foo3Method = klass->GetStaticMethod("foo3", nullptr);
    ASSERT(foo3Method);
    EtsMethod *foo4Method = klass->GetStaticMethod("foo4", nullptr);
    ASSERT(foo4Method);

    ASSERT_EQ(foo1Method->ResolveReturnType(), foo2Method->ResolveReturnType());
    ASSERT_EQ(foo3Method->ResolveReturnType(), foo4Method->ResolveReturnType());
}

TEST_F(EtsMethodTest, GetClassSourceFile)
{
    const char *source = R"(
        .language eTS
        .record Test {}

        .function i32 Test.foo() {
            ldai 0
            return
        }
    )";

    std::string sourceFilename = "EtsMethodTestSource.pa";

    EtsClass *klass = GetTestClass(source, "LTest;", sourceFilename);
    ASSERT(klass);
    EtsMethod *fooMethod = klass->GetStaticMethod("foo", nullptr);
    ASSERT(fooMethod);

    auto result = fooMethod->GetClassSourceFile();
    ASSERT(result.data);

    ASSERT_TRUE(!strcmp(reinterpret_cast<const char *>(result.data), sourceFilename.data()));
}

}  // namespace ark::ets::test

// NOLINTEND(readability-magic-numbers)
