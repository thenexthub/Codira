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
#include <cstdlib>

#include "assembly-parser.h"
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/mem/vm_handle.h"
#include "runtime/handle_scope-inl.h"
#include "runtime/include/coretypes/array.h"
#include "runtime/include/coretypes/line_string.h"
#include "runtime/mem/object_helpers-inl.h"

namespace ark::mem {

class StaticObjectHelpersTest : public testing::Test {
public:
    StaticObjectHelpersTest()
    {
        RuntimeOptions options;
        options.SetLoadRuntimes({"core"});
        options.SetGcType("epsilon");
        options.SetGcTriggerType("debug-never");
        auto *pandaStdLib = std::getenv("PANDA_STD_LIB");
        if (pandaStdLib == nullptr) {
            LOG(FATAL, RUNTIME) << "PANDA_STD_LIB env variable should be set and point to arkstdlib.abc";
        }
        options.SetBootPandaFiles({pandaStdLib});

        Runtime::Create(options);
    }

    ~StaticObjectHelpersTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(StaticObjectHelpersTest);
    NO_MOVE_SEMANTIC(StaticObjectHelpersTest);

    MTManagedThread *GetThread()
    {
        return MTManagedThread::GetCurrent();
    }

    Class *LoadTestClass(const char *source)
    {
        pandasm::Parser p;
        auto res = p.Parse(source);
        auto pf = pandasm::AsmEmitter::Emit(res.Value());
        EXPECT_NE(pf, nullptr);
        if (pf == nullptr) {
            return nullptr;
        }

        ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();
        classLinker->AddPandaFile(std::move(pf));
        auto *extension = classLinker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY);
        {
            ScopedManagedCodeThread s(GetThread());
            PandaString descriptor;
            Class *klass = extension->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("Test"), &descriptor));
            EXPECT_NE(nullptr, klass);
            if (klass != nullptr) {
                EXPECT_TRUE(classLinker->InitializeClass(GetThread(), klass));
            }
            return klass;
        }
    }

    ObjectHeader *NewTestInstance(const char *source)
    {
        Class *klass = LoadTestClass(source);
        if (klass == nullptr) {
            return nullptr;
        }
        return AllocObject(klass);
    }

    coretypes::String *AllocString()
    {
        Runtime *runtime = Runtime::GetCurrent();
        LanguageContext ctx = runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
        ScopedManagedCodeThread s(GetThread());
        return coretypes::String::CreateEmptyString(ctx, runtime->GetPandaVM());
    }

    ObjectHeader *AllocObject(Class *klass)
    {
        Runtime *runtime = Runtime::GetCurrent();
        ScopedManagedCodeThread s(GetThread());
        return runtime->GetPandaVM()->GetHeapManager()->AllocateObject(klass, klass->GetObjectSize());
    }

    coretypes::Array *AllocStringArray(size_t length)
    {
        Runtime *runtime = Runtime::GetCurrent();
        LanguageContext ctx = runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
        SpaceType spaceType = SpaceType::SPACE_TYPE_OBJECT;
        auto *klass = runtime->GetClassLinker()->GetExtension(ctx)->GetClassRoot(ClassRoot::ARRAY_STRING);
        ScopedManagedCodeThread s(GetThread());
        return coretypes::Array::Create(klass, length, spaceType);
    }
};

TEST_F(StaticObjectHelpersTest, TestPrimitiveField)
{
    Class *klass = LoadTestClass(R"(
        .record Test {
            i32 sfield <static>
            i32 ifield
        }
    )");
    ASSERT_NE(nullptr, klass);
    bool found = false;
    auto handler = [&found]([[maybe_unused]] ObjectHeader *obj, [[maybe_unused]] ObjectHeader *ref,
                            [[maybe_unused]] uint32_t offset, [[maybe_unused]] bool isVolatile) {
        found = true;
        return true;
    };
    GCStaticObjectHelpers::TraverseAllObjectsWithInfo<false>(klass->GetManagedObject(), handler);
    ASSERT_FALSE(found);
}

TEST_F(StaticObjectHelpersTest, TestStaticRefField)
{
    Class *klass = LoadTestClass(R"(
        .record panda.String <external>
        .record Test {
            panda.String field <static>
            panda.String nullField <static>
        }
    )");
    ASSERT_NE(klass, nullptr);
    Field *field = klass->GetStaticFieldByName(reinterpret_cast<const uint8_t *>("field"));
    ASSERT_NE(nullptr, field);
    ObjectHeader *expected = AllocString();
    ASSERT_NE(nullptr, expected);
    ObjectAccessor::SetFieldObject<false>(klass, *field, expected);

    size_t count = 0;
    auto handler = [klass, &count, expected](ObjectHeader *obj, ObjectHeader *ref, uint32_t offset, bool isVolatile) {
        ++count;
        EXPECT_EQ(obj, klass->GetManagedObject());
        EXPECT_EQ(expected, ref);
        EXPECT_EQ(ref, ObjectAccessor::GetObject<false>(obj, offset));
        EXPECT_FALSE(isVolatile);
        return true;
    };
    GCStaticObjectHelpers::TraverseAllObjectsWithInfo<false>(klass->GetManagedObject(), handler);
    ASSERT_EQ(1, count);
}

