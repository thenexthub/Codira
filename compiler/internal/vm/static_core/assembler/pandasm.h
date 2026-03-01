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

#ifndef PANDA_ASSEMBLER_PANDASM_H
#define PANDA_ASSEMBLER_PANDASM_H

#include "libarkbase/utils/pandargs.h"

namespace ark::pandasm {

void PrintError(const ark::pandasm::Error &e, const std::string &msg);

void PrintErrors(const ark::pandasm::ErrorList &warnings, const std::string &msg);

// CC-OFFNXT(G.FUN.01-CPP) solid logic
bool PrepareArgs(ark::PandArgParser &paParser, const ark::PandArg<std::string> &inputFile,
                 const ark::PandArg<std::string> &outputFile, const ark::PandArg<std::string> &logFile,
                 const ark::PandArg<bool> &help, const ark::PandArg<bool> &verbose, std::ifstream &inputfile, int argc,
                 char **argv);

bool Tokenize(ark::pandasm::Lexer &lexer, std::vector<std::vector<ark::pandasm::Token>> &tokens,
              std::ifstream &inputfile);

bool ParseProgram(ark::pandasm::Parser &parser, std::vector<std::vector<ark::pandasm::Token>> &tokens,
                  const ark::PandArg<std::string> &inputFile,
                  ark::Expected<ark::pandasm::Program, ark::pandasm::Error> &res);

bool DumpProgramInJson(ark::pandasm::Program &program, const ark::PandArg<std::string> &scopesFile);

bool EmitProgramInBinary(ark::pandasm::Program &program, ark::PandArgParser &paParser,
                         const ark::PandArg<std::string> &outputFile, ark::PandArg<bool> &optimize,
                         ark::PandArg<bool> &sizeStat);

// CC-OFFNXT(G.FUN.01-CPP) solid logic
bool BuildFiles(ark::pandasm::Program &program, ark::PandArgParser &paParser,
                const ark::PandArg<std::string> &outputFile, ark::PandArg<bool> &optimize, ark::PandArg<bool> &sizeStat,
                ark::PandArg<std::string> &scopesFile);

}  // namespace ark::pandasm

#endif  // PANDA_ASSEMBLER_PANDASM_H
