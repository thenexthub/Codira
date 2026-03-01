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

#include "util/tests/environment.h"

#include "util/parser/parser.h"

#include <cstring>
#include <charconv>

namespace ark::verifier::test {

// CC-OFFNXT(G.FUN.01, huge_method) solid logic
EnvOptions::EnvOptions(const char *envVarName)
{
    using ark::parser::Action;
    using ark::parser::Charset;
    using ark::parser::Parser;

    struct Context {
        std::string name;
        OptionValue value;
    };

    using Par = Parser<Context, const char, const char *>::Next<EnvOptions>;

    static const auto WS = Par::OfCharset(" \t\r\n");  // NOLINT(readability-static-accessed-through-instance)
    static const auto DELIM = Par::OfString(";");      // NOLINT(readability-static-accessed-through-instance)
    static const auto NAME_HANDLER = [](auto a, Context &c, auto s, auto e, [[maybe_unused]] auto end) {
        if (a == Action::PARSED) {
            c.name = std::string {s, e};
        }
        return true;
    };
    static const auto NAME =
        WS.OfCharset("abcdefghijklmnopqrstuvwxyz_")  // NOLINT(readability-static-accessed-through-instance)
        |= NAME_HANDLER;
    static const auto EQ = NAME.OfString("=");                   // NOLINT(readability-static-accessed-through-instance)
    static const auto BOOL_TRUE = EQ.OfString("true");           // NOLINT(readability-static-accessed-through-instance)
    static const auto BOOL_FALSE = BOOL_TRUE.OfString("false");  // NOLINT(readability-static-accessed-through-instance)
    static const auto BOOL_HANDLER = [](auto a, Context &c, auto s, [[maybe_unused]] auto to,
                                        [[maybe_unused]] auto end) {
        if (a == Action::PARSED) {
            if (*s == 'f') {
                c.value = false;
            } else {
                c.value = true;
            }
        }
        return true;
    };
    static const auto BOOL = BOOL_FALSE | BOOL_TRUE |= BOOL_HANDLER;
    static const auto DEC = BOOL.OfCharset("0123456789");  // NOLINT(readability-static-accessed-through-instance)
    static const auto HEX = DEC.OfString("0x") >> DEC;     // NOLINT(readability-static-accessed-through-instance)
    static const auto NUM_HANDLER = [](auto a, Context &c, auto s, auto e, [[maybe_unused]] auto end) {
        if (a == Action::PARSED) {
            int tempValue = 0;
            std::string value = std::string {s, e};
            std::from_chars(value.data(), &(*value.end()), tempValue, 0);
            c.value = tempValue;
        }
        return true;
    };
    static const auto NUM = HEX | DEC |= NUM_HANDLER;
    static const auto QUOTES = HEX.OfString("\"");  // NOLINT(readability-static-accessed-through-instance)
    static const auto NON_QUOTES =
        QUOTES.OfCharset(!Charset("\""));  // NOLINT(readability-static-accessed-through-instance)
    static const auto STRING_HANDLER = [](auto a, Context &c, auto s, auto e, [[maybe_unused]] auto end) {
        if (a == Action::PARSED) {
            c.value = std::string {s, e};
        }
        return true;
    };
    static const auto STRING = QUOTES >> (*NON_QUOTES |= STRING_HANDLER) >> QUOTES;

    static const auto VALUE = STRING | NUM | BOOL;

    static const auto KV_PAIR_HANDLER = [this](auto a, Context &c, [[maybe_unused]] auto f, [[maybe_unused]] auto t,
                                               [[maybe_unused]] auto e) {
        if (a == Action::PARSED) {
            options_[c.name] = c.value;
        }
        return true;
    };

    static const auto KV_PAIR = ~WS >> NAME >> ~WS >> EQ >> ~WS >> VALUE >> ~WS >> DELIM |= KV_PAIR_HANDLER;
    static const auto OPTIONS = *KV_PAIR;

    const char *s = std::getenv(envVarName);
    if (s == nullptr) {
        return;
    }

    Context c;

    if (!OPTIONS(c, s, s + strlen(s))) {  // NOLINT
        // NOTE(vdyadov): warning that some options were not parsed
    }
}

std::optional<OptionValue> EnvOptions::operator[](const std::string &name) const
{
    auto it = options_.find(name);
    if (it != options_.end()) {
        return it->second;
    }
    return {};
}

}  // namespace ark::verifier::test
