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

#ifndef CODIRA_CODECHECK_CODELINT_H
#define CODIRA_CODECHECK_CODELINT_H

#include <string>

namespace CodeLint {
/**
 * All Params used in the lint tool, i.e.
 * srcFileDir: Detected file directory, it can be absolute path or relative path,
 *             if it is directory, default file name is codeReport
 * modulesDir: Directory path where the modules directory is located,
 *             it can be absolute path or relative path to the executable file
 * excludeRule: Excluded files, directories or configurations, splitted by ':'.
 *              Regular expressions are supported
 * configFileDir: Directory path where the config directory is located,
 *                it can be absolute path or relative path to the
 * executable file reportFormat: Report file format, it can be csv or json, default is json
 * reportFile: Output file path, it can be absolute path or relative path
 */
struct ParamsInCODELint {
    std::string srcFileDir;
    std::string modulesDir;
    std::string excludeRule;
    std::string configFileDir;
    std::string reportFormat;
    std::string reportFile;
    std::string codeoPath;
};

void PrintHelp(void);
void PrintVersion(void);
int CODELint(const ParamsInCODELint &strsInCODELint);
int CODELint(const ParamsInCODELint& strsInCODELint, const char** envp);
}
#endif
