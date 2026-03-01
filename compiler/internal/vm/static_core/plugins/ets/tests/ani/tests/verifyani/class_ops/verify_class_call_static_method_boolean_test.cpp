/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "plugins/ets/tests/ani/ani_gtest/verify_ani_gtest.h"

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)
namespace ark::ets::ani::verify::testing {

class ClassCallStaticMethodBooleanTest : public VerifyAniTest {
public:
    void SetUp() override
    {
        VerifyAniTest::SetUp();

        std::string_view str("ani");
        ASSERT_EQ(env_->String_NewUTF8(str.data(), str.size(), &r_), ANI_OK);
    }

protected:
    const ani_boolean z_ = ANI_TRUE;  // NOLINT(misc-non-private-member-variables-in-classes)
    const ani_char c_ = 'a';          // NOLINT(misc-non-private-member-variables-in-classes)
    const ani_byte b_ = 0x24;         // NOLINT(misc-non-private-member-variables-in-classes)
    const ani_short s_ = 0x2012;      // NOLINT(misc-non-private-member-variables-in-classes)
    const ani_int i_ = 0xb0ba;        // NOLINT(misc-non-private-member-variables-in-classes)
    const ani_long l_ = 0xb1bab0ba;   // NOLINT(misc-non-private-member-variables-in-classes)
    const ani_float f_ = 0.2609;      // NOLINT(misc-non-private-member-variables-in-classes)
    const ani_double d_ = 21.03;      // NOLINT(misc-non-private-member-variables-in-classes)
    ani_string r_ {};                 // NOLINT(misc-non-private-member-variables-in-classes)
};

class ClassCallStaticMethodBooleanATest : public ClassCallStaticMethodBooleanTest {
public:
    // CC-OFFNXT(G.CLS.13) false positive
    void SetUp() override
    {
        ClassCallStaticMethodBooleanTest::SetUp();

        const int nr0 = 0;
        const int nr1 = 1;
        const int nr2 = 2;
        const int nr3 = 3;
        const int nr4 = 4;
        const int nr5 = 5;
        const int nr6 = 6;
        const int nr7 = 7;
        const int nr8 = 8;

        args_[nr0].z = z_;
        args_[nr1].c = c_;
        args_[nr2].b = b_;
        args_[nr3].s = s_;
        args_[nr4].i = i_;
        args_[nr5].l = l_;
        args_[nr6].f = f_;
        args_[nr7].d = d_;
        args_[nr8].r = r_;
    }

protected:
    static constexpr size_t NR_ARGS = 9;
    std::array<ani_value, NR_ARGS> args_ {};  // NOLINT(misc-non-private-member-variables-in-classes)
};

class A {
public:
    static ani_static_method g_method;  // NOLINT(readability-identifier-naming)

    static void NativeFoo(ani_env *env, ani_object /*unused*/)
    {
        ani_class cls {};
        ASSERT_EQ(env->FindClass("verify_class_call_static_method_boolean_test.Parent", &cls), ANI_OK);

        ASSERT_EQ(env->Class_FindStaticMethod(cls, "staticVoidParamMethod", ":z", &g_method), ANI_OK);
    }

    template <typename TestType>
    static void NativeBaz(ani_env *env, ani_object /*unused*/)
    {
        ani_class cls {};
        ASSERT_EQ(env->FindClass("verify_class_call_static_method_boolean_test.Parent", &cls), ANI_OK);

        ani_boolean result;
        if constexpr (std::is_same_v<TestType, ClassCallStaticMethodBooleanATest>) {
            ani_value args[2U];  // NOLINT(modernize-avoid-c-arrays)
            ASSERT_EQ(env->c_api->Class_CallStaticMethod_Boolean_A(env, cls, g_method, &result, args), ANI_OK);
        } else {
            ASSERT_EQ(env->c_api->Class_CallStaticMethod_Boolean(env, cls, g_method, &result), ANI_OK);
        }
    }

    template <typename TestType>
    static void NativeBar(ani_env *env, ani_object /*unused*/, ani_ref c, ani_long m)
    {
        auto cls = static_cast<ani_class>(c);
        auto method = reinterpret_cast<ani_static_method>(m);

        ani_boolean result;
        if constexpr (std::is_same_v<TestType, ClassCallStaticMethodBooleanATest>) {
            ani_value args[2U];  // NOLINT(modernize-avoid-c-arrays)
            ASSERT_EQ(env->c_api->Class_CallStaticMethod_Boolean_A(env, cls, method, &result, args), ANI_OK);
        } else {
            ASSERT_EQ(env->c_api->Class_CallStaticMethod_Boolean(env, cls, method, &result), ANI_OK);
        }
    }
};

ani_static_method A::g_method = nullptr;

TEST_F(ClassCallStaticMethodBooleanTest, wrong_env)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Parent", &cls), ANI_OK);
    ani_static_method method {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "staticBooleanMethod", "zcbsilfdC{std.core.String}:z", &method),
              ANI_OK);

    ani_boolean result;
    ASSERT_EQ(
        env_->c_api->Class_CallStaticMethod_Boolean(nullptr, cls, method, &result, z_, c_, b_, s_, i_, l_, f_, d_, r_),
        ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"class", "ani_class"},
        {"static_method", "ani_static_method"},
        {"result", "ani_boolean *"},
        {"...", "       "},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Class_CallStaticMethod_Boolean", testLines);
}

