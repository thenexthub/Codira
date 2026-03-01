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
#include <cstdint>
#include <libarkfile/include/source_lang_enum.h>

#include "assembly-emitter.h"
#include "assembly-parser.h"
#include "plugins/ets/runtime/static_object_accessor.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/types/ets_box_primitive-inl.h"
#include "plugins/ets/runtime/types/ets_class.h"
#include "types/ets_array.h"
#include "types/ets_escompat_array.h"
#include "types/ets_object.h"
#include "ets_vm.h"
#include "ets_class_linker_extension.h"
#include "libarkbase/utils/utils.h"

namespace ark::ets::test {

class StaticObjectAccessorTest : public testing::Test {
public:
    static constexpr EtsFloat VAL_FLOAT = 1.1F;
    static constexpr EtsDouble VAL_DOUBLE = 2.2F;
    static constexpr size_t ARRAY_LENGTH = 100U;
    StaticObjectAccessorTest()
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

    ~StaticObjectAccessorTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(StaticObjectAccessorTest);
    NO_MOVE_SEMANTIC(StaticObjectAccessorTest);

    void SetClassesPandasmSources();
    EtsClass *GetTestClass(std::string className);

    void SetUp() override
    {
        coroutine_ = EtsCoroutine::GetCurrent();
        coroutine_->ManagedCodeBegin();
    }

    void TearDown() override
    {
        coroutine_->ManagedCodeEnd();
    }

    template <typename T, typename BoxType>
    void CheckSetAndGetProperty(EtsClass *klass, const char *property, T value)
    {
        EtsObject *obj = EtsObject::Create(klass);
        StaticObjectAccessor staticObjectAccessor;
        auto *coro = EtsCoroutine::GetCurrent();
        EtsObject *valueObject = EtsBoxPrimitive<T>::Create(coro, value);

        staticObjectAccessor.SetProperty(nullptr, nullptr, property,
                                         reinterpret_cast<common::BaseObject *>(valueObject));
        staticObjectAccessor.SetProperty(nullptr, reinterpret_cast<common::BaseObject *>(obj), "test",
                                         reinterpret_cast<common::BaseObject *>(valueObject));
        staticObjectAccessor.SetProperty(nullptr, reinterpret_cast<common::BaseObject *>(obj), property,
                                         reinterpret_cast<common::BaseObject *>(valueObject));

        common::BaseObject *baseObject = staticObjectAccessor.GetProperty(nullptr, nullptr, property);
        ASSERT_EQ(baseObject, nullptr);
        baseObject = staticObjectAccessor.GetProperty(nullptr, reinterpret_cast<common::BaseObject *>(obj), "test");
        ASSERT_EQ(baseObject, nullptr);
        baseObject = staticObjectAccessor.GetProperty(nullptr, reinterpret_cast<common::BaseObject *>(obj), property);
        ASSERT_EQ(reinterpret_cast<BoxType *>(baseObject)->GetValue(), value);
        ASSERT_EQ(staticObjectAccessor.HasProperty(nullptr, reinterpret_cast<common::BaseObject *>(obj), property),
                  true);
        ASSERT_EQ(staticObjectAccessor.HasProperty(nullptr, reinterpret_cast<common::BaseObject *>(obj), "test"),
                  false);
    }

    template <typename T, typename BoxType>
    void CheckSetAndGetElementByIdx(T val)
    {
        auto *coro = EtsCoroutine::GetCurrent();
        auto *array = EtsEscompatArray::Create(coro, ARRAY_LENGTH);
        auto *baseObject = reinterpret_cast<common::BaseObject *>(array);
        ASSERT_NE(baseObject, nullptr);
        StaticObjectAccessor staticObjectAccessor;
        EtsObject *valueObject = BoxType::Create(coro, val);
        staticObjectAccessor.SetElementByIdx(nullptr, nullptr, 1, reinterpret_cast<common::BaseObject *>(valueObject));
        staticObjectAccessor.SetElementByIdx(nullptr, baseObject, 1,
                                             reinterpret_cast<common::BaseObject *>(valueObject));
        common::BaseObject *baseObject2 = staticObjectAccessor.GetElementByIdx(nullptr, nullptr, 1);
        ASSERT_EQ(baseObject2, nullptr);
        baseObject2 = staticObjectAccessor.GetElementByIdx(nullptr, baseObject, 1);
        ASSERT_EQ(reinterpret_cast<common::BaseObject *>(valueObject), baseObject2);
    }

private:
    RuntimeOptions options_;
    EtsCoroutine *coroutine_ = nullptr;
    std::unordered_map<std::string, const char *> sources_;

protected:
    const std::unordered_map<std::string, const char *> &GetSources()
    {
        return sources_;
    }
};

void StaticObjectAccessorTest::SetClassesPandasmSources()
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

