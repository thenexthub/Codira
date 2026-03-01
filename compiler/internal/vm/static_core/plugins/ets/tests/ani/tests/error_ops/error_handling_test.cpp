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
#include <regex>

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class ErrorHandlingTest : public AniTest {
public:
    // synchronized with `MAGIC_NUMBER` from ArkTS managed code
    static constexpr ani_int MAGIC_NUMBER = 5;
    static constexpr std::string_view MESSAGE_FROM_THROW_ERROR = "some error message from throwError";
    static constexpr std::string_view NAMESPACE_NAME = "testing";
    static const std::string NAMESPACE_DESCRIPTOR;
    static constexpr int32_t LOOP_COUNT = 3;

    static std::string GetTraceLine(std::string_view functionName)
    {
        std::stringstream ss;
        ss << "at error_handling_test." << NAMESPACE_NAME << '.' << functionName;
        return ss.str();
    }

public:
    ani_function GetThrowErrorFunction()
    {
        ani_namespace ns = nullptr;
        ani_function func = nullptr;
        GetFunctionFromModule(&ns, &func, "throwError", "i:i");
        return func;
    }

    ani_function GetThrowErrorNested()
    {
        ani_namespace ns = nullptr;
        ani_function func = nullptr;
        GetFunctionFromModule(&ns, &func, "throwErrorNested", "i:i");
        return func;
    }

    void GetThrowErrorThroughNative(ani_namespace *ns, ani_function *func)
    {
        GetFunctionFromModule(ns, func, "throwErrorThroughNative", "i:i");
    }

private:
    void GetFunctionFromModule(ani_namespace *ns, ani_function *func, const char *functionName, const char *signature)
    {
        ASSERT_NE(ns, nullptr);
        ASSERT_NE(func, nullptr);

        ASSERT_EQ(env_->FindNamespace(NAMESPACE_DESCRIPTOR.c_str(), ns), ANI_OK);
        ASSERT_EQ(env_->Namespace_FindFunction(*ns, functionName, signature, func), ANI_OK);
    }
};

// CC-OFFNXT(G.FMT.10-CPP) project code style
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
const std::string ErrorHandlingTest::NAMESPACE_DESCRIPTOR = "error_handling_test." + std::string(NAMESPACE_NAME);

TEST_F(ErrorHandlingTest, exist_unhandled_error_test)
{
    auto func = GetThrowErrorFunction();

    ani_int errorResult = 0;
    ani_boolean result = ANI_TRUE;
    ASSERT_EQ(env_->ExistUnhandledError(&result), ANI_OK);
    ASSERT_EQ(result, ANI_FALSE);
    ASSERT_EQ(env_->Function_Call_Int(func, &errorResult, MAGIC_NUMBER), ANI_PENDING_ERROR);
    ASSERT_EQ(env_->ExistUnhandledError(&result), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);
}

TEST_F(ErrorHandlingTest, exist_unhandled_error_invalid_env_test)
{
    ani_boolean result = ANI_TRUE;
    ASSERT_EQ(env_->c_api->ExistUnhandledError(nullptr, &result), ANI_INVALID_ARGS);
}

TEST_F(ErrorHandlingTest, reset_error_test)
{
    auto func = GetThrowErrorFunction();

    ani_int errorResult = 0;
    ani_boolean result = ANI_TRUE;
    ASSERT_EQ(env_->ExistUnhandledError(&result), ANI_OK);
    ASSERT_EQ(result, ANI_FALSE);
    ASSERT_EQ(env_->Function_Call_Int(func, &errorResult, MAGIC_NUMBER), ANI_PENDING_ERROR);
    ASSERT_EQ(env_->ExistUnhandledError(&result), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);
    ASSERT_EQ(env_->ResetError(), ANI_OK);
    ASSERT_EQ(env_->ExistUnhandledError(&result), ANI_OK);
    ASSERT_EQ(result, ANI_FALSE);
}

