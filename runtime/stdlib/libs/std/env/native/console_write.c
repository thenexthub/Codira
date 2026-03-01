/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "securec.h"

#ifdef WIN32
#include <windows.h>
#include <wchar.h>
#else
#include <unistd.h>
#endif  // WIN32

extern void CODE_CONSOLE_Flush(int32_t fd)
{
    if (fd == 1) {
        fflush(stdout);
    } else {
        fflush(stderr);
    }
}

#ifndef WIN32
// Linux-like platform
extern void CODE_CONSOLE_Write(int32_t fd, const uint8_t* str, const int64_t len, bool newLine)
{
    (void)write(fd, str, (size_t)len);

    if (newLine) {
        (void)write(fd, "\n", 1);
    }
}

#else
// WIN32
// The Windows platform needs to determine whether to redirect to a file.
static void ConsoleWriteW(HANDLE fd, const uint8_t* str, const int64_t len, bool newLine)
{
    int wstrlen = MultiByteToWideChar(CP_UTF8, 0, str, len, NULL, 0);
    wchar_t* wstr = (wchar_t*)malloc(sizeof(wchar_t) * wstrlen);
    if (wstr == NULL) {
        return;
    }
    MultiByteToWideChar(CP_UTF8, 0, str, len, wstr, wstrlen);
    // win32 API
    (void)WriteConsoleW(fd, wstr, wstrlen, NULL, NULL);
    free(wstr);

    if (newLine) {
        (void)WriteConsoleW(fd, L"\n", 1, NULL, NULL);
    }
}

static void ConsoleWriteFile(HANDLE fd, const uint8_t* str, const int64_t len, bool newLine)
{
    (void)WriteFile(fd, str, len, NULL, NULL);
    if (newLine) {
        (void)WriteFile(fd, "\n", 1, NULL, NULL);
    }
}

extern void CODE_CONSOLE_Write(int32_t fd, const uint8_t* str, const int64_t len, bool newLine)
{
    HANDLE handler = NULL;
    if (fd == 1) {
        handler = GetStdHandle(STD_OUTPUT_HANDLE);
    } else {
        handler = GetStdHandle(STD_ERROR_HANDLE);
    }

    if (GetFileType(handler) == FILE_TYPE_CHAR) {
        // a character file, typically an LPT device or a console
        ConsoleWriteW(handler, str, len, newLine);
    } else {
        ConsoleWriteFile(handler, str, len, newLine);
    }
}
#endif
