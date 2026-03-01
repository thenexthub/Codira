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

namespace ark::ets::ani::verify::testing {

class ObjectNewTest : public VerifyAniTest {
public:
    void SetUp() override
    {
        VerifyAniTest::SetUp();

        constexpr ani_byte B_VAL = 0x3f;
        constexpr ani_short S_VAL = 0x1f4f;
        constexpr ani_int I_VAL = 0x323fff23;
        constexpr ani_long L_VAL = 0xff3229884222;
        constexpr ani_float F_VAL = 0.234;
        constexpr ani_double D_VAL = 1.423;
        constexpr std::string_view LONG_SIGNATURE = "zC{std.core.String}sC{std.core.String}lfdC{std.core.String}:";

        ASSERT_EQ(env_->FindClass("verify_object_new_test.CheckCtorTypes", &cls_), ANI_OK);
        ASSERT_EQ(env_->FindClass("verify_object_new_test.A", &clsA_), ANI_OK);
        ASSERT_EQ(env_->FindClass("verify_object_new_test.B", &clsB_), ANI_OK);
        ASSERT_EQ(env_->FindClass("verify_object_new_test.C", &clsC_), ANI_OK);
        ASSERT_EQ(env_->FindClass("verify_object_new_test.CheckUnionType", &checkUnion_), ANI_OK);
        ASSERT_EQ(env_->Class_FindMethod(cls_, "<ctor>", "zcbsilfdC{std.core.String}:", &ctor_), ANI_OK);
        ASSERT_EQ(env_->Class_FindMethod(cls_, "<ctor>", LONG_SIGNATURE.data(), &ctorRefs_), ANI_OK);
        ASSERT_EQ(env_->Class_FindMethod(clsA_, "<ctor>", ":", &ctorA_), ANI_OK);
        ASSERT_EQ(env_->Class_FindMethod(clsB_, "<ctor>", ":", &ctorB_), ANI_OK);
        ASSERT_EQ(env_->Class_FindMethod(clsC_, "<ctor>", "C{verify_object_new_test.A}:", &ctorCA_), ANI_OK);
        ASSERT_EQ(env_->Class_FindMethod(clsC_, "<ctor>", "C{verify_object_new_test.B}:", &ctorCB_), ANI_OK);
        z_ = ANI_TRUE;
        c_ = 'c';
        b_ = B_VAL;
        s_ = S_VAL;
        i_ = I_VAL;
        l_ = L_VAL;
        f_ = F_VAL;
        d_ = D_VAL;
        std::string_view str("tra-ta-ta");
        ASSERT_EQ(env_->String_NewUTF8(str.data(), str.size(), &r_), ANI_OK);
    }

protected:
    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    ani_class cls_ {};
    ani_class clsA_ {};
    ani_class clsB_ {};
    ani_class clsC_ {};
    ani_class checkUnion_ {};
    ani_method ctor_ {};
    ani_method ctorRefs_ {};
    ani_method ctorA_ {};
    ani_method ctorB_ {};
    ani_method ctorCA_ {};
    ani_method ctorCB_ {};

    ani_boolean z_ {};
    ani_char c_ {};
    ani_byte b_ {};
    ani_short s_ {};
    ani_int i_ {};
    ani_long l_ {};
    ani_float f_ {};
    ani_double d_ {};
    ani_string r_ {};
    // NOLINTEND(misc-non-private-member-variables-in-classes)
};

TEST_F(ObjectNewTest, wrong_env)
{
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New(nullptr, cls_, ctor_, &obj, z_, c_, b_, s_, i_, l_, f_, d_, r_), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"cls", "ani_class"},
        {"ctor", "ani_method"},
        {"result", "ani_object *"},
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
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New", testLines);
}

TEST_F(ObjectNewTest, wrong_cls_0)
{
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, nullptr, ctor_, &obj, z_, c_, b_, s_, i_, l_, f_, d_, r_), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class", "wrong reference"},
        {"ctor", "ani_method", "wrong class for ctor"},
        {"result", "ani_object *"},
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
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New", testLines);
}