TEST_F(ClassCallStaticMethodBooleanTest, wrong_cls_0)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Parent", &cls), ANI_OK);
    ani_static_method method {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "staticBooleanMethod", "zcbsilfdC{std.core.String}:z", &method),
              ANI_OK);

    ani_boolean result;
    ASSERT_EQ(
        env_->c_api->Class_CallStaticMethod_Boolean(env_, nullptr, method, &result, z_, c_, b_, s_, i_, l_, f_, d_, r_),
        ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class", "wrong reference"},
        {"static_method", "ani_static_method", "wrong class for method"},
        {"result", "ani_boolean *"},
        {"...", "       "},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Class_CallStaticMethod_Boolean", testLines);
}

TEST_F(ClassCallStaticMethodBooleanTest, cls_1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Parent", &cls), ANI_OK);
    ani_static_method method {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "staticBooleanMethod", "zcbsilfdC{std.core.String}:z", &method),
              ANI_OK);

    ASSERT_EQ(env_->GetNull(reinterpret_cast<ani_ref *>(&cls)), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(
        env_->c_api->Class_CallStaticMethod_Boolean(env_, cls, method, &result, z_, c_, b_, s_, i_, l_, f_, d_, r_),
        ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class", "wrong reference type: null"},
        {"static_method", "ani_static_method", "wrong class for method"},
        {"result", "ani_boolean *"},
        {"...", "       "},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Class_CallStaticMethod_Boolean", testLines);
}

TEST_F(ClassCallStaticMethodBooleanTest, wrong_cls_2)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Parent", &cls), ANI_OK);
    ani_static_method method {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "staticBooleanMethod", "zcbsilfdC{std.core.String}:z", &method),
              ANI_OK);
    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Animal", &cls), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(
        env_->c_api->Class_CallStaticMethod_Boolean(env_, cls, method, &result, z_, c_, b_, s_, i_, l_, f_, d_, r_),
        ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"static_method", "ani_static_method", "wrong class for method"},
        {"result", "ani_boolean *"},
        {"...", "       "},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Class_CallStaticMethod_Boolean", testLines);
}

TEST_F(ClassCallStaticMethodBooleanTest, wrong_method_0)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Parent", &cls), ANI_OK);
    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "booleanMethod", "zcbsilfdC{std.core.String}:z", &method), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Boolean(env_, cls, reinterpret_cast<ani_static_method>(method),
                                                          &result, z_, c_, b_, s_, i_, l_, f_, d_, r_),
              ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"static_method", "ani_static_method", "wrong type: ani_method, expected: ani_static_method"},
        {"result", "ani_boolean *"},
        {"...", "       "},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Class_CallStaticMethod_Boolean", testLines);
}

TEST_F(ClassCallStaticMethodBooleanTest, wrong_method_1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Parent", &cls), ANI_OK);

    const auto fakeMethod =
        reinterpret_cast<ani_static_method>(static_cast<long>(0x0ff0f0f));  // NOLINT(google-runtime-int)

    ani_boolean result;
    ASSERT_EQ(
        env_->c_api->Class_CallStaticMethod_Boolean(env_, cls, fakeMethod, &result, z_, c_, b_, s_, i_, l_, f_, d_, r_),
        ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"static_method", "ani_static_method", "wrong static method"},
        {"result", "ani_boolean *"},
        {"...", "       ", "wrong method"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Class_CallStaticMethod_Boolean", testLines);
}

TEST_F(ClassCallStaticMethodBooleanTest, wrong_method_2)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Parent", &cls), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(
        env_->c_api->Class_CallStaticMethod_Boolean(env_, cls, nullptr, &result, z_, c_, b_, s_, i_, l_, f_, d_, r_),
        ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"static_method", "ani_static_method", "wrong static method"},
        {"result", "ani_boolean *"},
        {"...", "       ", "wrong method"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Class_CallStaticMethod_Boolean", testLines);
}

