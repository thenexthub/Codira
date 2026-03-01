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

/**
 * @file
 *
 * This file implements the FrontendOptions.
 */

#include "Codira/Frontend/FrontendOptions.h"

#define OPTION_TRUE_ACTION(EXPR) [](FrontendOptions& opts, OptionArgInstance&) { (EXPR); return true; }

using namespace Codira;

namespace {
std::unordered_map<Options::ID, std::function<bool(FrontendOptions&, OptionArgInstance&)>> g_actions = {
    {Options::ID::DUMP_TOKENS, OPTION_TRUE_ACTION(opts.dumpAction = FrontendOptions::DumpAction::DUMP_TOKENS)},
    {Options::ID::DUMP_SYMBOLS, OPTION_TRUE_ACTION(opts.dumpAction = FrontendOptions::DumpAction::DUMP_SYMBOLS)},
    {Options::ID::TYPE_CHECK, OPTION_TRUE_ACTION(opts.dumpAction = FrontendOptions::DumpAction::TYPE_CHECK)},
    // DUMP_DEPENDENT_PACKAGE (--scan-dependency option) should be handled with extra care. The behavior of
    // the option has defined in GlobalOptions. Since, we are overriding its behavior here, we need to
    // set `scanDepPkg` field again.
    {Options::ID::DUMP_DEPENDENT_PACKAGE,
        [](FrontendOptions& opts, OptionArgInstance& /* arg */) {
            opts.scanDepPkg = true;
            opts.dumpAction = FrontendOptions::DumpAction::DUMP_DEP_PKG;
            return true;
        }},
    {Options::ID::DESERIALIZE_CHIR_AND_DUMP,
        [](FrontendOptions& opts, [[maybe_unused]] const OptionArgInstance& arg) {
            opts.chirDeserialize = true;
            opts.dumpCHIR = true;
            opts.chirDeserializePath = arg.value;
            opts.dumpAction = FrontendOptions::DumpAction::DESERIALIZE_CHIR;
            return true;
        }},
    {Options::ID::COMPILE_CODED, OPTION_TRUE_ACTION(opts.compileCoded = true)},
};
}

std::optional<bool> FrontendOptions::ParseOption(OptionArgInstance& arg)
{
    if (g_actions.find(arg.info.GetID()) == g_actions.end()) {
        return GlobalOptions::ParseOption(arg);
    }
    return {g_actions[arg.info.GetID()](*this, arg)};
}
