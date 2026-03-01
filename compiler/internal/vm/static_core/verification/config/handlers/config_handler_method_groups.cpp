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
#include "verification/config/options/method_group_parser.h"
#include "verification/util/parser/parser.h"

#include "verifier_messages.h"

#include "runtime/include/mem/panda_string.h"

#include "literal_parser.h"

#include "runtime/include/method.h"
#include "runtime/include/runtime.h"

#include "libarkbase/utils/logger.h"

#include <string>
#include <cstring>
#include <cstdint>
#include <unordered_map>

#include <type_traits>

namespace ark::verifier::debug {

using ark::parser::Parser;
using ark::verifier::config::Section;

void RegisterConfigHandlerMethodGroups(Config *dcfg)
{
    static const auto CONFIG_DEBUG_METHOD_GROUPS_VERIFIER_OPTIONS = [](Config *cfg, const Section &section) {
        auto &verifOptions = cfg->opts;

        for (const auto &item : section.items) {
            struct Context {
                PandaString group;
                PandaString options;
            };

            using P = Parser<Context, const char, const char *>;
            const auto ws = P::OfCharset(" \t");

            const auto groupHandler = [](Context &c, PandaString &&group) {
                c.group = std::move(group);
                return true;
            };

            const auto optionsHandler = [](Context &c, PandaString &&options) {
                c.options = std::move(options);
                return true;
            };

            const auto line =
                ~ws >> MethodGroupParser<P>(groupHandler) >> ws >> LiteralParser<P>(optionsHandler) >> ~ws >> P::End();

            const char *start = item.c_str();
            const char *end = item.c_str() + item.length();  // NOLINT

            Context ctx;

            if (!line(ctx, start, end)) {
                LOG(DEBUG, VERIFIER) << "  Error: cannot parse config line '" << item << "'";
                return false;
            }

            if (!verifOptions.debug.GetMethodOptions().AddOptionsForGroup(ctx.group, ctx.options)) {
                LOG(DEBUG, VERIFIER) << "  Error: cannot set options for method group '" << ctx.group << "', options '"
                                     << ctx.options << "'";
                return false;
            }

            LOG(DEBUG, VERIFIER) << "  Set options for method group '" << ctx.group << "' : '" << ctx.options << "'";
        }

        return true;
    };

    config::RegisterConfigHandler(dcfg, "config.debug.method_groups.verifier.options",
                                  CONFIG_DEBUG_METHOD_GROUPS_VERIFIER_OPTIONS);
}

}  // namespace ark::verifier::debug