TEST_F(StaticObjectHelpersTest, TestStaticVolatileRefField)
{
    Class *klass = LoadTestClass(R"(
        .record panda.String <external>
        .record Test {
            panda.String field <static, volatile>
            panda.String nullField <static, volatile>
        }
    )");
    ASSERT_NE(klass, nullptr);
    Field *field = klass->GetStaticFieldByName(reinterpret_cast<const uint8_t *>("field"));
    ASSERT_NE(nullptr, field);
    ObjectHeader *expected = AllocString();
    ASSERT_NE(nullptr, expected);
    ObjectAccessor::SetFieldObject<false>(klass, *field, expected);

    size_t count = 0;
    auto handler = [klass, &count, expected](ObjectHeader *obj, ObjectHeader *ref, uint32_t offset, bool isVolatile) {
        ++count;
        EXPECT_EQ(obj, klass->GetManagedObject());
        EXPECT_EQ(expected, ref);
        EXPECT_EQ(ref, ObjectAccessor::GetObject<true>(obj, offset));
        EXPECT_TRUE(isVolatile);
        return true;
    };
    GCStaticObjectHelpers::TraverseAllObjectsWithInfo<false>(klass->GetManagedObject(), handler);
    ASSERT_EQ(1, count);
}

TEST_F(StaticObjectHelpersTest, TestInstanceRefField)
{
    ObjectHeader *object = NewTestInstance(R"(
        .record panda.String <external>
        .record Test {
            panda.String field
            panda.String nullField
        }
    )");

    ASSERT_NE(nullptr, object);
    auto *klass = object->ClassAddr<Class>();
    Field *field = klass->GetInstanceFieldByName(reinterpret_cast<const uint8_t *>("field"));
    ASSERT_NE(nullptr, field);
    ObjectHeader *expected = AllocString();
    ASSERT_NE(nullptr, expected);
    ObjectAccessor::SetFieldObject<false>(object, *field, expected);

    size_t count = 0;
    auto handler = [object, &count, expected](ObjectHeader *obj, ObjectHeader *ref, uint32_t offset, bool isVolatile) {
        ++count;
        EXPECT_EQ(object, obj);
        EXPECT_EQ(expected, ref);
        EXPECT_EQ(ref, ObjectAccessor::GetObject<false>(obj, offset));
        EXPECT_FALSE(isVolatile);
        return true;
    };
    GCStaticObjectHelpers::TraverseAllObjectsWithInfo<false>(object, handler);
    ASSERT_EQ(1, count);
}

TEST_F(StaticObjectHelpersTest, TestVolatileInstanceRefField)
{
    ObjectHeader *object = NewTestInstance(R"(
        .record panda.String <external>
        .record Test {
            panda.String field <volatile>
            panda.String nullField <volatile>
        }
    )");

    ASSERT_NE(nullptr, object);
    auto *klass = object->ClassAddr<Class>();
    Field *field = klass->GetInstanceFieldByName(reinterpret_cast<const uint8_t *>("field"));
    ASSERT_NE(nullptr, field);
    ObjectHeader *expected = AllocString();
    ASSERT_NE(nullptr, expected);
    ObjectAccessor::SetFieldObject<false>(object, *field, expected);

    size_t count = 0;
    auto handler = [object, &count, expected](ObjectHeader *obj, ObjectHeader *ref, uint32_t offset, bool isVolatile) {
        ++count;
        EXPECT_EQ(object, obj);
        EXPECT_EQ(expected, ref);
        EXPECT_EQ(ref, ObjectAccessor::GetObject<true>(obj, offset));
        EXPECT_TRUE(isVolatile);
        return true;
    };
    GCStaticObjectHelpers::TraverseAllObjectsWithInfo<false>(object, handler);
    ASSERT_EQ(1, count);
}

TEST_F(StaticObjectHelpersTest, TestArray)
{
    coretypes::Array *array = AllocStringArray(2);
    ASSERT_NE(nullptr, array);
    ObjectHeader *expected = AllocString();
    ASSERT_NE(nullptr, expected);
    array->Set(0, expected);

    size_t count = 0;
    auto handler = [array, &count, expected](ObjectHeader *obj, ObjectHeader *ref, uint32_t offset, bool isVolatile) {
        ++count;
        EXPECT_EQ(array, obj);
        EXPECT_EQ(expected, ref);
        EXPECT_EQ(ref, ObjectAccessor::GetObject<true>(obj, offset));
        EXPECT_FALSE(isVolatile);
        return true;
    };
    GCStaticObjectHelpers::TraverseAllObjectsWithInfo<false>(array, handler);
    ASSERT_EQ(1, count);
}

}  // namespace ark::mem
