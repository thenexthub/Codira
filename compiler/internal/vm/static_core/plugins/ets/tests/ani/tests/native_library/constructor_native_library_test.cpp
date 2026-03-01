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

class ConstructorNativeLibraryTest : public AniTest {};

TEST_F(ConstructorNativeLibraryTest, call_ANI_Constructor)
{
    ani_boolean unhandledError;

    // Call native library constructor
    auto calcRef = CallEtsFunction<ani_ref>("constructor_native_library_test", "new_Calc");
    ASSERT_EQ(env_->ExistUnhandledError(&unhandledError), ANI_OK);
    ASSERT_EQ(unhandledError, ANI_FALSE) << "Cannot load native library";

    // Call native method
    auto sum = CallEtsFunction<ani_int>("constructor_native_library_test", "Calc_sum", static_cast<ani_object>(calcRef),
                                        2U, 4U);
    ASSERT_EQ(env_->ExistUnhandledError(&unhandledError), ANI_OK);
    ASSERT_EQ(unhandledError, ANI_FALSE) << "Cannot call native method";

    // Check result
    ASSERT_EQ(sum, 2U + 4U);
}

}  // namespace ark::ets::ani::testing
