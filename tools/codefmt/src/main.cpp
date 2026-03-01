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

#include "Format/FormatCodeProcessor.h"
#include "Format/OptionContext.h"
#include "Format/TomlParser.h"
#include "Codira/Basic/Print.h"
#include "Codira/Parse/Parser.h"
#include "Codira/Utils/FileUtil.h"

#include <istream>
#include <string>

#include <regex>

using namespace Codira;
using namespace Codira::AST;
using namespace Codira::Format;

namespace {
constexpr int MINIMUM_CODE_FILE_NAME = 4;
constexpr int MIN_LINE_LIMIT_LENGTH = 1;
constexpr int MAX_LINE_LIMIT_LENGTH = 120;
constexpr int MIN_INDENT_WIDTH = 0;
constexpr int MAX_INDENT_WIDTH = 8;
constexpr int MIN_METHOD_CHAIN_LEVEL = 2;
constexpr int MAX_METHOD_CHAIN_LEVEL = 10;
#ifdef _WIN32
const std::string DELIMITER = "\\";
#else
const std::string DELIMITER = "/";
#endif

int FmtFile(const std::string& fmtFilePath, const std::string& fileOutputPath, const Region region)
{
    std::string source, rawCode;
    if (!FormatFile(rawCode, fmtFilePath, source, region)) {
        source = rawCode;
    }

    std::ofstream outStream(fileOutputPath, std::ios::out | std::ios::binary);
    if (!outStream.is_open()) {
        Errorln("Create target file error!");
        return ERR;
    }
    auto length = source.size();
    (void)outStream.write(source.data(), static_cast<long>(length));
    outStream.close();
    return OK;
}

int EditFileNameAndPath(std::string& fmtFilePath, std::string& fileOutputPath)
{
    std::string currentPath = FileUtil::GetAbsPath(".") | FileUtil::IdenticalFunc;
    if (!FileUtil::FileExist(fmtFilePath)) {
        Errorln("Source file not exist!");
        return ERR;
    }
    fmtFilePath = FileUtil::GetAbsPath(fmtFilePath) | FileUtil::IdenticalFunc;
    if (!fileOutputPath.empty()) {
        if (fileOutputPath.size() < MINIMUM_CODE_FILE_NAME || fileOutputPath.find(".code") == std::string::npos) {
            fileOutputPath = fileOutputPath + ".code";
        }
        const std::string outArgs = fileOutputPath;
        if (std::strrchr(outArgs.c_str(), DELIMITER[0])) {
            std::string outNamePtr = std::strrchr(outArgs.c_str(), DELIMITER[0]);
            if (fileOutputPath != outNamePtr) {
                std::string path =
                    fileOutputPath.substr(0, (fileOutputPath.size() - std::string(outNamePtr).size()) + 1);
                fileOutputPath =
                    (FileUtil::GetAbsPath(path) | FileUtil::IdenticalFunc) + DELIMITER + std::string(outNamePtr);
            }
        } else {
            fileOutputPath = currentPath + DELIMITER + fileOutputPath;
        }
    } else {
        fileOutputPath = fmtFilePath;
    }
    return OK;
}

void PrintHelp()
{
    Println("Usage: ");
    Println("     codefmt -f fileName [-o fileName] [-l start:end]");
    Println("     codefmt -d fileDir [-o fileDir]");
    Println("Options:");
    Println("   -h            Show usage");
    Println("                     eg: codefmt -h");
    Println("   -v            Show version");
    Println("                     eg: codefmt -v");
    Println("   -f            Specifies the file in the required format. The value can be a relative path or "
            "an absolute path.");
    Println("                     eg: codefmt -f test.code");
    Println("   -d            Specifies the file directory in the required format. The value can be a relative path or "
            "an absolute path.");
    Println("                     eg: codefmt -d test/");
    Println("   -o <value>    Output. If a single file is formatted, '-o' is followed by the file name. Relative and "
            "absolute paths are supported;");
    Println("                 If a file in the file directory is formatted, a path must be added after -o. The path "
            "can be a relative path or an absolute path.");
    Println("                     eg: codefmt -f a.code -o ./fmta.code");
    Println("                     eg: codefmt -d ~/testsrc -o ./testout");
    Println("   -c <value>    Specify the format configuration file, Relative and absolute paths are supported.");
    Println("                 If the specified configuration file fails to be read, codefmt will try to read the default "
            "configuration file in CODIRA_HOME");
    Println("                 If the default configuration file also fails to be read, will use the built-in "
            "configuration.");
    Println("                     eg: codefmt -f a.code -c ./config/cangjie-format.toml");
    Println("                     eg: codefmt -d ~/testsrc -c ~/home/project/config/cangjie-format.toml");
    Println("   -l <region>   Only format lines in the specified region for the provided file. Only valid if a single "
            "file was specified.");
    Println("                 Region has a format of [start:end] where 'start' and 'end' are integer numbers "
            "representing first and last lines to be formated in the specified file.");
    Println("                 Line count starts with 1.");
    Println("                     eg: codefmt -f a.code -o ./fmta.code -l 1:25");
}

void PrintVersion()
{
#ifdef CODEFMT_VERSION
    Println(std::string("codefmt Version: ") + std::string(CODEFMT_VERSION));
#else
    Errorln("Can not obtain codefmt version");
#endif
}

void SetOptionIndentWidth(TomlParser& parser, FormattingOptions& options)
{
    auto indentWidthOptional = parser.GetValue("indentWidth");
    if (!indentWidthOptional.has_value()) {
        Warningln("Can't find configuration options: indentWidth, built-in options will be used: indentWidth = 4");
        return;
    }
    std::optional<int> indentWidth;
    std::visit(
        [&indentWidth](auto&& value) {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, int>) {
                indentWidth = value;
            } else {
                Warningln("Configuration options: indentWidth's type should be integer, "
                          "Built-in options will be used: indentWidth = 4");
            }
        },
        indentWidthOptional.value());
    if (!indentWidth.has_value()) {
        return;
    }
    if (*indentWidth < MIN_INDENT_WIDTH || *indentWidth > MAX_INDENT_WIDTH) {
        Warningln("Configuration options: indentWidth is out of range [0, 8], "
                  "Built-in options will be used: indentWidth = 4");
        return;
    }
    options.indentWidth = *indentWidth;
}

