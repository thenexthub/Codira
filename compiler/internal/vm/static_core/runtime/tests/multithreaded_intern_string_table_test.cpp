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

#include "gtest/gtest.h"
#include "runtime/include/coretypes/line_string.h"
#include "runtime/include/runtime.h"
#include "runtime/include/thread.h"
#include "runtime/include/gc_task.h"
#include "runtime/handle_base-inl.h"

#include <array>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <random>
#include <climits>
#include <functional>

namespace ark::mem::test {

static constexpr uint32_t TEST_THREADS = 8;
static constexpr uint32_t TEST_ITERS = 1000;
static constexpr uint32_t TEST_ARRAY_SIZE = TEST_THREADS * 1000;

class MultithreadedInternStringTableTest : public testing::Test {
public:
    MultithreadedInternStringTableTest()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);

        options.SetGcType("epsilon");
        options.SetCompilerEnableJit(false);
        Runtime::Create(options);
    }

    ~MultithreadedInternStringTableTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(MultithreadedInternStringTableTest);
    NO_MOVE_SEMANTIC(MultithreadedInternStringTableTest);

    static coretypes::String *AllocUtf8String(std::vector<uint8_t> data)
    {
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
        return coretypes::String::CreateFromMUtf8(data.data(), utf::MUtf8ToUtf16Size(data.data()), ctx,
                                                  Runtime::GetCurrent()->GetPandaVM());
    }

    void SetUp() override
    {
        table_ = new StringTable();
        thread_ = ark::MTManagedThread::GetCurrent();
        thread_->ManagedCodeBegin();
    }

    void TearDown() override
    {
        thread_->ManagedCodeEnd();
        delete table_;
    }

    StringTable *GetTable()
    {
        return table_;
    }

    void PreCheck()
    {
        std::unique_lock<std::mutex> lk(preLock_);
        counterPre_++;
        if (counterPre_ == TEST_THREADS) {
            preCv_.notify_all();
            counterPre_ = 0;
        } else {
            preCv_.wait(lk);
        }
    }

    void CheckSameString(coretypes::String *string)
    {
        // Loop until lock is taken
        while (lock_.test_and_set(std::memory_order_seq_cst)) {
        }
        if (string_ != nullptr) {
            ASSERT_EQ(string_, string);
        } else {
            string_ = string;
        }
        lock_.clear(std::memory_order_seq_cst);
    }

    void PostFree()
    {
        std::unique_lock<std::mutex> lk(postLock_);
        counterPost_++;
        if (counterPost_ == TEST_THREADS) {
            // There should be just one element in table
            ASSERT_EQ(table_->Size(), 1);
            string_ = nullptr;

            {
                os::memory::WriteLockHolder holder(table_->table_.tableLock_);
                table_->table_.table_.clear();
            }
            {
                os::memory::WriteLockHolder holder(table_->internalTable_.tableLock_);
                table_->internalTable_.table_.clear();
            }

            postCv_.notify_all();
            counterPost_ = 0;
        } else {
            postCv_.wait(lk);
        }
    }

    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    std::mutex mutex;

private:
    ark::MTManagedThread *thread_ {};
    std::mutex preLock_;
    std::condition_variable preCv_;
    int counterPre_ = 0;
    std::mutex postLock_;
    std::condition_variable postCv_;
    int counterPost_ = 0;
    StringTable *table_ {};

    std::atomic_flag lock_ {false};
    coretypes::String *string_ {nullptr};
};

void TestThreadEntry(MultithreadedInternStringTableTest *test)
{
    auto *thisThread =
        ark::MTManagedThread::Create(ark::Runtime::GetCurrent(), ark::Runtime::GetCurrent()->GetPandaVM());
    thisThread->ManagedCodeBegin();
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    // NOLINTNEXTLINE(readability-magic-numbers)
    std::vector<uint8_t> data {0xc2, 0xa7, 0x34, 0x00};
    auto *table = test->GetTable();
    for (uint32_t i = 0; i < TEST_ITERS; i++) {
        test->PreCheck();
        auto *internedStr = table->GetOrInternString(data.data(), 2, ctx);
        test->CheckSameString(internedStr);
        test->PostFree();
    }
    thisThread->ManagedCodeEnd();
    thisThread->Destroy();
}

void TestConcurrentInsertion(const std::array<std::array<uint8_t, 4U>, TEST_ARRAY_SIZE> &strings, uint32_t &arrayItem,
                             MultithreadedInternStringTableTest *test)
{
    auto *thisThread =
        ark::MTManagedThread::Create(ark::Runtime::GetCurrent(), ark::Runtime::GetCurrent()->GetPandaVM());
    thisThread->ManagedCodeBegin();
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto *table = test->GetTable();

    uint32_t currentArrayItem = 0;
    while (true) {
        {
            std::lock_guard<std::mutex> lockGuard(test->mutex);
            if (arrayItem >= TEST_ARRAY_SIZE) {
                break;
            }
            currentArrayItem = arrayItem++;
        }
        table->GetOrInternString(strings[currentArrayItem].data(), 2U, ctx);
    }

    thisThread->ManagedCodeEnd();
    thisThread->Destroy();
}

TEST_F(MultithreadedInternStringTableTest, ConcurrentInsertion)
{
    std::array<std::thread, TEST_THREADS> threads;
    std::array<std::array<uint8_t, 4U>, TEST_ARRAY_SIZE> strings {};
    std::random_device randomDevice;
    std::mt19937 engine {randomDevice()};
    // NOLINTNEXTLINE(readability-magic-numbers)
    std::uniform_int_distribution<uint8_t> dist(1, 127U);
    uint32_t arrayItem = 0;

    for (uint32_t i = 0; i < TEST_ARRAY_SIZE; i++) {
        uint8_t second = utf::UTF8_2B_SECOND | static_cast<uint8_t>(dist(engine) >> 1U);
        // NOLINTNEXTLINE(readability-magic-numbers)
        strings[i] = {0xc2, second, dist(engine), 0x00};
    }

    for (uint32_t i = 0; i < TEST_THREADS; i++) {
        threads[i] = std::thread(TestConcurrentInsertion, std::ref(strings), std::ref(arrayItem), this);
    }

    for (uint32_t i = 0; i < TEST_THREADS; i++) {
        threads[i].join();
    }
}

TEST_F(MultithreadedInternStringTableTest, CheckInternReturnsSameString)
{
    std::array<std::thread, TEST_THREADS> threads;
    for (uint32_t i = 0; i < TEST_THREADS; i++) {
        threads[i] = std::thread(TestThreadEntry, this);
    }
    for (uint32_t i = 0; i < TEST_THREADS; i++) {
        threads[i].join();
    }
}
}  // namespace ark::mem::test
