/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"
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

#include "plugins/ets/runtime/ani/ani.h"

// NOLINTBEGIN(readability-identifier-naming, cppcoreguidelines-pro-type-vararg)

namespace ark::ets::test {
static const char *TEST_BIN_FILE_NAME = "StackTraceTest.abc";

class StackTraceTest : public testing::Test {
protected:
    static void SetUpTestCase()
    {
        if (std::getenv("PANDA_STD_LIB") == nullptr) {
            std::cerr << "PANDA_STD_LIB not set" << std::endl;
            std::abort();
        }
    }

    void SetUp() override
    {
        std::string bootPandaFilesOption =
            std::string("--ext:boot-panda-files=") + std::getenv("PANDA_STD_LIB") + ":" + TEST_BIN_FILE_NAME;
        ani_option option {bootPandaFilesOption.c_str(), nullptr};
        ani_options opts {1, &option};
        ASSERT_EQ(ANI_CreateVM(&opts, ANI_VERSION_1, &vm), ANI_OK) << "Cannot create ETS VM";
        ASSERT_EQ(vm->GetEnv(ANI_VERSION_1, &env), ANI_OK) << "Cannot get ani env";
    }

    void TearDown() override
    {
        ASSERT_EQ(vm->DestroyVM(), ANI_OK) << "Cannot destroy ETS VM";
    }

    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    ani_env *env {};
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    ani_vm *vm {};
};

TEST_F(StackTraceTest, PrintStackTrace)
{
    ani_class klass {};
    auto status = env->FindClass("StackTraceTest.StackTraceTest", &klass);
    ASSERT_EQ(status, ANI_OK);

    ani_static_method method {};
    status = env->Class_FindStaticMethod(klass, "invokeException", ":", &method);
    ASSERT_EQ(status, ANI_OK);

    testing::internal::CaptureStdout();

    status = env->Class_CallStaticMethod_Void(klass, method);
    ASSERT_EQ(status, ANI_OK);

    auto captured = testing::internal::GetCapturedStdout();
    auto pos = 0;

    pos = captured.find("Test message", pos);
    ASSERT_NE(pos, std::string::npos);
}

TEST_F(StackTraceTest, ErrorDescribe)
{
    ani_class klass {};
    auto status = env->FindClass("StackTraceTest.StackTraceTest", &klass);
    ASSERT_EQ(status, ANI_OK);

    ani_static_method method {};
    status = env->Class_FindStaticMethod(klass, "invokeUnhandledException", ":", &method);
    ASSERT_EQ(status, ANI_OK);

    testing::internal::CaptureStderr();

    status = env->Class_CallStaticMethod_Void(klass, method);
    ASSERT_EQ(status, ANI_PENDING_ERROR);

    status = env->DescribeError();
    ASSERT_EQ(status, ANI_OK);
    status = env->ResetError();
    ASSERT_EQ(status, ANI_OK);

    auto captured = testing::internal::GetCapturedStderr();
    auto pos = 0;

    pos = captured.find("Test message", pos);
    ASSERT_NE(pos, std::string::npos);

    pos = captured.find("StackTraceTest.throwing", pos);
    ASSERT_NE(pos, std::string::npos);

    pos = captured.find("StackTraceTest.nestedFunc2", pos);
    ASSERT_NE(pos, std::string::npos);

    pos = captured.find("StackTraceTest.nestedFunc1", pos);
    ASSERT_NE(pos, std::string::npos);

    pos = captured.find("StackTraceTest.invokeUnhandledException", pos);
    ASSERT_NE(pos, std::string::npos);
}

}  // namespace ark::ets::test

// NOLINTEND(readability-identifier-naming, cppcoreguidelines-pro-type-vararg)
