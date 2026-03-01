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

#include "verify.h"

#include "utils/pandargs.h"

void PrintHelp(panda::PandArgParser &pa_parser)
{
    std::cerr << "Usage:" << std::endl;
    std::cerr << "ark_verifier [options] input_file" << std::endl;
    std::cerr << "Supported options:" << std::endl;
    std::cerr << pa_parser.GetHelpString() << std::endl;
}

bool PorcessArgs(panda::PandArgParser &pa_parser, const panda::PandArg<std::string> &input_file, int argc,
                 const char **argv)
{
    if (!pa_parser.Parse(argc, argv)) {
        PrintHelp(pa_parser);
        return false;
    }

    if (input_file.GetValue().empty()) {
        PrintHelp(pa_parser);
        return false;
    }

    return true;
}

int main(int argc, const char **argv)
{
    panda::PandArg<bool> help("help", false, "Print this message and exit");
    panda::PandArg<std::string> input_file("input_file", "", "Path to the abc file");

    panda::PandArgParser pa_parser;
    pa_parser.Add(&help);
    pa_parser.Add(&input_file);

    if (!PorcessArgs(pa_parser, input_file, argc, argv)) {
        return 1;
    }

    return Verify(input_file.GetValue()) ? 0 : 1;
}