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

#include "disassembler.h"

#include "utils/logger.h"
#include "utils/pandargs.h"
#include "ark_version.h"
#include "file_format_version.h"

void PrintHelp(panda::PandArgParser &pa_parser)
{
    std::cerr << "Usage:" << std::endl;
    std::cerr << "ark_disasm [options] input_file output_file" << std::endl << std::endl;
    std::cerr << "Supported options:" << std::endl << std::endl;
    std::cerr << pa_parser.GetHelpString() << std::endl;
}

void Disassemble(const std::string &input_file, const std::string &output_file, const bool verbose, const bool quiet,
                 const bool skip_strings)
{
    LOG(DEBUG, DISASSEMBLER) << "[initializing disassembler]\nfile: " << input_file << "\n";

    panda::disasm::Disassembler disasm {};
    disasm.Disassemble(input_file, quiet, skip_strings);
    if (verbose) {
        disasm.CollectInfo();
    }

    LOG(DEBUG, DISASSEMBLER) << "[serializing results]\n";

    std::ofstream res_pa;
    res_pa.open(output_file, std::ios::trunc | std::ios::out);
    disasm.Serialize(res_pa, true, verbose);
    res_pa.close();
}

bool ProcessArgs(panda::PandArgParser &pa_parser, const panda::PandArg<std::string> &input_file,
                 const panda::PandArg<std::string> &output_file, panda::PandArg<bool> &debug,
                 const panda::PandArg<std::string> &debug_file, const panda::PandArg<bool> &help,
                 const panda::PandArg<bool> &version, int argc, const char **argv)
{
    if (!pa_parser.Parse(argc, argv)) {
        PrintHelp(pa_parser);
        return false;
    }

    if (version.GetValue()) {
        panda::PrintPandaVersion();
        panda::panda_file::PrintBytecodeVersion();
        return false;
    }

    if (input_file.GetValue().empty() || output_file.GetValue().empty() || help.GetValue()) {
        PrintHelp(pa_parser);
        return false;
    }

    panda::Logger::ComponentMask log_mask = panda::Logger::ComponentMask().set(panda::Logger::Component::DISASSEMBLER)
                                                                          .set(panda::Logger::Component::PANDAFILE);
    if (debug.GetValue()) {
        if (debug_file.GetValue().empty()) {
            panda::Logger::InitializeStdLogging(
                panda::Logger::Level::DEBUG, log_mask);
        } else {
            panda::Logger::InitializeFileLogging(
                debug_file.GetValue(), panda::Logger::Level::DEBUG, log_mask);
        }
    } else {
        panda::Logger::InitializeStdLogging(panda::Logger::Level::ERROR, log_mask);
    }

    return true;
}

int main(int argc, const char **argv)
{
    panda::PandArg<bool> help("help", false, "Print this message and exit");
    panda::PandArg<bool> verbose("verbose", false, "enable informative code output");
    panda::PandArg<bool> quiet("quiet", false, "enables all of the --skip-* flags");
    panda::PandArg<bool> skip_strings(
        "skip-string-literals", false,
        "replaces string literals with their respectie id's, thus shortening emitted code size");
    panda::PandArg<bool> debug(
        "debug", false, "enable debug messages (will be printed to standard output if no --debug-file was specified) ");
    panda::PandArg<std::string> debug_file("debug-file", "",
                                           "(--debug-file FILENAME) set debug file name. default is std::cout");
    panda::PandArg<std::string> input_file("input_file", "", "Path to the source binary code");
    panda::PandArg<std::string> output_file("output_file", "", "Path to the generated assembly code");
    panda::PandArg<bool> version {"version", false,
                                  "Ark version, file format version and minimum supported file format version"};

    panda::PandArgParser pa_parser;

    pa_parser.Add(&help);
    pa_parser.Add(&verbose);
    pa_parser.Add(&quiet);
    pa_parser.Add(&skip_strings);
    pa_parser.Add(&debug);
    pa_parser.Add(&debug_file);
    pa_parser.Add(&version);
    pa_parser.PushBackTail(&input_file);
    pa_parser.PushBackTail(&output_file);
    pa_parser.EnableTail();

    if (!ProcessArgs(pa_parser, input_file, output_file, debug, debug_file, help, version, argc, argv)) {
        return 1;
    }

    Disassemble(input_file.GetValue(), output_file.GetValue(), verbose.GetValue(), quiet.GetValue(),
                skip_strings.GetValue());

    pa_parser.DisableTail();

    return 0;
}
