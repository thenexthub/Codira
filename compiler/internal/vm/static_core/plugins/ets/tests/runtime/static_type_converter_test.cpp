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

#include <gtest/gtest.h>
#include "plugins/ets/runtime/static_type_converter.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/types/ets_bigint.h"
#include "plugins/ets/runtime/types/ets_box_primitive.h"
#include "plugins/ets/runtime/types/ets_box_primitive-inl.h"
#include "plugins/ets/runtime/types/ets_class.h"
#include "runtime/include/runtime_options.h"
#include "runtime/include/runtime.h"
#include "common_interfaces/objects/base_type.h"
#include "assembly-emitter.h"
#include "assembly-parser.h"
#include "ets_vm.h"

namespace ark::ets::test {
class StaticTypeConverterTest : public testing::Test {
public:
    StaticTypeConverterTest()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(true);
        options.SetShouldInitializeIntrinsics(true);
        options.SetCompilerEnableJit(false);
        options.SetGcType("epsilon");
        options.SetLoadRuntimes({"ets"});

        Logger::InitializeStdLogging(Logger::Level::ERROR, 0);

        auto stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to mock_stdlib.abc" << std::endl;
            std::abort();
        }
        options.SetBootPandaFiles({stdlib});

        ark::Runtime::Create(options);
        SetClassesPandasmSources();
    }

    ~StaticTypeConverterTest() override
    {
        ark::Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(StaticTypeConverterTest);
    NO_MOVE_SEMANTIC(StaticTypeConverterTest);

    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override
    {
        coroutine_ = EtsCoroutine::GetCurrent();
        coroutine_->ManagedCodeBegin();
    }
    void TearDown() override
    {
        coroutine_->ManagedCodeEnd();
    }
    EtsCoroutine *GetCoroutine() const
    {
        return coroutine_;
    }

    void SetClassesPandasmSources();
    EtsClass *GetTestClass(std::string className);

private:
    RuntimeOptions options_;
    EtsCoroutine *coroutine_ = nullptr;
    std::unordered_map<std::string, const char *> sources_;

protected:
    const std::unordered_map<std::string, const char *> &GetSources()
    {
        return sources_;
    }

    template <typename T>
    void CheckUnbox(T value)
    {
        StaticTypeConverter stcTypeConverter;
        auto *coro = EtsCoroutine::GetCurrent();
        EtsObject *boxed = EtsBoxPrimitive<T>::Create(coro, value);
        common::BaseType result = stcTypeConverter.UnwrapBoxed(reinterpret_cast<common::BoxedValue>(boxed));
        if constexpr (std::is_same_v<T, EtsBoolean>) {
            EXPECT_TRUE(std::holds_alternative<bool>(result));
            EXPECT_EQ(std::get<bool>(result), value);
        } else {
            EXPECT_TRUE(std::holds_alternative<T>(result));
            EXPECT_EQ(std::get<T>(result), value);
        }
    }
};

void StaticTypeConverterTest::SetClassesPandasmSources()
{
    const char *source = R"(
        .language eTS
        .record A {
            u1 boolean
            i8 byte
            u16 char
            i16 short
            i32 int
            i64 long
            f32 float
            f64 double
        }
    )";
    sources_["A"] = source;
}
EtsClass *StaticTypeConverterTest::GetTestClass(std::string className)
{
    std::unordered_map<std::string, const char *> sources = GetSources();
    pandasm::Parser p;

    auto res = p.Parse(sources[className]);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());

    auto classLinker = Runtime::GetCurrent()->GetClassLinker();
    classLinker->AddPandaFile(std::move(pf));

    className.insert(0, 1, 'L');
    className.push_back(';');

    EtsClass *klass = coroutine_->GetPandaVM()->GetClassLinker()->GetClass(className.c_str());
    ASSERT(klass);
    return klass;
}

// NOLINTBEGIN(readability-identifier-naming, readability-magic-numbers)
TEST_F(StaticTypeConverterTest, WrapBoxed_Test0)
{
    StaticTypeConverter stcTypeConverter;
    {
        common::BaseType value = std::monostate {};
        common::BoxedValue result = stcTypeConverter.WrapBoxed(value);
        EXPECT_EQ(result, nullptr);
    }
    {
        common::BaseType value = true;
        common::BoxedValue result = stcTypeConverter.WrapBoxed(value);
        EXPECT_EQ(reinterpret_cast<EtsBoxPrimitive<bool> *>(result)->GetValue(), std::get<bool>(value));
    }
    {
        common::BaseType value = false;
        common::BoxedValue result = stcTypeConverter.WrapBoxed(value);
        EXPECT_EQ(reinterpret_cast<EtsBoxPrimitive<bool> *>(result)->GetValue(), std::get<bool>(value));
    }
    {
        int16_t tarValue = 32767;
        common::BaseType value = tarValue;
        common::BoxedValue result = stcTypeConverter.WrapBoxed(value);
        EXPECT_EQ(reinterpret_cast<EtsBoxPrimitive<int16_t> *>(result)->GetValue(), std::get<int16_t>(value));
    }
    {
        uint16_t tarValue = 65535;
        common::BaseType value = tarValue;
        common::BoxedValue result = stcTypeConverter.WrapBoxed(value);
        EXPECT_EQ(reinterpret_cast<EtsBoxPrimitive<uint16_t> *>(result)->GetValue(), std::get<uint16_t>(value));
    }
    {
        uint16_t tarValue = 'A';
        common::BaseType value = tarValue;
        common::BoxedValue result = stcTypeConverter.WrapBoxed(value);
        EXPECT_EQ(reinterpret_cast<EtsBoxPrimitive<uint16_t> *>(result)->GetValue(), std::get<uint16_t>(value));
    }
}

