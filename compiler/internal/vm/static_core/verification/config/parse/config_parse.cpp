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

#include "verification/util/parser/parser.h"

#include "config_parse.h"

#include "runtime/include/mem/panda_string.h"

#include <cctype>
#include <vector>

namespace ark::verifier::config {

using ark::parser::Action;
using ark::parser::Parser;
using ark::verifier::config::Section;

namespace {

struct Context {
    Section current;
    std::vector<Section> sections;
};

}  // namespace

using P = ark::parser::Parser<Context, const char, const char *>;

static bool IsComment(const PandaString &s)
{
    for (char const c : s) {
        if (c == '#') {
            return true;
        }
        if (isspace(c) != 0) {
            return false;
        }
    }
    return false;
}

static bool HandleLine(Action a, Context &c, PandaString &&s)
{
    if (a == Action::PARSED) {
        if (!IsComment(s)) {
            c.current.items.push_back(std::move(s));
        }
    }
    return true;
}

static bool HandleSection(Action a, Context &c)
{
    if (a == Action::START) {
        c.sections.push_back(c.current);
        c.current.sections.clear();
    }
    if (a == Action::CANCEL) {
        c.current = c.sections.back();
        c.sections.pop_back();
    }
    if (a == Action::PARSED) {
        c.sections.back().sections.push_back(c.current);
        c.current = c.sections.back();
        c.sections.pop_back();
    }
    return true;
}

bool ParseConfig(const char *str, Section &cfg)
{
    using ark::parser::Charset;
    using P1 = P::P;
    using P2 = P1::P;
    using P3 = P2::P;
    using P4 = P3::P;
    using P5 = P4::P;
    using P6 = P5::P;

    static const auto CM = P::P::SkipComment("#");
    static const auto WS = ~CM >> P::OfCharset(" \t\r\n");
    static const auto NL = ~CM >> P1::OfCharset("\r\n");
    static const auto SP = P2::OfCharset(" \t");
    static const auto NAME_HANDLER = [](auto a, Context &c, auto from, auto to) {
        if (a == Action::PARSED) {
            c.current.name = PandaString {from, to};
        }
        return true;
    };
    static const auto NAME = P3::OfCharset("abcdefghijklmnopqrstuvwxyz_") |= NAME_HANDLER;

    static const auto LCURL = P4::OfString("{");
    static const auto RCURL = P5::OfString("}");

    static const auto LINE_HANDLER = [](auto a, Context &c, auto from, auto to) {
        return HandleLine(a, c, PandaString {from, to});
    };

    static const auto LINE = P6::OfCharset(!Charset {"\r\n"}) |= LINE_HANDLER;

    static const auto SECTION_END = ~SP >> RCURL >> ~SP >> NL;
    static const auto SECTION_START = ~SP >> NAME >> ~SP >> LCURL >> ~SP >> NL;
    static const auto ITEM = (!SECTION_END) & (~SP >> LINE >> NL);

    // CC-OFFNXT(G.NAM.03) false positive
    static const auto SECTION_HANDLER = [](auto a, Context &c) { return HandleSection(a, c); };

    static P::P sectionRec;

    static const auto SECTION = ~WS >> SECTION_START >> ~WS >> *sectionRec >> *ITEM >> SECTION_END >> ~WS |=
        SECTION_HANDLER;  // NOLINT

    sectionRec = SECTION;

    Context context;

    context.current.name = "config";

    if (SECTION(context, str, &str[std::strlen(str)])) {  // NOLINT
        cfg = context.current;
        return true;
    }

    return false;
}

}  // namespace ark::verifier::config
