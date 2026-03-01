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

#include "verification/public_internal.h"
#include "verification/config/process/config_process.h"
#include "verification/util/parser/parser.h"

#include "verifier_messages.h"

#include "runtime/include/method.h"
#include "runtime/include/mem/panda_string.h"

#include "libarkbase/utils/logger.h"

#include <string>
#include <cstring>
#include <cstdint>

namespace ark::verifier::debug {

using ark::parser::Action;
using ark::parser::Parser;
using ark::verifier::config::Section;

namespace {

struct Context {
    PandaString method;
    std::vector<uint32_t> offsets;
};

}  // namespace

const auto &BreakpointParser()
{
    struct Breakpoint;
    using ark::parser::Charset;
    using P = Parser<Context, const char, const char *>::Next<Breakpoint>;
    using P1 = P::P;
    using P2 = P1::P;
    using P3 = P2::P;
    using P4 = P3::P;
    using P5 = P4::P;

    static const auto WS = P::OfCharset(" \t");
    static const auto COMMA = P1::OfString(",");
    static const auto DEC = P2::OfCharset("0123456789");
    static const auto HEX = P3::OfCharset("0123456789abcdefABCDEF");

    static const auto OFFSET_HANDLER = [](Action a, Context &c, auto from) {
        if (a == Action::PARSED) {
            c.offsets.push_back(std::strtol(from, nullptr, 0));
        }
        return true;
    };

    static const auto OFFSET = (P4::OfString("0x") >> HEX) | DEC |= OFFSET_HANDLER;

    static const auto METHOD_NAME_HANDLER = [](Action a, Context &c, auto from, auto to) {
        if (a == Action::PARSED) {
            c.method = PandaString {from, to};
        }
        return true;
    };

    static const auto BREAKPOINT_HANDLER = [](Action a, Context &c) {
        if (a == Action::START) {
            c.method.clear();
            c.offsets.clear();
        }
        return true;
    };

    static const auto METHOD_NAME = P5::OfCharset(!Charset {" \t,"}) |= METHOD_NAME_HANDLER;
    static const auto BREAKPOINT = (~WS >> METHOD_NAME >> *(~WS >> COMMA >> ~WS >> OFFSET) >> ~WS >> P::End()) |
                                   (~WS >> P::End()) |= BREAKPOINT_HANDLER;  // NOLINT
    return BREAKPOINT;
}

static bool RegisterConfigHandlerBreakpointsVerifierAnalyzer(const struct Section &s, Config *&cfg)
{
    for (const auto &i : s.items) {
        Context c;
        const char *start = i.c_str();
        const char *end = i.c_str() + i.length();  // NOLINT
        if (!BreakpointParser()(c, start, end)) {
            LOG_VERIFIER_DEBUG_BREAKPOINT_WRONG_CFG_LINE(i);
            return false;
        }
        if (!c.method.empty()) {
            if (c.offsets.empty()) {
                c.offsets.push_back(0);
            }
            for (auto o : c.offsets) {
                cfg->debugCfg.AddBreakpointConfig(c.method, o);
            }
        }
    }
    return true;
}

void RegisterConfigHandlerBreakpoints(Config *dcfg)
{
    static const auto CONFIG_DEBUG_BREAKPOINTS = [](Config *cfg, const Section &section) {
        for (const auto &s : section.sections) {
            if (s.name == "verifier" && !RegisterConfigHandlerBreakpointsVerifierAnalyzer(s, cfg)) {
                return false;
            }
            if (s.name != "verifier") {
                LOG_VERIFIER_DEBUG_BREAKPOINT_WRONG_SECTION(s.name);
                return false;
            }
        }
        return true;
    };

    config::RegisterConfigHandler(dcfg, "config.debug.breakpoints", CONFIG_DEBUG_BREAKPOINTS);
}

}  // namespace ark::verifier::debug
