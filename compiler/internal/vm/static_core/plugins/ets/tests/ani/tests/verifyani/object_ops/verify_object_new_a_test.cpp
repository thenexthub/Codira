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

class ObjectNewATest : public VerifyAniTest {
public:
    void SetUp() override
    {
        VerifyAniTest::SetUp();

        const int nr0 = 0;
        const int nr1 = 1;
        const int nr2 = 2;
        const int nr3 = 3;
        const int nr4 = 4;
        const int nr5 = 5;
        const int nr6 = 6;
        const int nr7 = 7;
        const int nr8 = 8;

        const ani_byte bVal = 0x3f;
        const ani_short sVal = 0x1f4f;
        const ani_int iVal = 0x323fff23;
        const ani_long lVal = 0xff3229884222;
        const ani_float fVal = 0.234;
        const ani_double dVal = 1.423;

        ASSERT_EQ(env_->FindClass("verify_object_new_a_test.CheckCtorTypes", &cls_), ANI_OK);
        ASSERT_EQ(env_->Class_FindMethod(cls_, "<ctor>", "zcbsilfdC{std.core.Object}:", &ctor_), ANI_OK);

        ani_value fFVALue {};
        fFVALue.l = bit_cast<ani_long>(std::numeric_limits<uint64_t>::max());
        std::fill(std::begin(args_), std::end(args_), fFVALue);
        args_[nr0].b = ANI_TRUE;
        args_[nr1].c = 'c';
        args_[nr2].b = bVal;
        args_[nr3].s = sVal;
        args_[nr4].i = iVal;
        args_[nr5].l = lVal;
        args_[nr6].f = fVal;
        args_[nr7].d = dVal;
        ASSERT_EQ(env_->GetUndefined(&args_[nr8].r), ANI_OK);
    }

protected:
    static constexpr size_t NR_ARGS = 9;
    ani_class cls_ {};                        // NOLINT(misc-non-private-member-variables-in-classes)
    ani_method ctor_ {};                      // NOLINT(misc-non-private-member-variables-in-classes)
    std::array<ani_value, NR_ARGS> args_ {};  // NOLINT(misc-non-private-member-variables-in-classes)
};

TEST_F(ObjectNewATest, wrong_env)
{
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New_A(nullptr, cls_, ctor_, &obj, args_.data()), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"cls", "ani_class"},
        {"ctor", "ani_method"},
        {"result", "ani_object *"},
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
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New_A", testLines);
}

TEST_F(ObjectNewATest, wrong_cls_0)
{
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New_A(env_, nullptr, ctor_, &obj, args_.data()), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class", "wrong reference"},
        {"ctor", "ani_method", "wrong class for ctor"},
        {"result", "ani_object *"},
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
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New_A", testLines);
}

