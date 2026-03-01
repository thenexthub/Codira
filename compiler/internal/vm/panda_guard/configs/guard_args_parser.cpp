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

#include "guard_args_parser.h"

#include "utils/logger.h"
#include "utils/pandargs.h"

bool panda::guard::GuardArgsParser::Parse(int argc, const char **argv)
{
    PandArg help("help", false, "Print this message and exit");
    PandArg<bool> debug("debug", false,
                        "enable debug messages (will be printed to standard output if no --debug-file was specified)");
    PandArg<std::string> debugFile("debug-file", "",
                                   "(--debug-file FILENAME) set debug file name. default is std::cout");
    PandArg<std::string> configFilePath("config-file-path", "", "configuration file path");

    PandArgParser parser;
    parser.Add(&help);
    parser.Add(&debug);
    parser.Add(&debugFile);
    parser.PushBackTail(&configFilePath);
    parser.EnableTail();
    if (!parser.Parse(argc, argv)) {
        PrintErrorMsg(parser.GetHelpString());
        parser.DisableTail();
        return false;
    }

    Logger::ComponentMask component_mask;
    component_mask.set(Logger::Component::PANDAGUARD);
    component_mask.set(Logger::Component::ABC2PROGRAM);
    component_mask.set(Logger::Component::ASSEMBLER);
    if (debug.GetValue()) {
        debugMode_ = true;
        if (debugFile.GetValue().empty()) {
            Logger::InitializeStdLogging(Logger::Level::DEBUG, component_mask);
        } else {
            Logger::InitializeFileLogging(debugFile.GetValue(), Logger::Level::DEBUG, component_mask);
        }
    } else {
        Logger::InitializeStdLogging(Logger::Level::ERROR, component_mask);
    }

    configFilePath_ = configFilePath.GetValue();
    if (configFilePath_.empty()) {
        std::cerr << "The config-file-path value is empty. Please check if the config-file-path is set correctly."
                  << std::endl;
        parser.DisableTail();
        return false;
    }
    parser.DisableTail();
    return true;
}

void panda::guard::GuardArgsParser::PrintErrorMsg(const std::string &helpInfo)
{
    std::stringstream ss;
    ss << "Usage:" << std::endl;
    ss << "panda_guard [options] config-file-path" << std::endl;
    ss << "Supported options:" << std::endl;
    ss << helpInfo << std::endl;

    std::cerr << ss.str();
}

const std::string &panda::guard::GuardArgsParser::GetConfigFilePath() const
{
    return configFilePath_;
}

bool panda::guard::GuardArgsParser::IsDebugMode() const
{
    return debugMode_;
}