void SetOptionLineLength(TomlParser& parser, FormattingOptions& options)
{
    auto lineLimitLengthOptional = parser.GetValue("linelimitLength");
    if (!lineLimitLengthOptional.has_value()) {
        Warningln("Can't find configuration options: linelimitLength, "
                  "built-in options will be used: linelimitLength = 120");
        return;
    }
    std::optional<int> linelimitLength;
    std::visit(
        [&linelimitLength](auto&& value) {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, int>) {
                linelimitLength = value;
            } else {
                Warningln("Configuration options: linelimitLength's type should be integer, Built-in options will "
                          "be used: linelimitLength = 120");
            }
        },
        lineLimitLengthOptional.value());
    if (!linelimitLength.has_value()) {
        return;
    }
    if (*linelimitLength < MIN_LINE_LIMIT_LENGTH || *linelimitLength > MAX_LINE_LIMIT_LENGTH) {
        Warningln("Configuration options: linelimitLength is out of range [1, 120], "
                  "Built-in options will be used: linelimitLength = 120");
        return;
    }
    options.lineLength = *linelimitLength;
}

void SetOptionLineBreakType(TomlParser& parser, FormattingOptions& options)
{
    auto lineBreakTypeOptional = parser.GetValue("lineBreakType");
    if (!lineBreakTypeOptional.has_value()) {
        Warningln("Can't find configuration options: lineBreakType, "
                  "built-in options will be used: lineBreakType = \"LF\"");
        return;
    }
    std::optional<std::string> lineBreakType;
    std::visit(
        [&lineBreakType](auto&& value) {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, std::string>) {
                lineBreakType = value;
            } else {
                Warningln(
                    "Configuration options: lineBreakType's type should be string, "
                    "Built-in options will be used: lineBreakType = \"LF\"");
            }
        },
        lineBreakTypeOptional.value());

    if (!lineBreakType.has_value()) {
        return;
    }
    if (*lineBreakType != "CRLF" && *lineBreakType != "LF") {
        Warningln("\"Configuration options: neither CRLF nor LF, "
                  "Built-in options will be used: lineBreakType = \"LF\"");
        return;
    }
    options.newLine = *lineBreakType == "CRLF" ? "\r\n" : "\n";
}

