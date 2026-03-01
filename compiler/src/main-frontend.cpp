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

#include <iomanip>
#include <vector>
#include <windows.h>
#include <sstream>

namespace {

std::string SetDoubleQuoted(const std::string& str)
{
    std::stringstream ss;
    ss << "\"";
    for (char c : str) {
        // backslash cannot be used as escape character in Shell Command Language. To be able to
        // use double quote in a command, we generate backslash string and join them with
        // `"`. For example, ab"cd is transformed to "ab\"cd"; ab\cd is transformed to "ab"\\"cd".
        if (c == '"') {
            ss << "\\\"";
        } else if (c == '\\') {
            ss << "\"\\\\\"";
        } else {
            ss << c;
        }
    }
    ss << "\"";
    return ss.str();
}

} // namespace

/**
 * This simple program executes (exact*) the executing command with a different executable.
 * To be specific, the program starts `codec.exe` to process the command, but without changes
 * the executable name, i.e. argv[0], to `codec.exe`. If `codec.exe` is started by this program,
 * when `codec.exe` checks `argv[0]`, it would get the current program name, not `codec.exe`.
 */
int main(int argc, const char** argv)
{
    // Since we are using c++, there is no reason to handle unsafe char * instead of std::string.
    std::vector<std::string> args;
    for (int i = 0; i < argc; ++i) {
        args.emplace_back(argv[i]);
    }

    // Retrive user command by concatenating all arguments.
    std::ostringstream oss;
    for (size_t i = 0; i < args.size(); ++i) {
        if (i != 0) {
            oss << " ";
        }
        oss << SetDoubleQuoted(args[i]);
    }
    std::string commandLine = oss.str();

    // To keep the exact same behavior with symbolic link, we access the `codec.exe` which located
    // in the same directory current program in.
    char buffer[MAX_PATH];
    GetModuleFileName(NULL, buffer, MAX_PATH);
    std::string exePath = std::string(buffer);
    auto pos = exePath.find_last_of('\\');
    if (pos == std::string::npos) {
        return 1;
    }
    std::string codecPath = exePath.substr(0, pos) + "\\codec.exe";

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcessA(codecPath.c_str(), commandLine.data(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        return 1;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exit_code;
    if (FALSE == GetExitCodeProcess(pi.hProcess, &exit_code)) {
        return 1;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return (int)exit_code;
}
