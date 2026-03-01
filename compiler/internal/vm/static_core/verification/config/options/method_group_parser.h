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

#ifndef PANDA_VERIFIER_DEBUG_METHOD_GROUP_PARSER_H_
#define PANDA_VERIFIER_DEBUG_METHOD_GROUP_PARSER_H_

#include "verification/util/parser/parser.h"
#include "runtime/include/mem/panda_string.h"

namespace ark::verifier {

template <typename Parser, typename RegexHandler>
const auto &MethodGroupParser(RegexHandler &regexHandler)
{
    using ark::parser::Action;
    using ark::parser::Charset;
    // using ark::parser::Parser;

    struct MethodGroup;

    using P = typename Parser::template Next<MethodGroup>;
    using P1 = typename P::P;

    static const auto QUOTE = P::OfString("'");
    static const auto NON_QUOTES = P1::OfCharset(!Charset("'"));

    static const auto REGEX_HANDLER = [&regexHandler](Action a, typename P::Ctx &c, auto from, auto to) {
        if (a == Action::PARSED) {
            auto *start = from;
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            ++start;
            auto *end = to;
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            --end;
            return regexHandler(c, PandaString {start, end});
        }
        return true;
    };

    static const auto REGEX = QUOTE >> NON_QUOTES >> QUOTE |= REGEX_HANDLER;

    return REGEX;
}

}  // namespace ark::verifier

#endif  // PANDA_VERIFIER_DEBUG_METHOD_GROUP_PARSER_H_