TEST_F(ErrorHandlingTest, get_unhandled_error_test)
{
    auto func = GetThrowErrorFunction();

    ani_int errorResult = 0;
    ani_boolean result = ANI_TRUE;
    ASSERT_EQ(env_->ExistUnhandledError(&result), ANI_OK);
    ASSERT_EQ(result, ANI_FALSE);
    ASSERT_EQ(env_->Function_Call_Int(func, &errorResult, MAGIC_NUMBER), ANI_PENDING_ERROR);
    ASSERT_EQ(env_->ExistUnhandledError(&result), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);

    ani_error error {};
    ASSERT_EQ(env_->GetUnhandledError(&error), ANI_OK);
    ASSERT_NE(error, nullptr);
}

TEST_F(ErrorHandlingTest, throw_error_test)
{
    ani_int errorResult = 0;
    ani_boolean result = ANI_FALSE;

    auto func = GetThrowErrorFunction();
    ASSERT_EQ(env_->Function_Call_Int(func, &errorResult, MAGIC_NUMBER), ANI_PENDING_ERROR);
    ASSERT_EQ(env_->ExistUnhandledError(&result), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);

    ani_error error {};
    ASSERT_EQ(env_->GetUnhandledError(&error), ANI_OK);

    ASSERT_EQ(env_->ThrowError(error), ANI_OK);
    ASSERT_EQ(env_->ExistUnhandledError(&result), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);
    ASSERT_EQ(env_->ThrowError(nullptr), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->c_api->ThrowError(nullptr, error), ANI_INVALID_ARGS);
}

static size_t CountSubstr(const std::string &str, const std::string &substr)
{
    if (substr.empty()) {
        return 0;
    }
    size_t count = 0;
    size_t pos = str.find(substr);
    while (pos != std::string::npos) {
        ++count;
        pos = str.find(substr, pos + substr.length());
    }
    return count;
}

static void GetErrorDescription(ani_env *env, std::string &output)
{
    std::ostringstream buffer;

    std::streambuf *oldStderr = std::cerr.rdbuf(buffer.rdbuf());
    ani_status status = env->DescribeError();
    std::cerr.rdbuf(oldStderr);

    ASSERT_EQ(status, ANI_OK);
    output = buffer.str();
}

static void CheckErrorDescription(const std::string &errorDescription, const std::string &errorMessage,
                                  const std::vector<std::string> &stackTrace)
{
    std::vector<size_t> stackTraceIndexes;
    stackTraceIndexes.reserve(stackTrace.size() + 1);
    stackTraceIndexes.push_back(errorDescription.find(errorMessage));
    for (const auto &traceLine : stackTrace) {
        stackTraceIndexes.push_back(errorDescription.find(traceLine));
    }

    for (size_t i = 0; i < stackTraceIndexes.size(); ++i) {
        ASSERT_NE(stackTraceIndexes[i], std::string::npos)
            << "not found in error message: " << ((i == 0) ? errorMessage : stackTrace[i - 1]);
    }
    for (size_t i = 0; i < stackTraceIndexes.size() - 1; ++i) {
        ASSERT_LT(stackTraceIndexes[i], stackTraceIndexes[i + 1]) << "wrong order at line " << i;
    }
    std::string stackTracePrefix = "at ";
    ASSERT_EQ(CountSubstr(errorDescription, stackTracePrefix), stackTrace.size());
}

static void TestDescribeError(ani_env *env, ani_function func, ani_int arg, std::string &output)
{
    ani_boolean errorExists = ANI_FALSE;
    ASSERT_EQ(env->ExistUnhandledError(&errorExists), ANI_OK);
    ASSERT_EQ(errorExists, ANI_FALSE);
    ani_int ignored = 0;
    ASSERT_EQ(env->Function_Call_Int(func, &ignored, arg), ANI_PENDING_ERROR);
    ASSERT_EQ(env->ExistUnhandledError(&errorExists), ANI_OK);
    ASSERT_EQ(errorExists, ANI_TRUE);

    GetErrorDescription(env, output);

    // Check that error still exists
    ASSERT_EQ(env->ExistUnhandledError(&errorExists), ANI_OK);
    ASSERT_EQ(errorExists, ANI_TRUE);
}