TEST_F(ObjectNewTest, wrong_cls_1)
{
    ani_class cls;
    ASSERT_EQ(env_->GetNull(reinterpret_cast<ani_ref *>(&cls)), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls, ctor_, &obj, z_, c_, b_, s_, i_, l_, f_, d_, r_), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class", "wrong reference type: null"},
        {"ctor", "ani_method", "wrong class for ctor"},
        {"result", "ani_object *"},
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
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New", testLines);
}

TEST_F(ObjectNewTest, wrong_ctor_null)
{
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, nullptr, &obj, z_, c_, b_, s_, i_, l_, f_, d_, r_), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class"},
        {"ctor", "ani_method", "wrong ctor"},
        {"result", "ani_object *"},
        {"...", "       ", "wrong method"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New", testLines);
}

TEST_F(ObjectNewTest, wrong_ctor_fake)
{
    const auto fakeCtor = reinterpret_cast<ani_method>(static_cast<long>(0x0ff0f0f));  // NOLINT(google-runtime-int)
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, fakeCtor, &obj, z_, c_, b_, s_, i_, l_, f_, d_, r_), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class"},
        {"ctor", "ani_method", "wrong ctor"},
        {"result", "ani_object *"},
        {"...", "       ", "wrong method"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New", testLines);
}

TEST_F(ObjectNewTest, wrong_ctor_is_method)
{
    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls_, "voidMethod", ":", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, method, &obj, z_, c_, b_, s_, i_, l_, f_, d_, r_), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class"},
        {"ctor", "ani_method", "method is not ctor"},
        {"result", "ani_object *"},
        {"...", "       "},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New", testLines);
}

TEST_F(ObjectNewTest, wrong_ctor_3)
{
    ani_static_method method {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls_, "staticMethod", ":", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, reinterpret_cast<ani_method>(method), &obj, z_, c_, b_, s_, i_, l_,
                                      f_, d_, r_),
              ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class"},
        {"ctor", "ani_method", "wrong type: ani_static_method, expected: ani_method"},
        {"result", "ani_object *"},
        {"...", "       "},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New", testLines);
}

// NOTE: Enable when #25617 will be resolved
TEST_F(ObjectNewTest, DISABLED_wrong_ctor_invalidated)
{
    // Invalidate ctor
    ASSERT_EQ(env_->Reference_Delete(cls_), ANI_OK);
    ASSERT_EQ(env_->FindClass("verify_object_new_test.CheckCtorTypes", &cls_), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, c_, b_, s_, i_, l_, f_, d_, r_), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class"},
        {"ctor", "ani_method", "bla-bla-bla"},
        {"result", "ani_object *"},
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
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New", testLines);
}

TEST_F(ObjectNewTest, wrong_result)
{
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, nullptr, z_, c_, b_, s_, i_, l_, f_, d_, r_), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class"},
        {"ctor", "ani_method"},
        {"result", "ani_object *", "wrong pointer for storing 'ani_object'"},
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
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New", testLines);
}

TEST_F(ObjectNewTest, wrong_arg_boolean)
{
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, 0x2, c_, b_, s_, i_, l_, f_, d_, r_), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class"},
        {"ctor", "ani_method"},
        {"result", "ani_object *"},
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
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New", testLines);
}

TEST_F(ObjectNewTest, valid_arg_boolean)
{
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, ANI_FALSE, c_, b_, s_, i_, l_, f_, d_, r_), ANI_OK);
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, ANI_TRUE, c_, b_, s_, i_, l_, f_, d_, r_), ANI_OK);
}

TEST_F(ObjectNewTest, wrong_arg_char)
{
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, 0xffff32, b_, s_, i_, l_, f_, d_, r_), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class"},
        {"ctor", "ani_method"},
        {"result", "ani_object *"},
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
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New", testLines);
}

