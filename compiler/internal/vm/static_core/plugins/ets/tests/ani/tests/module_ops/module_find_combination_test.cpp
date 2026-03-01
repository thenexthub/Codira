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

class ModuleFindCombinationTest : public AniTest {};

TEST_F(ModuleFindCombinationTest, many_descriptor)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_combination_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);
    ani_function fn {};
    ani_variable variable {};
    char end = 'J';
    const int32_t loopCount = 3;
    for (int32_t i = 0; i < loopCount; i++) {
        std::string functionName = "setIntValue";
        std::string variableName = "module";

        char ch = (random() % (end - 'A') + 'A');
        functionName += ch;
        variableName += ch;
        ASSERT_EQ(env_->Module_FindFunction(module, functionName.c_str(), "i:", &fn), ANI_OK);
        ASSERT_EQ(env_->Module_FindVariable(module, variableName.c_str(), &variable), ANI_OK);
    }
}

TEST_F(ModuleFindCombinationTest, invalid_arg)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_combination_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);
    ani_function fn {};
    ani_variable variable {};
    ASSERT_EQ(env_->Module_FindFunction(module, "setIntValueAAAA", "i:", &fn), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Module_FindVariable(module, "moduleAAAA", &variable), ANI_NOT_FOUND);

    ASSERT_EQ(env_->Module_FindFunction(nullptr, "setIntValueA", "i:", &fn), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Module_FindVariable(nullptr, "moduleA", &variable), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