TEST_F(StaticTypeConverterTest, WrapBoxed_Test1)
{
    StaticTypeConverter stcTypeConverter;
    {
        int32_t tarValue = 2147483647;
        common::BaseType value = tarValue;
        common::BoxedValue result = stcTypeConverter.WrapBoxed(value);
        EXPECT_EQ(reinterpret_cast<EtsBoxPrimitive<int32_t> *>(result)->GetValue(), std::get<int32_t>(value));
    }
    {
        float tarValue = 3.14F;
        common::BaseType value = tarValue;
        common::BoxedValue result = stcTypeConverter.WrapBoxed(value);
        EXPECT_EQ(reinterpret_cast<EtsBoxPrimitive<float> *>(result)->GetValue(), std::get<float>(value));
    }
    {
        double tarValue = 2.71828;
        common::BaseType value = tarValue;
        common::BoxedValue result = stcTypeConverter.WrapBoxed(value);
        EXPECT_EQ(reinterpret_cast<EtsBoxPrimitive<double> *>(result)->GetValue(), std::get<double>(value));
    }
    {
        int64_t tarValue = 9223372036854775807LL;
        common::BaseType value = tarValue;
        common::BoxedValue result = stcTypeConverter.WrapBoxed(value);
        EXPECT_EQ(reinterpret_cast<EtsBoxPrimitive<int64_t> *>(result)->GetValue(), std::get<int64_t>(value));
    }
}

TEST_F(StaticTypeConverterTest, WrapBoxed_Test2)
{
    StaticTypeConverter stcTypeConverter;
    {
        common::BaseType value = common::BaseUndefined {};
        common::BoxedValue result = stcTypeConverter.WrapBoxed(value);
        EXPECT_EQ(result, nullptr);
    }
    {
        common::BaseType value = common::BaseNull {};
        common::BoxedValue result = stcTypeConverter.WrapBoxed(value);
        EXPECT_EQ(reinterpret_cast<EtsObject *>(result), EtsObject::FromCoreType(GetCoroutine()->GetNullValue()));
    }
    {
        common::BaseBigInt bigIntValue;
        uint32_t tarLength = 3;
        bigIntValue.length = tarLength;
        bigIntValue.sign = true;
        bigIntValue.data = {0x12345678, 0x9ABCDEF0, 0x13579BDF};
        common::BaseType value = bigIntValue;
        common::BoxedValue result = stcTypeConverter.WrapBoxed(value);

        auto bigInt = EtsBigInt::FromEtsObject(reinterpret_cast<EtsObject *>(result));

        EXPECT_EQ(bigInt->GetBytes()->GetLength(), bigIntValue.length);
        auto sign = bigInt->GetSign() < 0 ? 1 : 0;
        EXPECT_EQ(sign, bigIntValue.sign);

        for (uint32_t i = 0; i < bigIntValue.length; i++) {
            EXPECT_EQ(bigInt->GetBytes()->Get(i), bigIntValue.data[i]);
        }
    }
    {
        std::shared_ptr<common::BaseObject> sharedObj = std::make_shared<common::BaseObject>();
        common::BaseType value = sharedObj.get();
        common::BoxedValue result = stcTypeConverter.WrapBoxed(value);
        EXPECT_EQ(result, std::get<common::BaseObject *>(value));
    }
}

TEST_F(StaticTypeConverterTest, UnWrapBoxed_Boolean)
{
    bool trueValue = true;
    bool falseValue = false;
    CheckUnbox<EtsBoolean>(static_cast<EtsBoolean>(trueValue));
    CheckUnbox<EtsBoolean>(static_cast<EtsBoolean>(falseValue));
}

TEST_F(StaticTypeConverterTest, UnWrapBoxed_Int16)
{
    int16_t tarValue = 32767;
    int16_t tarValue2 = -32767;
    CheckUnbox<EtsShort>(tarValue);
    CheckUnbox<EtsShort>(tarValue2);
}