void SetOptionMethodChainning(TomlParser& parser, FormattingOptions& options)
{
    auto methodChainningOptional = parser.GetValue("allowMultiLineMethodChain");
    if (!methodChainningOptional.has_value()) {
        Warningln("Can't find configuration options: allowMultiLineMethodChain, "
                  "built-in options will be used: allowMultiLineMethodChain = false");
        return;
    }
    std::optional<bool> allowMultiLineMethodChain;
    std::visit(
        [&allowMultiLineMethodChain](auto&& value) {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, bool>) {
                allowMultiLineMethodChain = value;
            } else {
                Warningln("Configuration options: allowMultiLineMethodChain's type should be boolean, "
                          "Built-in options will be used: allowMultiLineMethodChain = false");
            }
        },
        methodChainningOptional.value());
    if (!allowMultiLineMethodChain.has_value()) {
        return;
    }
    options.allowMultiLineMethodChain = *allowMultiLineMethodChain;
}

void SetOptionMethodChainLevel(TomlParser& parser, FormattingOptions& options)
{
    auto methodChainningOptional = parser.GetValue("multipleLineMethodChainLevel");
    if (!methodChainningOptional.has_value()) {
        Warningln("Can't find configuration options: multipleLineMethodChainLevel, "
                  "built-in options will be used: multipleLineMethodChainLevel = 5");
        return;
    }
    std::optional<int> multipleLineMethodChainLevel;
    std::visit(
        [&multipleLineMethodChainLevel](auto&& value) {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, int>) {
                multipleLineMethodChainLevel = value;
            } else {
                Warningln("Configuration options: multipleLineMethodChainLevel's type should be integer, "
                          "Built-in options will be used: multipleLineMethodChainLevel = 5");
            }
        },
        methodChainningOptional.value());
    if (!multipleLineMethodChainLevel.has_value()) {
        return;
    }
    if (*multipleLineMethodChainLevel < MIN_METHOD_CHAIN_LEVEL ||
        *multipleLineMethodChainLevel > MAX_METHOD_CHAIN_LEVEL) {
        Warningln("Configuration options: multipleLineMethodChainLevel is out of range [2, 10], "
                  "Built-in options will be used: multipleLineMethodChainLevel = 5");
        return;
    }
    options.multipleLineMethodChainLevel = *multipleLineMethodChainLevel;
}

void SetOptionMultipleLineMethodChainOverLineLength(TomlParser& parser, FormattingOptions& options)
{
    auto methodChainningOptional = parser.GetValue("multipleLineMethodChainOverLineLength");
    if (!methodChainningOptional.has_value()) {
        Warningln("Can't find configuration options: multipleLineMethodChainOverLineLength, "
                  "built-in options will be used: multipleLineMethodChainOverLineLength = true");
        return;
    }
    std::optional<bool> multipleLineMethodChainOverLineLength;
    std::visit(
        [&multipleLineMethodChainOverLineLength](auto&& value) {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, bool>) {
                multipleLineMethodChainOverLineLength = value;
            } else {
                Warningln("Configuration options: multipleLineMethodChainOverLineLength's type should be boolean, "
                          "Built-in options will be used: multipleLineMethodChainOverLineLength = true");
            }
        },
        methodChainningOptional.value());
    if (!multipleLineMethodChainOverLineLength.has_value()) {
        return;
    }
    options.multipleLineMethodChainOverLineLength = *multipleLineMethodChainOverLineLength;
}

