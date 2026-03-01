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

#include "abc2program_options.h"

namespace panda::abc2program {

bool Abc2ProgramOptions::Parse(int argc, const char **argv)
{
    panda::PandArg<bool> help("help", false, "Print this message and exit");
    panda::PandArg<bool> debug(
        "debug", false, "enable debug messages (will be printed to standard output if no --debug-file was specified) ");
    panda::PandArg<std::string> debug_file("debug-file", "",
                                           "(--debug-file FILENAME) set debug file name. default is std::cout");
    panda::PandArg<std::string> input_file("input_file", "", "Path to the source binary code");
    panda::PandArg<std::string> output_file("output_file", "", "Path to the generated assembly code");
    pa_parser_.Add(&help);
    pa_parser_.Add(&debug);
    pa_parser_.Add(&debug_file);
    pa_parser_.PushBackTail(&input_file);
    pa_parser_.PushBackTail(&output_file);
    pa_parser_.EnableTail();

    if (!pa_parser_.Parse(argc, argv)) {
        ConstructErrorMsg();
        PrintErrorMsg();
        pa_parser_.DisableTail();
        return false;
    }
    if (debug.GetValue()) {
        if (debug_file.GetValue().empty()) {
            panda::Logger::InitializeStdLogging(
                panda::Logger::Level::DEBUG,
                panda::Logger::ComponentMask().set(panda::Logger::Component::ABC2PROGRAM));
        } else {
            panda::Logger::InitializeFileLogging(
                debug_file.GetValue(), panda::Logger::Level::DEBUG,
                panda::Logger::ComponentMask().set(panda::Logger::Component::ABC2PROGRAM));
        }
    } else {
        panda::Logger::InitializeStdLogging(panda::Logger::Level::ERROR,
                                            panda::Logger::ComponentMask().set(panda::Logger::Component::ABC2PROGRAM));
    }
    input_file_path_ = input_file.GetValue();
    output_file_path_ = output_file.GetValue();
    if (input_file_path_.empty() || output_file_path_.empty()) {
        ConstructErrorMsg();
        PrintErrorMsg();
        pa_parser_.DisableTail();
        return false;
    }

    pa_parser_.DisableTail();
    return true;
}

void Abc2ProgramOptions::ConstructErrorMsg()
{
    std::stringstream ss;
    ss << "Usage:" << std::endl;
    ss << "abc2prog [options] input_file output_file" << std::endl;
    ss << "Supported options:" << std::endl;
    ss << pa_parser_.GetHelpString() << std::endl;
    error_msg_ = ss.str();
}

const std::string &Abc2ProgramOptions::GetInputFilePath() const
{
    return input_file_path_;
}

const std::string &Abc2ProgramOptions::GetOutputFilePath() const
{
    return output_file_path_;
}

void Abc2ProgramOptions::PrintErrorMsg() const
{
    std::cerr << error_msg_;
}

}  // namespace panda::abc2program
