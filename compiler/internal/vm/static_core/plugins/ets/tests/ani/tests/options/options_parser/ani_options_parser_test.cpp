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

#include <gtest/gtest.h>
#include "plugins/ets/runtime/ani/ani_options_parser.h"

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ASSERT_ERROR_MSG(errorMsg, str) /* CC-OFF(G.PRE.02) necessary macro */ \
    do {                                                                       \
        std::string s(str); /* NOLINT(readability-redundant-string-init) */    \
        if ((errorMsg).has_value()) {                                          \
            ASSERT_STREQ((errorMsg).value().c_str(), s.c_str());               \
        } else {                                                               \
            ASSERT_STREQ("", s.c_str());                                       \
        }                                                                      \
    } while (false)

namespace ark::ets::ani::testing {

class AniOptionsParserTest : public ::testing::Test {
public:
    void CheckDefaultRuntimeOptions()
    {
        auto &options = parser_.GetRuntimeOptions();

        ASSERT_EQ(options.GetBootIntrinsicSpaces(), std::vector<std::string> {"ets"});
        ASSERT_EQ(options.GetBootClassSpaces(), std::vector<std::string> {"ets"});
        ASSERT_EQ(options.GetRuntimeType(), std::string {"ets"});
        ASSERT_EQ(options.GetLoadRuntimes(), std::vector<std::string> {"ets"});
    }

