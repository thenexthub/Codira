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
 * This file declares Color related apis.
 */

#ifndef CODIRA_BASIC_COLOR_H
#define CODIRA_BASIC_COLOR_H

#include <algorithm>
#include <cstdarg>
#include <iomanip>
#include <iostream>
#include <utility>

namespace Codira {

struct ColorSingleton {
public:
    static ColorSingleton& getInstance()
    {
        static ColorSingleton singleton;
        return singleton;
    }

    std::string ANSI_COLOR_RESET;
    std::string ANSI_COLOR_BRIGHT;
    std::string ANSI_COLOR_BLACK;
    std::string ANSI_COLOR_RED;
    std::string ANSI_COLOR_GREEN;
    std::string ANSI_COLOR_YELLOW;
    std::string ANSI_COLOR_BLUE;
    std::string ANSI_COLOR_MAGENTA;
    std::string ANSI_COLOR_CYAN;
    std::string ANSI_COLOR_WHITE;

    std::string ANSI_COLOR_WHITE_BACKGROUND_BLACK_FOREGROUND;

private:
#ifdef _WIN32
    unsigned long initialStdoutMode;
    unsigned long initialStderrMode;
#endif

    ColorSingleton();
    ~ColorSingleton();
};

inline const std::string ANSI_NO_COLOR;

inline const std::string ANSI_COLOR_RESET = ColorSingleton::getInstance().ANSI_COLOR_RESET;
inline const std::string ANSI_COLOR_BRIGHT = ColorSingleton::getInstance().ANSI_COLOR_BRIGHT;
inline const std::string ANSI_COLOR_BLACK = ColorSingleton::getInstance().ANSI_COLOR_BLACK;
inline const std::string ANSI_COLOR_RED = ColorSingleton::getInstance().ANSI_COLOR_RED;
inline const std::string ANSI_COLOR_GREEN = ColorSingleton::getInstance().ANSI_COLOR_GREEN;
inline const std::string ANSI_COLOR_YELLOW = ColorSingleton::getInstance().ANSI_COLOR_YELLOW;
inline const std::string ANSI_COLOR_BLUE = ColorSingleton::getInstance().ANSI_COLOR_BLUE;
inline const std::string ANSI_COLOR_MAGENTA = ColorSingleton::getInstance().ANSI_COLOR_MAGENTA;
inline const std::string ANSI_COLOR_CYAN = ColorSingleton::getInstance().ANSI_COLOR_CYAN;
inline const std::string ANSI_COLOR_WHITE = ColorSingleton::getInstance().ANSI_COLOR_WHITE;

inline const std::string ANSI_COLOR_WHITE_BACKGROUND_BLACK_FOREGROUND = \
    ColorSingleton::getInstance().ANSI_COLOR_WHITE_BACKGROUND_BLACK_FOREGROUND;

} // namespace Codira
#endif // CODIRA_BASIC_COLOR_H