    source = R"(
        .language eTS
        .record Triangle {
            f64 firSide
            i32 secSide
            f32 thirdSide
            i64 Color <static>
        }
    )";
    sources_["Triangle"] = source;

    source = R"(
        .language eTS
        .record Foo {
            i32 member
        }
        .record Bar {
            Foo foo1
            Foo foo2
        }
    )";
    sources_["Foo"] = source;
    sources_["Bar"] = source;
}

EtsClass *StaticObjectAccessorTest::GetTestClass(std::string className)
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

TEST_F(StaticObjectAccessorTest, SetAndGetPropertyValue0)
{
    EtsClass *klass = GetTestClass("A");
    CheckSetAndGetProperty<EtsInt, EtsBoxPrimitive<EtsInt>>(klass, "int", 1);
    // NOLINTNEXTLINE(readability-implicit-bool-conversion)
    CheckSetAndGetProperty<EtsBoolean, EtsBoxPrimitive<EtsBoolean>>(klass, "boolean", true);
    CheckSetAndGetProperty<EtsByte, EtsBoxPrimitive<EtsByte>>(klass, "byte", 1);
    CheckSetAndGetProperty<EtsChar, EtsBoxPrimitive<EtsChar>>(klass, "char", 'a');
    CheckSetAndGetProperty<EtsShort, EtsBoxPrimitive<EtsShort>>(klass, "short", 1);
    CheckSetAndGetProperty<EtsLong, EtsBoxPrimitive<EtsLong>>(klass, "long", 1);
    CheckSetAndGetProperty<EtsFloat, EtsBoxPrimitive<EtsFloat>>(klass, "float", VAL_FLOAT);
    CheckSetAndGetProperty<EtsDouble, EtsBoxPrimitive<EtsDouble>>(klass, "double", VAL_DOUBLE);
}

TEST_F(StaticObjectAccessorTest, SetAndGetPropertyValue1)
{
    EtsClass *barKlass = GetTestClass("Bar");
    EtsClass *fooKlass = GetTestClass("Foo");
    EtsObject *barObj = EtsObject::Create(barKlass);
    EtsObject *fooObj1 = EtsObject::Create(fooKlass);
    StaticObjectAccessor staticObjectAccessor;
    staticObjectAccessor.SetProperty(nullptr, reinterpret_cast<common::BaseObject *>(barObj), "foo1",
                                     reinterpret_cast<common::BaseObject *>(fooObj1));
    common::BaseObject *baseObject =
        staticObjectAccessor.GetProperty(nullptr, reinterpret_cast<common::BaseObject *>(barObj), "foo1");
    ASSERT_EQ(reinterpret_cast<EtsObject *>(baseObject), fooObj1);
    ASSERT_EQ(staticObjectAccessor.HasProperty(nullptr, reinterpret_cast<common::BaseObject *>(barObj), "foo1"), true);
    ASSERT_EQ(staticObjectAccessor.HasProperty(nullptr, reinterpret_cast<common::BaseObject *>(barObj), "test"), false);
}

