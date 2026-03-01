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

#ifndef ANI_GTEST_OBJECT_OPS_H
#define ANI_GTEST_OBJECT_OPS_H

#include "ani_gtest.h"

namespace ark::ets::ani::testing {

class AniGtestObjectOps : public AniTest {
public:
    /**
     * @brief Retrieves the ani_object and ani_method needed for testing.
     *
     * This method allocates an object and fetches a method from the class,
     * which can then be used in test cases for method invocation.
     *
     * @param className Pointer to store the class name.
     * @param methodName Pointer to store the method Name.
     * @param signature Pointer to store the method signature.
     * @param objectResult Pointer to store the allocated ani_object.
     * @param methodResult Pointer to store the retrieved ani_method.
     */
    void GetMethodAndObject(const char *className, const char *methodName, const char *signature, ani_object *object,
                            ani_method *method)
    {
        // NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)
        ani_class cls {};
        ASSERT_EQ(env_->FindClass(className, &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        ani_method ctor {};
        ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", ":", &ctor), ANI_OK);

        ASSERT_EQ(env_->Object_New(cls, ctor, object), ANI_OK);
        ASSERT_NE(object, nullptr);

        ASSERT_EQ(env_->Class_FindMethod(cls, methodName, signature, method), ANI_OK);
        ASSERT_NE(method, nullptr);
        // NOLINTEND(cppcoreguidelines-pro-type-vararg)
    }
};
}  // namespace ark::ets::ani::testing

#endif  // ANI_GTEST_OBJECT_OPS_H