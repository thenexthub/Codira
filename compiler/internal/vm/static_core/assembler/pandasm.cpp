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

#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "libarkbase/generated/ark_version.h"

#include "assembly-emitter.h"
#include "assembly-parser.h"
#ifdef PANDA_WITH_BYTECODE_OPTIMIZER
#include "bytecode_optimizer/optimize_bytecode.h"
#endif
#include <libarkfile/include/file_format_version.h>
#include "error.h"
#include "lexer.h"
#include "pandasm.h"
#include "libarkbase/utils/expected.h"
#include "libarkbase/utils/logger.h"
#include "libarkbase/utils/pandargs.h"

namespace ark::pandasm {

void PrintError(const ark::pandasm::Error &e, const std::string &msg)
{
    std::stringstream sos;
    std::cerr << msg << ": " << e.message << std::endl;
    sos << "      Line " << e.lineNumber << ", Column " << (e.pos + 1) << ": ";
    std::cerr << sos.str() << e.wholeLine << std::endl;
    std::cerr << std::setw(static_cast<int>(e.pos + sos.str().size()) + 1) << "^" << std::endl;
}

void PrintErrors(const ark::pandasm::ErrorList &warnings, const std::string &msg)
{
    for (const auto &iter : warnings) {
        PrintError(iter, msg);
    }
}

void PrintHelp(const ark::PandArgParser &paParser)
{
    std::cerr << "Usage:" << std::endl;
    std::cerr << "pandasm [OPTIONS] INPUT_FILE OUTPUT_FILE" << std::endl << std::endl;
    std::cerr << "Supported options:" << std::endl << std::endl;
    std::cerr << paParser.GetHelpString() << std::endl;
}

// CC-OFFNXT(G.FUN.01-CPP) solid logic
bool PrepareArgs(ark::PandArgParser &paParser, const ark::PandArg<std::string> &inputFile,
                 const ark::PandArg<std::string> &outputFile, const ark::PandArg<std::string> &logFile,
                 const ark::PandArg<bool> &help, const ark::PandArg<bool> &verbose, const ark::PandArg<bool> &version,
                 std::ifstream &inputfile, int argc, const char **argv)
{
    if (!paParser.Parse(argc, argv)) {
        PrintHelp(paParser);
        return false;
    }

    if (version.GetValue()) {
        ark::PrintPandaVersion();
        panda_file::PrintBytecodeVersion();
        return false;
    }

    if (inputFile.GetValue().empty() || outputFile.GetValue().empty() || help.GetValue()) {
        PrintHelp(paParser);
        return false;
    }

    if (verbose.GetValue()) {
        ark::Logger::ComponentMask componentMask;
        componentMask.set(ark::Logger::Component::ASSEMBLER);
        componentMask.set(ark::Logger::Component::BYTECODE_OPTIMIZER);
        if (logFile.GetValue().empty()) {
            ark::Logger::InitializeStdLogging(ark::Logger::Level::DEBUG, componentMask);
        } else {
            ark::Logger::InitializeFileLogging(logFile.GetValue(), ark::Logger::Level::DEBUG, componentMask);
        }
    }

    inputfile.open(inputFile.GetValue(), std::ifstream::in);

    if (!inputfile) {
        std::cerr << "The input file does not exist." << std::endl;
        return false;
    }

    return true;
}

bool Tokenize(ark::pandasm::Lexer &lexer, std::vector<std::vector<ark::pandasm::Token>> &tokens,
              std::ifstream &inputfile)
{
    std::string s;

    while (getline(inputfile, s)) {
        ark::pandasm::Tokens q = lexer.TokenizeString(s);

        auto e = q.second;

        if (e.err != ark::pandasm::Error::ErrorType::ERR_NONE) {
            e.lineNumber = tokens.size() + 1;
            PrintError(e, "ERROR");
            return false;
        }

        tokens.push_back(q.first);
    }

    return true;
}

bool ParseProgram(ark::pandasm::Parser &parser, std::vector<std::vector<ark::pandasm::Token>> &tokens,
                  const ark::PandArg<std::string> &inputFile,
                  ark::Expected<ark::pandasm::Program, ark::pandasm::Error> &res)
{
    res = parser.Parse(tokens, inputFile.GetValue());
    if (!res) {
        PrintError(res.Error(), "ERROR");
        return false;
    }

    return true;
}

bool DumpProgramInJson(ark::pandasm::Program &program, const ark::PandArg<std::string> &scopesFile)
{
    if (!scopesFile.GetValue().empty()) {
        std::ofstream dumpFile;
        dumpFile.open(scopesFile.GetValue());

        if (!dumpFile) {
            std::cerr << "Cannot write scopes into the given file." << std::endl;
            return false;
        }
        dumpFile << program.JsonDump();
    }

    return true;
}

bool EmitProgramInBinary(ark::pandasm::Program &program, ark::PandArgParser &paParser,
                         const ark::PandArg<std::string> &outputFile, ark::PandArg<bool> &optimize,
                         ark::PandArg<bool> &sizeStat)
{
    auto emitDebugInfo = !optimize.GetValue();
    std::map<std::string, size_t> stat;
    std::map<std::string, size_t> *statp = sizeStat.GetValue() ? &stat : nullptr;
    ark::pandasm::AsmEmitter::PandaFileToPandaAsmMaps maps {};
    ark::pandasm::AsmEmitter::PandaFileToPandaAsmMaps *mapsp = optimize.GetValue() ? &maps : nullptr;

    if (!ark::pandasm::AsmEmitter::Emit(outputFile.GetValue(), program, statp, mapsp, emitDebugInfo)) {
        std::cerr << "Failed to emit binary data: " << ark::pandasm::AsmEmitter::GetLastError() << std::endl;
        return false;
    }

#ifdef PANDA_WITH_BYTECODE_OPTIMIZER
    if (optimize.GetValue()) {
        bool isOptimized = ark::bytecodeopt::OptimizeBytecode(&program, mapsp, outputFile.GetValue());
        if (!ark::pandasm::AsmEmitter::Emit(outputFile.GetValue(), program, statp, mapsp, emitDebugInfo)) {
            std::cerr << "Failed to emit binary data: " << ark::pandasm::AsmEmitter::GetLastError() << std::endl;
            return false;
        }
        if (!isOptimized) {
            std::cerr << "Bytecode optimizer reported internal errors" << std::endl;
            return false;
        }
    }
#endif

    if (sizeStat.GetValue()) {
        size_t totalSize = 0;
        std::cout << "Panda file size statistic:" << std::endl;

        for (auto [name, size] : stat) {
            std::cout << name << " section: " << size << std::endl;
            totalSize += size;
        }

        std::cout << "total: " << totalSize << std::endl;
    }

    paParser.DisableTail();

    return true;
}

// CC-OFFNXT(G.FUN.01-CPP) solid logic
bool BuildFiles(ark::pandasm::Program &program, ark::PandArgParser &paParser,
                const ark::PandArg<std::string> &outputFile, ark::PandArg<bool> &optimize, ark::PandArg<bool> &sizeStat,
                ark::PandArg<std::string> &scopesFile)
{
    if (!DumpProgramInJson(program, scopesFile)) {
        return false;
    }

    if (!EmitProgramInBinary(program, paParser, outputFile, optimize, sizeStat)) {
        return false;
    }

    return true;
}

}  // namespace ark::pandasm

