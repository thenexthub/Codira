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

#include "ani.h"
#include "ani_gtest.h"
#include "include/mem/allocator.h"
#include "include/stack_walker.h"
#include "include/stack_walker-inl.h"

namespace ark::ets::ani::testing {

class CframeIteratorTest : public AniTest {
public:
    // CC-OFFNXT(G.FMT.10-CPP) project code style // NOLINTNEXTLINE (readability-identifier-naming)
    static constexpr const char *moduleName = "cframe_iterator_test";
    // CC-OFFNXT(G.FMT.10-CPP) project code style // NOLINTNEXTLINE (readability-identifier-naming)
    static constexpr const char *testClassDescriptor = "cframe_iterator_test.Test";
};

// CC-OFFNXT(G.FMT.10-CPP) project code style
static const std::array<const char *, 4> G_TYPE_ARRAY1 = {"OBJECT", "INT32", "INT64", "BOOL"};

static bool CheckVregDataType(const std::array<const char *, 4> &typeArray)
{
    uint8_t index = 0;
    auto stack = StackWalker::Create(ManagedThread::GetCurrent());
    bool res =
        stack.IterateVRegsWithInfo([&index, typeArray]([[maybe_unused]] auto &regInfo, [[maybe_unused]] auto &vReg) {
            if (strcmp(regInfo.GetTypeString(), typeArray[index]) == 0) {
                index++;
                return true;
            }
            index++;
            return false;
        });
    return res;
}

ani_boolean GenericMethod([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_object obj, [[maybe_unused]] ani_int a,
                          [[maybe_unused]] ani_long b, [[maybe_unused]] ani_boolean c)
{
    if (CheckVregDataType(G_TYPE_ARRAY1)) {
        return ANI_TRUE;
    }
    return ANI_FALSE;
}

TEST_F(CframeIteratorTest, genericMethodTest)
{
    // CC-OFFNXT(G.FMT.10-CPP) project code style // NOLINTNEXTLINE (readability-identifier-naming)
    static constexpr const char *methodName = "genericMethod";
    // CC-OFFNXT(G.FMT.10-CPP) project code style // NOLINTNEXTLINE (readability-identifier-naming)
    static constexpr const char *signature = "ilz:z";
    // CC-OFFNXT(G.FMT.10-CPP) project code style // NOLINTNEXTLINE (readability-identifier-naming)
    static constexpr const char *etsFunctionName = "callGenericMethod";

    ani_class klass {};
    ASSERT_EQ(env_->FindClass(testClassDescriptor, &klass), ANI_OK);
    ani_native_function fn {methodName, signature, reinterpret_cast<void *>(GenericMethod)};
    ASSERT_EQ(env_->Class_BindNativeMethods(klass, &fn, 1), ANI_OK);

    ani_int a = 1;
    ani_long b = 2;
    ani_boolean c = ANI_TRUE;
    auto ret = CallEtsFunction<ani_boolean>(moduleName, etsFunctionName, a, b, c);
    ASSERT_EQ(ret, ANI_TRUE);
}

ani_boolean StaticMethod([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_object obj, [[maybe_unused]] ani_int a,
                         [[maybe_unused]] ani_long b, [[maybe_unused]] ani_boolean c)
{
    if (CheckVregDataType(G_TYPE_ARRAY1)) {
        return ANI_TRUE;
    }
    return ANI_FALSE;
}

TEST_F(CframeIteratorTest, staticMethodTest)
{
    // CC-OFFNXT(G.FMT.10-CPP) project code style // NOLINTNEXTLINE (readability-identifier-naming)
    static constexpr const char *methodName = "staticMethod";
    // CC-OFFNXT(G.FMT.10-CPP) project code style // NOLINTNEXTLINE (readability-identifier-naming)
    static constexpr const char *signature = "ilz:z";
    // CC-OFFNXT(G.FMT.10-CPP) project code style // NOLINTNEXTLINE (readability-identifier-naming)
    static constexpr const char *etsFunctionName = "callStaticMethod";

    ani_class klass {};
    ASSERT_EQ(env_->FindClass(testClassDescriptor, &klass), ANI_OK);
    ani_native_function fn {methodName, signature, reinterpret_cast<void *>(StaticMethod)};
    ASSERT_EQ(env_->Class_BindStaticNativeMethods(klass, &fn, 1), ANI_OK);

    ani_int a = 1;
    ani_long b = 2;
    ani_boolean c = ANI_TRUE;
    auto ret = CallEtsFunction<ani_boolean>(moduleName, etsFunctionName, a, b, c);
    ASSERT_EQ(ret, ANI_TRUE);
}

ani_boolean QuickMethod([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_object obj, [[maybe_unused]] ani_int a,
                        [[maybe_unused]] ani_long b, [[maybe_unused]] ani_boolean c)
{
    if (CheckVregDataType(G_TYPE_ARRAY1)) {
        return ANI_TRUE;
    }
    return ANI_FALSE;
}

TEST_F(CframeIteratorTest, quickMethodTest)
{
    // CC-OFFNXT(G.FMT.10-CPP) project code style // NOLINTNEXTLINE (readability-identifier-naming)
    static constexpr const char *methodName = "quickMethod";
    // CC-OFFNXT(G.FMT.10-CPP) project code style // NOLINTNEXTLINE (readability-identifier-naming)
    static constexpr const char *signature = "ilz:z";
    // CC-OFFNXT(G.FMT.10-CPP) project code style // NOLINTNEXTLINE (readability-identifier-naming)
    static constexpr const char *etsFunctionName = "callQuickMethod";

    ani_class klass {};
    ASSERT_EQ(env_->FindClass(testClassDescriptor, &klass), ANI_OK);
    ani_native_function fn {methodName, signature, reinterpret_cast<void *>(QuickMethod)};
    ASSERT_EQ(env_->Class_BindStaticNativeMethods(klass, &fn, 1), ANI_OK);

    ani_int a = 1;
    ani_long b = 2;
    ani_boolean c = ANI_TRUE;
    auto ret = CallEtsFunction<ani_boolean>(moduleName, etsFunctionName, a, b, c);
    ASSERT_EQ(ret, ANI_TRUE);
}

}  // namespace ark::ets::ani::testing
