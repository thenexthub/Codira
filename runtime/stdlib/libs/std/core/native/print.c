/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include "core.h"

#ifdef WIN32
#include <wchar.h>
#include <windows.h>
#endif

static const char * const NEW_LINE = "\n";

#ifndef INT32_MAX
#define INT32_MAX 2147483647
#endif // INT32_MAX

static int64_t PrintUtf8(FILE* handle, const uint8_t* str, int64_t len, bool newLine)
{
    (void)fwrite(str, sizeof(uint8_t), (size_t)len, handle);

    if (newLine) {
        (void)fwrite(NEW_LINE, 1, 1, handle);
    }
    return 0;
}

#ifdef WIN32
// WIN32
// The Windows platform needs to determine whether to redirect to a file.
static int64_t ConsoleWriteW(HANDLE fd, const uint8_t* str, const int64_t len, bool newLine)
{
    int64_t remainingLen = len;
    size_t writelen = 0;
    while (remainingLen > 0) {
        if (remainingLen < INT32_MAX) {
            writelen = (size_t)remainingLen;
        } else {
            writelen = INT32_MAX;
        }
        int wstrlen = MultiByteToWideChar(CP_UTF8, 0, str, len, NULL, 0);
        wchar_t* wstr = (wchar_t*)malloc(sizeof(wchar_t) * wstrlen);
        if (wstr == NULL) {
            return -1;
        }
        MultiByteToWideChar(CP_UTF8, 0, str, len, wstr, wstrlen);
        // win32 API
        (void)WriteConsoleW(fd, wstr, wstrlen, NULL, NULL);
        free(wstr);
        remainingLen -= writelen;
    }

    if (newLine) {
        (void)WriteConsoleW(fd, L"\n", 1, NULL, NULL);
    }
    return 0;
}

static int64_t ConsoleWriteFile(HANDLE fd, const uint8_t* str, const int64_t len, bool newLine)
{
    (void)WriteFile(fd, str, len, NULL, NULL);
    if (newLine) {
        (void)WriteFile(fd, "\n", 1, NULL, NULL);
    }
    return 0;
}
#endif // WIN32

extern int64_t CODE_CORE_PrintUTF8(const uint8_t* str, int64_t len, bool newLine, bool flush)
{
    int64_t ret = 0;
#ifdef WIN32
    HANDLE handler = GetStdHandle(STD_OUTPUT_HANDLE);
    if (GetFileType(handler) == FILE_TYPE_CHAR) {
        // a character file, typically an LPT device or a console
        ret = ConsoleWriteW(handler, str, len, newLine);
    } else {
        ret = PrintUtf8(stdout, str, len, newLine);
    }
#else
    ret = PrintUtf8(stdout, str, len, newLine);
#endif

    if (flush) {
        (void)fflush(stdout);
    }
    return ret;
}

extern int64_t CODE_CORE_ErrorPrintUTF8(const uint8_t* str, int64_t len, bool newLine, bool flush)
{
    int64_t ret = 0;
#ifdef WIN32
    HANDLE handler = GetStdHandle(STD_ERROR_HANDLE);
    if (GetFileType(handler) == FILE_TYPE_CHAR) {
        // a character file, typically an LPT device or a console
        ret = ConsoleWriteW(handler, str, len, newLine);
    } else {
        ret = PrintUtf8(stderr, str, len, newLine);
    }
#else
    ret = PrintUtf8(stderr, str, len, newLine);
#endif

    if (flush) {
        (void)fflush(stderr);
    }
    return ret;
}

extern void CODE_CORE_PrintBool(bool b, bool newLine, bool flush)
{
    if (b) {
        (void)fwrite("true", 1, 4, stdout); // the number of bytes of true is 4.
    } else {
        (void)fwrite("false", 1, 5, stdout); // The number of bytes of false is 5.
    }
    if (newLine) {
        (void)fwrite(NEW_LINE, 1, 1, stdout);
    }
    if (flush) {
        (void)fflush(stdout);
    }
}

extern void CODE_CORE_PrintChar(uint32_t c, bool newLine, bool flush)
{
    uint8_t out[16] = {0};
    /*
     * CODE_CORE_FromCharToUtf8 can't return -1. because The value
     * of Codira Rune is a Unicode scalar value.
     */
    int64_t s = CODE_CORE_FromCharToUtf8(c, out);
    (void)PrintUtf8(stdout, out, s, newLine);

    if (flush) {
        (void)fflush(stdout);
    }
}

extern void CODE_CORE_PrintSigned(int64_t i, bool newLine, bool flush)
{
    (void)printf("%" PRId64, i);
    if (newLine) {
        (void)fwrite(NEW_LINE, 1, 1, stdout);
    }
    if (flush) {
        (void)fflush(stdout);
    }
}

extern void CODE_CORE_PrintUnsigned(uint64_t u, bool newLine, bool flush)
{
    (void)printf("%" PRIu64, u);
    if (newLine) {
        (void)fwrite(NEW_LINE, 1, 1, stdout);
    }
    if (flush) {
        (void)fflush(stdout);
    }
}

extern void CODE_CORE_PrintFloat(double f, bool newLine, bool flush)
{
    if (isnan(f)) {
        // Nan doesn't have a sign in cangjie. So, we should always print nan.
        (void)printf("nan");
    } else {
        (void)printf("%f", f);
    }
    if (newLine) {
        (void)fwrite(NEW_LINE, 1, 1, stdout);
    }
    if (flush) {
        (void)fflush(stdout);
    }
}

extern void CODE_CORE_PrintOomHint(void)
{
    static const char * const OOM_HINT = "An exception has occurred:\n    Out of memory\n";
    (void)fwrite(OOM_HINT, sizeof(uint8_t), strlen(OOM_HINT), stderr);
}
