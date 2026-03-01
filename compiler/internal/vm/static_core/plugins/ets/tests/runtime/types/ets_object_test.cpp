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

#include "types/ets_object.h"

#include "ets_vm.h"
#include "ets_coroutine.h"
#include "ets_class_linker_extension.h"
#include "assembly-emitter.h"
#include "assembly-parser.h"

// NOLINTBEGIN(readability-magic-numbers)

namespace ark::ets::test {

class EtsObjectTest : public testing::Test {
public:
    EtsObjectTest()
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

        SetClassesPandasmSources();
    }

    ~EtsObjectTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(EtsObjectTest);
    NO_MOVE_SEMANTIC(EtsObjectTest);

    void SetUp() override
    {
        coroutine_ = EtsCoroutine::GetCurrent();
        coroutine_->ManagedCodeBegin();
    }

    void TearDown() override
    {
        coroutine_->ManagedCodeEnd();
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
};

void EtsObjectTest::SetClassesPandasmSources()
{
    const char *source = R"(
        .language eTS
        .record Rectangle {
            i32 Width
            f32 Height
            i64 Color <static>
        }
    )";
    sources_["Rectangle"] = source;

    source = R"(
        .language eTS
        .record Triangle {
            i32 firSide
            i32 secSide
            i32 thirdSide
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

EtsClass *EtsObjectTest::GetTestClass(std::string className)
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

TEST_F(EtsObjectTest, GetClass)
{
    EtsClass *klass = nullptr;
    EtsObject *obj = nullptr;

    for (const auto &it : GetSources()) {
        klass = GetTestClass(it.first);
        obj = EtsObject::Create(klass);
        ASSERT_EQ(klass, obj->GetClass());
    }
}

TEST_F(EtsObjectTest, SetClass)
{
    EtsClass *klass1 = GetTestClass("Rectangle");
    EtsClass *klass2 = GetTestClass("Triangle");
    EtsObject *obj = EtsObject::Create(klass1);
    ASSERT_EQ(obj->GetClass(), klass1);
    obj->SetClass(klass2);
    ASSERT_EQ(obj->GetClass(), klass2);
}

TEST_F(EtsObjectTest, IsInstanceOf)
{
    EtsClass *klass1 = GetTestClass("Rectangle");
    EtsClass *klass2 = GetTestClass("Triangle");
    EtsObject *obj1 = EtsObject::Create(klass1);
    EtsObject *obj2 = EtsObject::Create(klass2);

    ASSERT_TRUE(obj1->IsInstanceOf(klass1));
    ASSERT_TRUE(obj2->IsInstanceOf(klass2));
    ASSERT_FALSE(obj1->IsInstanceOf(klass2));
    ASSERT_FALSE(obj2->IsInstanceOf(klass1));
}

TEST_F(EtsObjectTest, GetAndSetFieldObject)
{
    EtsClass *barKlass = GetTestClass("Bar");
    EtsClass *fooKlass = GetTestClass("Foo");

    EtsObject *barObj = EtsObject::Create(barKlass);
    EtsObject *fooObj1 = EtsObject::Create(fooKlass);
    EtsObject *fooObj2 = EtsObject::Create(fooKlass);

    EtsField *foo1Field = barKlass->GetFieldIDByName("foo1");
    EtsField *foo2Field = barKlass->GetFieldIDByName("foo2");

    barObj->SetFieldObject(foo1Field, fooObj1);
    barObj->SetFieldObject(foo2Field, fooObj2);
    ASSERT_EQ(barObj->GetFieldObject(foo1Field), fooObj1);
    ASSERT_EQ(barObj->GetFieldObject(foo2Field), fooObj2);

    EtsObject *res = barObj->GetAndSetFieldObject(foo2Field->GetOffset(), fooObj1, std::memory_order_relaxed);
    ASSERT_EQ(res, fooObj2);                                // returned pointer was in foo2_field
    ASSERT_EQ(barObj->GetFieldObject(foo2Field), fooObj1);  // now in foo2_field is pointer to Foo_obj1

    res = barObj->GetAndSetFieldObject(foo1Field->GetOffset(), fooObj2, std::memory_order_relaxed);
    ASSERT_EQ(res, fooObj1);
    ASSERT_EQ(barObj->GetFieldObject(foo1Field), fooObj2);
}

TEST_F(EtsObjectTest, SetAndGetFieldObject)
{
    EtsClass *barKlass = GetTestClass("Bar");
    EtsClass *fooKlass = GetTestClass("Foo");

    EtsObject *barObj = EtsObject::Create(barKlass);
    EtsObject *fooObj1 = EtsObject::Create(fooKlass);
    EtsObject *fooObj2 = EtsObject::Create(fooKlass);

    EtsField *foo1Field = barKlass->GetFieldIDByName("foo1");
    EtsField *foo2Field = barKlass->GetFieldIDByName("foo2");

    barObj->SetFieldObject(foo1Field, fooObj1);
    barObj->SetFieldObject(foo2Field, fooObj2);
    ASSERT_EQ(barObj->GetFieldObject(foo1Field), fooObj1);
    ASSERT_EQ(barObj->GetFieldObject(foo2Field), fooObj2);

    barObj->SetFieldObject(foo1Field, fooObj2);
    ASSERT_EQ(barObj->GetFieldObject(foo1Field), fooObj2);
    barObj->SetFieldObject(foo2Field, fooObj1);
    ASSERT_EQ(barObj->GetFieldObject(foo2Field), fooObj1);
}

TEST_F(EtsObjectTest, SetAndGetFieldPrimitive)
{
    EtsClass *klass = GetTestClass("Rectangle");
    EtsObject *obj = EtsObject::Create(klass);
    EtsField *field = klass->GetFieldIDByName("Width");
    int32_t testNmb1 = 77;
    obj->SetFieldPrimitive<int32_t>(field, testNmb1);
    ASSERT_EQ(obj->GetFieldPrimitive<int32_t>(field), testNmb1);

    field = klass->GetFieldIDByName("Height");
    float testNmb2 = 111.11;
    obj->SetFieldPrimitive<float>(field, testNmb2);
    ASSERT_EQ(obj->GetFieldPrimitive<float>(field), testNmb2);
}

TEST_F(EtsObjectTest, CompareAndSetFieldPrimitive)
{
    EtsClass *klass = GetTestClass("Rectangle");
    EtsObject *obj = EtsObject::Create(klass);
    EtsField *field = klass->GetFieldIDByName("Width");
    int32_t firNmb = 134;
    int32_t secNmb = 12;
    obj->SetFieldPrimitive(field, firNmb);
    obj->CompareAndSetFieldPrimitive(field->GetOffset(), firNmb, secNmb, std::memory_order_relaxed, true);
    ASSERT_EQ(obj->GetFieldPrimitive<int32_t>(field), secNmb);
}

TEST_F(EtsObjectTest, CompareAndSetFieldObject)
{
    EtsClass *barKlass = GetTestClass("Bar");
    EtsClass *fooKlass = GetTestClass("Foo");

    EtsObject *barObj = EtsObject::Create(barKlass);
    EtsObject *fooObj1 = EtsObject::Create(fooKlass);
    EtsObject *fooObj2 = EtsObject::Create(fooKlass);

    EtsField *foo1Field = barKlass->GetFieldIDByName("foo1");
    EtsField *foo2Field = barKlass->GetFieldIDByName("foo2");

    barObj->SetFieldObject(foo1Field, fooObj1);
    barObj->SetFieldObject(foo2Field, fooObj2);
    ASSERT_EQ(barObj->GetFieldObject(foo1Field), fooObj1);
    ASSERT_EQ(barObj->GetFieldObject(foo2Field), fooObj2);

    ASSERT_TRUE(
        barObj->CompareAndSetFieldObject(foo1Field->GetOffset(), fooObj1, fooObj2, std::memory_order_relaxed, true));
    ASSERT_EQ(barObj->GetFieldObject(foo1Field), fooObj2);

    ASSERT_TRUE(
        barObj->CompareAndSetFieldObject(foo2Field->GetOffset(), fooObj2, fooObj1, std::memory_order_relaxed, true));
    ASSERT_EQ(barObj->GetFieldObject(foo2Field), fooObj1);
}

TEST_F(EtsObjectTest, GetHashCode_SingleThread)
{
    // create class for testing
    EtsClass *barKlass = GetTestClass("Bar");
    EtsObject *obj = EtsObject::Create(barKlass);

    // after creation EtsObject should have no hash code
    ASSERT_FALSE(obj->IsHashed());

    // First we will test work with hash only
    // after getting hash it should be generated
    auto hash = obj->GetHashCode();
    ASSERT_TRUE(obj->IsHashed());
    ASSERT_EQ(hash, obj->GetHashCode());

    // next we will test usage of ets object state table
    // for this we should set interop hash
    static constexpr uint32_t INTEROP_INDEX_VALUE = 42U;
    obj->SetInteropIndex(INTEROP_INDEX_VALUE);
    // switched hashed to used info
    ASSERT_TRUE(obj->IsUsedInfo());
    ASSERT_TRUE(obj->IsHashed());
    ASSERT_TRUE(obj->HasInteropIndex());
    // after this getting hash should works correct
    ASSERT_EQ(hash, obj->GetHashCode());
    ASSERT_EQ(INTEROP_INDEX_VALUE, obj->GetInteropIndex());

    // next we should test droping of interop hash
    obj->DropInteropIndex();
    ASSERT_TRUE(obj->IsUsedInfo());
    ASSERT_TRUE(obj->IsHashed());
    ASSERT_FALSE(obj->HasInteropIndex());
    ASSERT_EQ(hash, obj->GetHashCode());

    // finally we deflect object
    EtsCoroutine::GetCurrent()->GetVM()->FreeInternalResources();
    ASSERT_TRUE(obj->IsHashed());
    ASSERT_EQ(hash, obj->GetHashCode());
}

TEST_F(EtsObjectTest, EtsMarkWordTest)
{
    // create mark word for testing using object
    EtsClass *barKlass = GetTestClass("Bar");
    EtsObject *obj = EtsObject::Create(barKlass);
    auto markWord = obj->GetMark();
    ASSERT_EQ(markWord.GetState(), EtsMarkWord::STATE_UNLOCKED);

    // CC-OFFNXT(G.NAM.03) project code style
    static constexpr uint32_t HASH_TO_DECODE = 12345U;
    auto hashedMarkWord = markWord.DecodeFromHash(HASH_TO_DECODE);
    ASSERT_EQ(hashedMarkWord.GetState(), EtsMarkWord::STATE_HASHED);
    ASSERT_EQ(hashedMarkWord.GetHash(), HASH_TO_DECODE);

    // CC-OFFNXT(G.NAM.03) project code style
    static constexpr uint32_t INTEROP_INDEX_TO_DECODE = 42U;
    auto markWordWithInteropIndex = markWord.DecodeFromInteropIndex(INTEROP_INDEX_TO_DECODE);
    ASSERT_EQ(markWordWithInteropIndex.GetState(), EtsMarkWord::STATE_HAS_INTEROP_INDEX);
    ASSERT_EQ(markWordWithInteropIndex.GetInteropIndex(), INTEROP_INDEX_TO_DECODE);

    // CC-OFFNXT(G.NAM.03) project code style
    static constexpr uint32_t USE_INFO_ID_TO_DECODE = 664U;
    auto markWordWithInfo = markWord.DecodeFromInfo(USE_INFO_ID_TO_DECODE);
    ASSERT_EQ(markWordWithInfo.GetState(), EtsMarkWord::STATE_USE_INFO);
    ASSERT_EQ(markWordWithInfo.GetInfoId(), USE_INFO_ID_TO_DECODE);
}

TEST_F(EtsObjectTest, SetGetAndDropInteropIndex_SingleThread)
{
    // create class for testing
    EtsClass *barKlass = GetTestClass("Bar");
    EtsObject *obj = EtsObject::Create(barKlass);
    // after creation EtsObject should have no hash code
    ASSERT_FALSE(obj->IsHashed());
    // we should set interop hash by object method
    // CC-OFFNXT(G.NAM.03) project code style
    static constexpr uint32_t INTEROP_INDEX_VALUE = 42U;
    obj->SetInteropIndex(INTEROP_INDEX_VALUE);
    ASSERT_TRUE(obj->HasInteropIndex());
    ASSERT_EQ(obj->GetInteropIndex(), INTEROP_INDEX_VALUE);

    // Next test of interop hash droping
    obj->DropInteropIndex();
    ASSERT_FALSE(obj->IsHashed());
    ASSERT_FALSE(obj->HasInteropIndex());

    // Next test of info table usage
    obj->SetInteropIndex(INTEROP_INDEX_VALUE);
    auto hash = obj->GetHashCode();
    // object still should be hashed
    ASSERT_TRUE(obj->IsUsedInfo());
    ASSERT_TRUE(obj->IsHashed());
    ASSERT_TRUE(obj->HasInteropIndex());
    // after this getting interop hash should works correct
    ASSERT_EQ(hash, obj->GetHashCode());
    ASSERT_EQ(INTEROP_INDEX_VALUE, obj->GetInteropIndex());

    // next we should test droping of interop hash
    obj->DropInteropIndex();
    ASSERT_TRUE(obj->IsUsedInfo());
    ASSERT_TRUE(obj->IsHashed());
    ASSERT_FALSE(obj->HasInteropIndex());
    ASSERT_EQ(hash, obj->GetHashCode());

    // finally we deflect object
    EtsCoroutine::GetCurrent()->GetVM()->FreeInternalResources();
    ASSERT_TRUE(obj->IsHashed());
    ASSERT_TRUE(obj->IsHashed());
    ASSERT_EQ(hash, obj->GetHashCode());
}

TEST_F(EtsObjectTest, InteropIndex_MultiThread)
{
    // CC-OFFNXT(G.NAM.03) project code style
    static constexpr uint32_t ITERATION_COUNT = 1000;
    for (size_t i = 0; i < ITERATION_COUNT; i++) {
        // create class for testing
        EtsClass *barKlass = GetTestClass("Bar");
        EtsObject *obj = EtsObject::Create(barKlass);
        std::atomic_bool finish = false;
        auto interopHashGetter = [obj, &finish, coro = EtsCoroutine::GetCurrent()] {
            EtsCoroutine::SetCurrent(coro);
            // CC-OFFNXT(G.NAM.03) project code style
            static constexpr uint32_t INTEROP_INDEX = 42U;
            obj->SetInteropIndex(INTEROP_INDEX);
            while (!finish) {
                ASSERT_EQ(INTEROP_INDEX, obj->GetInteropIndex());
            }
            obj->DropInteropIndex();
            ASSERT_TRUE(obj->IsHashed());
            ASSERT_FALSE(obj->HasInteropIndex());
        };

        auto interopThread = std::thread(interopHashGetter);
        auto hash = obj->GetHashCode();
        ASSERT_EQ(hash, obj->GetHashCode());
        finish = true;
        interopThread.join();
    }
}

TEST_F(EtsObjectTest, EtsHash_MultiThread)
{
    // CC-OFFNXT(G.NAM.03) project code style
    static constexpr uint32_t ITERATION_COUNT = 1000;
    for (size_t i = 0; i < ITERATION_COUNT; i++) {
        // create class for testing
        EtsClass *barKlass = GetTestClass("Bar");
        EtsObject *obj = EtsObject::Create(barKlass);
        std::atomic_bool finish = false;
        auto interopHashGetter = [obj, &finish, coro = EtsCoroutine::GetCurrent()] {
            EtsCoroutine::SetCurrent(coro);
            auto hash = obj->GetHashCode();
            while (!finish) {
                ASSERT_EQ(hash, obj->GetHashCode());
            }
            ASSERT_TRUE(obj->HasInteropIndex());
        };

        auto interopThread = std::thread(interopHashGetter);
        // CC-OFFNXT(G.NAM.03) project code style
        static constexpr uint32_t INTEROP_INDEX = 42U;
        obj->SetInteropIndex(INTEROP_INDEX);
        ASSERT_EQ(INTEROP_INDEX, obj->GetInteropIndex());
        finish = true;
        interopThread.join();
        obj->DropInteropIndex();
        ASSERT_TRUE(obj->IsUsedInfo());
        EtsCoroutine::GetCurrent()->GetVM()->FreeInternalResources();
        ASSERT_TRUE(obj->IsHashed());
    }
}

}  // namespace ark::ets::test

// NOLINTEND(readability-magic-numbers)
