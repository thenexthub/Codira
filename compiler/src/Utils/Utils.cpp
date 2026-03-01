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

/**
 * @file
 *
 * This file implements some utility functions.
 */

#ifdef __MINGW64__
#define _CRT_RAND_S // for secure random number generation on Windows.
#endif

#include "Codira/Utils/Utils.h"

#include <fcntl.h>
#include <functional>
#include <map>
#include <queue>

#include "Codira/AST/Node.h"
#include "Codira/Parse/Parser.h"
#include "Codira/Utils/StdUtils.h"

#ifdef __WIN32
#include <libloaderapi.h> // should be at the end to avoid same macro define
#endif
namespace Codira::Utils {
using namespace AST;

namespace {
const std::map<std::string, OverflowStrategy> NAMES_TO_OVERFLOW_STRATEGY = {
    {"no", OverflowStrategy::NA},
    {"checked", OverflowStrategy::CHECKED},
    {"wrapping", OverflowStrategy::WRAPPING},
    {"throwing", OverflowStrategy::THROWING},
    {"saturating", OverflowStrategy::SATURATING},
};

const std::map<OverflowStrategy, std::string> OVERFLOW_STRATEGY_TO_NAMES = {
    {OverflowStrategy::NA, "no"},
    {OverflowStrategy::CHECKED, "checked"},
    {OverflowStrategy::WRAPPING, "wrapping"},
    {OverflowStrategy::THROWING, "throwing"},
    {OverflowStrategy::SATURATING, "saturating"},
};
} // namespace

OverflowStrategy StringToOverflowStrategy(const std::string& name)
{
    auto found = NAMES_TO_OVERFLOW_STRATEGY.find(name);
    if (found == NAMES_TO_OVERFLOW_STRATEGY.end()) {
        return OverflowStrategy::NA;
    }
    return found->second;
}

bool ValidOverflowStrategy(const std::string& name)
{
    return NAMES_TO_OVERFLOW_STRATEGY.count(name) != 0;
}

std::string OverflowStrategyName(const OverflowStrategy& overflowStrategy)
{
    auto found = OVERFLOW_STRATEGY_TO_NAMES.find(overflowStrategy);
    CODEC_ASSERT(found != OVERFLOW_STRATEGY_TO_NAMES.end());
    return found->second;
}

std::string GenerateRandomHexString()
{
#ifdef _WIN32
    unsigned int randomInt = 0;
    rand_s(&randomInt);
    std::stringstream stream;
    stream << std::hex << static_cast<unsigned int>(randomInt);
    return stream.str();
#else
    int randomInt = 0;
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd > 0) {
        (void)read(fd, &randomInt, sizeof(int));
    }
    (void)close(fd);
    std::stringstream stream;
    stream << std::hex << static_cast<unsigned int>(randomInt);
    return stream.str();
#endif
}

std::optional<int> TryParseInt(const std::string& str)
{
    if (str.empty()) {
        return std::nullopt;
    }
    int res = 0;
    bool isValid = std::all_of(str.begin(), str.end(), [](auto c) { return std::isdigit(c); });
    if (isValid) {
        if (auto r = Stoi(str)) {
            res = *r;
        } else {
            isValid = false;
        }
    }
    if (!isValid) {
        return std::nullopt;
    }
    return {res};
}

/* Get Mangled name for wrapper function of macro. */
std::string GetMacroFuncName(const std::string& fullPackageName, bool isAttr, const std::string& ident)
{
    const std::string prefixForAttrMacro = "macroCall_a_";
    const std::string prefixForPlainMacro = "macroCall_c_";
    auto macroFuncName = (isAttr ? prefixForAttrMacro : prefixForPlainMacro) + ident + "_" + fullPackageName;
    /* '.' is not allowed in codira function name, so replace '.' to '_' */
    std::replace(macroFuncName.begin(), macroFuncName.end(), '.', '_');
    return macroFuncName;
}

std::vector<std::string> StringifyArgumentVector(int argc, const char** argv)
{
    std::vector<std::string> args;
    // Convert all arguments to strings
    for (int i = 0; i < argc; ++i) {
        if (!argv[i]) {
            continue;
        }
        args.emplace_back(argv[i]);
    }
    return args;
}

std::unordered_map<std::string, std::string> StringifyEnvironmentPointer(const char** envp)
{
    std::unordered_map<std::string, std::string> environmentVars;
    if (!envp) {
        return environmentVars;
    }
    // Read all environment variables
    for (int i = 0;; i++) {
        if (!envp[i]) {
            break;
        }
        std::string item(envp[i]);
        if (auto pos = item.find('='); pos != std::string::npos) {
            auto key = item.substr(0, pos);
#ifdef _WIN32
            // Environment variable names in Windows is case-insensitive, so they need to be unified to all UPPERCASE.
            std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) { return std::toupper(c); });
#endif
            environmentVars.emplace(key, item.substr(pos + 1));
        };
    }
    return environmentVars;
}

static std::vector<std::string> GetPathsFromEnvironmentVars(
    const std::unordered_map<std::string, std::string>& environmentVars)
{
    const std::string pathLit = "PATH";
    std::vector<std::string> searchPaths;
    if (environmentVars.find(pathLit) != environmentVars.end()) {
        searchPaths = Codira::FileUtil::SplitEnvironmentPaths(environmentVars.at(pathLit));
    }
    return searchPaths;
}

std::string GetRootPackageName(const std::string& fullPackageName)
{
    if (fullPackageName.empty()) {
        return "default";
    }
    if (std::count(fullPackageName.begin(), fullPackageName.end(), '`') % 2 != 0) { // 2 means '`' should be pairs.
        return "";
    }
    if (fullPackageName[0] == '`') {
        size_t secondPos = fullPackageName.find('`', 1);
        return fullPackageName.substr(0, secondPos + 1);
    }
    if (auto dotPos = fullPackageName.find("."); dotPos != std::string::npos) {
        return fullPackageName.substr(0, dotPos + 1);
    }
    return fullPackageName;
}

#ifdef _WIN32
std::optional<std::string> GetApplicationPath()
{
    char buffer[MAX_PATH];
    DWORD ret = GetModuleFileNameA(NULL, buffer, MAX_PATH);
    if (ret == 0) {
        Codira::Errorf("Get path of codec.exe failed.\n");
        return std::nullopt;
    }
    return {std::string(buffer)};
}
#else
std::optional<std::string> GetApplicationPath(
    const std::string& argv0, const std::unordered_map<std::string, std::string>& environmentVars)
{
    auto maybeExePath = Codira::FileUtil::GetAbsPath(
        Codira::FileUtil::FindProgramByName(argv0, GetPathsFromEnvironmentVars(environmentVars)));
    if (!maybeExePath.has_value()) {
        Codira::Errorf("Get path of %s failed.\n", argv0.c_str());
        return std::nullopt;
    }
    std::string exePath = maybeExePath.value();
    const std::string environmentPathsSpecialCharacters = ":;";
    // environmentPathsSpecialCharacters (for example, `:` or `;`) has special meaning in LD_LIBRARY_PATH.
    // To be able to call tools (to be specific, opt and llc) that require setting of LD_LIBRARY_PATH, appearance
    // of these special characters in codec installation path will cause problems.
    if (exePath.find_first_of(environmentPathsSpecialCharacters) != std::string::npos) {
        Codira::Errorf("Invalid codec installation path: %s\n", exePath.c_str());
        Codira::Infof("Do not install `codec` under a path that contains the following characters: %s\n",
            environmentPathsSpecialCharacters.c_str());
        return std::nullopt;
    }
    return {exePath};
}
#endif
} // namespace