TEST_F(ErrorHandlingTest, describe_error_test_one_frame)
{
    auto func = GetThrowErrorFunction();

    std::string output;
    TestDescribeError(env_, func, MAGIC_NUMBER, output);

    CheckErrorDescription(output, std::string(MESSAGE_FROM_THROW_ERROR),
                          {"at escompat.Error.<ctor>", GetTraceLine("throwError")});
}

TEST_F(ErrorHandlingTest, describe_error_test_nested)
{
    auto func = GetThrowErrorNested();

    std::string output;
    TestDescribeError(env_, func, MAGIC_NUMBER, output);

    CheckErrorDescription(output, std::string(MESSAGE_FROM_THROW_ERROR),
                          {"at escompat.Error.<ctor>", GetTraceLine("throwError"), GetTraceLine("bar"),
                           GetTraceLine("baz"), GetTraceLine("throwErrorNested")});
}

// NOLINTNEXTLINE(readability-identifier-naming)
static ani_long callThroughNative([[maybe_unused]] ani_env *env, ani_int a)
{
    // NOLINTBEGIN(clang-analyzer-deadcode.DeadStores)
    ani_namespace ns = nullptr;
    [[maybe_unused]] auto status = env->FindNamespace(ErrorHandlingTest::NAMESPACE_DESCRIPTOR.data(), &ns);
    ASSERT(status == ANI_OK);

    std::string_view methodName = "throwToNativeCaller";
    ani_function func = nullptr;
    status = env->Namespace_FindFunction(ns, methodName.data(), "i:i", &func);
    ASSERT(status == ANI_OK);

    ani_int result = 0;
    status = env->Function_Call_Int(func, &result, a);
    if (status == ANI_PENDING_ERROR) {
        return 0;
    }

    ani_boolean errorExists = ANI_FALSE;
    status = env->ExistUnhandledError(&errorExists);
    ASSERT(status == ANI_OK);
    ASSERT(errorExists == ANI_FALSE);
    return a + result;
    // NOLINTEND(clang-analyzer-deadcode.DeadStores)
}

TEST_F(ErrorHandlingTest, describe_error_thrown_through_native)
{
    ani_namespace ns = nullptr;
    ani_function func = nullptr;
    GetThrowErrorThroughNative(&ns, &func);

    ani_native_function fn {"callThroughNative", "i:i", reinterpret_cast<void *>(callThroughNative)};
    ASSERT_EQ(env_->Namespace_BindNativeFunctions(ns, &fn, 1), ANI_OK);

    std::string output;
    TestDescribeError(env_, func, MAGIC_NUMBER, output);

    CheckErrorDescription(output, "Error",
                          {"at escompat.Error.<ctor>", GetTraceLine("throwToNativeCaller"),
                           GetTraceLine("callThroughNative"), GetTraceLine("throwErrorThroughNative")});
}

TEST_F(ErrorHandlingTest, exist_invalid_args_test)
{
    ASSERT_EQ(env_->ExistUnhandledError(nullptr), ANI_INVALID_ARGS);
}

TEST_F(ErrorHandlingTest, exist_multiple_call_test_1)
{
    for (int32_t i = 0; i < LOOP_COUNT; i++) {
        ani_boolean result = ANI_TRUE;
        ASSERT_EQ(env_->ExistUnhandledError(&result), ANI_OK);
        ASSERT_EQ(result, ANI_FALSE);
    }
}

TEST_F(ErrorHandlingTest, exist_multiple_call_test_2)
{
    auto func = GetThrowErrorFunction();

    for (int32_t i = 0; i < LOOP_COUNT; i++) {
        ani_int errorResult = 0;
        ASSERT_EQ(env_->Function_Call_Int(func, &errorResult, MAGIC_NUMBER), ANI_PENDING_ERROR);

        ani_boolean result = ANI_FALSE;
        ASSERT_EQ(env_->ExistUnhandledError(&result), ANI_OK);
        ASSERT_EQ(result, ANI_TRUE);
    }
}

