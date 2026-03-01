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

#include "guard/guard.h"

#include "abckit_wrapper/file_view.h"

#include "args_parser.h"
#include "configuration/configuration_parser.h"
#include "logger.h"
#include "obfuscator/obfuscator.h"
#include "util/assert_util.h"

namespace {
void InitLogger(bool isDebug, const std::string &debugFilePath)
{
    ark::Logger::ComponentMask component_mask;
    component_mask.set(ark::Logger::Component::GUARD);
    component_mask.set(ark::Logger::Component::ABCKIT_WRAPPER);
    if (isDebug) {
        if (debugFilePath.empty()) {
            ark::Logger::InitializeStdLogging(ark::Logger::Level::DEBUG, component_mask);
        } else {
            ark::Logger::InitializeFileLogging(debugFilePath, ark::Logger::Level::DEBUG, component_mask);
        }
    } else {
        ark::Logger::InitializeStdLogging(ark::Logger::Level::ERROR, component_mask);
    }
}
}  // namespace

void ark::guard::ArkGuard::Run(int argc, const char **argv)
{
    ArgsParser argsParser;
    auto rc = argsParser.Parse(argc, argv);
    if (argsParser.IsHelp()) {
        return;
    }
    GUARD_ASSERT(!rc, ErrorCode::INVALID_PARAMS, "Parameter parsing failed: invalid arguments");

    InitLogger(argsParser.IsDebug(), argsParser.GetDebugFile());
    LOG_I << "param parse and log init success";

    ConfigurationParser configParser(argsParser.GetConfigFilePath());
    Configuration configuration;
    configParser.Parse(configuration);
    if (configuration.DisableObfuscation()) {
        LOG_I << "Obfuscation option is disabled";
        return;
    }
    LOG_I << "configuration file parse success";

    abckit_wrapper::FileView fileView;
    auto result = fileView.Init(configuration.GetAbcPath());
    GUARD_ASSERT(result != AbckitWrapperErrorCode::ERR_SUCCESS, ErrorCode::GENERIC_ERROR, "File View Init Failed");
    LOG_I << "original abc init success";

    Obfuscator obfuscator(configuration);
    GUARD_ASSERT(!obfuscator.Execute(fileView), ErrorCode::GENERIC_ERROR, "Obfuscated Failed");
    LOG_I << "obfuscator execute success";

    fileView.Save(configuration.GetObfAbcPath());
    LOG_I << "obf abc save success";
}