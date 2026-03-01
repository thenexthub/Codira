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
 * This file implements Tool class and its functions.
 */

#include "Codira/Driver/Tool.h"

#ifdef _WIN32
#include <windows.h>
#include <iomanip>
#else
#include <spawn.h>
#include <sys/wait.h>
#endif
#include <fstream>

#include "Codira/Utils/FileUtil.h"
#include "Codira/Utils/Semaphore.h"
#include "Codira/Driver/TempFileManager.h"
#include "Codira/Driver/Utils.h"

using namespace Codira;

#ifdef __APPLE__
const static std::string LD_LIBRARY_PATH = "DYLD_LIBRARY_PATH";
#elif !defined(_WIN32)
const static std::string LD_LIBRARY_PATH = "LD_LIBRARY_PATH";
#endif

void Tool::AppendMultiArgs(const std::string& argStr)
{
    auto splitArgs = Utils::SplitString(argStr, " ");
    for (const auto& a : splitArgs) {
        if (!a.empty()) {
            args.emplace_back(a);
        }
    }
}

std::string Tool::GenerateCommand() const
{
    std::string tempCommand;
    std::string argStr;
    auto& argList = GetArgs();
    std::for_each(argList.begin(), argList.end(), [&argStr](const std::string& s) {
#ifdef _WIN32
        argStr = argStr + FileUtil::GetQuoted(s) + " ";
#else
        argStr = argStr + GetSingleQuoted(s) + " ";
#endif
    });
#ifndef _WIN32
    if (!GetLdLibraryPath().empty()) {
        tempCommand = "LD_LIBRARY_PATH=" + GetSingleQuoted(GetLdLibraryPath()) + ":$" + LD_LIBRARY_PATH + " ";
    }
#endif

#ifdef _WIN32
    tempCommand += FileUtil::GetQuoted(name);
#else
    tempCommand += GetSingleQuoted(name);
#endif
    tempCommand += " " + argStr;
#ifdef _WIN32
    // Command run as `cmd /C`. The leading double quote will be removed by `cmd` causing the semantic of command
    // changes. Extra pair of double quotes is added to make the command works when there is leading double quote.
    return "\"" + tempCommand + "\"";
#else
    return tempCommand;
#endif
}

bool Tool::InternalImplementedCommandExec() const
{
    bool res = false;
    if (type == ToolType::INTERNAL_IMPLEMENTED && name == "CacheCopy") {
        auto& arguments = GetFullArgs();
        // arguments[0] - name of command
        // arguments[1] - absolute name of srcFile
        std::ifstream srcStream(arguments[1], std::ios::binary);
        // arguments[2] - absolute name of destFile
        std::ofstream destStream(arguments[2], std::ios::binary);
        destStream << srcStream.rdbuf();
        destStream.close();
#ifdef _WIN32
        FileUtil::HideFile(FileUtil::GetDirPath(arguments[2]));
#endif
        res = true;
    }
    return res;
}

std::unique_ptr<ToolFuture> Tool::Execute(bool verbose) const
{
    if (verbose) {
        Println(FileUtil::Normalize(GetCommandString()));
    }
    // If the file is deleted, the subsequent commands are not executed.
    if (TempFileManager::Instance().IsDeleted()) {
        return nullptr;
    }
    return Run();
}