TEST_F(ErrorHandlingTest, exist_multiple_call_test_3)
{
    auto func = GetThrowErrorFunction();

    for (int32_t i = 0; i < LOOP_COUNT; i++) {
        ani_int errorResult = 0;
        ASSERT_EQ(env_->Function_Call_Int(func, &errorResult, MAGIC_NUMBER), ANI_PENDING_ERROR);

        ani_boolean result = ANI_FALSE;
        ASSERT_EQ(env_->ExistUnhandledError(&result), ANI_OK);
        ASSERT_EQ(result, ANI_TRUE);

        ASSERT_EQ(env_->ResetError(), ANI_OK);
        ASSERT_EQ(env_->ExistUnhandledError(&result), ANI_OK);
        ASSERT_EQ(result, ANI_FALSE);
    }
}

TEST_F(ErrorHandlingTest, exist_multiple_call_test_4)
{
    for (int32_t i = 0; i < LOOP_COUNT; i++) {
        ani_boolean result = ANI_TRUE;
        ASSERT_EQ(env_->ExistUnhandledError(&result), ANI_OK);
        ASSERT_EQ(result, ANI_FALSE);
    }
}

TEST_F(ErrorHandlingTest, reset_multiple_call_test_1)
{
    auto func = GetThrowErrorFunction();

    for (int32_t i = 0; i < LOOP_COUNT; i++) {
        ani_int errorResult = 0;
        ASSERT_EQ(env_->Function_Call_Int(func, &errorResult, MAGIC_NUMBER), ANI_PENDING_ERROR);
        ASSERT_EQ(env_->ResetError(), ANI_OK);
    }
}

TEST_F(ErrorHandlingTest, reset_multiple_call_test_2)
{
    for (int32_t i = 0; i < LOOP_COUNT; i++) {
        ASSERT_EQ(env_->ResetError(), ANI_OK);
    }
}

TEST_F(ErrorHandlingTest, reset_multiple_call_test_3)
{
    auto func = GetThrowErrorFunction();

    for (int32_t i = 0; i < LOOP_COUNT; i++) {
        ani_int errorResult = 0;
        ASSERT_EQ(env_->Function_Call_Int(func, &errorResult, MAGIC_NUMBER), ANI_PENDING_ERROR);
        ASSERT_EQ(env_->ResetError(), ANI_OK);

        ASSERT_EQ(env_->Function_Call_Int(func, &errorResult, MAGIC_NUMBER), ANI_PENDING_ERROR);
        ASSERT_EQ(env_->ResetError(), ANI_OK);

        ani_boolean result = ANI_TRUE;
        ASSERT_EQ(env_->ExistUnhandledError(&result), ANI_OK);
        ASSERT_EQ(result, ANI_FALSE);
    }
}

TEST_F(ErrorHandlingTest, reset_multiple_call_test_4)
{
    for (int32_t i = 0; i < LOOP_COUNT; i++) {
        ASSERT_EQ(env_->ResetError(), ANI_OK);
    }
}

TEST_F(ErrorHandlingTest, reset_env_invalid_test)
{
    ASSERT_EQ(env_->c_api->ResetError(nullptr), ANI_INVALID_ARGS);
}

TEST_F(ErrorHandlingTest, describe_multiple_call_test_1)
{
    auto func = GetThrowErrorFunction();

    for (int32_t i = 0; i < LOOP_COUNT; i++) {
        ani_int errorResult = 0;
        ASSERT_EQ(env_->Function_Call_Int(func, &errorResult, MAGIC_NUMBER), ANI_PENDING_ERROR);
        ASSERT_EQ(env_->DescribeError(), ANI_OK);
    }
}

TEST_F(ErrorHandlingTest, describe_multiple_call_test_2)
{
    for (int32_t i = 0; i < LOOP_COUNT; i++) {
        ASSERT_EQ(env_->DescribeError(), ANI_OK);
    }
}

