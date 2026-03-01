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

#include "Codelint.h"
#include <common/CommonFunc.h>
#include <regex>
#include <string>
#include <vector>
#include "Codira/Lex/Lexer.h"
#include "checker/Checker.h"

using namespace Codira;
using namespace Codira::CodeCheck;
using Json = nlohmann::json;

namespace CodeLint {
#ifdef _WIN32
constexpr const char* MODULES_BACKEND_PATH = "/modules/windows_x86_64_codenative/";
#else
#ifdef __APPLE__
#ifdef __aarch64__
constexpr const char* MODULES_BACKEND_PATH = "/modules/darwin_aarch64_codenative/";
#else
constexpr const char* MODULES_BACKEND_PATH = "/modules/darwin_x86_64_codenative/";
#endif
#else
#ifdef __ARM__
constexpr const char* MODULES_BACKEND_PATH = "/modules/linux_aarch64_codenative/";
#else
constexpr const char* MODULES_BACKEND_PATH = "/modules/linux_x86_64_codenative/";
#endif
#endif
#endif

constexpr const char* JSONPATH = "/config/exclude_lists.json";
std::vector<CodelintIgnoreInfo> ignoreInfos;
std::unordered_map<std::string, std::string> StringifyEnvironmentPointer(const char** envp)
{
    std::unordered_map<std::string, std::string> environmentVars;
    if (!envp) {
        return environmentVars;
    }
    // Read all environment variables
    for (size_t i = 0;; ++i) {
        if (!envp[i]) {
            break;
        }
        std::string item(envp[i]);
        if (auto pos = item.find('='); pos != std::string::npos) {
            (void)environmentVars.emplace(item.substr(0, pos), item.substr(pos + 1));
        };
    }
    return environmentVars;
}

/**
 * print codelint commit number
 * print dependent Codira version
 */
void PrintVersion()
{
#ifndef CODELINT_VERSION
    Errorln("Can not obtain codelint version");
#else
    Println(std::string("Codira Lint: ") + std::string(CODELINT_VERSION));
#endif

#ifndef CODEC_VERSION
    Errorln("Can not obtain codec version");
#else
    // CODEC_VERSION comes with "Codira Compiler" version
    Println(CODEC_VERSION);
#endif
}

void PrintHelp(void)
{
    Println("Usage: ");
    Println("       ./codelint -f fileDir [option] fileDir...");
    Println("Options:");
    Println("   -h                      Show usage");
    Println("                               eg: ./codelint -h");
    Println("   -v                      Show version");
    Println("                               eg: ./codelint -v");
    Println("   -f <value>              Detected file directory, it can be absolute path or relative path");
    Println("                               eg: ./codelint -f fileDir -c . -m .");
    Println("   -e <v1:v2:...>          Excluded files, directories or configurations, splitted by ':'. "
            "Regular expressions are supported");
    Println("                               eg: ./codelint -f fileDir -e fileDir/a/:fileDir/b/*.code");
    Println("   -o <value>              Output file path, it can be absolute path or relative path, "
            "if it is directory, default file name is codeReport");
    Println("                               eg: ./codelint -f fileDir -o ./out");
    Println("   -r [csv|json]           Report file format, it can be csv or json, default is json");
    Println("                               eg: ./codelint -f fileDir -r csv -o ./out");
    Println("   -c <value>              Directory path where the config directory is located, it can be absolute path "
            "or relative path to the executable file");
    Println("                               eg: ./codelint -f fileDir -c .");
    Println("   -m <value>              Directory path where the modules directory is located, it can be absolute path "
            "or relative path to the executable file");
    Println("                               eg: ./codelint -f fileDir -m .");
    Println("   --import-path <value>   Add .codeo search path");
}

static void GetCODEFilePath(const std::string& path, std::vector<std::string>& codePath)
{
    auto files = FileUtil::GetAllFilesUnderCurrentPath(path, "code", false);
    auto dirs = FileUtil::GetAllDirsUnderCurrentPath(path);
    for (auto& f : files) {
        codePath.emplace_back(FileUtil::JoinPath(path, f));
    }
    for (auto& d : dirs) {
        auto subFiles = FileUtil::GetAllFilesUnderCurrentPath(d, "code", false);
        for (auto& sf : subFiles) {
            codePath.emplace_back(FileUtil::JoinPath(d, sf));
        }
    }
}

static void SplitLine(const std::string& comment, const std::string& path, int linePos)
{
    auto lineSplit = Utils::SplitLines(comment);
    for (auto& line : lineSplit) {
        std::smatch sm;
        std::regex codelintIgnoreRegex("codelint-ignore(\\s+-start|\\s+-end)?(\\s+(!([^\\s(*/)])*))*|"
                                     "codelint-ignore\\s+(-start|-end)");
        std::regex codelintRules("!([A-Za-z\\.]+)[0-9]+");
        if (!std::regex_search(line, sm, codelintIgnoreRegex)) {
            continue;
        }
        CodelintIgnoreInfo codelintIgnoreInfo;
        codelintIgnoreInfo.path = path;
        codelintIgnoreInfo.pos = linePos;
        auto stringSplit = Utils::SplitString(sm.str(), " ");
        bool multiPosSetFlag = true;
        for (auto& str : stringSplit) {
            if (str == "-start" && multiPosSetFlag) {
                codelintIgnoreInfo.start = linePos;
                multiPosSetFlag = false;
            } else if (str == "-end" && multiPosSetFlag) {
                codelintIgnoreInfo.end = linePos;
                multiPosSetFlag = false;
            } else if (std::regex_match(str, codelintRules)) {
                codelintIgnoreInfo.rules.emplace_back(str.substr(1));
            }
        }
        ignoreInfos.emplace_back(codelintIgnoreInfo);
        break;
    }
}

static void SplitComments(const std::vector<Token>& comments, const std::string& path)
{
    for (auto& c : comments) {
        SplitLine(c.Value(), path, c.Begin().line);
    }
}

static void GetComments(std::vector<std::string>& codePaths)
{
    DiagnosticEngine diag;
    SourceManager sm;
    for (auto& p : codePaths) {
        std::ifstream instream(p);
        std::string buffer((std::istreambuf_iterator<char>(instream)), std::istreambuf_iterator<char>());
        instream.close();
        auto rawCode = buffer;
        auto fileID = sm.AddSource(p, rawCode);
        auto lexer = Lexer(fileID, rawCode, diag, sm);
        (void)lexer.GetTokens();
        auto comments = lexer.GetComments();
        SplitComments(comments, p);
    }
}

static void AnalyseComments(
    std::stack<CodelintIgnoreInfo>& multiCodelintIgnore, std::vector<CodelintIgnoreInfo>& finalCodelintIgoreInfos)
{
    for (auto& item : ignoreInfos) {
        if (item.start != 0) {
            multiCodelintIgnore.push(item);
            continue;
        }
        if (item.end != 0 && !multiCodelintIgnore.empty()) {
            auto info = multiCodelintIgnore.top();
            if (info.path == item.path && info.start < item.end) {
                info.end = item.end;
                finalCodelintIgoreInfos.emplace_back(info);
                multiCodelintIgnore.pop();
            }
            continue;
        }
        finalCodelintIgoreInfos.emplace_back(item);
    }
}

static int CheckCode()
{
    ConfigContext& configContext = ConfigContext::GetInstance();
    auto srcFileDirs = configContext.GetSrcFileDir();
    auto reportFile = configContext.GetReportFile();
    auto format = configContext.GetReportFormat();
    SourceManager sm;
    CodeCheckDiagnosticEngine diagEngine;
    diagEngine.SetSourceManager(&sm);
    Json jsonInfo;
    (void)CommonFunc::ReadJsonFileToJsonInfo(JSONPATH, configContext, jsonInfo);
    diagEngine.SetJsonInfo(jsonInfo);
    std::vector<std::string> codePaths;
    for (const auto& srcFileDir : srcFileDirs) {
        std::vector<std::string> codePathsTemp;
        GetCODEFilePath(srcFileDir, codePathsTemp);
        configContext.FilterCompileList(codePathsTemp);
        codePathsTemp = configContext.GetSrcFileList();
        codePaths.insert(codePaths.end(), codePathsTemp.begin(), codePathsTemp.end());
    }
    if (codePaths.empty()) {
        Errorln("There is no cangjie file or all cangjie files are excluded, "
                "please check file directory and exclude rules");
        return ERR;
    }
    configContext.SetSrcFileList(codePaths);
    GetComments(codePaths);
    std::stack<CodelintIgnoreInfo> multiCodelintIgnore;
    std::vector<CodelintIgnoreInfo> finalCodelintIgoreInfos;
    AnalyseComments(multiCodelintIgnore, finalCodelintIgoreInfos);
    diagEngine.SetCodelintIgnoreInfos(finalCodelintIgoreInfos);

    if (!reportFile.empty()) {
        diagEngine.SetReportToFile(reportFile, format);
    }
    std::string path = configContext.GetModulesDir();
    if (path.empty()) {
        path = configContext.GetCodiraHomePath();
        configContext.SetModulesDir(path);
    }
    if (access((path + MODULES_BACKEND_PATH).c_str(), R_OK) != 0) {
        Errorln("Can not find modules, please setup CODIRA_HOME or use -m option to setup modules path");
        return ERR;
    }

    auto checker = Checker(&diagEngine);
    auto res = checker.CheckCode();
    return res;
}

static int CheckAndSetCodiraHome(const char** envp)
{
    ConfigContext& configContext = ConfigContext::GetInstance();
    std::unordered_map<std::string, std::string> environmentVars = StringifyEnvironmentPointer(envp);
    if (environmentVars.find(CODIRA_HOME) == environmentVars.end()) {
        Errorln("Can not find CODIRA_HOME, please setup CODIRA_HOME");
        return ERR;
    }
    auto cangjieHome = FileUtil::GetAbsPath(environmentVars.at(CODIRA_HOME));
    if (!cangjieHome.has_value()) {
        Errorln("Can not find realpath of CODIRA_HOME, please setup CODIRA_HOME");
        return ERR;
    }
    configContext.SetCodiraHome(cangjieHome.value());
    return OK;
}

// Check if context initialization is correct.
static int CheckInitialContext()
{
    ConfigContext& configContext = ConfigContext::GetInstance();
    // Check src directory
    auto srcFileDirs = configContext.GetSrcFileDir();
    for (size_t index = 0; index < srcFileDirs.size(); index++) {
        auto srcFileDir = configContext.GetSrcFileDir()[index];
        if (srcFileDir.empty()) {
            Errorf("Action has no specific file to operate on.\n");
            CodeLint::PrintHelp();
            return ERR;
        }
        auto absFileDir = FileUtil::GetAbsPath(srcFileDir);
        if (absFileDir.has_value()) {
            configContext.SetSrcFileDir(index, absFileDir.value());
        }
    }
    // Check report file
    auto reportFile = configContext.GetReportFile();
    auto configFileDir = configContext.GetConfigFileDir();
    auto modulesDir = configContext.GetModulesDir();
    if (!reportFile.empty()) {
        if (reportFile == ".") {
            (void)reportFile.append("/codeReport");
        }
        if (reportFile.substr(reportFile.length() - 1, 1) == PATH_SEPARATOR) {
            (void)reportFile.append("codeReport");
        }
    }
    // Set report, config and module.
    std::optional<std::string> absReportFile, absConfigFileDir, absModulesDir;
    CommonFunc::getAbsPath(reportFile, absReportFile);
    configContext.SetReportFile(absReportFile.value());
    CommonFunc::getAbsPath(configFileDir, absConfigFileDir);
    configContext.SetConfigFileDir(absConfigFileDir.value());
    CommonFunc::getAbsPath(modulesDir, absModulesDir);
    configContext.SetModulesDir(absModulesDir.value());
    return OK;
}

// Check exclude rules
static int CheckExcludeContext(std::vector<std::string>& excludeInputList)
{
    ConfigContext& configContext = ConfigContext::GetInstance();
    for (auto& excludeInput : excludeInputList) {
        auto excludeFileList = Utils::SplitString(excludeInput, " ");
        for (auto& excludeFile : excludeFileList) {
            configContext.AddExcludeList(excludeFile);
        }
    }
    configContext.CheckDefaultExclusion();

    return OK;
}

static int CODELintHelper(const ParamsInCODELint& strsInCODELint)
{
    ConfigContext& configContext = ConfigContext::GetInstance();
    std::vector<std::string> excludeInputList;
    std::vector<std::string> codeoPathList;
    if (!strsInCODELint.reportFile.empty()) {
        configContext.SetReportFile(strsInCODELint.reportFile);
    }
    if (!strsInCODELint.reportFormat.empty() && !configContext.SetReportFormat(strsInCODELint.reportFormat)) {
        Errorln(strsInCODELint.reportFormat, " is not a supported report file format.");
        return ERR;
    }
    if (!strsInCODELint.configFileDir.empty()) {
        configContext.SetConfigFileDir(strsInCODELint.configFileDir);
    }
    if (!strsInCODELint.srcFileDir.empty()) {
        configContext.AddSrcFileDir(strsInCODELint.srcFileDir);
    }
    if (!strsInCODELint.modulesDir.empty()) {
        configContext.SetModulesDir(strsInCODELint.modulesDir);
    }
    if (!strsInCODELint.excludeRule.empty()) {
        excludeInputList.emplace_back(strsInCODELint.excludeRule);
    }
    if (!strsInCODELint.codeoPath.empty()) {
        configContext.SetCodeoPath(strsInCODELint.codeoPath);
        excludeInputList.emplace_back(strsInCODELint.codeoPath);
    }
    if ((CheckInitialContext() != OK) || (CheckExcludeContext(excludeInputList) != OK)) {
        return ERR;
    }
    return CheckCode();
}

int CODELint(const ParamsInCODELint& strsInCODELint, const char** envp)
{
    if (CheckAndSetCodiraHome(envp) != OK) {
        return ERR;
    }
    return CODELintHelper(strsInCODELint);
}
} // namespace CodeLint
