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

#ifndef LSPSERVER_MULTIMODULECOMMON_H
#define LSPSERVER_MULTIMODULECOMMON_H

#include <string>
#include <sstream>
#include <unordered_map>
#include <regex>

namespace ark {
enum class CodiraFileKind {
    MISSING,
    IN_OLD_PACKAGE,
    IN_NEW_PACKAGE,
    IN_PROJECT_NOT_IN_SOURCE
};

struct ModuleInfo {
    std::string moduleName;
    std::string modulePath;
    std::unordered_map<std::string, std::string> codeoRequiresMap;
    std::string srcPath;
};

std::string GetLSPServerDir();

std::string GetFileContents(const std::string &fileName);

void GetMacroLibPath(const std::string &targetLib,
                     const std::unordered_map<std::string, ModuleInfo> &moduleInfoMap,
                     std::vector<std::string> &macroLibs);
 
std::string GetCodecPath(const std::string &oldPath);

std::string GetModulesHome(const std::string &modulesHomeOption, const std::string &environmentHome);
} // namespace ark

#endif // LSPSERVER_MULTIMODULECOMMON_H
