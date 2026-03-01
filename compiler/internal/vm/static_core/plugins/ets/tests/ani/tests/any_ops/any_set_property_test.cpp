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

class AnySetPropertyTest : public AniTest {
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
        ASSERT_EQ(env_->FindModule("any_set_property_test", &md), ANI_OK);
        ASSERT_NE(md, nullptr);

        ani_function fn {};
        ASSERT_EQ(env_->Module_FindFunction(md, fnName, ":C{std.core.Array}", &fn), ANI_OK);
        ASSERT_NE(fn, nullptr);

        *mdResult = md;
        *fnResult = fn;
    }
};
TEST_F(AnySetPropertyTest, AnySetProperty_Valid)
{
    const char *clsName = "any_set_property_test.Student";
    ani_class cls {};
    ASSERT_EQ(env_->FindClass(clsName, &cls), ANI_OK);
    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", nullptr, &ctor), ANI_OK);

    std::string_view oldName = "Old";
    ani_string strRef {};
    ASSERT_EQ(env_->String_NewUTF8(oldName.data(), oldName.size(), &strRef), ANI_OK);

    ani_object res {};
    ASSERT_EQ(env_->Object_New(cls, ctor, &res, ani_int(1), strRef), ANI_OK);

    std::string_view newName = "Leechy";
    ani_string newStrRef {};
    ASSERT_EQ(env_->String_NewUTF8(newName.data(), newName.size(), &newStrRef), ANI_OK);

    ASSERT_EQ(env_->Any_SetProperty(res, "name", newStrRef), ANI_OK);

    ani_ref result {};
    ASSERT_EQ(env_->Any_GetProperty(res, "name", &result), ANI_OK);
    CheckANIStr(static_cast<ani_string>(result), "Leechy");
}

TEST_F(AnySetPropertyTest, AnySetByIndex_Valid)
{
    ani_module md {};
    ani_function fn {};
    GetMethod("GetNumericArray", &md, &fn);
    ani_ref arrayRef {};
    ASSERT_EQ(env_->Function_Call_Ref(fn, &arrayRef), ANI_OK);

    ani_class intCls {};
    ani_method intCtor {};
    ASSERT_EQ(env_->FindClass("std.core.Int", &intCls), ANI_OK);
    ASSERT_EQ(env_->Class_FindMethod(intCls, "<ctor>", "i:", &intCtor), ANI_OK);
    ani_object intObj {};
    ani_int intValue = 100U;
    ASSERT_EQ(env_->Object_New(intCls, intCtor, &intObj, intValue), ANI_OK);
    ASSERT_EQ(env_->Any_SetByIndex(arrayRef, 0U, static_cast<ani_ref>(intObj)), ANI_OK);

    ani_ref result {};
    ASSERT_EQ(env_->Any_GetByIndex(arrayRef, 0U, &result), ANI_OK);
    ani_int intResult = 0;
    ASSERT_EQ(env_->Object_CallMethodByName_Int(static_cast<ani_object>(result), "toInt", nullptr, &intResult), ANI_OK);
    ASSERT_EQ(intResult, intValue);
}

TEST_F(AnySetPropertyTest, AnySetByValue_Valid)
{
    const char *clsName = "any_set_property_test.Student";
    ani_class cls {};
    ASSERT_EQ(env_->FindClass(clsName, &cls), ANI_OK);
    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", nullptr, &ctor), ANI_OK);

    std::string_view name = "Initial";
    ani_string nameRef {};
    ASSERT_EQ(env_->String_NewUTF8(name.data(), name.size(), &nameRef), ANI_OK);

    ani_object res {};
    ASSERT_EQ(env_->Object_New(cls, ctor, &res, ani_int(2U), nameRef), ANI_OK);

    std::string_view keyStr = "name";
    ani_string keyRef {};
    ASSERT_EQ(env_->String_NewUTF8(keyStr.data(), keyStr.size(), &keyRef), ANI_OK);

    std::string_view newValue = "Changed";
    ani_string newValueRef {};
    ASSERT_EQ(env_->String_NewUTF8(newValue.data(), newValue.size(), &newValueRef), ANI_OK);

    ASSERT_EQ(env_->Any_SetByValue(res, keyRef, newValueRef), ANI_OK);

    ani_ref result {};
    ASSERT_EQ(env_->Any_GetProperty(res, "name", &result), ANI_OK);
    CheckANIStr(static_cast<ani_string>(result), "Changed");
}

TEST_F(AnySetPropertyTest, AnySetProperty_InvalidArgs)
{
    ani_ref dummyRef {};
    std::string_view name = "name";
    std::string_view value = "Invalid";
    ani_string valRef {};
    ASSERT_EQ(env_->String_NewUTF8(value.data(), value.size(), &valRef), ANI_OK);

    ASSERT_EQ(env_->Any_SetProperty(nullptr, name.data(), valRef), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Any_SetProperty(dummyRef, nullptr, valRef), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Any_SetProperty(dummyRef, name.data(), nullptr), ANI_INVALID_ARGS);
}

TEST_F(AnySetPropertyTest, AnySetByIndex_Invalid)
{
    ani_module md {};
    ani_function fn {};
    GetMethod("GetNumericArray", &md, &fn);
    ani_ref arrayRef {};
    ASSERT_EQ(env_->Function_Call_Ref(fn, &arrayRef), ANI_OK);

    ani_class intCls {};
    ani_method intCtor {};
    ASSERT_EQ(env_->FindClass("std.core.Int", &intCls), ANI_OK);
    ASSERT_EQ(env_->Class_FindMethod(intCls, "<ctor>", "i:", &intCtor), ANI_OK);

    ani_object intObj {};
    ani_int intValue = 42;
    ASSERT_EQ(env_->Object_New(intCls, intCtor, &intObj, intValue), ANI_OK);

    ASSERT_EQ(env_->Any_SetByIndex(nullptr, 0U, static_cast<ani_ref>(intObj)), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Any_SetByIndex(arrayRef, 0U, nullptr), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Any_SetByIndex(arrayRef, 1000U, static_cast<ani_ref>(intObj)), ANI_PENDING_ERROR);
    ASSERT_EQ(env_->ResetError(), ANI_OK);
}

TEST_F(AnySetPropertyTest, AnySetByValue_InvalidKey)
{
    const char *clsName = "any_set_property_test.Student";
    ani_class cls {};
    ASSERT_EQ(env_->FindClass(clsName, &cls), ANI_OK);
    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", nullptr, &ctor), ANI_OK);

    std::string_view name = "X";
    ani_string nameRef {};
    ASSERT_EQ(env_->String_NewUTF8(name.data(), name.size(), &nameRef), ANI_OK);

    ani_object res {};
    ASSERT_EQ(env_->Object_New(cls, ctor, &res, ani_int(3U), nameRef), ANI_OK);

    const char *objClsName = "std.core.Object";
    ani_class keyCls {};
    ASSERT_EQ(env_->FindClass(objClsName, &keyCls), ANI_OK);
    ani_method objCtor {};
    ASSERT_EQ(env_->Class_FindMethod(keyCls, "<ctor>", nullptr, &objCtor), ANI_OK);
    ani_object keyObj {};
    ASSERT_EQ(env_->Object_New(keyCls, objCtor, &keyObj), ANI_OK);

    ASSERT_EQ(env_->Any_SetByValue(res, keyObj, nameRef), ANI_PENDING_ERROR);
}
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
}  // namespace ark::ets::ani::testing
