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
#include "verification/config/context/context.h"
#include "verification/config/process/config_process.h"
#include "verification/util/parser/parser.h"

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

const auto &WhitelistMethodParser()
{
    struct Whitelist;

    using ark::parser::Charset;
    using P = Parser<PandaString, const char, const char *>::Next<Whitelist>;
    using P1 = P::P;

    static const auto WS = P::OfCharset(" \t");

    static const auto METHOD_NAME_HANDLER = [](Action a, PandaString &c, auto from, auto to) {
        if (a == Action::PARSED) {
            c = PandaString {from, to};
        }
        return true;
    };

    static const auto METHOD_NAME = P1::OfCharset(!Charset {" \t,"}) |= METHOD_NAME_HANDLER;  // NOLINT

    static const auto WHITELIST_METHOD = (~WS >> METHOD_NAME >> ~WS >> P::End()) | (~WS >> P::End());

    return WHITELIST_METHOD;
}

static bool RegisterConfigHandlerWhitelistSectionHandler(Config *config, const struct Section &s)
{
    WhitelistKind kind;
    if (s.name == "class") {
        kind = WhitelistKind::CLASS;
    } else if (s.name == "method") {
        kind = WhitelistKind::METHOD;
    } else if (s.name == "method_call") {
        kind = WhitelistKind::METHOD_CALL;
    } else {
        LOG(DEBUG, VERIFIER) << "Wrong debug verifier whitelist section: '" << s.name << "'";
        return false;
    }
    for (const auto &i : s.items) {
        PandaString c;
        const char *start = i.c_str();
        const char *end = i.c_str() + i.length();  // NOLINT
        if (!WhitelistMethodParser()(c, start, end)) {
            LOG(DEBUG, VERIFIER) << "Wrong whitelist line: '" << i << "'";
            return false;
        }
        if (!c.empty()) {
            if (kind == WhitelistKind::CLASS) {
                LOG(DEBUG, VERIFIER) << "Added to whitelist config '" << s.name << "' methods from class " << c;
            } else {
                LOG(DEBUG, VERIFIER) << "Added to whitelist config '" << s.name << "' methods named " << c;
            }
            config->debugCfg.AddWhitelistMethodConfig(kind, c);
        }
    }
    return true;
}

void RegisterConfigHandlerWhitelist(Config *dcfg)
{
    static const auto CONFIG_DEBUG_WHITELIST_VERIFIER = [](Config *config, const Section &section) {
        for (const auto &s : section.sections) {
            if (!RegisterConfigHandlerWhitelistSectionHandler(config, s)) {
                return false;
            }
        }
        return true;
    };

    config::RegisterConfigHandler(dcfg, "config.debug.whitelist.verifier", CONFIG_DEBUG_WHITELIST_VERIFIER);
}

}  // namespace ark::verifier::debug