TEST_F(ObjectNewATest, wrong_cls_1)
{
    ani_class cls;
    ASSERT_EQ(env_->GetNull(reinterpret_cast<ani_ref *>(&cls)), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New_A(env_, cls, ctor_, &obj, args_.data()), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class", "wrong reference type: null"},
        {"ctor", "ani_method", "wrong class for ctor"},
        {"result", "ani_object *"},
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
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New_A", testLines);
}

TEST_F(ObjectNewATest, wrong_ctor_0)
{
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New_A(env_, cls_, nullptr, &obj, args_.data()), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class"},
        {"ctor", "ani_method", "wrong ctor"},
        {"result", "ani_object *"},
        {"args", "ani_value *", "wrong method"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New_A", testLines);
}

TEST_F(ObjectNewATest, wrong_ctor_1)
{
    const auto ctor = reinterpret_cast<ani_method>(static_cast<long>(0x0ff0f0f));  // NOLINT(google-runtime-int)
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New_A(env_, cls_, ctor, &obj, args_.data()), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class"},
        {"ctor", "ani_method", "wrong ctor"},
        {"result", "ani_object *"},
        {"args", "ani_value *", "wrong method"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New_A", testLines);
}

TEST_F(ObjectNewATest, wrong_ctor_2)
{
    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls_, "voidMethod", ":", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New_A(env_, cls_, method, &obj, args_.data()), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class"},
        {"ctor", "ani_method", "method is not ctor"},
        {"result", "ani_object *"},
        {"args", "ani_value *"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New_A", testLines);
}

TEST_F(ObjectNewATest, wrong_ctor_3)
{
    ani_static_method method {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls_, "staticMethod", ":", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New_A(env_, cls_, reinterpret_cast<ani_method>(method), &obj, args_.data()),
              ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class"},
        {"ctor", "ani_method", "wrong type: ani_static_method, expected: ani_method"},
        {"result", "ani_object *"},
        {"args", "ani_value *"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New_A", testLines);
}

TEST_F(ObjectNewATest, wrong_result)
{
    ASSERT_EQ(env_->c_api->Object_New_A(env_, cls_, ctor_, nullptr, args_.data()), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class"},
        {"ctor", "ani_method"},
        {"result", "ani_object *", "wrong pointer for storing 'ani_object'"},
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
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New_A", testLines);
}

TEST_F(ObjectNewATest, wrong_args)
{
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New_A(env_, cls_, ctor_, &obj, nullptr), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class"},
        {"ctor", "ani_method"},
        {"result", "ani_object *"},
        {"args", "ani_value *", "wrong arguments value"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New_A", testLines);
}

TEST_F(ObjectNewATest, wrong_arg_boolean)
{
    args_[0].b = 0x2;
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New_A(env_, cls_, ctor_, &obj, args_.data()), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class"},
        {"ctor", "ani_method"},
        {"result", "ani_object *"},
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
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New_A", testLines);
}

TEST_F(ObjectNewATest, valid_arg_boolean)
{
    ani_object obj {};
    args_[0].b = ANI_FALSE;
    ASSERT_EQ(env_->c_api->Object_New_A(env_, cls_, ctor_, &obj, args_.data()), ANI_OK);
    args_[0].b = ANI_TRUE;
    ASSERT_EQ(env_->c_api->Object_New_A(env_, cls_, ctor_, &obj, args_.data()), ANI_OK);
}

TEST_F(ObjectNewATest, wrong_arg_ref_0)
{
    const int nr8 = 8;
    args_[nr8].r = nullptr;
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New_A(env_, cls_, ctor_, &obj, args_.data()), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class"},
        {"ctor", "ani_method"},
        {"result", "ani_object *"},
        {"args", "ani_value *", "wrong method arguments"},
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
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New_A", testLines);
}

TEST_F(ObjectNewATest, wrong_arg_ref_1)
{
    const int nr8 = 8;
    const ani_long newVal = 0x03f;
    args_[nr8].l = newVal;
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New_A(env_, cls_, ctor_, &obj, args_.data()), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class"},
        {"ctor", "ani_method"},
        {"result", "ani_object *"},
        {"args", "ani_value *", "wrong method arguments"},
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
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New_A", testLines);
}

TEST_F(ObjectNewATest, cls_undefined)
{
    ani_ref undef {};
    ASSERT_EQ(env_->GetUndefined(&undef), ANI_OK);
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New_A(env_, static_cast<ani_class>(undef), ctor_, &obj, args_.data()), ANI_ERROR);

    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class", "wrong reference type: undefined"},
        {"ctor", "ani_method", "wrong class for ctor"},
        {"result", "ani_object *"},
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
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New_A", testLines);
}

TEST_F(ObjectNewATest, wrong_all_args)
{
    ASSERT_EQ(env_->c_api->Object_New_A(nullptr, nullptr, nullptr, nullptr, nullptr), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"cls", "ani_class", "wrong reference"},
        {"ctor", "ani_method", "wrong ctor"},
        {"result", "ani_object *", "wrong pointer for storing 'ani_object'"},
        {"args", "ani_value *", "wrong arguments value"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New_A", testLines);
}

TEST_F(ObjectNewATest, success)
{
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New_A(env_, cls_, ctor_, &obj, args_.data()), ANI_OK);
}

}  // namespace ark::ets::ani::verify::testing