void SetFmtConfigOptions(TomlParser& parser, FormattingOptions& options)
{
    SetOptionIndentWidth(parser, options);
    SetOptionLineLength(parser, options);
    SetOptionLineBreakType(parser, options);
    SetOptionMethodChainning(parser, options);
    SetOptionMethodChainLevel(parser, options);
    SetOptionMultipleLineMethodChainOverLineLength(parser, options);
}

std::optional<FormattingOptions> ReadConfigHelper(
    TomlParser& parser, const std::string& configPath, FormattingOptions& options, const std::string& configType)
{
    if (FileUtil::FileExist(configPath)) {
        auto absConfigPath = FileUtil::GetAbsPath(configPath).value();
        if (!absConfigPath.empty() && parser.ReadFile(absConfigPath)) {
            Println("Reading " + configType + " format configuration files...");
            SetFmtConfigOptions(parser, options);
            return options;
        } else {
            Warningln("Failed to read " + configType + " format configuration files.");
            return std::nullopt;
        }
    }
    Warningln(configType + " format configuration files not Exist");
    return std::nullopt;
}

FormattingOptions GetOptionsInfo(const std::string& configPath)
{
    OptionContext& optionContext = OptionContext::GetInstance();
    FormattingOptions options;
    TomlParser parser;

    if (!configPath.empty()) {
        auto customizedConfig = ReadConfigHelper(parser, configPath, options, "customized");
        if (customizedConfig.has_value()) {
            return customizedConfig.value();
        }
    }

    auto cangjieHome = optionContext.GetCodiraHome();
    if (!cangjieHome.empty()) {
        std::string defaultConfigPath = FileUtil::JoinPath(cangjieHome, "/tools/config/cangjie-format.toml");
        auto defaultConfig = ReadConfigHelper(parser, defaultConfigPath, options, "default");
        if (defaultConfig.has_value()) {
            return defaultConfig.value();
        }
    }

    Println("Using built-in formatting options");
    return options;
}

int DoFormat(const Region& region)
{
    OptionContext& optionContext = OptionContext::GetInstance();
    auto fmtFilePath = optionContext.GetFmtFilePath();
    auto fmtDirPath = optionContext.GetFmtDirPath();
    auto fileOutputPath = optionContext.GetFileOutputPath();
    auto dirOutputPath = optionContext.GetDirOutputPath();
    auto configPath = optionContext.GetConfigFilePath();

    if (fmtFilePath.empty() && fmtDirPath.empty()) {
        Errorf("Action has no specific file or directory to operate on.\n");
        PrintHelp();
        return ERR;
    }
    Println("Formatting start...");
    optionContext.SetConfigOptions(GetOptionsInfo(configPath));
    int result;
    if (!fmtDirPath.empty()) {
        result = FmtDir(fmtDirPath, dirOutputPath);
        Println("Formatting complete.");
        return result;
    } else {
        if (fmtFilePath.size() < MINIMUM_CODE_FILE_NAME || !HasEnding(fmtFilePath, ".code")) {
            Errorln("CodiraFormat only support Codira source code file!");
            PrintHelp();
            return ERR;
        }
        if (EditFileNameAndPath(fmtFilePath, fileOutputPath) == ERR) {
            Println("Formatting complete.");
            return ERR;
        }
        result = FmtFile(fmtFilePath, fileOutputPath, region);
        Println("Formatting complete.");
        return result;
    }
}