TEST_F(ErrorHandlingTest, describe_multiple_call_test_3)
{
    auto func = GetThrowErrorFunction();

    for (int32_t i = 0; i < LOOP_COUNT; i++) {
        ani_int errorResult = 0;
        ASSERT_EQ(env_->Function_Call_Int(func, &errorResult, MAGIC_NUMBER), ANI_PENDING_ERROR);
        ASSERT_EQ(env_->DescribeError(), ANI_OK);

        ASSERT_EQ(env_->ResetError(), ANI_OK);
        ASSERT_EQ(env_->DescribeError(), ANI_OK);
    }
}

TEST_F(ErrorHandlingTest, describe_multiple_call_test_4)
{
    auto func = GetThrowErrorFunction();

    for (int32_t i = 0; i < LOOP_COUNT; i++) {
        ani_int errorResult = 0;
        ASSERT_EQ(env_->Function_Call_Int(func, &errorResult, MAGIC_NUMBER), ANI_PENDING_ERROR);
        ASSERT_EQ(env_->DescribeError(), ANI_OK);

        ASSERT_EQ(env_->Function_Call_Int(func, &errorResult, MAGIC_NUMBER), ANI_PENDING_ERROR);
        ASSERT_EQ(env_->DescribeError(), ANI_OK);
    }
}

TEST_F(ErrorHandlingTest, describe_multiple_call_test_5)
{
    for (int i = 0; i < LOOP_COUNT; i++) {
        ASSERT_EQ(env_->DescribeError(), ANI_OK);
    }
}

TEST_F(ErrorHandlingTest, describe_env_invalid_test)
{
    ASSERT_EQ(env_->c_api->DescribeError(nullptr), ANI_INVALID_ARGS);
}

TEST_F(ErrorHandlingTest, throw_multiple_call_test)
{
    auto func = GetThrowErrorFunction();

    ani_int errorResult = 0;
    ani_boolean result = ANI_FALSE;
    ani_error error {};
    ASSERT_EQ(env_->Function_Call_Int(func, &errorResult, MAGIC_NUMBER), ANI_PENDING_ERROR);
    ASSERT_EQ(env_->ExistUnhandledError(&result), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);
    ASSERT_EQ(env_->GetUnhandledError(&error), ANI_OK);
    for (int32_t i = 0; i < LOOP_COUNT; i++) {
        ASSERT_EQ(env_->ThrowError(error), ANI_OK);
    }
}

TEST_F(ErrorHandlingTest, manual_create_and_throw_error_test)
{
    ani_class errorClass {};
    ASSERT_EQ(env_->FindClass("escompat.Error", &errorClass), ANI_OK);
    ASSERT_NE(errorClass, nullptr);

    ani_method constructor {};
    ASSERT_EQ(env_->Class_FindMethod(errorClass, "<ctor>", "C{std.core.String}:", &constructor), ANI_OK);
    ASSERT_NE(constructor, nullptr);

    ani_string errorMessage {};
    ASSERT_EQ(env_->String_NewUTF8(MESSAGE_FROM_THROW_ERROR.data(), MESSAGE_FROM_THROW_ERROR.size(), &errorMessage),
              ANI_OK);
    ASSERT_NE(errorMessage, nullptr);

    ani_object errorObject {};
    ASSERT_EQ(env_->Object_New(errorClass, constructor, &errorObject, errorMessage), ANI_OK);
    ASSERT_NE(errorObject, nullptr);

    ani_boolean hasError = ANI_TRUE;
    ASSERT_EQ(env_->ExistUnhandledError(&hasError), ANI_OK);
    ASSERT_EQ(hasError, ANI_FALSE);

    ASSERT_EQ(env_->ThrowError(static_cast<ani_error>(errorObject)), ANI_OK);
    ASSERT_EQ(env_->ExistUnhandledError(&hasError), ANI_OK);
    ASSERT_EQ(hasError, ANI_TRUE);

    ani_error thrownError {};
    ASSERT_EQ(env_->GetUnhandledError(&thrownError), ANI_OK);
    ASSERT_NE(thrownError, nullptr);

    std::string errorDescription {};
    GetErrorDescription(env_, errorDescription);
    CheckErrorDescription(errorDescription, std::string(MESSAGE_FROM_THROW_ERROR), {"at escompat.Error.<ctor>"});

    ASSERT_EQ(env_->ResetError(), ANI_OK);
    ASSERT_EQ(env_->ExistUnhandledError(&hasError), ANI_OK);
    ASSERT_EQ(hasError, ANI_FALSE);
}