TEST_F(ClassCallStaticMethodBooleanTest, wrong_result)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Parent", &cls), ANI_OK);
    ani_static_method method {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "staticBooleanMethod", "zcbsilfdC{std.core.String}:z", &method),
              ANI_OK);

    ASSERT_EQ(
        env_->c_api->Class_CallStaticMethod_Boolean(env_, cls, method, nullptr, z_, c_, b_, s_, i_, l_, f_, d_, r_),
        ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"static_method", "ani_static_method"},
        {"result", "ani_boolean *", "wrong pointer for storing 'ani_boolean'"},
        {"...", "       "},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Class_CallStaticMethod_Boolean", testLines);
}

TEST_F(ClassCallStaticMethodBooleanTest, wrong_boolean_arg)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Parent", &cls), ANI_OK);
    ani_static_method method {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "staticBooleanMethod", "zcbsilfdC{std.core.String}:z", &method),
              ANI_OK);

    ani_boolean result;
    ASSERT_EQ(
        env_->c_api->Class_CallStaticMethod_Boolean(env_, cls, method, &result, 0xb1ba, c_, b_, s_, i_, l_, f_, d_, r_),
        ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"static_method", "ani_static_method"},
        {"result", "ani_boolean *"},
        {"...", "       ", "wrong method arguments"},
        {"[0]", "ani_boolean", "wrong value"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Class_CallStaticMethod_Boolean", testLines);
}

TEST_F(ClassCallStaticMethodBooleanTest, wrong_char_arg)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Parent", &cls), ANI_OK);
    ani_static_method method {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "staticBooleanMethod", "zcbsilfdC{std.core.String}:z", &method),
              ANI_OK);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Boolean(env_, cls, method, &result, z_, 0xb1bab0ba, b_, s_, i_, l_,
                                                          f_, d_, r_),
              ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"static_method", "ani_static_method"},
        {"result", "ani_boolean *"},
        {"...", "       ", "wrong method arguments"},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char", "wrong value"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Class_CallStaticMethod_Boolean", testLines);
}

TEST_F(ClassCallStaticMethodBooleanTest, wrong_byte_arg)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Parent", &cls), ANI_OK);
    ani_static_method method {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "staticBooleanMethod", "zcbsilfdC{std.core.String}:z", &method),
              ANI_OK);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Boolean(env_, cls, method, &result, z_, c_, 0xb1bab0ba, s_, i_, l_,
                                                          f_, d_, r_),
              ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"static_method", "ani_static_method"},
        {"result", "ani_boolean *"},
        {"...", "       ", "wrong method arguments"},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte", "wrong value"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Class_CallStaticMethod_Boolean", testLines);
}

TEST_F(ClassCallStaticMethodBooleanTest, wrong_short_arg)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Parent", &cls), ANI_OK);
    ani_static_method method {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "staticBooleanMethod", "zcbsilfdC{std.core.String}:z", &method),
              ANI_OK);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Boolean(env_, cls, method, &result, z_, c_, b_, 0xb1bab0ba, i_, l_,
                                                          f_, d_, r_),
              ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"static_method", "ani_static_method"},
        {"result", "ani_boolean *"},
        {"...", "       ", "wrong method arguments"},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short", "wrong value"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Class_CallStaticMethod_Boolean", testLines);
}

TEST_F(ClassCallStaticMethodBooleanTest, wrong_ref_arg)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Parent", &cls), ANI_OK);
    ani_static_method method {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "staticBooleanMethod", "zcbsilfdC{std.core.String}:z", &method),
              ANI_OK);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Boolean(env_, cls, method, &result, z_, c_, b_, s_, i_, l_, f_, d_,
                                                          nullptr),
              ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"static_method", "ani_static_method"},
        {"result", "ani_boolean *"},
        {"...", "       ", "wrong method arguments"},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref", "wrong value"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Class_CallStaticMethod_Boolean", testLines);
}

TEST_F(ClassCallStaticMethodBooleanTest, wrong_all_args)
{
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Boolean(nullptr, nullptr, nullptr, nullptr, nullptr), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"class", "ani_class", "wrong reference"},
        {"static_method", "ani_static_method", "wrong static method"},
        {"result", "ani_boolean *", "wrong pointer for storing 'ani_boolean'"},
        {"...", "       ", "wrong method"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Class_CallStaticMethod_Boolean", testLines);
}

TEST_F(ClassCallStaticMethodBooleanTest, wrong_return_type)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Parent", &cls), ANI_OK);
    ani_static_method method {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "staticIntMethod", "zcbsilfdC{std.core.String}:i", &method), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(
        env_->c_api->Class_CallStaticMethod_Boolean(env_, cls, method, &result, z_, c_, b_, s_, i_, l_, f_, d_, r_),
        ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"static_method", "ani_static_method", "wrong return type"},
        {"result", "ani_boolean *"},
        {"...", "       "},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Class_CallStaticMethod_Boolean", testLines);
}