TEST_F(StaticTypeConverterTest, UnWrapBoxed_UInt16)
{
    uint16_t tarValue = 65535;
    uint16_t tarValue2 = 'A';
    CheckUnbox<EtsChar>(tarValue);
    CheckUnbox<EtsChar>(tarValue2);
}

TEST_F(StaticTypeConverterTest, UnWrapBoxed_Int32)
{
    int32_t tarValue = 2147483647;
    int32_t tarValue2 = 42;
    CheckUnbox<EtsInt>(tarValue);
    CheckUnbox<EtsInt>(tarValue2);
}

TEST_F(StaticTypeConverterTest, UnWrapBoxed_Int64)
{
    int64_t tarValue = 9223372036854775807LL;
    int64_t tarValue2 = 123456789012345LL;
    CheckUnbox<EtsLong>(tarValue);
    CheckUnbox<EtsLong>(tarValue2);
}

TEST_F(StaticTypeConverterTest, UnWrapBoxed_Float)
{
    float tarValue = 3.14159F;
    CheckUnbox<EtsFloat>(tarValue);
}

TEST_F(StaticTypeConverterTest, UnWrapBoxed_Double)
{
    double tarValue = 3.14159;
    double tarValue2 = 2.71828;
    CheckUnbox<EtsDouble>(tarValue);
    CheckUnbox<EtsDouble>(tarValue2);
}
// NOLINTBEGIN(modernize-avoid-c-arrays,-warnings-as-errors)
TEST_F(StaticTypeConverterTest, UnwrapBaseBigInt)
{
    const uint32_t LENGTH = 2;
    int32_t tarSign = 1;
    bool sign = false;
    uint32_t digits[LENGTH] = {123456789, 987654321};
    auto ptypes = PlatformTypes(GetCoroutine());
    auto etsIntArray = EtsIntArray::Create(LENGTH);
    for (uint32_t i = 0; i < LENGTH; i++) {
        etsIntArray->Set(i, static_cast<int32_t>(digits[i]));
    }
    ASSERT(ptypes->coreBigInt != nullptr);
    auto bigInt = EtsBigInt::FromEtsObject(EtsObject::Create(ptypes->coreBigInt));
    bigInt->SetFieldObject(EtsBigInt::GetBytesOffset(), reinterpret_cast<EtsObject *>(etsIntArray));
    bigInt->SetFieldPrimitive(EtsBigInt::GetSignOffset(), tarSign);

    StaticTypeConverter stcTypeConverter;
    auto boxed = reinterpret_cast<EtsObject *>(bigInt);
    common::BaseType result = stcTypeConverter.UnwrapBoxed(reinterpret_cast<common::BoxedValue>(boxed));
    EXPECT_TRUE(std::holds_alternative<common::BaseBigInt>(result));
    const common::BaseBigInt &baseBigInt = std::get<common::BaseBigInt>(result);

    EXPECT_EQ(baseBigInt.length, LENGTH);
    EXPECT_EQ(baseBigInt.sign, sign);
    for (uint32_t i = 0; i < LENGTH; i++) {
        EXPECT_EQ(baseBigInt.data[i], digits[i]);
    }
}
// NOLINTEND(modernize-avoid-c-arrays,-warnings-as-errors)

TEST_F(StaticTypeConverterTest, UnwrapBoxedNull)
{
    StaticTypeConverter stcTypeConverter;
    auto *coro = EtsCoroutine::GetCurrent();
    auto *nullObj = coro->GetNullValue();
    auto result = stcTypeConverter.UnwrapBoxed(reinterpret_cast<common::BoxedValue>(nullObj));
    ASSERT_TRUE(std::holds_alternative<common::BaseNull>(result));
}

TEST_F(StaticTypeConverterTest, UnwrapBoxedUndefined)
{
    StaticTypeConverter stcTypeConverter;
    auto result = stcTypeConverter.UnwrapBoxed(nullptr);
    ASSERT_TRUE(std::holds_alternative<common::BaseUndefined>(result));
}

TEST_F(StaticTypeConverterTest, UnwrapBoxedObject)
{
    EtsClass *klass = GetTestClass("A");
    EtsObject *testObj = EtsObject::Create(klass);
    StaticTypeConverter stcTypeConverter;
    common::BaseType result = stcTypeConverter.UnwrapBoxed(reinterpret_cast<common::BoxedValue>(testObj));
    ASSERT_TRUE(std::holds_alternative<common::BaseObject *>(result));
    EXPECT_EQ(std::get<common::BoxedValue>(result), reinterpret_cast<common::BoxedValue>(testObj));
}
// NOLINTEND(readability-identifier-naming, readability-magic-numbers)
}  // namespace ark::ets::test
