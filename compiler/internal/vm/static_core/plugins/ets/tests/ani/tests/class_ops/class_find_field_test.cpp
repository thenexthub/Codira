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

#include "ani_gtest.h"

namespace ark::ets::ani::testing {

class ClassFindFieldTest : public AniTest {};

TEST_F(ClassFindFieldTest, get_point_field)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("class_find_field_test.Point", &cls), ANI_OK);

    ani_field fieldX;
    ASSERT_EQ(env_->Class_FindField(cls, "x", &fieldX), ANI_OK);
    ASSERT_NE(fieldX, nullptr);

    ani_field fieldY;
    ASSERT_EQ(env_->Class_FindField(cls, "y", &fieldY), ANI_OK);
    ASSERT_NE(fieldY, nullptr);

    ani_field fieldGetRadius;
    ASSERT_EQ(env_->Class_FindField(cls, "getRadius", &fieldGetRadius), ANI_OK);
    ASSERT_NE(fieldGetRadius, nullptr);
}

TEST_F(ClassFindFieldTest, point_field_not_found)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("class_find_field_test.Point", &cls), ANI_OK);

    ani_field fieldZ = nullptr;
    ASSERT_EQ(env_->Class_FindField(cls, "z", &fieldZ), ANI_NOT_FOUND);
    ASSERT_EQ(fieldZ, nullptr);
}

TEST_F(ClassFindFieldTest, get_extended_point_field)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("class_find_field_test.ExtendedPoint", &cls), ANI_OK);

    ani_field fieldX;
    ASSERT_EQ(env_->Class_FindField(cls, "x", &fieldX), ANI_OK);
    ASSERT_NE(fieldX, nullptr);

    ani_field fieldY;
    ASSERT_EQ(env_->Class_FindField(cls, "y", &fieldY), ANI_OK);
    ASSERT_NE(fieldY, nullptr);

    ani_field fieldGetRadius;
    ASSERT_EQ(env_->Class_FindField(cls, "getRadius", &fieldGetRadius), ANI_OK);
    ASSERT_NE(fieldGetRadius, nullptr);

    ani_field fieldZ;
    ASSERT_EQ(env_->Class_FindField(cls, "z", &fieldZ), ANI_OK);
    ASSERT_NE(fieldZ, nullptr);
}

TEST_F(ClassFindFieldTest, get_vector_field)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("class_find_field_test.Vector", &cls), ANI_OK);

    ani_field fieldP1;
    ASSERT_EQ(env_->Class_FindField(cls, "p1", &fieldP1), ANI_OK);
    ASSERT_NE(fieldP1, nullptr);

    ani_field fieldP2;
    ASSERT_EQ(env_->Class_FindField(cls, "p2", &fieldP2), ANI_OK);
    ASSERT_NE(fieldP2, nullptr);

    ani_field fieldLength;
    ASSERT_EQ(env_->Class_FindField(cls, "length", &fieldLength), ANI_OK);
    ASSERT_NE(fieldLength, nullptr);
}

TEST_F(ClassFindFieldTest, get_circle_field)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("class_find_field_test.Circle", &cls), ANI_OK);

    ani_field fieldPoints;
    ASSERT_EQ(env_->Class_FindField(cls, "points", &fieldPoints), ANI_OK);
    ASSERT_NE(fieldPoints, nullptr);

    ani_field fieldVectors;
    ASSERT_EQ(env_->Class_FindField(cls, "vectors", &fieldVectors), ANI_OK);
    ASSERT_NE(fieldVectors, nullptr);

    ani_field fieldRadius;
    ASSERT_EQ(env_->Class_FindField(cls, "radius", &fieldRadius), ANI_OK);
    ASSERT_NE(fieldRadius, nullptr);

    ani_field fieldCenter;
    ASSERT_EQ(env_->Class_FindField(cls, "center", &fieldCenter), ANI_OK);
    ASSERT_NE(fieldCenter, nullptr);
}

TEST_F(ClassFindFieldTest, get_sphere_field)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("class_find_field_test.Sphere", &cls), ANI_OK);

    ani_field fieldPoints;
    ASSERT_EQ(env_->Class_FindField(cls, "points", &fieldPoints), ANI_OK);
    ASSERT_NE(fieldPoints, nullptr);

    ani_field fieldVectors;
    ASSERT_EQ(env_->Class_FindField(cls, "vectors", &fieldVectors), ANI_OK);
    ASSERT_NE(fieldVectors, nullptr);

    ani_field fieldRadius;
    ASSERT_EQ(env_->Class_FindField(cls, "radius", &fieldRadius), ANI_OK);
    ASSERT_NE(fieldRadius, nullptr);

    ani_field fieldCenter;
    ASSERT_EQ(env_->Class_FindField(cls, "center", &fieldCenter), ANI_OK);
    ASSERT_NE(fieldCenter, nullptr);
}