TEST_F(ObjectNewTest, valid_arg_char)
{
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, 0xffdf, b_, s_, i_, l_, f_, d_, r_), ANI_OK);
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, 0x001f, b_, s_, i_, l_, f_, d_, r_), ANI_OK);
}

TEST_F(ObjectNewTest, wrong_arg_byte_0)
{
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, c_, 0x1ff, s_, i_, l_, f_, d_, r_), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class"},
        {"ctor", "ani_method"},
        {"result", "ani_object *"},
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
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New", testLines);
}

TEST_F(ObjectNewTest, wrong_arg_byte_1)
{
    const ani_int b = -129;
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, c_, b, s_, i_, l_, f_, d_, r_), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class"},
        {"ctor", "ani_method"},
        {"result", "ani_object *"},
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
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New", testLines);
}

TEST_F(ObjectNewTest, wrong_arg_byte_2)
{
    const ani_int b = 128;
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, c_, b, s_, i_, l_, f_, d_, r_), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class"},
        {"ctor", "ani_method"},
        {"result", "ani_object *"},
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
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New", testLines);
}

TEST_F(ObjectNewTest, valid_arg_byte)
{
    constexpr ani_byte MIN_BYTE = std::numeric_limits<ani_byte>::min();
    constexpr ani_byte MAX_BYTE = std::numeric_limits<ani_byte>::max();
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, c_, MIN_BYTE, s_, i_, l_, f_, d_, r_), ANI_OK);
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, c_, 0, s_, i_, l_, f_, d_, r_), ANI_OK);
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, c_, MAX_BYTE, s_, i_, l_, f_, d_, r_), ANI_OK);
}

TEST_F(ObjectNewTest, wrong_arg_short)
{
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, c_, b_, 0x3ff64, i_, l_, f_, d_, r_), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class"},
        {"ctor", "ani_method"},
        {"result", "ani_object *"},
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
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New", testLines);
}

TEST_F(ObjectNewTest, valid_arg_short)
{
    constexpr ani_short MIN_SHORT = std::numeric_limits<ani_short>::min();
    constexpr ani_short MAX_SHORT = std::numeric_limits<ani_short>::max();
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, c_, b_, MIN_SHORT, i_, l_, f_, d_, r_), ANI_OK);
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, c_, b_, 0, i_, l_, f_, d_, r_), ANI_OK);
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, c_, b_, MAX_SHORT, i_, l_, f_, d_, r_), ANI_OK);
}

TEST_F(ObjectNewTest, valid_arg_int)
{
    constexpr ani_int MIN_INT = std::numeric_limits<ani_int>::min();
    constexpr ani_int MAX_INT = std::numeric_limits<ani_int>::max();
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, c_, b_, s_, MIN_INT, l_, f_, d_, r_), ANI_OK);
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, c_, b_, s_, 0, l_, f_, d_, r_), ANI_OK);
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, c_, b_, s_, MAX_INT, l_, f_, d_, r_), ANI_OK);
}

TEST_F(ObjectNewTest, valid_arg_float)
{
    constexpr ani_float MIN_FLOAT = std::numeric_limits<ani_float>::min();
    constexpr ani_float MAX_FLOAT = std::numeric_limits<ani_float>::max();
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, c_, b_, s_, i_, l_, MIN_FLOAT, d_, r_), ANI_OK);
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, c_, b_, s_, i_, l_, -0.0F, d_, r_), ANI_OK);
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, c_, b_, s_, i_, l_, MAX_FLOAT, d_, r_), ANI_OK);
}

TEST_F(ObjectNewTest, valid_arg_double)
{
    constexpr ani_double MIN_DOUBLE = std::numeric_limits<ani_double>::min();
    constexpr ani_double MAX_DOUBLE = std::numeric_limits<ani_double>::max();
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, c_, b_, s_, i_, l_, f_, MIN_DOUBLE, r_), ANI_OK);
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, c_, b_, s_, i_, l_, f_, -0.0, r_), ANI_OK);
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, c_, b_, s_, i_, l_, f_, MAX_DOUBLE, r_), ANI_OK);
}

