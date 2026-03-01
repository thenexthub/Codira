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

#ifndef STRING_TABLE_BASE_TEST_H
#define STRING_TABLE_BASE_TEST_H

#include "gtest/gtest.h"
#include "runtime/include/coretypes/line_string.h"
#include "runtime/include/runtime.h"
#include "runtime/include/thread.h"
#include "runtime/include/gc_task.h"
#include "runtime/include/panda_vm.h"
#include "runtime/handle_base-inl.h"
#include "runtime/mem/refstorage/global_object_storage.h"
#include "runtime/include/thread_scopes.h"
#include "test_utils.h"

#include "libarkfile/file.h"
#include "libarkfile/file_item_container.h"
#include "libarkfile/file_writer.h"

#include <limits>

namespace ark::mem::test {
class StringTableTest : public testing::TestWithParam<const char *> {
public:
    static constexpr size_t G1_YOUNG_TEST_SIZE = 1_MB;
    static constexpr size_t NON_G1_YOUNG_TEST_SIZE = 18_MB;
    static constexpr size_t TEST_HEAP_SIZE = 36_MB;
    StringTableTest()
    {
        const std::string gcType = GetParam();
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        options.SetExplicitConcurrentGcEnabled(false);
        if (gcType == "g1-gc") {
            options.SetYoungSpaceSize(G1_YOUNG_TEST_SIZE);
        } else {
            options.SetYoungSpaceSize(NON_G1_YOUNG_TEST_SIZE);
        }

        options.SetHeapSizeLimit(TEST_HEAP_SIZE);
        options.SetGcType(gcType);
        options.SetCompilerEnableJit(false);
        Runtime::Create(options);

        thread_ = ark::MTManagedThread::GetCurrent();
    }

    NO_COPY_SEMANTIC(StringTableTest);
    NO_MOVE_SEMANTIC(StringTableTest);

    ~StringTableTest() override
    {
        Runtime::Destroy();
    }

    static coretypes::String *AllocUtf8String(std::vector<uint8_t> data, bool isMovable = true)
    {
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
        return coretypes::String::CreateFromMUtf8(data.data(), utf::MUtf8ToUtf16Size(data.data()), ctx,
                                                  Runtime::GetCurrent()->GetPandaVM(), isMovable);
    }

    void RunStringTableTests()
    {
        EmptyTable();
        InternCompressedUtf8AndString();
        InternUncompressedUtf8AndString();
        InternTheSameUtf16String();
        InternManyStrings();
        SweepObjectInTable();
        SweepNonMovableObjectInTable();
        SweepHumongousObjectInTable();
        InternTooLongString();
    }

    void EmptyTable()
    {
        ScopedManagedCodeThread s(thread_);
        auto table = StringTable();
        ASSERT_EQ(table.Size(), 0);
    }

