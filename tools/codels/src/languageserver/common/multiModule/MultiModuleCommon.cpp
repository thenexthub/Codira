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

#include "MultiModuleCommon.h"
#include "Codira/Utils/FileUtil.h"
#include "../../logger/Logger.h"
#include "../Utils.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif

using namespace Codira;
using namespace CONSTANTS;
using namespace Codira::FileUtil;

namespace {
bool startsWith(const std::string& str, const std::string prefix)
{
    return (str.rfind(prefix, 0) == 0);
}
}

namespace ark {
#ifdef _WIN32
const std::string MACRO_DETAIL = "dll";
#elif __APPLE__
const std::string MACRO_DETAIL = "dylib";
#else
const std::string MACRO_DETAIL = "so";
#endif
const std::unordered_set<std::string> IGNORE_DYNAMIC = {"libstdx.fuzz.fuzz"};

std::string GetLSPServerDir()
{
    char path[PATH_MAX] = {0};
#ifdef _WIN32
    if (GetModuleFileName(nullptr, path, sizeof(path)) == 0) {
        return "";
    }
#else
    if (readlink("/proc/self/exe", path, sizeof(path)) == 0) {
        return "";
    }
#endif
    std::string dirPath(path);
    return GetDirPath(dirPath);
}

std::string GetFileContents(const std::string &fileName)
{
    std::string failedReason;
    auto contents = ReadFileContent(fileName, failedReason);
    if (!failedReason.empty()) {
        Logger::Instance().LogMessage(MessageType::MSG_WARNING,
                                      "error to read form file:" + fileName + " reason:" + failedReason);
        // file is empty, buffer is nullptr, not need delete
        return "";
    }
    if (!contents) {
        return "";
    }
    return *contents;
}

void GetMacroLibPath(const std::string &targetLib,
                     const std::unordered_map<std::string, ModuleInfo> &moduleInfoMap,
                     std::vector<std::string> &macroLibs)
{
    std::string soPath;
    std::string fullName;
    std::unordered_set<std::string> soRecordMap;
    std::unordered_set<std::string> folderRecordMap;
    for (const auto &item : moduleInfoMap) {
        const std::string soFolderPath = JoinPath(targetLib, item.second.moduleName);
        if (!FileExist(soFolderPath) || folderRecordMap.find(soFolderPath) != folderRecordMap.end()) {
            continue;
        }
        for (const auto &file : GetAllFilesUnderCurrentPath(soFolderPath, MACRO_DETAIL)) {
            soPath = JoinPath(soFolderPath, file);
            fullName = LSPJoinPath(GetDirName(soFolderPath), file);
            macroLibs.emplace_back(soPath);
            (void)soRecordMap.emplace(fullName);
        }
        (void)folderRecordMap.emplace(soFolderPath);
    }
    for (const auto &item : moduleInfoMap) {
        for (auto &path : item.second.codeoRequiresMap) {
            const std::string soDirPath = GetDirPath(path.second);
            if (!FileExist(soDirPath) || folderRecordMap.find(soDirPath) != folderRecordMap.end()) {
                continue;
            }
            for (auto &file : GetAllFilesUnderCurrentPath(soDirPath, MACRO_DETAIL)) {
                if (!startsWith(file, "lib-macro_")) {
                    continue;
                }
                soPath = JoinPath(soDirPath, file);
                fullName = LSPJoinPath(GetDirName(soDirPath), file);
                std::string dynamicName = GetFileNameWithoutExtension(soPath);
                if (soRecordMap.find(fullName) == soRecordMap.end() &&
                    IGNORE_DYNAMIC.find(dynamicName) == IGNORE_DYNAMIC.end()) {
                    macroLibs.emplace_back(soPath);
                    (void)soRecordMap.emplace(fullName);
                }
            }
            (void)folderRecordMap.emplace(soDirPath);
        }
    }
} // namespace ark
 
std::string GetCodecPath(const std::string &oldPath)
{
    std::string path = ark::PathWindowsToLinux(Normalize(oldPath));
    std::regex pathRegex("/runtime/lib");
    std::vector<std::string> vectorString(std::sregex_token_iterator(path.begin(), path.end(), pathRegex, -1),
        std::sregex_token_iterator());
    path = vectorString[0];
    std::string codecPath;
    const std::string codecBinPath = "bin/";
    codecPath = JoinPath(path, codecBinPath);
    Logger::Instance().LogMessage(MessageType::MSG_INFO, "codecPath is " + codecPath);
    return codecPath;
}

// modulesHomeOption > LSPServerDir > environment
std::string GetModulesHome(const std::string &modulesHomeOption, const std::string &environmentHome)
{
    Logger &logger = Logger::Instance();
    std::stringstream log;
    std::string modules = JoinPath(modulesHomeOption, "modules");
    if (!modulesHomeOption.empty() && FileExist(modules)) {
        CleanAndLog(log, "Load modules from (modulesHomeOption):" + modulesHomeOption);
        logger.LogMessage(MessageType::MSG_INFO, log.str());
        return modulesHomeOption;
    }
    std::string lspServerHome = GetLSPServerDir();
    modules = JoinPath(lspServerHome, "modules");
    if (!lspServerHome.empty() && FileExist(modules)) {
        CleanAndLog(log, "Load modules from (LSPServerDir):" + lspServerHome);
        logger.LogMessage(MessageType::MSG_INFO, log.str());
        return lspServerHome;
    }
    modules = JoinPath(environmentHome, "modules");
    if (!environmentHome.empty() && FileExist(modules)) {
        CleanAndLog(log, "Load modules from (environment):" + environmentHome);
        logger.LogMessage(MessageType::MSG_INFO, log.str());
        return environmentHome;
    }
    CleanAndLog(log, "Load modules fail");
    logger.LogMessage(MessageType::MSG_ERROR, log.str());
    return "";
}
}