TEST_F(ClassFindFieldTest, get_space_field)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("class_find_field_test.Space", &cls), ANI_OK);

    ani_field fieldObjects;
    ASSERT_EQ(env_->Class_FindField(cls, "objects", &fieldObjects), ANI_OK);
    ASSERT_NE(fieldObjects, nullptr);

    ani_field fieldDimension;
    ASSERT_EQ(env_->Class_FindField(cls, "dimension", &fieldDimension), ANI_OK);
    ASSERT_NE(fieldDimension, nullptr);

    ani_field fieldName;
    ASSERT_EQ(env_->Class_FindField(cls, "name", &fieldName), ANI_OK);
    ASSERT_NE(fieldName, nullptr);
}

TEST_F(ClassFindFieldTest, get_partial_field)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("class_find_field_test.UniversalFigureHandler", &cls), ANI_OK);

    ani_field fieldFigure;
    ASSERT_EQ(env_->Class_FindField(cls, "figure", &fieldFigure), ANI_OK);
    ASSERT_NE(fieldFigure, nullptr);
}

TEST_F(ClassFindFieldTest, get_required_field)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("class_find_field_test.NamedSpaceHandler", &cls), ANI_OK);

    ani_field fieldSpace;
    ASSERT_EQ(env_->Class_FindField(cls, "space", &fieldSpace), ANI_OK);
    ASSERT_NE(fieldSpace, nullptr);
}

TEST_F(ClassFindFieldTest, invalid_argument1)
{
    ani_field field;
    ASSERT_EQ(env_->Class_FindField(nullptr, "x", &field), ANI_INVALID_ARGS);
}

TEST_F(ClassFindFieldTest, invalid_argument2)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("class_find_field_test.Point", &cls), ANI_OK);

    ani_field field;
    ASSERT_EQ(env_->Class_FindField(cls, nullptr, &field), ANI_INVALID_ARGS);
}

TEST_F(ClassFindFieldTest, invalid_argument3)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("class_find_field_test.Point", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_FindField(cls, "x", nullptr), ANI_INVALID_ARGS);
}

TEST_F(ClassFindFieldTest, check_initialization)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_find_field_test.Point", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_find_field_test.Point"));
    ani_field field {};
    ASSERT_EQ(env_->Class_FindField(cls, "x", &field), ANI_OK);
    ASSERT_FALSE(IsRuntimeClassInitialized("class_find_field_test.Point"));
}

TEST_F(ClassFindFieldTest, check_hierarchy)
{
    ani_class clsParent {};
    ASSERT_EQ(env_->FindClass("class_find_field_test.Parent", &clsParent), ANI_OK);
    ani_class clsChild {};
    ASSERT_EQ(env_->FindClass("class_find_field_test.Child", &clsChild), ANI_OK);

    // |-----------------------------------------------------------------------------------------|
    // |                 |    Parent class   |     Child class   |    The same field is found    |
    // |-----------------|-------------------|-------------------|-------------------------------|
    // |  Parent field   |       ANI_OK      |       ANI_OK      |              Yes              |
    // |   Child field   |   ANI_NOT_FOUND   |       ANI_OK      |               No              |
    // | Overrided field |       ANI_OK      |       ANI_OK      |              Yes              |
    // |-----------------|-------------------|-------------------|-------------------------------|

    ani_field parentFieldInParent {};
    ASSERT_EQ(env_->Class_FindField(clsParent, "parentField", &parentFieldInParent), ANI_OK);
    ani_field childFieldInParent {};
    ASSERT_EQ(env_->Class_FindField(clsParent, "childField", &childFieldInParent), ANI_NOT_FOUND);
    ani_field overridedFieldInParent {};
    ASSERT_EQ(env_->Class_FindField(clsParent, "overridedField", &overridedFieldInParent), ANI_OK);

    ani_field parentFieldInChild {};
    ASSERT_EQ(env_->Class_FindField(clsChild, "parentField", &parentFieldInChild), ANI_OK);
    ani_field childFieldInChild {};
    ASSERT_EQ(env_->Class_FindField(clsChild, "childField", &childFieldInChild), ANI_OK);
    ani_field overridedFieldInChild {};
    ASSERT_EQ(env_->Class_FindField(clsChild, "overridedField", &overridedFieldInChild), ANI_OK);

    ASSERT_EQ(parentFieldInParent, parentFieldInChild);
    ASSERT_NE(childFieldInParent, childFieldInChild);
    ASSERT_EQ(overridedFieldInParent, overridedFieldInChild);
}

}  // namespace ark::ets::ani::testing
