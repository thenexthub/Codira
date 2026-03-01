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

#include "libarkbase/utils/logger.h"
#include "libarkbase/utils/pandargs.h"
#include "libarkbase/generated/ark_version.h"
#include <libarkfile/include/file_format_version.h>

// NOLINTBEGIN(misc-non-private-member-variables-in-classes)
struct Options {
    ark::PandArg<bool> help {"help", false, "Print this message and exit"};
    ark::PandArg<bool> verbose {"verbose", false, "enable informative code output"};
    ark::PandArg<bool> quiet {"quiet", false, "enables all of the --skip-* flags"};
    ark::PandArg<bool> skipStrings {
        "skip-string-literals", false,
        "replaces string literals with their respectie id's, thus shortening emitted code size"};
    ark::PandArg<bool> withSeparators {"with_separators", false,
                                       "adds comments that separate sections in the output file"};
    ark::PandArg<bool> debug {
        "debug", false, "enable debug messages (will be printed to standard output if no --debug-file was specified) "};
    ark::PandArg<std::string> debugFile {"debug-file", "",
                                         "(--debug-file FILENAME) set debug file name. default is std::cout"};
    ark::PandArg<std::string> inputFile {"input_file", "", "Path to the source binary code"};
    ark::PandArg<std::string> outputFile {"output_file", "", "Path to the generated assembly code"};
    ark::PandArg<bool> version {"version", false,
                                "Ark version, file format version and minimum supported file format version"};

    explicit Options(ark::PandArgParser &paParser)
    {
        paParser.Add(&help);
        paParser.Add(&verbose);
        paParser.Add(&quiet);
        paParser.Add(&skipStrings);
        paParser.Add(&debug);
        paParser.Add(&debugFile);
        paParser.Add(&version);
        paParser.PushBackTail(&inputFile);
        paParser.PushBackTail(&outputFile);
        paParser.EnableTail();
    }
};
// NOLINTEND(misc-non-private-member-variables-in-classes)

static void PrintHelp(ark::PandArgParser &paParser)
{
    std::cerr << "Usage:" << std::endl;
    std::cerr << "ark_disasm [options] input_file output_file" << std::endl << std::endl;
    std::cerr << "Supported options:" << std::endl << std::endl;
    std::cerr << paParser.GetHelpString() << std::endl;
}

static void Disassemble(const Options &options)
{
    auto inputFile = options.inputFile.GetValue();
    LOG(DEBUG, DISASSEMBLER) << "[initializing disassembler]\nfile: " << inputFile << "\n";

    ark::disasm::Disassembler disasm {};
    disasm.Disassemble(inputFile, options.quiet.GetValue(), options.skipStrings.GetValue());
    auto verbose = options.verbose.GetValue();
    if (verbose) {
        disasm.CollectInfo();
    }
    LOG(DEBUG, DISASSEMBLER) << "[serializing results]\n";

    std::ofstream resPa;
    resPa.open(options.outputFile.GetValue(), std::ios::trunc | std::ios::out);
    disasm.Serialize(resPa, options.withSeparators.GetValue(), verbose);
    resPa.close();
}

static bool ProcessArgs(ark::PandArgParser &paParser, const Options &options, int argc, const char **argv)
{
    if (!paParser.Parse(argc, argv)) {
        PrintHelp(paParser);
        return false;
    }

    if (options.version.GetValue()) {
        ark::PrintPandaVersion();
        ark::panda_file::PrintBytecodeVersion();
        return false;
    }

    if (options.inputFile.GetValue().empty() || options.outputFile.GetValue().empty() || options.help.GetValue()) {
        PrintHelp(paParser);
        return false;
    }

    if (options.debug.GetValue()) {
        auto debugFile = options.debugFile.GetValue();
        if (debugFile.empty()) {
            ark::Logger::InitializeStdLogging(ark::Logger::Level::DEBUG, ark::Logger::ComponentMask()
                                                                             .set(ark::Logger::Component::DISASSEMBLER)
                                                                             .set(ark::Logger::Component::PANDAFILE));
        } else {
            ark::Logger::InitializeFileLogging(debugFile, ark::Logger::Level::DEBUG,
                                               ark::Logger::ComponentMask()
                                                   .set(ark::Logger::Component::DISASSEMBLER)
                                                   .set(ark::Logger::Component::PANDAFILE));
        }
    } else {
        ark::Logger::InitializeStdLogging(ark::Logger::Level::ERROR, ark::Logger::ComponentMask()
                                                                         .set(ark::Logger::Component::DISASSEMBLER)
                                                                         .set(ark::Logger::Component::PANDAFILE));
    }

    return true;
}

int main(int argc, const char **argv)
{
    ark::PandArgParser paParser;
    Options options {paParser};

    if (!ProcessArgs(paParser, options, argc, argv)) {
        return 1;
    }

    Disassemble(options);

    paParser.DisableTail();

    return 0;
}
