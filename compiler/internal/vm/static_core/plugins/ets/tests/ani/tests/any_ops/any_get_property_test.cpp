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
// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
namespace ark::ets::ani::testing {

class AnyGetPropertyTest : public AniTest {
protected:
    void CheckANIStr(const ani_string &str, const std::string &expected) const
    {
        const ani_size utfBufferSize = 20;
        std::array<char, utfBufferSize> utfBuffer = {0};
        ani_size resultSize;
        const ani_size offset = 0;
        ASSERT_EQ(env_->String_GetUTF8SubString(str, offset, expected.size(), utfBuffer.data(), utfBuffer.size(),
                                                &resultSize),
                  ANI_OK);
        ASSERT_STREQ(utfBuffer.data(), expected.c_str());
    }

    void GetMethod(const char *fnName, ani_module *mdResult, ani_function *fnResult)
    {
        ani_module md {};
        ASSERT_EQ(env_->FindModule("any_get_property_test", &md), ANI_OK);
        ASSERT_NE(md, nullptr);

        ani_function fn {};
        ASSERT_EQ(env_->Module_FindFunction(md, fnName, ":C{std.core.Array}", &fn), ANI_OK);
        ASSERT_NE(fn, nullptr);

        *mdResult = md;
        *fnResult = fn;
    }
};

TEST_F(AnyGetPropertyTest, AnyGetProperty_Valid)
{
    const char *clsName = "any_get_property_test.Student";
    ani_class cls {};
    ASSERT_EQ(env_->FindClass(clsName, &cls), ANI_OK);
    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", nullptr, &ctor), ANI_OK);
    std::string_view nameStr = "Leechy";
    ani_string strRef {};
    ASSERT_EQ(env_->String_NewUTF8(nameStr.data(), nameStr.size(), &strRef), ANI_OK);

    ani_object res {};
    ASSERT_EQ(env_->Object_New(cls, ctor, &res, ani_int(10U), strRef), ANI_OK);
    ani_ref nameRef {};
    ASSERT_EQ(env_->Any_GetProperty(static_cast<ani_ref>(res), "name", &nameRef), ANI_OK);
    CheckANIStr(static_cast<ani_string>(nameRef), "Leechy");
}

TEST_F(AnyGetPropertyTest, AnyGetPropertyUTF16_Valid)
{
    const char *clsName = "any_get_property_test.Client";
    ani_class cls {};
    ASSERT_EQ(env_->FindClass(clsName, &cls), ANI_OK);
    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", nullptr, &ctor), ANI_OK);
    std::u16string_view nameStr = u"中國";
    ani_string strRef {};
    ASSERT_EQ(env_->String_NewUTF16(reinterpret_cast<const uint16_t *>(nameStr.data()), nameStr.size(), &strRef),
              ANI_OK);
    ani_object res {};
    ASSERT_EQ(env_->Object_New(cls, ctor, &res, strRef), ANI_OK);

    ani_ref nameRef {};
    ASSERT_EQ(env_->Any_GetProperty(static_cast<ani_ref>(res), "原產地", &nameRef), ANI_OK);
    const ani_size bufferSize = 3U;
    uint16_t utf16Buffer[bufferSize] = {0U};
    ani_size substrOffset = 0U;
    ani_size substrSize = 2U;
    ani_size result = 0U;
    ASSERT_EQ(env_->String_GetUTF16SubString(static_cast<ani_string>(nameRef), substrOffset, substrSize, utf16Buffer,
                                             bufferSize, &result),
              ANI_OK);
    ASSERT_EQ(result, substrSize);
    ASSERT_EQ(std::u16string_view(reinterpret_cast<const char16_t *>(utf16Buffer), bufferSize - 1), nameStr);
}

TEST_F(AnyGetPropertyTest, AnyGetByIndex_Valid)
{
    ani_module md {};
    ani_function fn {};
    GetMethod("getArray", &md, &fn);
    ani_ref arrayRef {};
    ASSERT_EQ(env_->Function_Call_Ref(fn, &arrayRef), ANI_OK);
    ani_ref nameRefByIdx {};
    ASSERT_EQ(env_->Any_GetByIndex(arrayRef, 0U, &nameRefByIdx), ANI_OK);
    CheckANIStr(static_cast<ani_string>(nameRefByIdx), "Leechy");
}

TEST_F(AnyGetPropertyTest, AnyGetByIndex_Invalid)
{
    ani_ref dummyResult {};

    ASSERT_EQ(env_->Any_GetByIndex(nullptr, 0U, &dummyResult), ANI_INVALID_ARGS);

    ani_module md {};
    ani_function fn {};
    GetMethod("getArray", &md, &fn);
    ani_ref arrayRef {};
    ASSERT_EQ(env_->Function_Call_Ref(fn, &arrayRef), ANI_OK);

    ASSERT_EQ(env_->Any_GetByIndex(arrayRef, 0U, nullptr), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->c_api->Any_GetByIndex(nullptr, arrayRef, 0U, &dummyResult), ANI_INVALID_ARGS);
    ani_ref outOfBounds {};
    ASSERT_EQ(env_->Any_GetByIndex(arrayRef, 1000U, &outOfBounds), ANI_PENDING_ERROR);
    ASSERT_EQ(env_->ResetError(), ANI_OK);
}

TEST_F(AnyGetPropertyTest, AnyGetByValue_Valid)
{
    const char *clsName = "any_get_property_test.Worker";
    ani_class cls {};
    ASSERT_EQ(env_->FindClass(clsName, &cls), ANI_OK);
    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "iC{std.core.String}:", &ctor), ANI_OK);
    std::string_view nameStr = "Leechy";
    ani_string strRef {};
    ASSERT_EQ(env_->String_NewUTF8(nameStr.data(), nameStr.size(), &strRef), ANI_OK);

    ani_object res {};
    ASSERT_EQ(env_->Object_New(cls, ctor, &res, ani_int(10U), strRef), ANI_OK);
    ani_ref nameRef {};
    std::string_view fieldNameStr = "_name";
    ani_string filedNameRef {};
    ASSERT_EQ(env_->String_NewUTF8(fieldNameStr.data(), fieldNameStr.size(), &filedNameRef), ANI_OK);

    ASSERT_EQ(env_->Any_GetByValue(static_cast<ani_ref>(res), static_cast<ani_ref>(filedNameRef), &nameRef), ANI_OK);
    CheckANIStr(static_cast<ani_string>(nameRef), "Leechy");
}

