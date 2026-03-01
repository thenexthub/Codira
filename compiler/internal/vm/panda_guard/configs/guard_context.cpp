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

#include "guard_context.h"

#include "utils/logger.h"

#include "generator/order_name_generator.h"
#include "guard_args_parser.h"
#include "util/assert_util.h"
#include "version.h"

namespace {
constexpr std::string_view TAG = "[Guard_Context]";
// The reserved fields here are to ensure basic confusion operation in scenarios where the fronted is not configured
const std::set<std::string> RESERVED_NAMES = {"prototype", "default", "*default*"};
/* When the path is split, there will be/ xxx.ts, Even some paths can be written as/ sss/../../xx.ts, This is a folder
 * name that needs to be preserved */
const std::set<std::string> RESERVED_FILE_NAMES = {".", ".."};
}  // namespace

std::shared_ptr<panda::guard::GuardContext> panda::guard::GuardContext::GetInstance()
{
    static auto instance = std::make_shared<GuardContext>();
    return instance;
}

void panda::guard::GuardContext::Init(int argc, const char **argv)
{
    GuardArgsParser parser;
    PANDA_GUARD_ASSERT_PRINT(!parser.Parse(argc, argv), TAG, ErrorCode::COMMAND_PARAMS_PARSE_ERROR,
                             "obfuscate command param parse failed");

    LOG(INFO, PANDAGUARD) << TAG << GUARD_DYNAMIC_VERSION << ":" << OBFUSCATION_TOOL_VERSION;
    LOG(INFO, PANDAGUARD) << TAG << "[param parse success], config file:" << parser.GetConfigFilePath();
    debugMode_ = parser.IsDebugMode();

    options_ = std::make_shared<GuardOptions>();
    options_->Load(parser.GetConfigFilePath());
    LOG(INFO, PANDAGUARD) << TAG << "[load config file success]";

    nameCache_ = std::make_shared<NameCache>(options_);
    std::string applyNameCachePath =
        options_->GetApplyNameCache().empty() ? options_->GetDefaultNameCachePath() : options_->GetApplyNameCache();
    nameCache_->Load(applyNameCachePath);

    nameMapping_ = std::make_shared<NameMapping>(nameCache_, options_);
    nameMapping_->AddNameMapping(RESERVED_NAMES);
    nameMapping_->AddFileNameMapping(RESERVED_FILE_NAMES);
    LOG(INFO, PANDAGUARD) << TAG << "[name mapping init success]";
}

void panda::guard::GuardContext::Finalize()
{
    graphContext_->Finalize();
}

const std::shared_ptr<panda::guard::GuardOptions> &panda::guard::GuardContext::GetGuardOptions() const
{
    return options_;
}

const std::shared_ptr<panda::guard::NameCache> &panda::guard::GuardContext::GetNameCache() const
{
    return nameCache_;
}

const std::shared_ptr<panda::guard::NameMapping> &panda::guard::GuardContext::GetNameMapping() const
{
    return nameMapping_;
}

void panda::guard::GuardContext::CreateGraphContext(const panda_file::File &file)
{
    graphContext_ = std::make_shared<GraphContext>(file);
    graphContext_->Init();
}

const std::shared_ptr<panda::guard::GraphContext> &panda::guard::GuardContext::GetGraphContext() const
{
    return graphContext_;
}

bool panda::guard::GuardContext::IsDebugMode() const
{
    return debugMode_;
}