TEST_F(StaticObjectAccessorTest, SetAndGetPropertyValue2)
{
    EtsClass *barKlass = GetTestClass("Bar");
    EtsClass *fooKlass = GetTestClass("Foo");
    EtsObject *barObj = EtsObject::Create(barKlass);
    EtsObject *fooObj1 = EtsObject::Create(fooKlass);
    auto *coro = EtsCoroutine::GetCurrent();
    EtsInt val = 1;
    EtsObject *valueObject = EtsBoxPrimitive<EtsInt>::Create(coro, val);
    StaticObjectAccessor staticObjectAccessor;
    staticObjectAccessor.SetProperty(nullptr, reinterpret_cast<common::BaseObject *>(fooObj1), "member",
                                     reinterpret_cast<common::BaseObject *>(valueObject));
    staticObjectAccessor.SetProperty(nullptr, reinterpret_cast<common::BaseObject *>(barObj), "foo1",
                                     reinterpret_cast<common::BaseObject *>(fooObj1));
    common::BaseObject *baseObject =
        staticObjectAccessor.GetProperty(nullptr, reinterpret_cast<common::BaseObject *>(barObj), "foo1");
    common::BaseObject *reObject = staticObjectAccessor.GetProperty(nullptr, baseObject, "member");
    ASSERT_EQ(reinterpret_cast<EtsBoxPrimitive<EtsInt> *>(reObject)->GetValue(), 1);
}

TEST_F(StaticObjectAccessorTest, SetAndGetElementByIdx0)
{
    CheckSetAndGetElementByIdx<EtsInt, EtsBoxPrimitive<EtsInt>>(1);
    // NOLINTNEXTLINE(readability-implicit-bool-conversion)
    CheckSetAndGetElementByIdx<EtsBoolean, EtsBoxPrimitive<EtsBoolean>>(true);
    CheckSetAndGetElementByIdx<EtsByte, EtsBoxPrimitive<EtsByte>>(1);
    CheckSetAndGetElementByIdx<EtsChar, EtsBoxPrimitive<EtsChar>>('a');
    CheckSetAndGetElementByIdx<EtsShort, EtsBoxPrimitive<EtsShort>>(1);
    CheckSetAndGetElementByIdx<EtsLong, EtsBoxPrimitive<EtsLong>>(1);
    CheckSetAndGetElementByIdx<EtsFloat, EtsBoxPrimitive<EtsFloat>>(VAL_FLOAT);
    CheckSetAndGetElementByIdx<EtsDouble, EtsBoxPrimitive<EtsDouble>>(VAL_DOUBLE);
}

TEST_F(StaticObjectAccessorTest, SetAndGetElementByIdx1)
{
    EtsClass *klass = GetTestClass("Triangle");
    ASSERT_NE(klass, nullptr);
    auto *coro = EtsCoroutine::GetCurrent();
    auto *array = EtsEscompatArray::Create(coro, ARRAY_LENGTH);
    ASSERT_NE(array, nullptr);
    auto obj = EtsObject::Create(klass);
    EtsObject *valueObject = EtsBoxPrimitive<EtsDouble>::Create(coro, VAL_DOUBLE);
    StaticObjectAccessor staticObjectAccessor;
    staticObjectAccessor.SetProperty(nullptr, reinterpret_cast<common::BaseObject *>(obj), "firSide",
                                     reinterpret_cast<common::BaseObject *>(valueObject));
    staticObjectAccessor.SetElementByIdx(nullptr,
                                         reinterpret_cast<common::BaseObject *>(reinterpret_cast<EtsObject *>(array)),
                                         1, reinterpret_cast<common::BaseObject *>(obj));
    common::BaseObject *baseObject =
        staticObjectAccessor.GetElementByIdx(nullptr, reinterpret_cast<common::BaseObject *>(array), 1);
    ASSERT_EQ(baseObject, reinterpret_cast<common::BaseObject *>(obj));
    common::BaseObject *reObject =
        staticObjectAccessor.GetProperty(nullptr, reinterpret_cast<common::BaseObject *>(baseObject), "firSide");
    ASSERT_EQ(reinterpret_cast<EtsBoxPrimitive<EtsDouble> *>(reObject)->GetValue(), VAL_DOUBLE);
}
}  // namespace ark::ets::test
