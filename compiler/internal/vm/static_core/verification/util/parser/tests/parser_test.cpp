/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
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

#include <string>

#include <gtest/gtest.h>

#include "util/parser/parser.h"

namespace ark::parser::test {
namespace {
struct Context {};
struct smth;

// NOLINTNEXTLINE (cppcoreguidelines-interfaces-global-init)
const auto FOO = [](Action act, [[maybe_unused]] Context &, auto &it) {
    switch (act) {
        case Action::START:
            if (*it != 'f') {
                return false;
            }
            ++it;
            if (*it != 'o') {
                return false;
            }
            ++it;
            if (*it != 'o') {
                return false;
            }
            ++it;
            return true;

        case Action::CANCEL:
            return true;

        case Action::PARSED:
            return true;
        default:
            UNREACHABLE();
            return false;
    }
};

// NOLINTNEXTLINE (cppcoreguidelines-interfaces-global-init)
const auto BAR = [](Action act, [[maybe_unused]] Context &, auto &it) {
    switch (act) {
        case Action::START:
            if (*it != 'b') {
                return false;
            }
            ++it;
            if (*it != 'a') {
                return false;
            }
            ++it;
            if (*it != 'r') {
                return false;
            }
            ++it;
            return true;

        case Action::CANCEL:
            return true;

        case Action::PARSED:
            return true;
        default:
            UNREACHABLE();
            return false;
    }
};

using P = typename Parser<Context, const char, const char *>::template Next<smth>;
using P1 = typename P::P;
using P2 = typename P1::P;
using P3 = typename P2::P;
using P4 = typename P3::P;
using P5 = typename P4::P;
using P6 = typename P5::P;

using It = const char *;
}  // namespace

// NOLINTBEGIN(fuchsia-statically-constructed-objects,readability-identifier-naming)
const std::string aBc("aBc");
const std::string string("string");
const std::string d("d");
const std::string acstring("ACstring");
const std::string fooAcB("fooAcB");
const auto ABCP = P::OfCharset(Charset("abcABC"));
const auto DEFP = P1::OfCharset(Charset("defDEF"));
const auto STRINGP = P2::OfString("string");
const auto ENDP = P3::End();
// NOLINTEND(fuchsia-statically-constructed-objects,readability-identifier-naming)

// NOLINTBEGIN(readability-magic-numbers)
static void VerifierParserTest1(Context &cont, It &start, It &end)
{
    start = &(string[0U]);
    end = &(string[0U]);
    EXPECT_TRUE(ENDP(cont, start, end));
    end = &(string[2U]);
    EXPECT_FALSE(ENDP(cont, start, end));

    static const auto ACSTRINGP = ~ABCP >> STRINGP;
    start = &(string[0U]);
    end = &(string[6U]);
    EXPECT_TRUE(ACSTRINGP(cont, start, end));
    start = &(acstring[0U]);
    end = &(acstring[8U]);
    EXPECT_TRUE(ACSTRINGP(cont, start, end));
    end = &(acstring[7U]);
    EXPECT_FALSE(ACSTRINGP(cont, start, end));

    static const auto FOOABCP = ABCP |= FOO;
    static const auto BARABCP = ABCP |= BAR;
    start = &(string[0U]);
    end = &(string[6U]);
    EXPECT_FALSE(FOOABCP(cont, start, end));
    start = &(fooAcB[0U]);
    end = &(fooAcB[6U]);
    EXPECT_TRUE(FOOABCP(cont, start, end));
    start = &(fooAcB[0U]);
    EXPECT_FALSE(BARABCP(cont, start, end));

    static const auto ABCDEFP = ABCP | DEFP;
    start = &(aBc[0U]);
    end = &(aBc[3U]);
    EXPECT_TRUE(ABCDEFP(cont, start, end));
    start = &(string[0U]);
    end = &(string[6U]);
    EXPECT_FALSE(ABCDEFP(cont, start, end));
    start = &(d[0U]);
    end = &(d[1U]);
    EXPECT_TRUE(ABCDEFP(cont, start, end));
}

static void VerifierParserTest2(Context &cont, It &start, It &end)
{
    static const auto EMPTYP = ABCP & DEFP;
    start = &(aBc[0U]);
    end = &(aBc[3U]);
    EXPECT_FALSE(EMPTYP(cont, start, end));
    start = &(string[0U]);
    end = &(string[6U]);
    EXPECT_FALSE(EMPTYP(cont, start, end));
    start = &(d[0U]);
    end = &(d[1U]);
    EXPECT_FALSE(EMPTYP(cont, start, end));

    static const auto ABC2P = ABCP << STRINGP >> STRINGP;
    start = &(acstring[0U]);
    end = &(acstring[8U]);
    EXPECT_TRUE(ABC2P(cont, start, end));
    start = &(string[0U]);
    end = &(string[6U]);
    EXPECT_FALSE(ABC2P(cont, start, end));
    start = &(d[0U]);
    end = &(d[1U]);
    EXPECT_FALSE(ABC2P(cont, start, end));

    static const auto NOABCP = !ABCP;
    start = &(aBc[0U]);
    end = &(aBc[3U]);
    EXPECT_FALSE(NOABCP(cont, start, end));
    start = &(string[0U]);
    end = &(string[6U]);
    EXPECT_TRUE(NOABCP(cont, start, end));
    start = &(d[0U]);
    end = &(d[1U]);
    EXPECT_TRUE(NOABCP(cont, start, end));

    static const auto STRINGSTRINGENDP = *STRINGP >> ENDP;
    static const auto STRINGENDP = STRINGP >> ENDP;
    std::string stringstring {"stringstring"};
    start = &(stringstring[0U]);
    end = &(stringstring[12U]);
    EXPECT_FALSE(STRINGENDP(cont, start, end));
    start = &(stringstring[0U]);
    EXPECT_TRUE(STRINGSTRINGENDP(cont, start, end));
}

TEST(VerifierParserTest, Parser)
{
    Context cont;
    It start = &(aBc[0U]);
    It end = &(aBc[3U]);

    EXPECT_TRUE(ABCP(cont, start, end));
    start = &(aBc[1U]);
    EXPECT_TRUE(ABCP(cont, start, end));
    start = &(aBc[0U]);
    EXPECT_FALSE(DEFP(cont, start, end));
    start = &(aBc[0U]);
    EXPECT_FALSE(STRINGP(cont, start, end));
    start = &(string[0U]);
    end = &(string[6U]);
    EXPECT_FALSE(ABCP(cont, start, end));
    start = &(string[0U]);
    EXPECT_FALSE(DEFP(cont, start, end));
    start = &(string[0U]);
    EXPECT_TRUE(STRINGP(cont, start, end));
    start = &(d[0U]);
    end = &(d[1U]);
    EXPECT_FALSE(ABCP(cont, start, end));
    start = &(d[0U]);
    EXPECT_TRUE(DEFP(cont, start, end));
    start = &(d[0U]);
    EXPECT_FALSE(STRINGP(cont, start, end));
    start = &(string[0U]);
    end = &(string[3U]);
    EXPECT_FALSE(ABCP(cont, start, end));
    start = &(string[0U]);
    EXPECT_FALSE(DEFP(cont, start, end));
    start = &(string[0U]);
    EXPECT_FALSE(STRINGP(cont, start, end));

    VerifierParserTest1(cont, start, end);
    VerifierParserTest2(cont, start, end);
}
// NOLINTEND(readability-magic-numbers)
}  // namespace ark::parser::test