    void InternCompressedUtf8AndString()
    {
        ScopedManagedCodeThread s(thread_);
        auto table = StringTable();
        std::vector<uint8_t> data {0x01, 0x02, 0x03, 0x00};  // NOLINT(readability-magic-numbers)
        auto *string = AllocUtf8String(data);
        auto *internedStr1 =
            table.GetOrInternString(data.data(), data.size() - 1,
                                    Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY));
        auto *internedStr2 = table.GetOrInternString(
            string, Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY));
        ASSERT_EQ(internedStr1, internedStr2);
        ASSERT_EQ(table.Size(), 1);
    }

    void InternUncompressedUtf8AndString()
    {
        ScopedManagedCodeThread s(thread_);
        auto table = StringTable();
        std::vector<uint8_t> data {0xc2, 0xa7, 0x34, 0x00};  // NOLINT(readability-magic-numbers)
        auto *string = AllocUtf8String(data);
        auto *internedStr1 = table.GetOrInternString(
            data.data(), 2, Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY));
        auto *internedStr2 = table.GetOrInternString(
            string, Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY));
        ASSERT_EQ(internedStr1, internedStr2);
        ASSERT_EQ(table.Size(), 1);
    }

    void InternTheSameUtf16String()
    {
        ScopedManagedCodeThread s(thread_);
        auto table = StringTable();
        std::vector<uint16_t> data {0xffc3, 0x33, 0x00};  // NOLINT(readability-magic-numbers)

        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
        auto *firstString =
            coretypes::String::CreateFromUtf16(data.data(), data.size(), ctx, Runtime::GetCurrent()->GetPandaVM());
        auto *secondString =
            coretypes::String::CreateFromUtf16(data.data(), data.size(), ctx, Runtime::GetCurrent()->GetPandaVM());

        auto *internedStr1 = table.GetOrInternString(firstString, ctx);
        auto *internedStr2 = table.GetOrInternString(secondString, ctx);
        ASSERT_EQ(internedStr1, internedStr2);
        ASSERT_EQ(table.Size(), 1);
    }

    void InternManyStrings()
    {
        ScopedManagedCodeThread s(thread_);
        static constexpr size_t ITERATIONS = 50;
        auto table = StringTable();
        std::vector<uint8_t> data {0x00};
        const unsigned numberOfLetters = 25;

        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
        for (size_t i = 0; i < ITERATIONS; i++) {
            data.insert(data.begin(), (('a' + i) % numberOfLetters) + 1);
            [[maybe_unused]] auto *firstPointer = table.GetOrInternString(AllocUtf8String(data), ctx);
            [[maybe_unused]] auto *secondPointer =
                table.GetOrInternString(data.data(), utf::MUtf8ToUtf16Size(data.data()), ctx);
            auto *thirdPointer = table.GetOrInternString(AllocUtf8String(data), ctx);
            ASSERT_EQ(firstPointer, secondPointer);
            ASSERT_EQ(secondPointer, thirdPointer);
        }
        ASSERT_EQ(table.Size(), ITERATIONS);
    }

    void SweepObjectInTable()
    {
        ScopedManagedCodeThread s(thread_);
        auto table = thread_->GetVM()->GetStringTable();
        auto tableInitSize = table->Size();
        std::vector<uint8_t> data1 {0x01, 0x00};
        std::vector<uint8_t> data2 {0x02, 0x00};
        std::vector<uint8_t> data3 {0x03, 0x00};
        const unsigned expectedTableSize = 2;

        auto storage = thread_->GetVM()->GetGlobalObjectStorage();

        auto *s1 = AllocUtf8String(data1);
        auto ref1 = storage->Add(s1, Reference::ObjectType::GLOBAL);
        auto *s2 = AllocUtf8String(data2);
        auto ref2 = storage->Add(s2, Reference::ObjectType::GLOBAL);
        auto *s3 = AllocUtf8String(data3);
        auto ref3 = storage->Add(s3, Reference::ObjectType::GLOBAL);

        auto pandaClassContext = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
        table->GetOrInternString(reinterpret_cast<coretypes::String *>(storage->Get(ref1)), pandaClassContext);
        table->GetOrInternString(reinterpret_cast<coretypes::String *>(storage->Get(ref2)), pandaClassContext);
        table->GetOrInternString(reinterpret_cast<coretypes::String *>(storage->Get(ref3)), pandaClassContext);

        storage->Remove(ref2);

        thread_->GetVM()->GetGC()->WaitForGCInManaged(ark::GCTask(ark::GCTaskCause::EXPLICIT_CAUSE));
        // Collect all heap for EXPLICIT_CAUSE
        ASSERT_EQ(table->Size(), tableInitSize + expectedTableSize);

        storage->Remove(ref1);
        storage->Remove(ref3);
        thread_->GetVM()->GetGC()->WaitForGCInManaged(ark::GCTask(ark::GCTaskCause::EXPLICIT_CAUSE));
        // Collect all heap for EXPLICIT_CAUSE
        ASSERT_EQ(table->Size(), tableInitSize);
    }

    void SweepNonMovableObjectInTable()
    {
        ScopedManagedCodeThread s(thread_);
        auto table = thread_->GetVM()->GetStringTable();
        auto tableInitSize = table->Size();
        std::vector<uint8_t> data1 {0x01, 0x00};
        std::vector<uint8_t> data2 {0x02, 0x00};
        std::vector<uint8_t> data3 {0x03, 0x00};
        const unsigned expectedTableSize = 2;

        auto storage = thread_->GetVM()->GetGlobalObjectStorage();

        auto *s1 = AllocUtf8String(data1, false);
        ASSERT_NE(s1, nullptr);
        auto ref1 = storage->Add(s1, Reference::ObjectType::GLOBAL);
        auto *s2 = AllocUtf8String(data2, false);
        ASSERT_NE(s2, nullptr);
        auto ref2 = storage->Add(s2, Reference::ObjectType::GLOBAL);
        auto *s3 = AllocUtf8String(data3, false);
        ASSERT_NE(s3, nullptr);
        auto ref3 = storage->Add(s3, Reference::ObjectType::GLOBAL);

        auto pandaClassContext = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
        table->GetOrInternString(reinterpret_cast<coretypes::String *>(storage->Get(ref1)), pandaClassContext);
        table->GetOrInternString(reinterpret_cast<coretypes::String *>(storage->Get(ref2)), pandaClassContext);
        table->GetOrInternString(reinterpret_cast<coretypes::String *>(storage->Get(ref3)), pandaClassContext);

        storage->Remove(ref2);

        thread_->GetVM()->GetGC()->WaitForGCInManaged(ark::GCTask(ark::GCTaskCause::EXPLICIT_CAUSE));
        // Collect all heap for EXPLICIT_CAUSE
        ASSERT_EQ(table->Size(), tableInitSize + expectedTableSize);

        storage->Remove(ref1);
        storage->Remove(ref3);
        thread_->GetVM()->GetGC()->WaitForGCInManaged(ark::GCTask(ark::GCTaskCause::EXPLICIT_CAUSE));
        // Collect all heap for EXPLICIT_CAUSE
        ASSERT_EQ(table->Size(), tableInitSize);
    }

    void SweepHumongousObjectInTable()
    {
        ScopedManagedCodeThread s(thread_);
        auto table = thread_->GetVM()->GetStringTable();
        static constexpr size_t HUMONGOUS_STRING_SIZE = 1_MB;
        std::vector<uint8_t> data1 {0x01};
        std::vector<uint8_t> data2 {0x02};
        std::vector<uint8_t> data3 {0x03};
        for (size_t i = 0; i < HUMONGOUS_STRING_SIZE; i++) {
            data1.push_back(0x05);
            data2.push_back(0x05);
            data3.push_back(0x05);
        }
        data1.push_back(0x00);
        data2.push_back(0x00);
        data3.push_back(0x00);
        auto tableInitSize = table->Size();
        const unsigned expectedTableSize = 2;

        auto storage = thread_->GetVM()->GetGlobalObjectStorage();

        auto *s1 = AllocUtf8String(data1);
        ASSERT_NE(s1, nullptr);
        auto ref1 = storage->Add(s1, Reference::ObjectType::GLOBAL);
        auto *s2 = AllocUtf8String(data2);
        ASSERT_NE(s2, nullptr);
        auto ref2 = storage->Add(s2, Reference::ObjectType::GLOBAL);
        auto *s3 = AllocUtf8String(data3);
        ASSERT_NE(s3, nullptr);
        auto ref3 = storage->Add(s3, Reference::ObjectType::GLOBAL);

        auto pandaClassContext = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
        table->GetOrInternString(reinterpret_cast<coretypes::String *>(storage->Get(ref1)), pandaClassContext);
        table->GetOrInternString(reinterpret_cast<coretypes::String *>(storage->Get(ref2)), pandaClassContext);
        table->GetOrInternString(reinterpret_cast<coretypes::String *>(storage->Get(ref3)), pandaClassContext);

        storage->Remove(ref2);

        thread_->GetVM()->GetGC()->WaitForGCInManaged(ark::GCTask(ark::GCTaskCause::EXPLICIT_CAUSE));
        // Collect all heap for EXPLICIT_CAUSE
        ASSERT_EQ(table->Size(), tableInitSize + expectedTableSize);

        storage->Remove(ref1);
        storage->Remove(ref3);
        thread_->GetVM()->GetGC()->WaitForGCInManaged(ark::GCTask(ark::GCTaskCause::EXPLICIT_CAUSE));
        // Collect all heap for EXPLICIT_CAUSE
        ASSERT_EQ(table->Size(), tableInitSize);
    }

    void InternTooLongString()
    {
        ScopedManagedCodeThread s(thread_);
        auto table = StringTable();
        auto *runtime = Runtime::GetCurrent();
        auto pandaClassContext = runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);

        auto *thread = ManagedThread::GetCurrent();

        [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);

        PandaVector<VMHandle<ObjectHeader>> objects;
        constexpr size_t STRING_DATA_SIZE = 20000U;
        constexpr uint8_t START_VALUE_IN_STRING_DATA = 0x30;
        std::vector<uint8_t> stringData(STRING_DATA_SIZE, START_VALUE_IN_STRING_DATA);
        stringData.push_back(0x00);

        auto fillHeap = [&stringData, &thread, &objects](bool isMovable) {
            while (true) {
                auto *obj = AllocUtf8String(stringData, isMovable);
                if (obj == nullptr) {
                    thread->ClearException();
                    break;
                }
                objects.emplace_back(thread, obj);
            }
        };

        {
            fillHeap(true);
            auto *res = table.GetOrInternString(stringData.data(), stringData.size() - 1, pandaClassContext);
            ASSERT_EQ(res, nullptr);
            ManagedThread::GetCurrent()->ClearException();
        }

        {
            panda_file::ItemContainer container;
            panda_file::MemoryWriter writer;

            auto *stringItem = container.GetOrCreateStringItem(reinterpret_cast<char *>(stringData.data()));

            container.Write(&writer);
            auto data = writer.GetData();

            auto id = panda_file::File::EntityId(stringItem->GetOffset());

            os::mem::ConstBytePtr ptr(reinterpret_cast<std::byte *>(data.data()), data.size(),
                                      [](std::byte *, size_t) noexcept {});

            auto pf = panda_file::File::OpenFromMemory(std::move(ptr));

            fillHeap(false);
            auto *res = table.GetOrInternInternalString(*pf, id, pandaClassContext);
            ASSERT_EQ(res, nullptr);
            ManagedThread::GetCurrent()->ClearException();
        }
    }

protected:
    ark::MTManagedThread *thread_;  // NOLINT(misc-non-private-member-variables-in-classes)
};

TEST_P(StringTableTest, StringTableGCsTest)
{
    RunStringTableTests();
}

INSTANTIATE_TEST_SUITE_P(StringTableTestOnDiffGCs, StringTableTest, ::testing::ValuesIn(TESTED_GC));
}  // namespace ark::mem::test

#endif  // STRING_TABLE_BASE_TEST_H
