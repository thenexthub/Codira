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
#include "verification/config/config.h"
#include "verification/config/default/default_config.h"
#include "verification/config/options/method_options.h"
#include "verification/config/options/method_options_config.h"
#include "verification/config/process/config_process.h"
#include "verification/config/parse/config_parse.h"
#include "verification/config/options/msg_set_parser.h"

#include "verifier_messages.h"

#include "literal_parser.h"

#include "runtime/include/mem/panda_string.h"
#include "runtime/include/mem/panda_containers.h"

#include "libarkbase/utils/logger.h"

namespace ark::verifier::debug {

using ark::verifier::config::Section;

namespace {

using Literals = PandaVector<PandaString>;

auto GetNameHandler()
{
    using ark::parser::Action;

    return [](Action a, MessageSetContext &c, auto from, auto to) {
        if (a == Action::PARSED) {
            std::string_view name {from, static_cast<size_t>(to - from)};
            auto num = static_cast<size_t>(ark::verifier::StringToVerifierMessage(name));
            c.stack.push_back(std::make_pair(num, num));
        }
        return true;
    };
}

auto GetNumHandler()
{
    using ark::parser::Action;

    return [](Action a, MessageSetContext &c, auto from) {
        if (a == Action::PARSED) {
            auto num = static_cast<size_t>(std::strtol(from, nullptr, 0));
            c.stack.push_back(std::make_pair(num, num));
        }
        return true;
    };
}

auto GetRangeHandler()
{
    using ark::parser::Action;

    return [](Action a, MessageSetContext &c) {
        if (a == Action::PARSED) {
            auto numEnd = c.stack.back();
            c.stack.pop_back();
            auto numStart = c.stack.back();
            c.stack.pop_back();

            c.stack.push_back(std::make_pair(numStart.first, numEnd.first));
        }
        return true;
    };
}

auto GetItemHandler()
{
    using ark::parser::Action;

    return [](Action a, MessageSetContext &c) {
        if (a == Action::START) {
            c.stack.clear();
        }
        if (a == Action::PARSED) {
            auto range = c.stack.back();
            c.stack.pop_back();

            for (auto i = range.first; i <= range.second; ++i) {
                c.nums.insert(i);
            }
        }
        return true;
    };
}

const auto &MessageSetParser()
{
    using ark::parser::Parser;

    using P = Parser<MessageSetContext, const char, const char *>;
    using P1 = typename P::P;
    using P2 = typename P1::P;
    using P3 = typename P2::P;
    using P4 = typename P3::P;

    static const auto WS = P::OfCharset(" \t\r\n");
    static const auto COMMA = P1::OfCharset(",");
    static const auto DEC = P2::OfCharset("0123456789");

    static const auto NAME_HANDLER = GetNameHandler();
    static const auto NUM_HANDLER = GetNumHandler();

    static const auto RANGE_HANDLER = GetRangeHandler();

    static const auto ITEM_HANDLER = GetItemHandler();

    static const auto NAME = P3::OfCharset("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789") |=
        NAME_HANDLER;

    static const auto NUM = DEC |= NUM_HANDLER;

    static const auto MSG = NUM | NAME;

    static const auto MSG_RANGE = MSG >> ~WS >> P4::OfString("-") >> ~WS >> MSG |= RANGE_HANDLER;

    static const auto ITEM = ~WS >> ((MSG_RANGE | MSG) |= ITEM_HANDLER) >> ~WS >> ~COMMA;

    // clang-tidy bug, use lambda instead of ITEMS = *ITEM
    static const auto ITEMS = [](MessageSetContext &c, const char *&start, const char *end) {
        while (true) {
            auto saved = start;
            if (!ITEM(c, start, end)) {
                start = saved;
                break;
            }
        }
        return true;
    };

    return ITEMS;
}

bool ProcessSectionMsg(MethodOption::MsgClass msgClass, const PandaString &items, MethodOptions *options)
{
    MessageSetContext c;

    const char *start = items.c_str();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    if (!MessageSetParser()(c, start, items.c_str() + items.length())) {
        LOG(ERROR, VERIFIER) << "Unexpected set of messages: '" << items << "'";
        return false;
    }

    for (const auto msgNum : c.nums) {
        options->SetMsgClass(VerifierMessageIsValid, msgNum, msgClass);
    }

    return true;
}

bool ProcessSectionShow(const Literals &literals, MethodOptions *options)
{
    for (const auto &option : literals) {
        if (option == "context") {
            options->SetShow(MethodOption::InfoType::CONTEXT);
        } else if (option == "reg-changes") {
            options->SetShow(MethodOption::InfoType::REG_CHANGES);
        } else if (option == "cflow") {
            options->SetShow(MethodOption::InfoType::CFLOW);
        } else if (option == "jobfill") {
            options->SetShow(MethodOption::InfoType::JOBFILL);
        } else {
            LOG(ERROR, VERIFIER) << "Unexpected show option: '" << option << "'";
            return false;
        }
    }

    return true;
}

bool ProcessSectionUplevel(const Literals &uplevelOptions, const MethodOptionsConfig &allMethodOptions,
                           MethodOptions *options)
{
    for (const auto &uplevel : uplevelOptions) {
        if (!allMethodOptions.IsOptionsPresent(uplevel)) {
            LOG(ERROR, VERIFIER) << "Cannot find uplevel options: '" << uplevel << "'";
            return false;
        }
        options->AddUpLevel(allMethodOptions.GetOptions(uplevel));
    }

    return true;
}

bool ProcessSectionCheck(const Literals &checks, MethodOptions *options)
{
    auto &optionsCheck = options->Check();
    for (const auto &c : checks) {
        if (c == "cflow") {
            optionsCheck |= MethodOption::CheckType::CFLOW;
        } else if (c == "reg-usage") {
            optionsCheck |= MethodOption::CheckType::REG_USAGE;
        } else if (c == "resolve-id") {
            optionsCheck |= MethodOption::CheckType::RESOLVE_ID;
        } else if (c == "typing") {
            optionsCheck |= MethodOption::CheckType::TYPING;
        } else if (c == "absint") {
            optionsCheck |= MethodOption::CheckType::ABSINT;
        } else {
            LOG(ERROR, VERIFIER) << "Unexpected check type: '" << c << "'";
            return false;
        }
    }

    return true;
}

}  // namespace

static bool MethodOptionsProcessorProcessSection(const struct Section &s, const Section &section,
                                                 MethodOptionsConfig &allOptions, MethodOptions &options)
{
    const PandaString &name = s.name;
    PandaString items;
    for (const auto &item : s.items) {
        items += item;
        items += " ";
    }

    if (name == "error") {
        if (!ProcessSectionMsg(MethodOption::MsgClass::ERROR, items, &options)) {
            return false;
        }
    } else if (name == "warning") {
        if (!ProcessSectionMsg(MethodOption::MsgClass::WARNING, items, &options)) {
            return false;
        }
    } else if (name == "hidden") {
        if (!ProcessSectionMsg(MethodOption::MsgClass::HIDDEN, items, &options)) {
            return false;
        }
    } else {
        Literals literals;
        const char *start = items.c_str();
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if (!LiteralsParser()(literals, start, start + items.length())) {
            LOG(ERROR, VERIFIER) << "Failed to parse '" << name << "' under '" << section.name << "'";
            return false;
        }

        if (name == "show") {
            if (!ProcessSectionShow(literals, &options)) {
                return false;
            }
        } else if (name == "uplevel") {
            if (!ProcessSectionUplevel(literals, allOptions, &options)) {
                return false;
            }
        } else if (name == "check") {
            if (!ProcessSectionCheck(literals, &options)) {
                return false;
            }
        } else {
            LOG(ERROR, VERIFIER) << "Unexpected section name: '" << name << "' under '" << section.name << "'";
            return false;
        }
    }
    return true;
}

const auto &MethodOptionsProcessor()
{
    static const auto PROCESS_METHOD_OPTIONS = [](Config *cfg, const Section &section) {
        MethodOptionsConfig &allOptions = cfg->opts.debug.GetMethodOptions();
        MethodOptions &options = allOptions.NewOptions(section.name);

        for (const auto &s : section.sections) {
            if (!MethodOptionsProcessorProcessSection(s, section, allOptions, options)) {
                return false;
            }
        }

        LOG(DEBUG, VERIFIER) << options.Image();

        return true;
    };

    return PROCESS_METHOD_OPTIONS;
}

void RegisterConfigHandlerMethodOptions(Config *dcfg)
{
    static const auto CONFIG_DEBUG_METHOD_OPTIONS_VERIFIER = [](Config *ddcfg, const Section &section) {
        bool defaultPresent = false;
        for (const auto &s : section.sections) {
            if (s.name == "default") {
                defaultPresent = true;
                break;
            }
        }
        if (!defaultPresent) {
            // take default section from inlined config
            Section cfg;
            if (!ParseConfig(ark::verifier::config::VERIFIER_DEBUG_DEFAULT_CONFIG, cfg)) {
                LOG(ERROR, VERIFIER) << "Cannot parse default verifier config";
                return false;
            }
            if (!MethodOptionsProcessor()(ddcfg, cfg["debug"]["method_options"]["verifier"]["default"])) {
                LOG(ERROR, VERIFIER) << "Cannot parse default method options";
                return false;
            }
        }
        for (const auto &s : section.sections) {
            if (!MethodOptionsProcessor()(ddcfg, s)) {
                LOG(ERROR, VERIFIER) << "Cannot parse section '" << s.name << "'";
                return false;
            }
        }
        return true;
    };

    config::RegisterConfigHandler(dcfg, "config.debug.method_options.verifier", CONFIG_DEBUG_METHOD_OPTIONS_VERIFIER);
}

void SetDefaultMethodOptions(Config *dcfg)
{
    auto &verifOptions = dcfg->opts;
    auto &options = verifOptions.debug.GetMethodOptions();
    if (!options.IsOptionsPresent("default")) {
        // take default section from inlined config
        Section cfg;
        if (!ParseConfig(ark::verifier::config::VERIFIER_DEBUG_DEFAULT_CONFIG, cfg)) {
            LOG(FATAL, VERIFIER) << "Cannot parse default internal config. Internal error.";
        }
        if (!MethodOptionsProcessor()(dcfg, cfg["debug"]["method_options"]["verifier"]["default"])) {
            LOG(FATAL, VERIFIER) << "Cannot parse default section";
        }
    }
}

}  // namespace ark::verifier::debug