TEST_F(ClassCallStaticMethodBooleanTest, class_from_local_scope)
{
    const int nr3 = 3;
    ASSERT_EQ(env_->CreateLocalScope(nr3), ANI_OK);
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Child", &cls), ANI_OK);
    ASSERT_EQ(env_->DestroyLocalScope(), ANI_OK);

    ani_static_method method {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "staticBooleanMethod", "zcbsilfdC{std.core.String}:z", &method),
              ANI_OK);
    ani_boolean result;
    ASSERT_EQ(
        env_->c_api->Class_CallStaticMethod_Boolean(env_, cls, method, &result, z_, c_, b_, s_, i_, l_, f_, d_, r_),
        ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class", "wrong reference"},
        {"static_method", "ani_static_method", "wrong class for method"},
        {"result", "ani_boolean *"},
        {"...", "       "},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Class_CallStaticMethod_Boolean", testLines);
}

TEST_F(ClassCallStaticMethodBooleanTest, cross_thread_method_call_from_native_method)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Parent", &cls), ANI_OK);
    std::array methods = {
        ani_native_function {"foo", ":", reinterpret_cast<void *>(A::NativeFoo)},
        ani_native_function {"baz", ":", reinterpret_cast<void *>(A::NativeBaz<ClassCallStaticMethodBooleanTest>)}};
    ASSERT_EQ(env_->Class_BindNativeMethods(cls, methods.data(), methods.size()), ANI_OK);

    std::thread([this]() {
        ani_env *env {};
        ASSERT_EQ(vm_->AttachCurrentThread(nullptr, ANI_VERSION_1, &env), ANI_OK);

        ani_class tcls {};
        ASSERT_EQ(env->FindClass("verify_class_call_static_method_boolean_test.Parent", &tcls), ANI_OK);
        ani_method ctor {};
        ASSERT_EQ(env->Class_FindMethod(tcls, "<ctor>", ":", &ctor), ANI_OK);
        ani_object object {};
        ASSERT_EQ(env->Object_New(tcls, ctor, &object), ANI_OK);

        // In the native "foo" method, the "staticVoidParamMethod" method is found
        ASSERT_EQ(env->Object_CallMethodByName_Void(object, "foo", ":"), ANI_OK);

        ASSERT_EQ(vm_->DetachCurrentThread(), ANI_OK);
    }).join();

    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", ":", &ctor), ANI_OK);
    ani_object object {};
    ASSERT_EQ(env_->Object_New(cls, ctor, &object), ANI_OK);
    // In the native "baz" method, the "staticVoidParamMethod" method is called
    ASSERT_EQ(env_->Object_CallMethodByName_Void(object, "baz", ":"), ANI_OK);
}

TEST_F(ClassCallStaticMethodBooleanTest, class_from_escape_local_scope)
{
    const int nr3 = 3;
    ani_class cls {};
    ASSERT_EQ(env_->CreateEscapeLocalScope(nr3), ANI_OK);
    ani_class lcls {};
    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Parent", &lcls), ANI_OK);
    ASSERT_EQ(env_->DestroyEscapeLocalScope(lcls, reinterpret_cast<ani_ref *>(&cls)), ANI_OK);

    ani_static_method method {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "staticBooleanMethod", "zcbsilfdC{std.core.String}:z", &method),
              ANI_OK);

    ani_boolean result;
    ASSERT_EQ(
        env_->c_api->Class_CallStaticMethod_Boolean(env_, cls, method, &result, z_, c_, b_, s_, i_, l_, f_, d_, r_),
        ANI_OK);
}

TEST_F(ClassCallStaticMethodBooleanTest, call_method_on_global_ref)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Parent", &cls), ANI_OK);
    ani_static_method method {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "staticBooleanMethod", "zcbsilfdC{std.core.String}:z", &method),
              ANI_OK);

    ani_class gcls {};
    ASSERT_EQ(env_->GlobalReference_Create(cls, reinterpret_cast<ani_ref *>(&gcls)), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(
        env_->c_api->Class_CallStaticMethod_Boolean(env_, gcls, method, &result, z_, c_, b_, s_, i_, l_, f_, d_, r_),
        ANI_OK);
}

TEST_F(ClassCallStaticMethodBooleanTest, cross_thread_method_call)
{
    ani_static_method method;

    std::thread([this, &method]() {
        ani_env *env {};
        ASSERT_EQ(vm_->AttachCurrentThread(nullptr, ANI_VERSION_1, &env), ANI_OK);

        ani_class cls {};
        ASSERT_EQ(env->FindClass("verify_class_call_static_method_boolean_test.Parent", &cls), ANI_OK);
        ASSERT_EQ(env->Class_FindStaticMethod(cls, "staticBooleanMethod", "zcbsilfdC{std.core.String}:z", &method),
                  ANI_OK);

        ASSERT_EQ(vm_->DetachCurrentThread(), ANI_OK);
    }).join();

    ani_class cls {};
    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Parent", &cls), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(
        env_->c_api->Class_CallStaticMethod_Boolean(env_, cls, method, &result, z_, c_, b_, s_, i_, l_, f_, d_, r_),
        ANI_OK);
}