#ifdef _WIN32
std::unique_ptr<ToolFuture> Tool::Run() const
{
    if (type == ToolType::INTERNAL_IMPLEMENTED) {
        Utils::Semaphore::Get().Acquire();
        return std::make_unique<ThreadFuture>(std::async(&Tool::InternalImplementedCommandExec, this));
    }
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    auto& arguments = GetFullArgs();
    std::ostringstream oss;
    for (size_t i = 0; i < arguments.size(); ++i) {
        if (i != 0) {
            oss << " ";
        }
        if (arguments[i].empty() || arguments[i].find_first_of("\t \"&\'()*<>\\`^|\n") != std::string::npos) {
            oss << std::quoted(arguments[i]);
        } else {
            oss << arguments[i];
        }
    }
    std::string commandLine = oss.str();

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    Utils::Semaphore::Get().Acquire();
    if (!CreateProcessA(
        name.c_str(), const_cast<char*>(commandLine.c_str()), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
        Utils::Semaphore::Get().Release();
        return nullptr;
    }

    return std::make_unique<WindowsProcessFuture>(pi);
}
#else
std::unique_ptr<ToolFuture> Tool::Run() const
{
    if (type == ToolType::INTERNAL_IMPLEMENTED) {
        Utils::Semaphore::Get().Acquire();
        return std::make_unique<ThreadFuture>(std::async(&Tool::InternalImplementedCommandExec, this));
    }
    auto& arguments = GetFullArgs();

    // Convert arguments to char * array for argv argument.
    std::vector<char*> rawArgumentArray = {};
    rawArgumentArray.reserve(arguments.size() + 1);
    size_t index = 0;
    while (index < arguments.size()) {
        rawArgumentArray[index] = const_cast<char*>(arguments[index].c_str());
        index++;
    }
    rawArgumentArray[index] = nullptr;

    // Note: `childEnvironment` must be alive until the call of `posix_spawn` finishes. The c pointer
    // of environment we get will be invalid if `childEnvironment` is destroyed too early.
    std::list<std::string> childEnvironment = BuildEnvironmentVector();

    // Make an array for envp argument.
    std::vector<char*> rawEnvironmentArray;
    for (const std::string& s : std::as_const(childEnvironment)) {
        rawEnvironmentArray.emplace_back(const_cast<char*>(s.c_str()));
    }
    rawEnvironmentArray.push_back(nullptr);

    pid_t pid;
    int status = 0;
    Utils::Semaphore::Get().Acquire();
    // If name has no directory separator, then it should be a program name that needs to be
    // searched in PATH. `posix_spawnp` does program searches for us.
    if (name.find_first_of(DIR_SEPARATOR) == std::string::npos) {
        status =
            posix_spawnp(&pid, name.c_str(), nullptr, nullptr, rawArgumentArray.data(), rawEnvironmentArray.data());
    } else {
        status = posix_spawn(&pid, name.c_str(), nullptr, nullptr, rawArgumentArray.data(), rawEnvironmentArray.data());
    }

    // Failed to start the child process
    if (status != 0) {
        Utils::Semaphore::Get().Release();
        return nullptr;
    }

    return std::make_unique<LinuxProcessFuture>(pid);
}

std::list<std::string> Tool::BuildEnvironmentVector() const
{
    std::list<std::string> environment;
    // Append our ld library path to the front of LD_LIBRARY_PATH
    if (ldLibraryPath.empty()) {
        if (environmentVars.find(LD_LIBRARY_PATH) != environmentVars.end()) {
            environment.emplace_back(LD_LIBRARY_PATH + "=" + environmentVars.at(LD_LIBRARY_PATH));
        }
    } else {
        std::string newLdLibraryPath = ldLibraryPath;
        // If LD_LIBRARY_PATH is empty, we don't append `:` or ld.so will treat the trailing empty path as ".".
        // See documentation of ld.so for details.
        if (environmentVars.find(LD_LIBRARY_PATH) != environmentVars.end() &&
            !environmentVars.at(LD_LIBRARY_PATH).empty()) {
            newLdLibraryPath += ":" + environmentVars.at(LD_LIBRARY_PATH);
        }
        environment.emplace_back(LD_LIBRARY_PATH + "=" + newLdLibraryPath);
    }
    for (const auto& environmentVar : environmentVars) {
        if (environmentVar.first == LD_LIBRARY_PATH) {
            continue;
        }
        environment.emplace_back(environmentVar.first + "=" + environmentVar.second);
    }
    return environment;
}
#endif