TEST_F(AnyGetPropertyTest, AnyGetProperty_InvalidArgs)
{
    ani_ref dummyResult {};
    std::string propName = "name";

    ASSERT_EQ(env_->Any_GetProperty(nullptr, propName.c_str(), &dummyResult), ANI_INVALID_ARGS);

    const char *clsName = "any_get_property_test.Student";
    ani_class cls {};
    ASSERT_EQ(env_->FindClass(clsName, &cls), ANI_OK);
    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", nullptr, &ctor), ANI_OK);

    std::string_view nameStr = "Leechy";
    ani_string strRef {};
    ASSERT_EQ(env_->String_NewUTF8(nameStr.data(), nameStr.size(), &strRef), ANI_OK);

    ani_object res {};
    ASSERT_EQ(env_->Object_New(cls, ctor, &res, ani_int(10U), strRef), ANI_OK);

    ASSERT_EQ(env_->Any_GetProperty(static_cast<ani_ref>(res), nullptr, &dummyResult), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Any_GetProperty(static_cast<ani_ref>(res), propName.c_str(), nullptr), ANI_INVALID_ARGS);
}

TEST_F(AnyGetPropertyTest, AnyGetByValue_InvalidArgs)
{
    ani_ref dummyResult {};

    ASSERT_EQ(env_->Any_GetByValue(nullptr, nullptr, &dummyResult), ANI_INVALID_ARGS);

    const char *clsName = "any_get_property_test.Student";
    ani_class cls {};
    ASSERT_EQ(env_->FindClass(clsName, &cls), ANI_OK);
    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", nullptr, &ctor), ANI_OK);

    std::string_view nameStr = "Leechy";
    ani_string strRef {};
    ASSERT_EQ(env_->String_NewUTF8(nameStr.data(), nameStr.size(), &strRef), ANI_OK);

    ani_object res {};
    ASSERT_EQ(env_->Object_New(cls, ctor, &res, ani_int(10U), strRef), ANI_OK);

    ASSERT_EQ(env_->Any_GetByValue(static_cast<ani_ref>(res), nullptr, &dummyResult), ANI_INVALID_ARGS);

    std::string_view fieldName = "name";
    ani_string keyStr {};
    ASSERT_EQ(env_->String_NewUTF8(fieldName.data(), fieldName.size(), &keyStr), ANI_OK);

    ASSERT_EQ(env_->Any_GetByValue(static_cast<ani_ref>(res), static_cast<ani_ref>(keyStr), nullptr), ANI_INVALID_ARGS);
}

TEST_F(AnyGetPropertyTest, AnyGetByValue_InvalidKey)
{
    const char *clsName = "any_get_property_test.Student";
    ani_class cls {};
    ASSERT_EQ(env_->FindClass(clsName, &cls), ANI_OK);
    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", nullptr, &ctor), ANI_OK);

    std::string_view nameStr = "Leechy";
    ani_string strRef {};
    ASSERT_EQ(env_->String_NewUTF8(nameStr.data(), nameStr.size(), &strRef), ANI_OK);

    ani_object res {};
    ASSERT_EQ(env_->Object_New(cls, ctor, &res, ani_int(10U), strRef), ANI_OK);

    const char *invalidClsName = "std.core.Object";
    ani_class dummyCls {};
    ASSERT_EQ(env_->FindClass(invalidClsName, &dummyCls), ANI_OK);
    ani_method dummyCtor {};
    ASSERT_EQ(env_->Class_FindMethod(dummyCls, "<ctor>", nullptr, &dummyCtor), ANI_OK);
    ani_object dummyKey {};
    ASSERT_EQ(env_->Object_New(dummyCls, dummyCtor, &dummyKey), ANI_OK);

    ani_ref result {};
    ASSERT_EQ(env_->Any_GetByValue(static_cast<ani_ref>(res), static_cast<ani_ref>(dummyKey), &result),
              ANI_PENDING_ERROR);
}
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
}  // namespace ark::ets::ani::testing