// Issue 28700
TEST_F(ClassCallStaticMethodBooleanTest, DISABLED_method_call_from_native_method_0)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Parent", &cls), ANI_OK);

    std::array methods = {ani_native_function {
        "bar", "C{std.core.Object}l:", reinterpret_cast<void *>(A::NativeBar<ClassCallStaticMethodBooleanTest>)}};
    ASSERT_EQ(env_->Class_BindNativeMethods(cls, methods.data(), methods.size()), ANI_OK);

    ani_static_method method;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "staticVoidParamMethod", ":z", &method), ANI_OK);

    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", ":", &ctor), ANI_OK);
    ani_object object {};
    ASSERT_EQ(env_->Object_New(cls, ctor, &object), ANI_OK);

    // In the native "bar" method, the "staticVoidParamMethod" method is called
    ASSERT_EQ(env_->c_api->Object_CallMethodByName_Void(env_, object, "bar", "C{std.core.Object}l:", cls, method),
              ANI_OK);
}

TEST_F(ClassCallStaticMethodBooleanTest, method_call_from_native_method_1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Parent", &cls), ANI_OK);
    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", ":", &ctor), ANI_OK);
    ani_object object {};
    ASSERT_EQ(env_->Object_New(cls, ctor, &object), ANI_OK);

    std::array methods = {
        ani_native_function {"foo", ":", reinterpret_cast<void *>(A::NativeFoo)},
        ani_native_function {"baz", ":", reinterpret_cast<void *>(A::NativeBaz<ClassCallStaticMethodBooleanTest>)}};
    ASSERT_EQ(env_->Class_BindNativeMethods(cls, methods.data(), methods.size()), ANI_OK);

    // In the native "foo" method, the "staticVoidParamMethod" method is found
    ASSERT_EQ(env_->Object_CallMethodByName_Void(object, "foo", ":"), ANI_OK);
    // In the native "baz" method, the "staticVoidParamMethod" method is called
    ASSERT_EQ(env_->Object_CallMethodByName_Void(object, "baz", ":"), ANI_OK);
}

TEST_F(ClassCallStaticMethodBooleanTest, method_call_in_local_scope)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Parent", &cls), ANI_OK);
    ani_static_method method;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "staticBooleanMethod", "zcbsilfdC{std.core.String}:z", &method),
              ANI_OK);

    const int nr3 = 3;
    ASSERT_EQ(env_->CreateLocalScope(nr3), ANI_OK);
    ani_boolean result;
    ASSERT_EQ(
        env_->c_api->Class_CallStaticMethod_Boolean(env_, cls, method, &result, z_, c_, b_, s_, i_, l_, f_, d_, r_),
        ANI_OK);
    ASSERT_EQ(env_->DestroyLocalScope(), ANI_OK);
}

TEST_F(ClassCallStaticMethodBooleanTest, call_parent_method)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Parent", &cls), ANI_OK);
    ani_static_method method;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "staticBooleanMethod", "zcbsilfdC{std.core.String}:z", &method),
              ANI_OK);

    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Child", &cls), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(
        env_->c_api->Class_CallStaticMethod_Boolean(env_, cls, method, &result, z_, c_, b_, s_, i_, l_, f_, d_, r_),
        ANI_OK);
}

TEST_F(ClassCallStaticMethodBooleanTest, success)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Parent", &cls), ANI_OK);
    ani_static_method method;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "staticBooleanMethod", "zcbsilfdC{std.core.String}:z", &method),
              ANI_OK);

    ani_boolean result;
    ASSERT_EQ(
        env_->c_api->Class_CallStaticMethod_Boolean(env_, cls, method, &result, z_, c_, b_, s_, i_, l_, f_, d_, r_),
        ANI_OK);
}

TEST_F(ClassCallStaticMethodBooleanATest, wrong_env)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Parent", &cls), ANI_OK);
    ani_static_method method;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "staticBooleanMethod", "zcbsilfdC{std.core.String}:z", &method),
              ANI_OK);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Boolean_A(nullptr, cls, method, &result, args_.data()), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"class", "ani_class"},
        {"static_method", "ani_static_method"},
        {"result", "ani_boolean *"},
        {"args", "ani_value *"},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Class_CallStaticMethod_Boolean_A", testLines);
}