int main(int argc, const char *argv[])
{
    ark::PandArg<bool> verbose("verbose", false, "Enable verbose output (will be printed to standard output)");
    ark::PandArg<std::string> logFile("log-file", "", "(--log-file FILENAME) Set log file name");
    ark::PandArg<std::string> scopesFile("dump-scopes", "", "(--dump-scopes FILENAME) Enable dump of scopes to file");
    ark::PandArg<bool> help("help", false, "Print this message and exit");
    ark::PandArg<bool> sizeStat("size-stat", false, "Print panda file size statistic");
    ark::PandArg<bool> optimize("optimize", false, "Run the bytecode optimization");
    ark::PandArg<bool> version {"version", false,
                                "Ark version, file format version and minimum supported file format version"};
    // tail arguments
    ark::PandArg<std::string> inputFile("INPUT_FILE", "", "Path to the source assembly code");
    ark::PandArg<std::string> outputFile("OUTPUT_FILE", "", "Path to the generated binary code");
    ark::PandArgParser paParser;
    paParser.Add(&verbose);
    paParser.Add(&help);
    paParser.Add(&logFile);
    paParser.Add(&scopesFile);
    paParser.Add(&sizeStat);
    paParser.Add(&optimize);
    paParser.Add(&version);
    paParser.PushBackTail(&inputFile);
    paParser.PushBackTail(&outputFile);
    paParser.EnableTail();

    std::ifstream inputfile;

    if (!ark::pandasm::PrepareArgs(paParser, inputFile, outputFile, logFile, help, verbose, version, inputfile, argc,
                                   argv)) {
        return 1;
    }

    LOG(DEBUG, ASSEMBLER) << "Lexical analysis:";

    ark::pandasm::Lexer lexer;

    std::vector<std::vector<ark::pandasm::Token>> tokens;

    if (!Tokenize(lexer, tokens, inputfile)) {
        return 1;
    }

    LOG(DEBUG, ASSEMBLER) << "parsing:";

    ark::pandasm::Parser parser;

    ark::Expected<ark::pandasm::Program, ark::pandasm::Error> res;
    if (!ark::pandasm::ParseProgram(parser, tokens, inputFile, res)) {
        return 1;
    }

    auto &program = res.Value();

    auto w = parser.ShowWarnings();
    if (!w.empty()) {
        ark::pandasm::PrintErrors(w, "WARNING");
    }

    if (!ark::pandasm::BuildFiles(program, paParser, outputFile, optimize, sizeStat, scopesFile)) {
        return 1;
    }

    return 0;
}