TEST_F(ErrorHandlingTest, throw_combined_scenes_test_1)
{
    auto func = GetThrowErrorFunction();

    ani_int errorResult = 0;
    ani_error error {};
    ani_boolean result = ANI_FALSE;
    ASSERT_EQ(env_->Function_Call_Int(func, &errorResult, MAGIC_NUMBER), ANI_PENDING_ERROR);
    ASSERT_EQ(env_->GetUnhandledError(&error), ANI_OK);
    ASSERT_EQ(env_->ThrowError(error), ANI_OK);
    ASSERT_EQ(env_->ExistUnhandledError(&result), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);

    ASSERT_EQ(env_->DescribeError(), ANI_OK);
    ASSERT_EQ(env_->ResetError(), ANI_OK);
    ASSERT_EQ(env_->ExistUnhandledError(&result), ANI_OK);
    ASSERT_EQ(result, ANI_FALSE);
}

TEST_F(ErrorHandlingTest, get_unhandled_invalid_args_test)
{
    ani_error error {};
    ASSERT_EQ(env_->GetUnhandledError(nullptr), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->c_api->GetUnhandledError(nullptr, &error), ANI_INVALID_ARGS);
}

TEST_F(ErrorHandlingTest, get_unhandled_multiple_call_test_1)
{
    auto func = GetThrowErrorFunction();

    ani_int errorResult = 0;
    ani_error error {};
    for (int32_t i = 0; i < LOOP_COUNT; i++) {
        ASSERT_EQ(env_->Function_Call_Int(func, &errorResult, MAGIC_NUMBER), ANI_PENDING_ERROR);
        ASSERT_EQ(env_->GetUnhandledError(&error), ANI_OK);
    }
}

TEST_F(ErrorHandlingTest, get_unhandled_multiple_call_test_2)
{
    ani_error error {};
    for (int32_t i = 0; i < LOOP_COUNT; i++) {
        ASSERT_EQ(env_->GetUnhandledError(&error), ANI_ERROR);
    }
}

TEST_F(ErrorHandlingTest, get_unhandled_multiple_call_test_3)
{
    auto func = GetThrowErrorFunction();

    ani_int errorResult = 0;
    ani_error error {};
    for (int32_t i = 0; i < LOOP_COUNT; i++) {
        ASSERT_EQ(env_->Function_Call_Int(func, &errorResult, MAGIC_NUMBER), ANI_PENDING_ERROR);
        ASSERT_EQ(env_->GetUnhandledError(&error), ANI_OK);

        ASSERT_EQ(env_->ResetError(), ANI_OK);
        ASSERT_EQ(env_->GetUnhandledError(&error), ANI_ERROR);
    }
}

TEST_F(ErrorHandlingTest, get_unhandled_multiple_call_test_4)
{
    auto func = GetThrowErrorFunction();

    ani_int errorResult = 0;
    ani_error error {};
    for (int32_t i = 0; i < LOOP_COUNT; i++) {
        ASSERT_EQ(env_->Function_Call_Int(func, &errorResult, MAGIC_NUMBER), ANI_PENDING_ERROR);
        ASSERT_EQ(env_->GetUnhandledError(&error), ANI_OK);

        ASSERT_EQ(env_->Function_Call_Int(func, &errorResult, MAGIC_NUMBER), ANI_PENDING_ERROR);
        ASSERT_EQ(env_->GetUnhandledError(&error), ANI_OK);
    }
}

TEST_F(ErrorHandlingTest, get_unhandled_multiple_call_test_5)
{
    ani_error error {};
    for (int32_t i = 0; i < LOOP_COUNT; i++) {
        ASSERT_EQ(env_->GetUnhandledError(&error), ANI_ERROR);
    }
}

