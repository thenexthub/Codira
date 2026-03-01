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

#include "Codira/Basic/Print.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace Codira {

#ifdef _WIN32

// Enables OS earlier than Windos10 1511 to compile normally
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

const int WINDOWS_10_VERSION_1511_BUILD_NUMBER = 10586;
const int WINDOWS_10 = 10;
ColorSingleton::ColorSingleton()
{
    DWORD stdoutMode;
    DWORD stderrMode;
    GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &stdoutMode);
    GetConsoleMode(GetStdHandle(STD_ERROR_HANDLE), &stderrMode);

    // store initial console mode
    initialStdoutMode = stdoutMode;
    initialStderrMode = stderrMode;

    // get current Windows os version
    auto osVersion = Utils::GetOSVersion();
    if (osVersion.dwMajorVersion >= WINDOWS_10 && osVersion.dwBuildNumber >= WINDOWS_10_VERSION_1511_BUILD_NUMBER) {
        // current Windows os version is or newer than Windows10 (version 1511)
        stdoutMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        stderrMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), stdoutMode);
        SetConsoleMode(GetStdHandle(STD_ERROR_HANDLE), stderrMode);

        ANSI_COLOR_RESET = "\x1b[0m";
        ANSI_COLOR_BRIGHT = "\x1b[1m";
        ANSI_COLOR_BLACK = "\x1b[30m";
        ANSI_COLOR_RED = "\x1b[31m";
        ANSI_COLOR_GREEN = "\x1b[32m";
        ANSI_COLOR_YELLOW = "\x1b[33m";
        ANSI_COLOR_BLUE = "\x1b[34m";
        ANSI_COLOR_MAGENTA = "\x1b[35m";
        ANSI_COLOR_CYAN = "\x1b[36m";
        ANSI_COLOR_WHITE = "\x1b[37m";

        ANSI_COLOR_WHITE_BACKGROUND_BLACK_FOREGROUND = "\x1b[30;47m";
    }
};

ColorSingleton::~ColorSingleton()
{
    // restore to the initial console mode
    SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), initialStdoutMode);
    SetConsoleMode(GetStdHandle(STD_ERROR_HANDLE), initialStderrMode);
}
#else
ColorSingleton::ColorSingleton()
{
    ANSI_COLOR_RESET = "\x1b[0m";
    ANSI_COLOR_BRIGHT = "\x1b[1m";
    ANSI_COLOR_BLACK = "\x1b[30m";
    ANSI_COLOR_RED = "\x1b[31m";
    ANSI_COLOR_GREEN = "\x1b[32m";
    ANSI_COLOR_YELLOW = "\x1b[33m";
    ANSI_COLOR_BLUE = "\x1b[34m";
    ANSI_COLOR_MAGENTA = "\x1b[35m";
    ANSI_COLOR_CYAN = "\x1b[36m";
    ANSI_COLOR_WHITE = "\x1b[37m";

    ANSI_COLOR_WHITE_BACKGROUND_BLACK_FOREGROUND = "\x1b[30;47m";
};

ColorSingleton::~ColorSingleton(){};
#endif
} // namespace Codira
