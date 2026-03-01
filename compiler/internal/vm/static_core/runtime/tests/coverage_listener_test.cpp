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

#include <atomic>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <thread>

#include "runtime/include/runtime.h"
#include "runtime/include/method.h"
#include "runtime/tests/test_utils.h"
#include "runtime/tooling/coverage_listener.h"

#include "assembly-emitter.h"
#include "assembly-parser.h"

using namespace std::chrono;  // NOLINT(google-build-using-namespace)

namespace ark::tooling::test {
constexpr uint32_t TEST_BUFFER_SIZE = 200;
constexpr uint32_t MAP_COUNT = 2;
constexpr uint32_t WRITE_THREAD_TIMEOUT_MS = 1100;

class BytecodeCounterTest : public testing::Test {
public:
    BytecodeCounterTest()
    {
        RuntimeOptions options;
        Runtime::Create(options);
    }

    void SetUp() override {}

    void TearDown() override
    {
        Runtime::Destroy();
    }

    std::shared_ptr<Method> CreateMethod()
    {
        pandasm::Parser p;
        auto source = R"(
            .function void foo(i32 a0, i32 a1) {
                movi v0, 1
                movi v1, 2
                return.void
            }
        )";

        std::string srcFilename = "src.pa";
        auto res = p.Parse(source, srcFilename);
        auto filePtr = pandasm::AsmEmitter::Emit(res.Value());

        PandaString descriptor;
        auto classId = filePtr->GetClassId(ClassHelper::GetDescriptor(utf::CStringAsMutf8("_GLOBAL"), &descriptor));

        panda_file::ClassDataAccessor cda(*filePtr, classId);
        panda_file::File::EntityId methodId;
        panda_file::File::EntityId codeId;

        cda.EnumerateMethods([&](panda_file::MethodDataAccessor &mda) {
            methodId = mda.GetMethodId();
            ASSERT_TRUE(mda.GetCodeId());
            codeId = mda.GetCodeId().value();
        });

        panda_file::CodeDataAccessor codeDataAccessor(*filePtr, codeId);
        auto nargs = codeDataAccessor.GetNumArgs();
        auto method = std::make_shared<Method>(nullptr, filePtr.get(), methodId, codeId, 0, nargs, nullptr);
        return method;
    }
};

TEST_F(BytecodeCounterTest, BytecodeCounterGetNullQueueTest)
{
    auto method = CreateMethod();
    BytecodeCounter counter(TEST_BUFFER_SIZE);

    std::optional<std::queue<BytecodeCountMap>> queueOpt;
    std::thread writeThread([&queueOpt, &counter]() { queueOpt = counter.GetQueue(); });
    for (uint32_t i = 0; i < TEST_BUFFER_SIZE - 1; i++) {
        counter.BytecodePcChanged(nullptr, method.get(), i);
    }
    writeThread.join();

    ASSERT_FALSE(queueOpt.has_value());
}

TEST_F(BytecodeCounterTest, BytecodeCounterGetSingleQueueTest)
{
    auto method = CreateMethod();
    BytecodeCounter counter(TEST_BUFFER_SIZE);

    std::optional<std::queue<BytecodeCountMap>> queueOpt;
    std::thread writeThread([&queueOpt, &counter]() { queueOpt = counter.GetQueue(); });
    for (uint32_t i = 0; i < TEST_BUFFER_SIZE; i++) {
        counter.BytecodePcChanged(nullptr, method.get(), i);
    }
    writeThread.join();

    ASSERT_TRUE(queueOpt.has_value());
    auto &queue = queueOpt.value();
    ASSERT_EQ(queue.size(), 1);
    auto &bytecodeCountMap = queue.front();
    ASSERT_EQ(bytecodeCountMap.size(), TEST_BUFFER_SIZE);
}

TEST_F(BytecodeCounterTest, BytecodeCounterGetMultiQueueTest)
{
    auto method = CreateMethod();
    BytecodeCounter counter(TEST_BUFFER_SIZE);

    std::vector<BytecodeCountMap> countMapList;
    std::thread writeThread([&counter, &countMapList]() {
        auto startTime = system_clock::now();
        while (duration_cast<milliseconds>(system_clock::now() - startTime).count() < WRITE_THREAD_TIMEOUT_MS &&
               countMapList.size() < MAP_COUNT) {
            auto queueOpt = counter.GetQueue();
            if (!queueOpt.has_value()) {
                continue;
            }
            auto &queue = queueOpt.value();
            while (!queue.empty()) {
                countMapList.push_back(queue.front());
                queue.pop();
            }
        }
    });
    for (uint32_t i = 0; i < TEST_BUFFER_SIZE * MAP_COUNT; i++) {
        counter.BytecodePcChanged(nullptr, method.get(), i);
    }
    writeThread.join();

    ASSERT_EQ(countMapList.size(), MAP_COUNT);
    auto &bytecodeCountMap = countMapList[0];
    ASSERT_EQ(bytecodeCountMap.size(), TEST_BUFFER_SIZE);
}

TEST_F(BytecodeCounterTest, BytecodeCounterGetRemainingQueueTest)
{
    auto method = CreateMethod();
    BytecodeCounter counter(TEST_BUFFER_SIZE);

    std::optional<std::queue<BytecodeCountMap>> queueOpt;
    std::thread writeThread([&queueOpt, &counter]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(BytecodeCounter::MAX_DUMP_INTERVAL_MS + 1));
        queueOpt = counter.GetQueue();
    });
    for (uint32_t i = 0; i < TEST_BUFFER_SIZE / MAP_COUNT; i++) {
        counter.BytecodePcChanged(nullptr, method.get(), i);
    }
    writeThread.join();

    ASSERT_TRUE(queueOpt.has_value());
    auto &queue = queueOpt.value();
    ASSERT_EQ(queue.size(), 1);
    auto &bytecodeCountMap = queue.front();
    ASSERT_EQ(bytecodeCountMap.size(), TEST_BUFFER_SIZE / MAP_COUNT);
}
}  // namespace ark::tooling::test
