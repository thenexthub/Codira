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

#include "ani_gtest.h"
#include <array>

namespace ark::ets::ani::testing {

class ClassBindStaticNativeMethodsTest : public AniTest {
    void SetUp() override
    {
        AniTest::SetUp();
        ASSERT_EQ(env_->FindClass("@defModule.class_bind_static_native_methods_test.SmallBox", &smallBoxCls_), ANI_OK);
        ASSERT_EQ(env_->FindClass("@defModule.class_bind_static_native_methods_test.BigBox", &bigBoxCls_), ANI_OK);
        ASSERT_EQ(env_->FindClass("@defModule.class_bind_static_native_methods_test.CheckSignature", &checkSignCls_),
                  ANI_OK);
    }

protected:
    ani_class smallBoxCls_ {};   // NOLINT(misc-non-private-member-variables-in-classes,-warnings-as-errors)
    ani_class bigBoxCls_ {};     // NOLINT(misc-non-private-member-variables-in-classes,-warnings-as-errors)
    ani_class checkSignCls_ {};  // NOLINT(misc-non-private-member-variables-in-classes,-warnings-as-errors)
};

static ani_int NativeMethod([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_class cls)
{
    const ani_int answer = 43U;
    return answer;
}

static void CheckSignature([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_class cls,
                           [[maybe_unused]] ani_string str)
{
}

TEST_F(ClassBindStaticNativeMethodsTest, wrong_null_args)
{
    std::array m = {
        ani_native_function {"foo", ":i", reinterpret_cast<void *>(NativeMethod)},
    };
    ASSERT_EQ(env_->c_api->Class_BindStaticNativeMethods(nullptr, smallBoxCls_, m.data(), m.size()), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->c_api->Class_BindStaticNativeMethods(env_, nullptr, m.data(), m.size()), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->c_api->Class_BindStaticNativeMethods(env_, smallBoxCls_, nullptr, m.size()), ANI_INVALID_ARGS);
}

TEST_F(ClassBindStaticNativeMethodsTest, wrong_bind_to_instance_method)
{
    std::array m = {
        ani_native_function {"bar", ":i", reinterpret_cast<void *>(NativeMethod)},
    };
    ASSERT_EQ(env_->c_api->Class_BindStaticNativeMethods(env_, smallBoxCls_, m.data(), m.size()), ANI_NOT_FOUND);
}

TEST_F(ClassBindStaticNativeMethodsTest, wrong_bind_to_parent_instance_method)
{
    std::array m = {
        ani_native_function {"bar2", ":i", reinterpret_cast<void *>(NativeMethod)},
    };
    ASSERT_EQ(env_->c_api->Class_BindStaticNativeMethods(env_, bigBoxCls_, m.data(), m.size()), ANI_NOT_FOUND);
}

TEST_F(ClassBindStaticNativeMethodsTest, wrong_bind_to_parent_static_method)
{
    std::array m = {
        ani_native_function {"foo", ":i", reinterpret_cast<void *>(NativeMethod)},
    };
    ASSERT_EQ(env_->c_api->Class_BindStaticNativeMethods(env_, bigBoxCls_, m.data(), m.size()), ANI_NOT_FOUND);
}

TEST_F(ClassBindStaticNativeMethodsTest, method_not_found)
{
    std::array m = {
        ani_native_function {"foo", ":i", reinterpret_cast<void *>(NativeMethod)},
        ani_native_function {"foo1", ":i", reinterpret_cast<void *>(NativeMethod)},
        ani_native_function {"foo2", ":i", reinterpret_cast<void *>(NativeMethod)},
    };
    ASSERT_EQ(env_->c_api->Class_BindStaticNativeMethods(env_, smallBoxCls_, m.data(), m.size()), ANI_NOT_FOUND);
}

TEST_F(ClassBindStaticNativeMethodsTest, wrong_bind_to_overload)
{
    std::array m = {
        ani_native_function {"overload_fn", nullptr, reinterpret_cast<void *>(NativeMethod)},
    };
    ASSERT_EQ(env_->c_api->Class_BindStaticNativeMethods(env_, smallBoxCls_, m.data(), m.size()), ANI_NOT_FOUND);
}

TEST_F(ClassBindStaticNativeMethodsTest, wrong_bind_to_not_native_method)
{
    std::array m = {
        ani_native_function {"not_native", nullptr, reinterpret_cast<void *>(NativeMethod)},
    };
    ASSERT_EQ(env_->c_api->Class_BindStaticNativeMethods(env_, smallBoxCls_, m.data(), m.size()), ANI_NOT_FOUND);
}

TEST_F(ClassBindStaticNativeMethodsTest, success)
{
    std::array m = {
        ani_native_function {"foo", ":i", reinterpret_cast<void *>(NativeMethod)},
        ani_native_function {"foo1", ":i", reinterpret_cast<void *>(NativeMethod)},
    };
    ASSERT_EQ(env_->c_api->Class_BindNativeMethods(env_, smallBoxCls_, m.data(), m.size()), ANI_NOT_FOUND);
    ASSERT_EQ(env_->c_api->Class_BindStaticNativeMethods(env_, smallBoxCls_, m.data(), m.size()), ANI_OK);
}

TEST_F(ClassBindStaticNativeMethodsTest, success_with_overload_methods)
{
    std::array m = {
        ani_native_function {"overload_fn_int", "i:", reinterpret_cast<void *>(NativeMethod)},
        ani_native_function {"overload_fn_long", "l:", reinterpret_cast<void *>(NativeMethod)},
    };
    ASSERT_EQ(env_->c_api->Class_BindNativeMethods(env_, smallBoxCls_, m.data(), m.size()), ANI_NOT_FOUND);
    ASSERT_EQ(env_->c_api->Class_BindStaticNativeMethods(env_, smallBoxCls_, m.data(), m.size()), ANI_OK);
}

TEST_F(ClassBindStaticNativeMethodsTest, success_without_signature)
{
    std::array m = {
        ani_native_function {"foo", nullptr, reinterpret_cast<void *>(NativeMethod)},
        ani_native_function {"foo1", nullptr, reinterpret_cast<void *>(NativeMethod)},
    };
    ASSERT_EQ(env_->c_api->Class_BindNativeMethods(env_, smallBoxCls_, m.data(), m.size()), ANI_NOT_FOUND);
    ASSERT_EQ(env_->c_api->Class_BindStaticNativeMethods(env_, smallBoxCls_, m.data(), m.size()), ANI_OK);
}

TEST_F(ClassBindStaticNativeMethodsTest, success_for_child_class)
{
    std::array m = {
        ani_native_function {"bar", nullptr, reinterpret_cast<void *>(NativeMethod)},
    };
    ASSERT_EQ(env_->c_api->Class_BindNativeMethods(env_, bigBoxCls_, m.data(), m.size()), ANI_NOT_FOUND);
    ASSERT_EQ(env_->c_api->Class_BindStaticNativeMethods(env_, bigBoxCls_, m.data(), m.size()), ANI_OK);
}

TEST_F(ClassBindStaticNativeMethodsTest, method_bind_bad_signature)
{
    std::array m = {
        ani_native_function {"method", "C{std/core/String}:", reinterpret_cast<void *>(CheckSignature)},
    };
    ASSERT_EQ(env_->c_api->Class_BindStaticNativeMethods(env_, checkSignCls_, m.data(), m.size()),
              ANI_INVALID_DESCRIPTOR);

    m = {
        ani_native_function {"method", "C{std.core.String}:", reinterpret_cast<void *>(CheckSignature)},
    };
    ASSERT_EQ(env_->c_api->Class_BindStaticNativeMethods(env_, checkSignCls_, m.data(), m.size()), ANI_OK);
}

}  // namespace ark::ets::ani::testing