int GetRegionFromArgs(Region& region, std::string optarg)
{
    std::string lineNumbers = std::move(optarg);
    std::regex reg = std::regex("\\d+:\\d+");
    if (!std::regex_match(lineNumbers, reg)) {
        Errorln("Invalid region format, use the following format: <start>:<end> "
                "where <start> and <end> are integer numbers separated by colon.");
        return ERR;
    }
    auto colonPos = lineNumbers.find(':');
#ifndef CODIRA_ENABLE_GCOV
    try {
#endif
        region.startLine = std::stoi(lineNumbers.substr(0, colonPos));
        region.endLine = std::stoi(lineNumbers.substr(colonPos + 1, (lineNumbers.size() - colonPos) - 1));
        region.isWholeFile = false;
#ifndef CODIRA_ENABLE_GCOV
    } catch (const std::out_of_range& e) {
        Errorln("Invalid region format, number out of range.");
        return ERR;
    } catch (const std::invalid_argument& e) {
        Errorln("Invalid region format, invalid number.");
        return ERR;
    }
#endif

    if (region.startLine > region.endLine) {
        Errorln("Invalid region format, start line number should be less than or equal end line number.");
        return ERR;
    }
    return OK;
}

int VerifydirOutputPath(const std::string& dirOutputPath)
{
    std::regex reg = std::regex("[^*?\"<>|]*");
    if (!std::regex_match(dirOutputPath, reg)) {
        Errorln("The output path cannot contain invalid characters: *? \" < > |");
        return ERR;
    }
    return OK;
}

int ConfigureOutputArgs(const std::string& optarg, bool& isFile, OptionContext& optionContext)
{
    if (isFile) {
        optionContext.SetFileOutputPath(optarg);
        isFile = false;
        return OK;
    }
    optionContext.SetDirOutputPath(optarg);
    return VerifydirOutputPath(optionContext.GetDirOutputPath());
}

int HandleOption(char ch, const char* optarg, bool& isFile, Region& region, OptionContext& optionContext)
{
    switch (ch) {
        case 'h':
            PrintHelp();
            return OK;
        case 'v':
            PrintVersion();
            return OK;
        case 'd':
            if (!FileUtil::FileExist(optarg)) {
                Errorln("Source file path not exist!");
                return ERR;
            }
            optionContext.SetFmtDirPath(optarg);
            break;
        case 'o':
            return ConfigureOutputArgs(optarg, isFile, optionContext);
        case 'f':
            optionContext.SetFmtFilePath(optarg);
            isFile = true;
            break;
        case 'l':
            return GetRegionFromArgs(region, optarg);
        case 'c':
            optionContext.SetConfigFilePath(optarg);
            break;
        default:
            Error("invalid option or option requires an argument: -", static_cast<char>(optopt), "\n",
                "Try: 'codefmt -h' for more information.\n");
            return ERR;
    }
    return OK;
}

static bool CheckArguments(const int& argc, char** argv)
{
    bool flag = true;
    for (; optind < argc; ++optind) {
        flag = false;
        Error("invalid argument: ", argv[optind], "\n", "Try: 'codefmt -h' for more information.\n");
    }
    return flag;
}

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

std::string CheckAndSetCodiraHome(const char** envp)
{
    std::unordered_map<std::string, std::string> environmentVars = StringifyEnvironmentPointer(envp);
    if (environmentVars.find(CODIRA_HOME) == environmentVars.end()) {
        return "";
    }
    auto cangjieHome = FileUtil::GetAbsPath(environmentVars.at(CODIRA_HOME));
    if (!cangjieHome.has_value()) {
        return "";
    }
    return cangjieHome.value();
}
} // namespace

int main(int argc, char** argv, const char** envp)
{
    OptionContext& optionContext = OptionContext::GetInstance();
    bool isFile = false;
    Region region = Region::wholeFile;
    optionContext.SetCodiraHome(CheckAndSetCodiraHome(envp));

    int ch;
    const std::string optString = "hvo:d:f:l:c:";
    while ((ch = getopt(argc, argv, optString.c_str())) != -1) {
        int res = HandleOption(ch, optarg, isFile, region, optionContext);
        if (res != OK || ch == 'h' || ch == 'v') {
            return res;
        }
    }
    return CheckArguments(argc, argv) ? DoFormat(region) : ERR;
}
