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

namespace ark::ets::ani::testing {

class ObjectSetFieldByNameRefTest : public AniTest {
public:
    ani_object NewAnimal()
    {
        auto animalRef = CallEtsFunction<ani_ref>("object_set_field_by_name_ref_test", "newAnimalObject");
        return static_cast<ani_object>(animalRef);
    }

    ani_size CreateAniString(const std::string &example, ani_ref &outRef)
    {
        const size_t strLen = example.size();
        ani_string aniStringPtr = nullptr;
        env_->String_NewUTF8(example.c_str(), strLen, &aniStringPtr);
        outRef = static_cast<ani_ref>(aniStringPtr);  // 返回引用
        return strLen;
    }
};

TEST_F(ObjectSetFieldByNameRefTest, set_field)
{
    ani_ref nameRef {};
    auto strLen = CreateAniString("example", nameRef);

    ani_object animal = NewAnimal();
    const int32_t loopCount = 3;
    for (int32_t i = 1; i <= loopCount; i++) {
        ASSERT_EQ(env_->Object_SetFieldByName_Ref(animal, "name", nameRef), ANI_OK);

        ASSERT_EQ(env_->Object_GetFieldByName_Ref(animal, "name", &nameRef), ANI_OK);
        auto name = static_cast<ani_string>(nameRef);
        const uint32_t size = 15;
        std::array<char, size> buffer {};
        ani_size nameSize;
        ASSERT_EQ(env_->String_GetUTF8SubString(name, 0U, strLen, buffer.data(), buffer.size(), &nameSize), ANI_OK);
        ASSERT_EQ(nameSize, strLen);
        ASSERT_STREQ(buffer.data(), "example");
    }
}

TEST_F(ObjectSetFieldByNameRefTest, invalid_env)
{
    ani_ref nameRef {};
    ani_object animal = NewAnimal();
    CreateAniString("example", nameRef);

    ASSERT_EQ(env_->c_api->Object_SetFieldByName_Ref(nullptr, animal, "x", nameRef), ANI_INVALID_ARGS);
}

TEST_F(ObjectSetFieldByNameRefTest, not_found_name)
{
    ani_ref nameRef {};
    CreateAniString("example", nameRef);

    ani_object animal = NewAnimal();
    ASSERT_EQ(env_->Object_SetFieldByName_Ref(animal, "x", nameRef), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Object_SetFieldByName_Ref(animal, "", nameRef), ANI_NOT_FOUND);
}

TEST_F(ObjectSetFieldByNameRefTest, invalid_type)
{
    ani_ref nameRef {};
    CreateAniString("example", nameRef);

    ani_object animal = NewAnimal();
    ASSERT_EQ(env_->Object_SetFieldByName_Ref(animal, "age", nameRef), ANI_INVALID_TYPE);
}

TEST_F(ObjectSetFieldByNameRefTest, invalid_object)
{
    ani_ref nameRef {};
    CreateAniString("example", nameRef);

    ASSERT_EQ(env_->Object_SetFieldByName_Ref(nullptr, "x", nameRef), ANI_INVALID_ARGS);
}

TEST_F(ObjectSetFieldByNameRefTest, invalid_name)
{
    ani_ref nameRef {};
    CreateAniString("example", nameRef);

    ani_object animal = NewAnimal();
    ASSERT_EQ(env_->Object_SetFieldByName_Ref(animal, nullptr, nameRef), ANI_INVALID_ARGS);
}

TEST_F(ObjectSetFieldByNameRefTest, invalid_value)
{
    ani_ref nameRef {};
    CreateAniString("example", nameRef);

    ani_object animal = NewAnimal();
    ASSERT_EQ(env_->Object_SetFieldByName_Ref(animal, "x", nullptr), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