TEST_F(ClassCallStaticMethodBooleanATest, wrong_cls_0)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Parent", &cls), ANI_OK);
    ani_static_method method;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "staticBooleanMethod", "zcbsilfdC{std.core.String}:z", &method),
              ANI_OK);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Boolean_A(env_, nullptr, method, &result, args_.data()), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class", "wrong reference"},
        {"static_method", "ani_static_method", "wrong class for method"},
        {"result", "ani_boolean *"},
        {"args", "ani_value *"},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Class_CallStaticMethod_Boolean_A", testLines);
}

TEST_F(ClassCallStaticMethodBooleanATest, cls_1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Parent", &cls), ANI_OK);
    ani_static_method method;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "staticBooleanMethod", "zcbsilfdC{std.core.String}:z", &method),
              ANI_OK);

    ASSERT_EQ(env_->GetNull(reinterpret_cast<ani_ref *>(&cls)), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Boolean_A(env_, cls, method, &result, args_.data()), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class", "wrong reference type: null"},
        {"static_method", "ani_static_method", "wrong class for method"},
        {"result", "ani_boolean *"},
        {"args", "ani_value *"},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Class_CallStaticMethod_Boolean_A", testLines);
}

TEST_F(ClassCallStaticMethodBooleanATest, wrong_cls_2)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Parent", &cls), ANI_OK);
    ani_static_method method;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "staticBooleanMethod", "zcbsilfdC{std.core.String}:z", &method),
              ANI_OK);
    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Animal", &cls), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Boolean_A(env_, cls, method, &result, args_.data()), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"static_method", "ani_static_method", "wrong class for method"},
        {"result", "ani_boolean *"},
        {"args", "ani_value *"},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Class_CallStaticMethod_Boolean_A", testLines);
}

TEST_F(ClassCallStaticMethodBooleanATest, wrong_method_0)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Parent", &cls), ANI_OK);
    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "booleanMethod", "zcbsilfdC{std.core.String}:z", &method), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Boolean_A(env_, cls, reinterpret_cast<ani_static_method>(method),
                                                            &result, args_.data()),
              ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"static_method", "ani_static_method", "wrong type: ani_method, expected: ani_static_method"},
        {"result", "ani_boolean *"},
        {"args", "ani_value *"},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Class_CallStaticMethod_Boolean_A", testLines);
}

TEST_F(ClassCallStaticMethodBooleanATest, wrong_method_1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Parent", &cls), ANI_OK);
    const auto fakeMethod =
        reinterpret_cast<ani_static_method>(static_cast<long>(0x0ff0f0f));  // NOLINT(google-runtime-int)

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Boolean_A(env_, cls, fakeMethod, &result, args_.data()), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"static_method", "ani_static_method", "wrong static method"},
        {"result", "ani_boolean *"},
        {"args", "ani_value *", "wrong method"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Class_CallStaticMethod_Boolean_A", testLines);
}

TEST_F(ClassCallStaticMethodBooleanATest, wrong_method_2)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Parent", &cls), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Boolean_A(env_, cls, nullptr, &result, args_.data()), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"static_method", "ani_static_method", "wrong static method"},
        {"result", "ani_boolean *"},
        {"args", "ani_value *", "wrong method"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Class_CallStaticMethod_Boolean_A", testLines);
}

TEST_F(ClassCallStaticMethodBooleanATest, wrong_result)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Parent", &cls), ANI_OK);
    ani_static_method method;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "staticBooleanMethod", "zcbsilfdC{std.core.String}:z", &method),
              ANI_OK);

    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Boolean_A(env_, cls, method, nullptr, args_.data()), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"static_method", "ani_static_method"},
        {"result", "ani_boolean *", "wrong pointer for storing 'ani_boolean'"},
        {"args", "ani_value *"},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Class_CallStaticMethod_Boolean_A", testLines);
}

TEST_F(ClassCallStaticMethodBooleanATest, wrong_args)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Parent", &cls), ANI_OK);
    ani_static_method method;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "staticBooleanMethod", "zcbsilfdC{std.core.String}:z", &method),
              ANI_OK);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Boolean_A(env_, cls, method, &result, nullptr), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"static_method", "ani_static_method"},
        {"result", "ani_boolean *"},
        {"args", "ani_value *", "wrong arguments value"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Class_CallStaticMethod_Boolean_A", testLines);
}

TEST_F(ClassCallStaticMethodBooleanATest, wrong_all_args)
{
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Boolean_A(nullptr, nullptr, nullptr, nullptr, nullptr), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"class", "ani_class", "wrong reference"},
        {"static_method", "ani_static_method", "wrong static method"},
        {"result", "ani_boolean *", "wrong pointer for storing 'ani_boolean'"},
        {"args", "ani_value *", "wrong arguments value"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Class_CallStaticMethod_Boolean_A", testLines);
}