TEST_F(ObjectNewTest, wrong_arg_ref_null)
{
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, c_, b_, s_, i_, l_, f_, d_, nullptr), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class"},
        {"ctor", "ani_method"},
        {"result", "ani_object *"},
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
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New", testLines);
}

TEST_F(ObjectNewTest, wrong_arg_ref_fake)
{
    const auto fakeRef = reinterpret_cast<ani_ref>(static_cast<long>(0x0ff0f1f));  // NOLINT(google-runtime-int)
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, c_, b_, s_, i_, l_, f_, d_, fakeRef), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class"},
        {"ctor", "ani_method"},
        {"result", "ani_object *"},
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
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New", testLines);
}

TEST_F(ObjectNewTest, wrong_arg_ref_invalid_type)
{
    ani_object argObj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &argObj, z_, c_, b_, s_, i_, l_, f_, d_, r_), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, c_, b_, s_, i_, l_, f_, d_, argObj), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class"},
        {"ctor", "ani_method"},
        {"result", "ani_object *"},
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
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New", testLines);
}

TEST_F(ObjectNewTest, wrong_arg_ref_check_args1)
{
    ani_object objA;
    ani_object objB;
    ASSERT_EQ(env_->c_api->Object_New(env_, clsA_, ctorA_, &objA), ANI_OK);
    ASSERT_EQ(env_->c_api->Object_New(env_, clsB_, ctorB_, &objB), ANI_OK);

    ani_object argObj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, clsC_, ctorCA_, &argObj, objA), ANI_OK);
    ASSERT_EQ(env_->c_api->Object_New(env_, clsC_, ctorCA_, &argObj, objB), ANI_OK);

    ASSERT_EQ(env_->c_api->Object_New(env_, clsC_, ctorCB_, &argObj, objB), ANI_OK);
    ASSERT_EQ(env_->c_api->Object_New(env_, clsC_, ctorCB_, &argObj, objA), ANI_ERROR);

    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class"},
        {"ctor", "ani_method"},
        {"result", "ani_object *"},
        {"...", "       ", "wrong method arguments"},
        {"[0]", "ani_ref", "wrong value"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New", testLines);
}

TEST_F(ObjectNewTest, wrong_arg_ref_check_args2)
{
    ani_object argObj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctorRefs_, &argObj, z_, r_, s_, r_, l_, f_, d_, r_), ANI_OK);

    ani_object noObj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctorRefs_, &argObj, z_, noObj, s_, r_, l_, f_, d_, r_), ANI_ERROR);

    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class"},
        {"ctor", "ani_method"},
        {"result", "ani_object *"},
        {"...", "       ", "wrong method arguments"},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_ref", "wrong value"},
        {"[2]", "ani_short"},
        {"[3]", "ani_ref"},
        {"[4]", "ani_long"},
        {"[5]", "ani_float"},
        {"[6]", "ani_double"},
        {"[7]", "ani_ref"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New", testLines);
}

TEST_F(ObjectNewTest, wrong_arg_ref_check_args3)
{
    ani_object argObj {};
    ani_object noObj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctorRefs_, &argObj, z_, noObj, s_, noObj, l_, f_, d_, noObj),
              ANI_ERROR);

    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class"},
        {"ctor", "ani_method"},
        {"result", "ani_object *"},
        {"...", "       ", "wrong method arguments"},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_ref", "wrong value"},
        {"[2]", "ani_short"},
        {"[3]", "ani_ref", "wrong value"},
        {"[4]", "ani_long"},
        {"[5]", "ani_float"},
        {"[6]", "ani_double"},
        {"[7]", "ani_ref", "wrong value"},
    };
    // clang-format on

    ASSERT_ERROR_ANI_ARGS_MSG("Object_New", testLines);
}

