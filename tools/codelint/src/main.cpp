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

#include <common/CommonFunc.h>
#include <regex>
#include <string>
#include <vector>
#include "Codelint.h"

using namespace Codira;
using namespace Codira::CodeCheck;
using namespace CodeLint;

static const std::unordered_map<std::string, std::function<void(ParamsInCODELint&, char*)>> optionMap = {
    {"-o", [](ParamsInCODELint& params, char* optarg) { params.reportFile = optarg; }},
    {"-r", [](ParamsInCODELint& params, char* optarg) { params.reportFormat = optarg; }},
    {"-c", [](ParamsInCODELint& params, char* optarg) { params.configFileDir = optarg; }},
    {"-f", [](ParamsInCODELint& params, char* optarg) { params.srcFileDir = optarg; }},
    {"-m", [](ParamsInCODELint& params, char* optarg) { params.modulesDir = optarg; }},
    {"-e", [](ParamsInCODELint& params, char* optarg) { params.excludeRule = optarg; }},
    {"--import-path", [](ParamsInCODELint& params, char* optarg) { params.codeoPath = optarg; }}};

int main(int argc, char **argv, const char **envp)
{
    if (argc == 1) {
        PrintHelp();
        return OK;
    }
    ParamsInCODELint params;
    int i = 1;
    while (i < argc) {
        std::string arg = argv[i];
        if (arg == "-v") {
            PrintVersion();
            return OK;
        }
        if (arg == "-h") {
            PrintHelp();
            return OK;
        }
        auto handleOption = optionMap.find(arg);
        if (handleOption == optionMap.end()) {
            Errorln("Illegal option: ", arg);
            Println("Try: 'codelint -h' for more information.");
            return ERR;
        }
        if (i + 1 >= argc || optionMap.find(argv[i + 1]) != optionMap.end()) {
            Errorln("Option that requires an argument: ", arg);
            return ERR;
        }
        handleOption->second(params, argv[i + 1]);
        // Jump to the next option ('2' means option and argument)
        i += 2;
    }
    int errCode = CODELint(params, envp);
    _exit(errCode);
    return OK;
}