TEST_F(ClassCallStaticMethodBooleanATest, wrong_boolean_arg)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Parent", &cls), ANI_OK);
    ani_static_method method;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "staticBooleanMethod", "zcbsilfdC{std.core.String}:z", &method),
              ANI_OK);

    const int fakeBoolean = 0xb1ba;
    args_[0].i = fakeBoolean;
    ani_boolean result;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Boolean_A(env_, cls, method, &result, args_.data()), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"static_method", "ani_static_method"},
        {"result", "ani_boolean *"},
        {"args", "ani_value *", "wrong method arguments"},
        {"[0]", "ani_boolean", "wrong value"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Class_CallStaticMethod_Boolean_A", testLines);
}

TEST_F(ClassCallStaticMethodBooleanATest, wrong_return_type)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Parent", &cls), ANI_OK);
    ani_static_method method;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "staticIntMethod", "zcbsilfdC{std.core.String}:i", &method), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Boolean_A(env_, cls, method, &result, args_.data()), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class"},
        {"static_method", "ani_static_method", "wrong return type"},
        {"result", "ani_boolean *"},
        {"args", "ani_value *"},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Class_CallStaticMethod_Boolean_A", testLines);
}

TEST_F(ClassCallStaticMethodBooleanATest, class_from_local_scope)
{
    ani_class cls {};
    const int nr3 = 3;
    ASSERT_EQ(env_->CreateLocalScope(nr3), ANI_OK);
    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Parent", &cls), ANI_OK);
    ASSERT_EQ(env_->DestroyLocalScope(), ANI_OK);

    ani_static_method method;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "staticBooleanMethod", "zcbsilfdC{std.core.String}:z", &method),
              ANI_OK);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Boolean_A(env_, cls, method, &result, args_.data()), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"class", "ani_class", "wrong reference"},
        {"static_method", "ani_static_method", "wrong class for method"},
        {"result", "ani_boolean *"},
        {"args", "ani_value *"},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Class_CallStaticMethod_Boolean_A", testLines);
}

TEST_F(ClassCallStaticMethodBooleanATest, cross_thread_method_call_from_native_method)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Parent", &cls), ANI_OK);
    std::array methods = {
        ani_native_function {"foo", ":", reinterpret_cast<void *>(A::NativeFoo)},
        ani_native_function {"baz", ":", reinterpret_cast<void *>(A::NativeBaz<ClassCallStaticMethodBooleanATest>)}};
    ASSERT_EQ(env_->Class_BindNativeMethods(cls, methods.data(), methods.size()), ANI_OK);

    std::thread([this]() {
        ani_env *env {};
        ASSERT_EQ(vm_->AttachCurrentThread(nullptr, ANI_VERSION_1, &env), ANI_OK);

        ani_class tcls {};
        ASSERT_EQ(env->FindClass("verify_class_call_static_method_boolean_test.Parent", &tcls), ANI_OK);
        ani_method ctor {};
        ASSERT_EQ(env->Class_FindMethod(tcls, "<ctor>", ":", &ctor), ANI_OK);
        ani_object object {};
        ASSERT_EQ(env->Object_New(tcls, ctor, &object), ANI_OK);

        // In the native "foo" method, the "staticVoidParamMethod" method is found
        ASSERT_EQ(env->Object_CallMethodByName_Void(object, "foo", ":"), ANI_OK);

        ASSERT_EQ(vm_->DetachCurrentThread(), ANI_OK);
    }).join();

    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", ":", &ctor), ANI_OK);
    ani_object object {};
    ASSERT_EQ(env_->Object_New(cls, ctor, &object), ANI_OK);
    // In the native "baz" method, the "staticVoidParamMethod" method is called
    ASSERT_EQ(env_->Object_CallMethodByName_Void(object, "baz", ":"), ANI_OK);
}

TEST_F(ClassCallStaticMethodBooleanATest, class_from_escape_local_scope)
{
    ani_class cls {};
    const int nr3 = 3;
    ASSERT_EQ(env_->CreateEscapeLocalScope(nr3), ANI_OK);
    ani_class lcls {};
    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Parent", &lcls), ANI_OK);
    ASSERT_EQ(env_->DestroyEscapeLocalScope(lcls, reinterpret_cast<ani_ref *>(&cls)), ANI_OK);

    ani_static_method method;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "staticBooleanMethod", "zcbsilfdC{std.core.String}:z", &method),
              ANI_OK);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Boolean_A(env_, cls, method, &result, args_.data()), ANI_OK);
}

