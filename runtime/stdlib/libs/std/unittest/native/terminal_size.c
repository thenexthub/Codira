/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */
#include "terminal_size.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

// Windows impl
#ifdef _WIN32

uint64_t GetTerminalWidth(void)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    BOOL isSuccess = GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    if (!isSuccess)
        return 0;
    uint64_t width = (uint64_t)(csbi.srWindow.Right - csbi.srWindow.Left + 1);
    return width;
}

uint64_t GetTerminalHeight(void)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    BOOL isSuccess = GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    if (!isSuccess)
        return 0;
    uint64_t height = (uint64_t)(csbi.srWindow.Bottom - csbi.srWindow.Top + 1);
    return height;
}
#else
// Linux impl

uint64_t GetTerminalWidth(void)
{
    struct winsize w;
    int isFailure = ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    if (isFailure != 0) {
        return 0;
    }

    return (uint64_t)w.ws_col;
}

uint64_t GetTerminalHeight(void)
{
    struct winsize w;
    int isFailure = ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    if (isFailure != 0) {
        return 0;
    }

    return (uint64_t)w.ws_row;
}

#endif
