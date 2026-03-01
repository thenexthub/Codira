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

#include "libarkbase/mem/mem.h"
#include "runtime/include/thread.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/ani/scoped_objects_fix.h"
#include "plugins/ets/tests/mock/mock_test_helper.h"

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)

namespace ark::ets::test {

static const char *g_testBinFileName = "RegisterNativeAllocationTest.abc";

class CallingMethodsTestGeneral : public MockEtsNapiTestBaseClass {
public:
    CallingMethodsTestGeneral() : MockEtsNapiTestBaseClass(g_testBinFileName) {}
};

class RegisterNativeAllocationTest : public CallingMethodsTestGeneral {
    void SetUp() override
    {
        const char *stdlib = std::getenv("PANDA_STD_LIB");
        ASSERT_NE(stdlib, nullptr);

        // Create boot-panda-files options
        std::string bootFileString =
            std::string("--ext:boot-panda-files=") + std::string(stdlib) + ":" + g_testBinFileName;

        // clang-format off
        std::vector<ani_option> optionsVector{
            {bootFileString.data(), nullptr},
            {"--ext:gc-type=g1-gc", nullptr},
            {"--ext:run-gc-in-place", nullptr},
            {"--ext:compiler-enable-jit=false", nullptr},
        };
        // clang-format on
        ani_options optionsPtr = {optionsVector.size(), optionsVector.data()};

        ASSERT_EQ(ANI_CreateVM(&optionsPtr, ANI_VERSION_1, &vm_), ANI_OK) << "Cannot create ETS VM";
        ASSERT_EQ(vm_->GetEnv(ANI_VERSION_1, &env_), ANI_OK) << "Cannot get ani env";
    }
};

TEST_F(RegisterNativeAllocationTest, testNativeAllocation)
{
    mem::MemStatsType *memStats = Thread::GetCurrent()->GetVM()->GetMemStats();

    ani_class testClass;
    ASSERT_EQ(env_->FindClass("RegisterNativeAllocationTest.NativeAllocationTest", &testClass), ANI_OK);

    ani_static_method allocMethod {};
    ASSERT_EQ(env_->Class_FindStaticMethod(testClass, "allocate_object", ":i", &allocMethod), ANI_OK);
    ani_int value1 = -1;
    ASSERT_EQ(env_->Class_CallStaticMethod_Int(testClass, allocMethod, &value1), ANI_OK);
    ASSERT_EQ(value1, 0);

    size_t heapFreedBeforeMethod;
    {
        ark::ets::ani::ScopedManagedCodeFix s(env_);
        heapFreedBeforeMethod = memStats->GetFreedHeap();
    }

    ani_static_method mainMethod {};
    ASSERT_EQ(env_->Class_FindStaticMethod(testClass, "main_method", ":i", &mainMethod), ANI_OK);
    ani_int value2 = -1;
    ASSERT_EQ(env_->Class_CallStaticMethod_Int(testClass, mainMethod, &value2), ANI_OK);
    ASSERT_EQ(value2, 0);

    size_t heapFreedAfterMethod;
    {
        ark::ets::ani::ScopedManagedCodeFix s(env_);
        heapFreedAfterMethod = memStats->GetFreedHeap();
    }

    ASSERT_GT(heapFreedAfterMethod, heapFreedBeforeMethod);
}

}  // namespace ark::ets::test

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
