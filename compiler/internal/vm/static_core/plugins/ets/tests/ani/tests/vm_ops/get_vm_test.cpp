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

class GetVMTest : public AniTest {};

TEST_F(GetVMTest, valid_argument)
{
    ani_vm *vm = nullptr;
    ASSERT_EQ(env_->GetVM(&vm), ANI_OK);
    ASSERT_NE(vm, nullptr);
}

TEST_F(GetVMTest, invalid_argument)
{
    ASSERT_EQ(env_->GetVM(nullptr), ANI_INVALID_ARGS);
    ani_vm *vm = nullptr;
    ASSERT_EQ(env_->c_api->GetVM(nullptr, &vm), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
