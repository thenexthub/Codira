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

#include <iostream>

#include "quick.h"

#include "libarkbase/utils/logger.h"
#include "libarkbase/utils/pandargs.h"
#include "libarkfile/file_writer.h"

static void PrintHelp(ark::PandArgParser &pa_parser)
{
    std::cerr << "Usage:" << std::endl;
    std::cerr << "arkquicker [options] INPUT_FILE OUTPUT_FILE" << std::endl << std::endl;
    std::cerr << "Supported options:" << std::endl << std::endl;
    std::cerr << pa_parser.GetHelpString() << std::endl;
}

static bool ParseArgs(ark::PandArgParser &pa_parser, int argc, const char **argv)
{
    if (!pa_parser.Parse(argc, argv)) {
        PrintHelp(pa_parser);
        return false;
    }

    return true;
}

static bool ProcessArgs(ark::PandArgParser &pa_parser, const ark::PandArg<std::string> &input,
                        const ark::PandArg<std::string> &output, const ark::PandArg<bool> &help)
{
    if (input.GetValue().empty() || output.GetValue().empty() || help.GetValue()) {
        PrintHelp(pa_parser);
        return false;
    }

    ark::Logger::InitializeStdLogging(
        ark::Logger::Level::ERROR,
        ark::Logger::ComponentMask().set(ark::Logger::Component::QUICKENER).set(ark::Logger::Component::PANDAFILE));

    return true;
}

int main(int argc, const char **argv)
{
    ark::PandArg<bool> help("help", false, "Print this message and exit");
    ark::PandArg<std::string> input("INPUT", "", "Path to the input binary file");
    ark::PandArg<std::string> output("OUTPUT", "", "Path to the output binary file");

    ark::PandArgParser pa_parser;

    pa_parser.Add(&help);
    pa_parser.PushBackTail(&input);
    pa_parser.PushBackTail(&output);
    pa_parser.EnableTail();

    if (!ParseArgs(pa_parser, argc, argv)) {
        return 1;
    }

    if (!ProcessArgs(pa_parser, input, output, help)) {
        return 1;
    }

    auto input_file = ark::panda_file::File::Open(input.GetValue());
    if (!input_file) {
        LOG(ERROR, QUICKENER) << "Cannot open file '" << input.GetValue() << "'";
        return 1;
    }
    ark::panda_file::FileReader reader(std::move(input_file));
    if (!reader.ReadContainer()) {
        LOG(ERROR, QUICKENER) << "Cannot read container";
        return 1;
    }

    ark::panda_file::ItemContainer *container = reader.GetContainerPtr();

    ark::quick::Quickener quickener(container, const_cast<ark::panda_file::File *>(reader.GetFilePtr()),
                                    reader.GetItems());
    quickener.QuickContainer();

    auto writer = ark::panda_file::FileWriter(output.GetValue());
    if (!writer) {
        PLOG(ERROR, QUICKENER) << "Cannot create file writer with path '" << output.GetValue() << "'";
        return 1;
    }

    if (!container->Write(&writer, false)) {
        PLOG(ERROR, QUICKENER) << "Cannot write panda file '" << output.GetValue() << "'";
        return 1;
    }

    return 0;
}