TEST_F(ClassCallStaticMethodBooleanATest, call_method_on_global_ref)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Parent", &cls), ANI_OK);
    ani_static_method method;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "staticBooleanMethod", "zcbsilfdC{std.core.String}:z", &method),
              ANI_OK);

    ani_class gcls {};
    ASSERT_EQ(env_->GlobalReference_Create(cls, reinterpret_cast<ani_ref *>(&gcls)), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Boolean_A(env_, gcls, method, &result, args_.data()), ANI_OK);
}

TEST_F(ClassCallStaticMethodBooleanATest, cross_thread_method_call)
{
    ani_static_method method;

    std::thread([this, &method]() {
        ani_env *env {};
        ASSERT_EQ(vm_->AttachCurrentThread(nullptr, ANI_VERSION_1, &env), ANI_OK);

        ani_class cls {};
        ASSERT_EQ(env->FindClass("verify_class_call_static_method_boolean_test.Parent", &cls), ANI_OK);
        ASSERT_EQ(env->Class_FindStaticMethod(cls, "staticBooleanMethod", "zcbsilfdC{std.core.String}:z", &method),
                  ANI_OK);

        ASSERT_EQ(vm_->DetachCurrentThread(), ANI_OK);
    }).join();

    ani_class cls {};
    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Parent", &cls), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Boolean_A(env_, cls, method, &result, args_.data()), ANI_OK);
}

// Issue 28700
TEST_F(ClassCallStaticMethodBooleanATest, DISABLED_method_call_from_native_method_0)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Parent", &cls), ANI_OK);

    std::array methods = {ani_native_function {
        "bar", "C{std.core.Object}l:", reinterpret_cast<void *>(A::NativeBar<ClassCallStaticMethodBooleanATest>)}};
    ASSERT_EQ(env_->Class_BindNativeMethods(cls, methods.data(), methods.size()), ANI_OK);

    ani_static_method method;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "staticVoidParamMethod", ":z", &method), ANI_OK);

    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", ":", &ctor), ANI_OK);
    ani_object object {};
    ASSERT_EQ(env_->Object_New(cls, ctor, &object), ANI_OK);
    // In the native "bar" method, the "staticVoidParamMethod" method is called
    ASSERT_EQ(env_->c_api->Object_CallMethodByName_Void(env_, object, "bar", "C{std.core.Object}l:", cls, method),
              ANI_OK);
}

TEST_F(ClassCallStaticMethodBooleanATest, method_call_from_native_method_1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Parent", &cls), ANI_OK);

    std::array methods = {
        ani_native_function {"foo", ":", reinterpret_cast<void *>(A::NativeFoo)},
        ani_native_function {"baz", ":", reinterpret_cast<void *>(A::NativeBaz<ClassCallStaticMethodBooleanATest>)}};
    ASSERT_EQ(env_->Class_BindNativeMethods(cls, methods.data(), methods.size()), ANI_OK);

    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", ":", &ctor), ANI_OK);
    ani_object object {};
    ASSERT_EQ(env_->Object_New(cls, ctor, &object), ANI_OK);

    // In the native "foo" method, the "voidParamMethod" method is found
    ASSERT_EQ(env_->Object_CallMethodByName_Void(object, "foo", ":"), ANI_OK);
    // In the native "baz" method, the "voidParamMethod" method is called
    ASSERT_EQ(env_->Object_CallMethodByName_Void(object, "baz", ":"), ANI_OK);
}

TEST_F(ClassCallStaticMethodBooleanATest, method_call_in_local_scope)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Parent", &cls), ANI_OK);
    ani_static_method method;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "staticBooleanMethod", "zcbsilfdC{std.core.String}:z", &method),
              ANI_OK);
    const int nr3 = 3;

    ASSERT_EQ(env_->CreateLocalScope(nr3), ANI_OK);
    ani_boolean result;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Boolean_A(env_, cls, method, &result, args_.data()), ANI_OK);
    ASSERT_EQ(env_->DestroyLocalScope(), ANI_OK);
}

TEST_F(ClassCallStaticMethodBooleanATest, call_parent_method)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Parent", &cls), ANI_OK);
    ani_static_method method;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "staticBooleanMethod", "zcbsilfdC{std.core.String}:z", &method),
              ANI_OK);
    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Child", &cls), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Boolean_A(env_, cls, method, &result, args_.data()), ANI_OK);
}

TEST_F(ClassCallStaticMethodBooleanATest, success)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("verify_class_call_static_method_boolean_test.Parent", &cls), ANI_OK);
    ani_static_method method;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "staticBooleanMethod", "zcbsilfdC{std.core.String}:z", &method),
              ANI_OK);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Boolean_A(env_, cls, method, &result, args_.data()), ANI_OK);
}

}  // namespace ark::ets::ani::verify::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
