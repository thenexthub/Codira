/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ani.h"
#include "ani_gtest.h"

namespace ark::ets::ani::testing {

class ExampleTest : public AniTest {
public:
    // CC-OFFNXT(G.FMT.10-CPP) project code style
    static constexpr const char *MODULE_NAME = "example";
    // CC-OFFNXT(G.FMT.10-CPP) project code style
    static constexpr const char *TEST_CLASS_DESCRIPTOR = "example.Test";
};

// NOLINTBEGIN(readability-magic-numbers)
TEST_F(ExampleTest, EtsFunctionCall)
{
    ani_double p1 = 5.0;
    ani_double p2 = 6.0;

    auto res = CallEtsFunction<ani_double>(MODULE_NAME, "exampleFunction", p1, p2);
    ASSERT_EQ(res, p1 + p2);
}

ani_long NativeFuncExample([[maybe_unused]] ani_env *env, ani_long param1, ani_long param2)
{
    return param1 * param2;
}

TEST_F(ExampleTest, CallNativeFunction)
{
    NativeFunction fn(MODULE_NAME, "nativeExampleFunction", NativeFuncExample);

    ani_long p1 = 12;
    ani_long p2 = -123;

    // Generic call
    auto res = CallEtsNativeMethod<ani_long>(fn, p1, p2);
    ASSERT_EQ(res, p1 * p2);
}

static void QuickFunctionImpl(ani_env *env, ani_string str, ani_long delimiter, ani_string *result)
{
    ASSERT_NE(result, nullptr);
    ani_size sz = 0;
    ASSERT_EQ(env->String_GetUTF8Size(str, &sz), ANI_OK);
    ani_size newSz = sz / static_cast<ani_size>(delimiter);
    if (newSz > 0) {
        std::string buffer(newSz + 1, 0);
        ani_size written = 0;
        ASSERT_EQ(env->String_GetUTF8SubString(str, 0, newSz, buffer.data(), buffer.size(), &written), ANI_OK);
        ASSERT_EQ(newSz, written);
        buffer.resize(newSz);
        ASSERT_EQ(env->String_NewUTF8(buffer.data(), newSz, result), ANI_OK);
    } else {
        ASSERT_EQ(env->String_NewUTF8("", 0, result), ANI_OK);
    }
}

static ani_string QuickFunction(ani_env *env, [[maybe_unused]] ani_class cls, ani_string str, ani_long delimiter)
{
    ani_string result {};
    QuickFunctionImpl(env, str, delimiter, &result);
    return result;
}

TEST_F(ExampleTest, CallNativeQuickFunction)
{
    // CC-OFFNXT(G.FMT.10-CPP) project code style
    static constexpr const char *METHOD_NAME = "quickMethod";
    // CC-OFFNXT(G.FMT.10-CPP) project code style
    static constexpr const char *SIGNATURE = "C{std.core.String}l:C{std.core.String}";
    // CC-OFFNXT(G.FMT.10-CPP) project code style
    static constexpr std::string_view SAMPLE_STRING = "abcd";

    ani_class klass {};
    ASSERT_EQ(env_->FindClass(TEST_CLASS_DESCRIPTOR, &klass), ANI_OK);
    ani_native_function fn {METHOD_NAME, SIGNATURE, reinterpret_cast<void *>(QuickFunction)};
    ASSERT_EQ(env_->Class_BindStaticNativeMethods(klass, &fn, 1), ANI_OK);

    ani_string sample {};
    ASSERT_EQ(env_->String_NewUTF8(SAMPLE_STRING.data(), SAMPLE_STRING.size(), &sample), ANI_OK);
    ani_size delimiter = 2;
    ani_ref resultRef {};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Ref(klass, METHOD_NAME, SIGNATURE, &resultRef, sample, delimiter),
              ANI_OK);
    auto result = static_cast<ani_string>(resultRef);

    ani_size sz = 0;
    ASSERT_EQ(env_->String_GetUTF8Size(result, &sz), ANI_OK);
    auto expectedString = std::string(SAMPLE_STRING.substr(0, SAMPLE_STRING.size() / delimiter));
    ASSERT_EQ(sz, expectedString.size());

    std::string buffer(expectedString.size() + 1, 0);
    ani_size written = 0;
    ASSERT_EQ(env_->String_GetUTF8(result, buffer.data(), buffer.size(), &written), ANI_OK);
    ASSERT_EQ(written, expectedString.size());
    buffer.resize(expectedString.size());
    ASSERT_STREQ(buffer.c_str(), expectedString.data());
}

// CC-OFFNXT(G.FUN.01-CPP) number of arguments is required for the purpose of test
static ani_int DirectFunction(ani_boolean b0, ani_byte i0, ani_char i1, ani_short i2, ani_int i3, ani_long i4,
                              ani_float f0, ani_double f1)
{
    return static_cast<ani_int>(b0) + i0 + i1 + i2 + i3 + i4 + static_cast<ani_int>(f0) + static_cast<ani_int>(f1);
}

template <typename R, typename... Args>
constexpr std::integral_constant<size_t, sizeof...(Args)> GetNumberOfArgs(R (* /* unused */)(Args...))
{
    return std::integral_constant<size_t, sizeof...(Args)> {};
}

TEST_F(ExampleTest, CallNativeDirectFunction)
{
    // CC-OFFNXT(G.FMT.10-CPP) project code style
    static constexpr const char *METHOD_NAME = "directMethod";
    // CC-OFFNXT(G.FMT.10-CPP) project code style
    static constexpr const char *SIGNATURE = "zbcsilfd:i";

    ani_class cls {};
    ASSERT_EQ(env_->FindClass(TEST_CLASS_DESCRIPTOR, &cls), ANI_OK);
    ani_native_function fn {METHOD_NAME, SIGNATURE, reinterpret_cast<void *>(DirectFunction)};
    ASSERT_EQ(env_->Class_BindStaticNativeMethods(cls, &fn, 1), ANI_OK);

    ani_int result = 0;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(cls, METHOD_NAME, SIGNATURE, &result, ANI_TRUE, 1, 1, 1, 1, 1,
                                                     1.0F, 1.0),
              ANI_OK);
    ASSERT_EQ(result, GetNumberOfArgs(DirectFunction));
}
// NOLINTEND(readability-magic-numbers)

}  // namespace ark::ets::ani::testing
