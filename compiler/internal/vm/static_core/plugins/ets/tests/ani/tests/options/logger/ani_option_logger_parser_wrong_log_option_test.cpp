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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "ani.h"
#include "ani_option_logger_callback.h"

namespace ark::ets::ani::testing {

TEST(AniOptionLoggerTest, logger_options_parser_faild)
{
    ASSERT_EQ(g_msgs.size(), 0);

    std::array optionsArray {
        ani_option {"--logger", reinterpret_cast<void *>(LoggerCallback)},
        ani_option {"--ext:log-components=invalid_component", nullptr},
    };

    ani_options options = {optionsArray.size(), optionsArray.data()};
    ani_vm *vm;
    ASSERT_EQ(ANI_CreateVM(&options, ANI_VERSION_1, &vm), ANI_ERROR);

    ASSERT_EQ(g_msgs.size(), 1);
    ASSERT_EQ(g_msgs[0].level, ANI_LOGLEVEL_ERROR);
    ASSERT_STREQ(g_msgs[0].component.c_str(), "ani");
    ASSERT_THAT(g_msgs[0].message.c_str(),
                ::testing::HasSubstr("argument --log-components: invalid value: 'invalid_component'."));
}

}  // namespace ark::ets::ani::testing