TEST_F(ErrorHandlingTest, combined_scenes_test_1)
{
    auto func = GetThrowErrorFunction();

    ani_int errorResult = 0;
    ani_error error {};
    ani_boolean result = ANI_FALSE;
    ASSERT_EQ(env_->Function_Call_Int(func, &errorResult, MAGIC_NUMBER), ANI_PENDING_ERROR);
    ASSERT_EQ(env_->ExistUnhandledError(&result), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);
    ASSERT_EQ(env_->GetUnhandledError(&error), ANI_OK);
    ASSERT_EQ(env_->DescribeError(), ANI_OK);
    ASSERT_EQ(env_->ResetError(), ANI_OK);
    ASSERT_EQ(env_->ExistUnhandledError(&result), ANI_OK);
    ASSERT_EQ(result, ANI_FALSE);
}

TEST_F(ErrorHandlingTest, combined_scenes_test_2)
{
    auto func = GetThrowErrorFunction();

    ani_int errorResult = 0;
    ani_error error {};
    ani_boolean result = ANI_TRUE;
    ASSERT_EQ(env_->Function_Call_Int(func, &errorResult, MAGIC_NUMBER), ANI_PENDING_ERROR);
    ASSERT_EQ(env_->GetUnhandledError(&error), ANI_OK);
    ASSERT_EQ(env_->ThrowError(error), ANI_OK);
    ASSERT_EQ(env_->ResetError(), ANI_OK);
    ASSERT_EQ(env_->ExistUnhandledError(&result), ANI_OK);
    ASSERT_EQ(result, ANI_FALSE);
}

TEST_F(ErrorHandlingTest, combined_scenes_test_3)
{
    auto func = GetThrowErrorFunction();

    ani_int errorResult = 0;
    ani_error error {};
    ASSERT_EQ(env_->Function_Call_Int(func, &errorResult, MAGIC_NUMBER), ANI_PENDING_ERROR);
    ASSERT_EQ(env_->GetUnhandledError(&error), ANI_OK);
    ASSERT_EQ(env_->ThrowError(error), ANI_OK);
}

static std::string GetErrorProperty(ani_env *aniEnv, ani_error aniError, const char *property)
{
    std::string propertyValue;
    ani_type errorType = nullptr;
    if ((aniEnv->Object_GetType(aniError, &errorType)) != ANI_OK) {
        return propertyValue;
    }
    ani_method getterMethod = nullptr;
    if ((aniEnv->Class_FindGetter(static_cast<ani_class>(errorType), property, &getterMethod)) != ANI_OK) {
        return propertyValue;
    }
    ani_ref aniRef = nullptr;
    if ((aniEnv->Object_CallMethod_Ref(aniError, getterMethod, &aniRef)) != ANI_OK) {
        return propertyValue;
    }
    auto aniString = reinterpret_cast<ani_string>(aniRef);
    ani_size sz {};
    if ((aniEnv->String_GetUTF8Size(aniString, &sz)) != ANI_OK) {
        return propertyValue;
    }
    propertyValue.resize(sz + 1);
    if ((aniEnv->String_GetUTF8SubString(aniString, 0, sz, propertyValue.data(), propertyValue.size(), &sz)) !=
        ANI_OK) {
        return propertyValue;
    }
    propertyValue.resize(sz);
    return propertyValue;
}

TEST_F(ErrorHandlingTest, call_error_stack)
{
    auto func = GetThrowErrorFunction();

    ani_int errorResult = 0;
    ani_error error {};
    ASSERT_EQ(env_->Function_Call_Int(func, &errorResult, MAGIC_NUMBER), ANI_PENDING_ERROR);
    ASSERT_EQ(env_->GetUnhandledError(&error), ANI_OK);
    ASSERT_EQ(env_->ResetError(), ANI_OK);

    std::string stack = GetErrorProperty(env_, error, "stack");
    std::stringstream ss(stack);
    std::regex pattern(R"(\s*at .*(\d+):(\d+)\))");
    std::string token;

    while (std::getline(ss, token, '\n')) {
        ASSERT_TRUE(std::regex_match(token, pattern));
    }
}

}  // namespace ark::ets::ani::testing
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
