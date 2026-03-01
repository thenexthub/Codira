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

#include "args_parser.h"

#include <iostream>

#include "libarkbase/utils/pandargs.h"

namespace {
void PrintHelp(const std::string &helpInfo)
{
    std::stringstream ss;
    ss << "Usage:" << std::endl;
    ss << "guard [options] config-file-path" << std::endl;
    ss << "Supported options:" << std::endl;
    ss << helpInfo << std::endl;

    std::cerr << ss.str();
}

bool PrintHelpAndReturnFalse(ark::PandArgParser &parser)
{
    PrintHelp(parser.GetHelpString());
    parser.DisableTail();
    return false;
}
}  // namespace

bool ark::guard::ArgsParser::Parse(int argc, const char **argv)
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
        return PrintHelpAndReturnFalse(parser);
    }

    if (help.GetValue()) {
        isHelp_ = true;
        return PrintHelpAndReturnFalse(parser);
    }

    if (configFilePath.GetValue().empty()) {
        return PrintHelpAndReturnFalse(parser);
    }

    configFilePath_ = configFilePath.GetValue();
    isDebug_ = debug.GetValue();
    debugFile_ = debugFile.GetValue();

    parser.DisableTail();
    return true;
}

const std::string &ark::guard::ArgsParser::GetConfigFilePath() const
{
    return configFilePath_;
}

bool ark::guard::ArgsParser::IsDebug() const
{
    return isDebug_;
}

bool ark::guard::ArgsParser::IsHelp() const
{
    return isHelp_;
}

const std::string &ark::guard::ArgsParser::GetDebugFile() const
{
    return debugFile_;
}