TEST_F(ObjectNewTest, arg_undef)
{
    ani_object obj {};
    ani_ref undef {};
    ASSERT_EQ(env_->GetUndefined(&undef), ANI_OK);
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctorRefs_, &obj, z_, r_, s_, r_, l_, f_, d_, undef), ANI_OK);
}

TEST_F(ObjectNewTest, arg_null)
{
    ani_object obj {};
    ani_ref nullRef {};
    ASSERT_EQ(env_->GetNull(&nullRef), ANI_OK);
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctorRefs_, &obj, z_, r_, s_, r_, l_, f_, d_, nullRef), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class"},
        {"ctor", "ani_method"},
        {"result", "ani_object *"},
        {"...", "       ", "wrong method arguments"},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_ref"},
        {"[2]", "ani_short"},
        {"[3]", "ani_ref"},
        {"[4]", "ani_long"},
        {"[5]", "ani_float"},
        {"[6]", "ani_double"},
        {"[7]", "ani_ref", "wrong value"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New", testLines);
}
TEST_F(ObjectNewTest, cls_undefined)
{
    ani_ref undef {};
    ASSERT_EQ(env_->GetUndefined(&undef), ANI_OK);
    ani_object argObj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, static_cast<ani_class>(undef), ctor_, &argObj, z_, c_, b_, s_, i_, l_, f_,
                                      d_, r_),
              ANI_ERROR);

    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class", "wrong reference type: undefined"},
        {"ctor", "ani_method", "wrong class for ctor"},
        {"result", "ani_object *"},
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
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New", testLines);
}

// NOTE(ypigunova): Add check for union types #31219
TEST_F(ObjectNewTest, DISABLED_arg_union1)
{
    constexpr std::string_view LONG_SIGNATURE =
        "X{C{std.core.String}C{verify_object_new_test.A}C{verify_object_new_test.C}}:";
    ani_method unionMethod {};
    ani_object obj {};
    ASSERT_EQ(env_->Class_FindMethod(checkUnion_, "<ctor>", LONG_SIGNATURE.data(), &unionMethod), ANI_OK);

    ASSERT_EQ(env_->c_api->Object_New(env_, checkUnion_, unionMethod, &obj, r_), ANI_OK);
}

// NOTE(ypigunova): Add check for union types #31219
TEST_F(ObjectNewTest, DISABLED_arg_union2)
{
    constexpr std::string_view LONG_SIGNATURE =
        "X{C{std.core.String}C{verify_object_new_test.A}C{verify_object_new_test.C}}:";
    ani_method unionMethod {};
    ani_object obj {};
    ASSERT_EQ(env_->Class_FindMethod(checkUnion_, "<ctor>", LONG_SIGNATURE.data(), &unionMethod), ANI_OK);

    ani_class boolClass {};
    ani_method ctor {};
    ASSERT_EQ(env_->FindClass("std.core.Boolean", &boolClass), ANI_OK);
    ASSERT_EQ(env_->Class_FindMethod(boolClass, "<ctor>", "z:", &ctor), ANI_OK);
    ani_object arg {};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Object_New(boolClass, ctor, &arg, z_), ANI_OK);

    ASSERT_EQ(env_->c_api->Object_New(env_, checkUnion_, unionMethod, &obj, arg), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class"},
        {"ctor", "ani_method"},
        {"result", "ani_object *"},
        {"...", "       ", "wrong method arguments"},
        {"[0]", "ani_ref", "wrong value"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New", testLines);
}

TEST_F(ObjectNewTest, wrong_all_args)
{
    ASSERT_EQ(env_->c_api->Object_New(nullptr, nullptr, nullptr, nullptr), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"cls", "ani_class", "wrong reference"},
        {"ctor", "ani_method", "wrong ctor"},
        {"result", "ani_object *", "wrong pointer for storing 'ani_object'"},
        {"...", "       ", "wrong method"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New", testLines);
}

TEST_F(ObjectNewTest, success)
{
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, c_, b_, s_, i_, l_, f_, d_, r_), ANI_OK);
}

}  // namespace ark::ets::ani::verify::testing