    void TearDown() override
    {
        CheckDefaultRuntimeOptions();
    }

protected:
    OptionsParser parser_;  // NOLINT(misc-non-private-member-variables-in-classes)
};

TEST_F(AniOptionsParserTest, defaultOptions)
{
    OptionsParser::ErrorMsg errorMsg = parser_.Parse(nullptr);
    ASSERT_ERROR_MSG(errorMsg, "");
}

TEST_F(AniOptionsParserTest, loggerOption)
{
    std::array optionsArray {
        ani_option {"--logger", reinterpret_cast<void *>(1L)},
    };
    ani_options opts = {
        optionsArray.size(),
        optionsArray.data(),
    };
    OptionsParser::ErrorMsg errorMsg = parser_.Parse(&opts);
    ASSERT_ERROR_MSG(errorMsg, "");
}

TEST_F(AniOptionsParserTest, loggerOptionWithValue)
{
    std::array optionsArray {
        ani_option {"--logger=enable", reinterpret_cast<void *>(1L)},
    };
    ani_options opts = {
        optionsArray.size(),
        optionsArray.data(),
    };
    OptionsParser::ErrorMsg errorMsg = parser_.Parse(&opts);
    ASSERT_ERROR_MSG(errorMsg, "'--logger' option mustn't have value, value='enable'");
}

TEST_F(AniOptionsParserTest, loggerOptionWithNull)
{
    std::array optionsArray {
        ani_option {"--logger", nullptr},
    };
    ani_options opts = {
        optionsArray.size(),
        optionsArray.data(),
    };
    OptionsParser::ErrorMsg errorMsg = parser_.Parse(&opts);
    ASSERT_ERROR_MSG(errorMsg, "'--logger' option has 'extra==NULL'");
}

TEST_F(AniOptionsParserTest, unsupporetdOption)
{
    std::array optionsArray {
        ani_option {"--bla-bla-bla", nullptr},
    };
    ani_options opts = {
        optionsArray.size(),
        optionsArray.data(),
    };
    OptionsParser::ErrorMsg errorMsg = parser_.Parse(&opts);
    ASSERT_ERROR_MSG(errorMsg, "'--bla-bla-bla' option is not supported");
}

TEST_F(AniOptionsParserTest, interop)
{
    std::array optionsArray {
        ani_option {"--ext:interop", nullptr},
    };
    ani_options opts = {
        optionsArray.size(),
        optionsArray.data(),
    };
    OptionsParser::ErrorMsg errorMsg = parser_.Parse(&opts);
#ifdef PANDA_ETS_INTEROP_JS
    ASSERT_ERROR_MSG(errorMsg, "");
#else
    ASSERT_ERROR_MSG(errorMsg, "'--ext:interop' option is not supported");
#endif
}

TEST_F(AniOptionsParserTest, invalidExtOption)
{
    std::array optionsArray {
        ani_option {"--ext:log-level3=info", nullptr},
    };
    ani_options opts = {
        optionsArray.size(),
        optionsArray.data(),
    };
    OptionsParser::ErrorMsg errorMsg = parser_.Parse(&opts);
    ASSERT_ERROR_MSG(errorMsg, "'--ext:log-level3' option is not supported");
}

TEST_F(AniOptionsParserTest, extOptionHasExtra)
{
    std::array optionsArray {
        ani_option {"--ext:log-level=error", reinterpret_cast<void *>(1L)},
    };
    ani_options opts = {
        optionsArray.size(),
        optionsArray.data(),
    };
    OptionsParser::ErrorMsg errorMsg = parser_.Parse(&opts);
    ASSERT_ERROR_MSG(errorMsg, "Extended '--ext:log-level' option doesn't support 'extra' field, 'extra' must be NULL");
}

TEST_F(AniOptionsParserTest, validExtLoggerOptionValue)
{
    std::array optionsArray {
        ani_option {"--ext:log-level=info", nullptr},
    };
    ani_options opts = {
        optionsArray.size(),
        optionsArray.data(),
    };
    OptionsParser::ErrorMsg errorMsg = parser_.Parse(&opts);
    ASSERT_ERROR_MSG(errorMsg, "");
}

TEST_F(AniOptionsParserTest, invalidExtLoggerOptionValue)
{
    std::array optionsArray {
        ani_option {"--ext:log-level=infoX", nullptr},
    };
    ani_options opts = {
        optionsArray.size(),
        optionsArray.data(),
    };
    OptionsParser::ErrorMsg errorMsg = parser_.Parse(&opts);
    ASSERT_ERROR_MSG(errorMsg,
                     "argument --log-level: invalid value: 'infoX'. Possible values: [\"debug\", \"info\", "
                     "\"warning\", \"error\", \"fatal\"]");
}

TEST_F(AniOptionsParserTest, validExtRuntimeOptionValue)
{
    std::array optionsArray {
        ani_option {"--ext:gc-type=g1-gc", nullptr},
    };
    ani_options opts = {
        optionsArray.size(),
        optionsArray.data(),
    };
    OptionsParser::ErrorMsg errorMsg = parser_.Parse(&opts);
    ASSERT_ERROR_MSG(errorMsg, "");
}

TEST_F(AniOptionsParserTest, invalidExtRuntimeOptionValue)
{
    std::array optionsArray {
        ani_option {"--ext:compiler-enable-jit=no_value", nullptr},
    };
    ani_options opts = {
        optionsArray.size(),
        optionsArray.data(),
    };
    OptionsParser::ErrorMsg errorMsg = parser_.Parse(&opts);
    ASSERT_ERROR_MSG(errorMsg, "pandargs: Bool argument compiler-enable-jit has unexpected parameter value no_value");
}

TEST_F(AniOptionsParserTest, validExtCompilerOptionValue)
{
    std::array optionsArray {
        ani_option {"--ext:compiler-inline-external-methods=false", nullptr},
    };
    ani_options opts = {
        optionsArray.size(),
        optionsArray.data(),
    };
    OptionsParser::ErrorMsg errorMsg = parser_.Parse(&opts);
    ASSERT_ERROR_MSG(errorMsg, "");
}

TEST_F(AniOptionsParserTest, invalidExtCompilerOptionValue)
{
    std::array optionsArray {
        ani_option {"--ext:compiler-inline-external-methods=no_value", nullptr},
    };
    ani_options opts = {
        optionsArray.size(),
        optionsArray.data(),
    };
    OptionsParser::ErrorMsg errorMsg = parser_.Parse(&opts);
    ASSERT_ERROR_MSG(
        errorMsg, "pandargs: Bool argument compiler-inline-external-methods has unexpected parameter value no_value");
}

TEST_F(AniOptionsParserTest, validExtCompoundOptions)
{
    std::array optionsArray {
        ani_option {"--ext:g1-pause-time-goal", nullptr},
        ani_option {"--ext:g1-pause-time-goal:max-gc-pause=12", nullptr},
        ani_option {"--ext:g1-pause-time-goal:gc-pause-interval=10", nullptr},
    };
    ani_options opts = {
        optionsArray.size(),
        optionsArray.data(),
    };
    OptionsParser::ErrorMsg errorMsg = parser_.Parse(&opts);
    ASSERT_ERROR_MSG(errorMsg, "");
}

TEST_F(AniOptionsParserTest, check_PANDA_PRODUCT_BUILD)
{
    std::array optionsArray {
        ani_option {"--ext:enable-an:force", nullptr},
    };
    ani_options opts = {
        optionsArray.size(),
        optionsArray.data(),
    };
    OptionsParser::ErrorMsg errorMsg = parser_.Parse(&opts);
#ifdef PANDA_PRODUCT_BUILD
    ASSERT_ERROR_MSG(errorMsg, "'--ext:enable-an:force' option is not supported");
#else
    ASSERT_ERROR_MSG(errorMsg, "");
#endif  // PANDA_PRODUCT_BUILD
}

}  // namespace ark::ets::ani::testing
