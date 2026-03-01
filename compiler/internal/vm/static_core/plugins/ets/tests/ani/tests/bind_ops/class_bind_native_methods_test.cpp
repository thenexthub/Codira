/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
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

#include "ani_gtest.h"
#include <array>
#include <string_view>

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class ClassBindNativeMethodsTest : public AniTest {};
constexpr const char *MODULE_NAME = "@defModule.class_bind_native_methods_test";

// NOLINTNEXTLINE(readability-named-parameter)
static ani_int NativeMethodsFooNative(ani_env *, ani_object)
{
    const ani_int answer = 42U;
    return answer;
}

// NOLINTNEXTLINE(readability-named-parameter)
static ani_int NativeMethodsFooNativeOverride(ani_env *, ani_object)
{
    const ani_int answer = 43U;
    return answer;
}

// NOLINTNEXTLINE(readability-named-parameter)
static ani_long NativeMethodsLongFooNative(ani_env *, ani_object)
{
    // NOLINTNEXTLINE(readability-magic-numbers)
    return static_cast<ani_long>(84L);
}

// NOLINTNEXTLINE(readability-named-parameter)
static void CheckSignature(ani_env *, ani_object, ani_string) {}

TEST_F(ClassBindNativeMethodsTest, class_bindNativeMethods_combine_scenes_002)
{
    ani_class cls {};
    const std::string clsName = std::string(MODULE_NAME).append(".test002A.test002B.TestA002");
    ASSERT_EQ(env_->FindClass(clsName.c_str(), &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    std::array methods = {
        ani_native_function {"foo", ":i", reinterpret_cast<void *>(NativeMethodsFooNative)},
        ani_native_function {"long_foo", ":l", reinterpret_cast<void *>(NativeMethodsLongFooNative)},
    };
    ASSERT_EQ(env_->Class_BindStaticNativeMethods(cls, methods.data(), methods.size()), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_BindNativeMethods(cls, methods.data(), methods.size()), ANI_OK);

    ani_method constructorMethod {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", nullptr, &constructorMethod), ANI_OK);
    ASSERT_NE(constructorMethod, nullptr);

    ani_object object {};
    ASSERT_EQ(env_->Object_New(cls, constructorMethod, &object), ANI_OK);
    ASSERT_NE(object, nullptr);

    ani_method intFooMethod {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "foo", nullptr, &intFooMethod), ANI_OK);
    ASSERT_NE(intFooMethod, nullptr);

    ani_int fooResult = 0;
    ASSERT_EQ(env_->Object_CallMethod_Int(object, intFooMethod, &fooResult), ANI_OK);
    ASSERT_EQ(fooResult, 42U);

    ani_method longFooMethod {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "long_foo", nullptr, &longFooMethod), ANI_OK);
    ASSERT_NE(longFooMethod, nullptr);

    ani_long longFooResult = 0L;
    ASSERT_EQ(env_->Object_CallMethod_Long(object, longFooMethod, &longFooResult), ANI_OK);
    ASSERT_EQ(longFooResult, 84L);
}

TEST_F(ClassBindNativeMethodsTest, class_bindNativeMethods_combine_scenes_003)
{
    ani_class cls;
    const std::string clsName = std::string(MODULE_NAME).append(".TestB003");
    ASSERT_EQ(env_->FindClass(clsName.c_str(), &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    std::array methods = {
        ani_native_function {"foo", ":i", reinterpret_cast<void *>(NativeMethodsFooNative)},
        ani_native_function {"long_foo", ":l", reinterpret_cast<void *>(NativeMethodsLongFooNative)},
    };
    ASSERT_EQ(env_->Class_BindStaticNativeMethods(cls, methods.data(), methods.size()), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_BindNativeMethods(cls, methods.data(), methods.size()), ANI_OK);

    ani_method constructorMethod {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", nullptr, &constructorMethod), ANI_OK);
    ASSERT_NE(constructorMethod, nullptr);

    ani_object object {};
    ASSERT_EQ(env_->Object_New(cls, constructorMethod, &object), ANI_OK);
    ASSERT_NE(object, nullptr);

    ani_method intFooMethod {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "foo", nullptr, &intFooMethod), ANI_OK);
    ASSERT_NE(intFooMethod, nullptr);

    ani_int fooResult = 0;
    ASSERT_EQ(env_->Object_CallMethod_Int(object, intFooMethod, &fooResult), ANI_OK);
    ASSERT_EQ(fooResult, 42U);

    ani_method longFooMethod {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "long_foo", nullptr, &longFooMethod), ANI_OK);
    ASSERT_NE(longFooMethod, nullptr);

    ani_long longFooResult = 0L;
    ASSERT_EQ(env_->Object_CallMethod_Long(object, longFooMethod, &longFooResult), ANI_OK);
    ASSERT_EQ(longFooResult, 84L);
}

TEST_F(ClassBindNativeMethodsTest, class_bindNativeMethods_combine_scenes_004)
{
    ani_class cls {};
    const std::string clsName = std::string(MODULE_NAME).append(".TestA004");
    ASSERT_EQ(env_->FindClass(clsName.c_str(), &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    std::array methods = {
        ani_native_function {"foo", ":i", reinterpret_cast<void *>(NativeMethodsFooNative)},
        ani_native_function {"long_foo", ":l", reinterpret_cast<void *>(NativeMethodsLongFooNative)},
    };
    ASSERT_EQ(env_->Class_BindStaticNativeMethods(cls, methods.data(), methods.size()), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_BindNativeMethods(cls, methods.data(), methods.size()), ANI_OK);

    ani_method constructorMethod {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", nullptr, &constructorMethod), ANI_OK);
    ASSERT_NE(constructorMethod, nullptr);

    ani_object object {};
    ASSERT_EQ(env_->Object_New(cls, constructorMethod, &object), ANI_OK);
    ASSERT_NE(object, nullptr);

    ani_method intFooMethod {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "foo", nullptr, &intFooMethod), ANI_OK);
    ASSERT_NE(intFooMethod, nullptr);

    ani_int fooResult = 0;
    ASSERT_EQ(env_->Object_CallMethod_Int(object, intFooMethod, &fooResult), ANI_OK);
    ASSERT_EQ(fooResult, 42U);

    ani_method longFooMethod {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "long_foo", nullptr, &longFooMethod), ANI_OK);
    ASSERT_NE(longFooMethod, nullptr);

    ani_long longFooResult = 0L;
    ASSERT_EQ(env_->Object_CallMethod_Long(object, longFooMethod, &longFooResult), ANI_OK);
    ASSERT_EQ(longFooResult, 84L);
}

TEST_F(ClassBindNativeMethodsTest, class_bindNativeMethods_combine_scenes_005)
{
    ani_class cls {};
    const std::string clsName = std::string(MODULE_NAME).append(".TestA005");
    ASSERT_EQ(env_->FindClass(clsName.c_str(), &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    std::array methods = {
        ani_native_function {"foo", ":i", reinterpret_cast<void *>(NativeMethodsFooNative)},
    };
    ASSERT_EQ(env_->Class_BindNativeMethods(cls, methods.data(), methods.size()), ANI_NOT_FOUND);
}

TEST_F(ClassBindNativeMethodsTest, BindNativesInheritanceBTest)
{
    ani_class cls {};
    const std::string clsName = std::string(MODULE_NAME).append(".B");
    ASSERT_EQ(env_->FindClass(clsName.c_str(), &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    std::array method = {
        ani_native_function {"method_A", ":i", reinterpret_cast<void *>(NativeMethodsFooNative)},
    };
    ASSERT_EQ(env_->Class_BindNativeMethods(cls, method.data(), method.size()), ANI_NOT_FOUND);

    method = {
        ani_native_function {"method1", ":Iface", reinterpret_cast<void *>(NativeMethodsFooNative)},
    };
    ASSERT_EQ(env_->Class_BindNativeMethods(cls, method.data(), method.size()), ANI_INVALID_DESCRIPTOR);

    method = {
        ani_native_function {"method2", "C{@defModule.class_bind_native_methods_test.Impl}:",
                             reinterpret_cast<void *>(NativeMethodsFooNative)},
    };

    ASSERT_EQ(env_->Class_BindNativeMethods(cls, method.data(), method.size()), ANI_NOT_FOUND);

    std::array methods = {
        ani_native_function {"method1", ":C{@defModule.class_bind_native_methods_test.Impl}",
                             reinterpret_cast<void *>(NativeMethodsFooNative)},
        ani_native_function {"method2", "C{@defModule.class_bind_native_methods_test.Iface}:",
                             reinterpret_cast<void *>(NativeMethodsFooNative)},
    };
    ASSERT_EQ(env_->Class_BindStaticNativeMethods(cls, methods.data(), methods.size()), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_BindNativeMethods(cls, methods.data(), methods.size()), ANI_OK);
}

TEST_F(ClassBindNativeMethodsTest, BindNativesInheritanceCTest)
{
    ani_class cls {};
    const std::string clsName = std::string(MODULE_NAME).append(".C");
    ASSERT_EQ(env_->FindClass(clsName.c_str(), &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    std::array method = {
        ani_native_function {"method_A", ":i", reinterpret_cast<void *>(NativeMethodsFooNative)},
    };
    ASSERT_EQ(env_->Class_BindNativeMethods(cls, method.data(), method.size()), ANI_NOT_FOUND);

    method = {
        ani_native_function {"method_B", ":i", reinterpret_cast<void *>(NativeMethodsFooNative)},
    };
    ASSERT_EQ(env_->Class_BindNativeMethods(cls, method.data(), method.size()), ANI_NOT_FOUND);

    method = {
        ani_native_function {"method1", ":C{@defModule.class_bind_native_methods_test.Iface}",
                             reinterpret_cast<void *>(NativeMethodsFooNative)},
    };
    ASSERT_EQ(env_->Class_BindNativeMethods(cls, method.data(), method.size()), ANI_NOT_FOUND);

    method = {
        ani_native_function {"method2", "C{@defModule.class_bind_native_methods_test.Iface}:",
                             reinterpret_cast<void *>(NativeMethodsFooNative)},
    };
    ASSERT_EQ(env_->Class_BindNativeMethods(cls, method.data(), method.size()), ANI_NOT_FOUND);
}

TEST_F(ClassBindNativeMethodsTest, class_bindNativeMethods_combine_scenes_007)
{
    ani_class cls {};
    const std::string clsName = std::string(MODULE_NAME).append(".TestA006");
    ASSERT_EQ(env_->FindClass(clsName.c_str(), &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    std::array methods = {
        ani_native_function {"foo", "ii:i", reinterpret_cast<void *>(NativeMethodsFooNative)},
        ani_native_function {"foo", "iii:i", reinterpret_cast<void *>(NativeMethodsFooNativeOverride)},
    };
    ASSERT_EQ(env_->Class_BindStaticNativeMethods(cls, methods.data(), methods.size()), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_BindNativeMethods(cls, methods.data(), methods.size()), ANI_OK);

    ani_method constructorMethod {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", nullptr, &constructorMethod), ANI_OK);
    ASSERT_NE(constructorMethod, nullptr);

    ani_object object {};
    ASSERT_EQ(env_->Object_New(cls, constructorMethod, &object), ANI_OK);
    ASSERT_NE(object, nullptr);

    ani_method fooMethod {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "foo", "ii:i", &fooMethod), ANI_OK);
    ASSERT_NE(fooMethod, nullptr);

    ani_method fooMethodOverride {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "foo", "iii:i", &fooMethodOverride), ANI_OK);
    ASSERT_NE(fooMethodOverride, nullptr);

    ani_int result = 0;
    ASSERT_EQ(env_->Object_CallMethod_Int(object, fooMethod, &result, 0, 1), ANI_OK);
    ASSERT_EQ(result, 42U);

    ASSERT_EQ(env_->Object_CallMethod_Int(object, fooMethodOverride, &result, 0, 1, 2U), ANI_OK);
    ASSERT_EQ(result, 43U);
}

static void Ctor([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_object object) {}

TEST_F(ClassBindNativeMethodsTest, bind_constructor)
{
    std::string_view className = "@defModule.class_bind_native_methods_test.D";

    ani_class cls {};

    ASSERT_EQ(env_->FindClass(className.data(), &cls), ANI_OK);

    std::array methods = {
        ani_native_function {"<ctor>", nullptr, reinterpret_cast<void *>(Ctor)},
    };
    ASSERT_EQ(env_->Class_BindStaticNativeMethods(cls, methods.data(), methods.size()), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_BindNativeMethods(cls, methods.data(), methods.size()), ANI_OK);
}

TEST_F(ClassBindNativeMethodsTest, method_bind_bad_signature)
{
    ani_class cls {};
    const std::string clsName = std::string(MODULE_NAME).append(".CheckWrongSignature");
    ASSERT_EQ(env_->FindClass(clsName.c_str(), &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    std::array m = {
        ani_native_function {"method", "C{std/core/String}:", reinterpret_cast<void *>(CheckSignature)},
    };
    ASSERT_EQ(env_->c_api->Class_BindNativeMethods(env_, cls, m.data(), m.size()), ANI_INVALID_DESCRIPTOR);

    m = {
        ani_native_function {"method", "C{std.core.String}:", reinterpret_cast<void *>(CheckSignature)},
    };
    ASSERT_EQ(env_->c_api->Class_BindNativeMethods(env_, cls, m.data(), m.size()), ANI_OK);
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
