/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#ifdef _WIN32
#include <windows.h>
#endif

#include "terminal_ansi.h"

#ifdef _WIN32
// Enables OS earlier than Windos10 1511 to compile normally
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

#ifndef ENABLE_PROCESSED_OUTPUT
#define ENABLE_PROCESSED_OUTPUT 0x0001
#endif

#define WINDOWS_10_VERSION_1511_BUILD_NUMBER 10586
#define WINDOWS_10 10

typedef struct WindowsOsVersion {
    unsigned long dwMajorVersion;
    unsigned long dwMinorVersion;
    unsigned long dwBuildNumber;
    unsigned long dwPlatformId;
} WindowsOsVersion;

typedef NTSTATUS(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);
WindowsOsVersion GetOSVersion()
{
    HMODULE hMod = GetModuleHandleA("ntdll.dll");
    RTL_OSVERSIONINFOW osVersionInfo = {0};
    if (hMod) {
        RtlGetVersionPtr RtlGetVersion = (RtlGetVersionPtr)GetProcAddress(hMod, "RtlGetVersion");
        if (RtlGetVersion != NULL) {
            osVersionInfo.dwOSVersionInfoSize = sizeof(osVersionInfo);
            RtlGetVersion(&osVersionInfo);
        }
    }
    WindowsOsVersion osVersion = {osVersionInfo.dwMajorVersion, osVersionInfo.dwMinorVersion,
        osVersionInfo.dwBuildNumber, osVersionInfo.dwPlatformId};
    return osVersion;
}

unsigned long initialStdoutMode;
unsigned long initialStderrMode;

bool EnableANSITerminal(void)
{
    DWORD stdoutMode;
    DWORD stderrMode;
    GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &stdoutMode);
    GetConsoleMode(GetStdHandle(STD_ERROR_HANDLE), &stderrMode);
    initialStdoutMode = stdoutMode;
    initialStderrMode = stderrMode;
    // get current Windows os version
    WindowsOsVersion osVersion = GetOSVersion();
    if (osVersion.dwMajorVersion >= WINDOWS_10 && osVersion.dwBuildNumber >= WINDOWS_10_VERSION_1511_BUILD_NUMBER) {
        // current Windows os version is or newer than Windows10 (version 1511)
        stdoutMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        stdoutMode |= ENABLE_PROCESSED_OUTPUT;
        stderrMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        stderrMode |= ENABLE_PROCESSED_OUTPUT;
        SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), stdoutMode);
        SetConsoleMode(GetStdHandle(STD_ERROR_HANDLE), stderrMode);
        return true;
    }
    return false;
}
void DisableANSITerminal(void)
{
    SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), initialStdoutMode);
    SetConsoleMode(GetStdHandle(STD_ERROR_HANDLE), initialStderrMode);
}
#else
bool EnableANSITerminal(void)
{
    return true;
}
void DisableANSITerminal(void)
{
    // Empty here.
}
#endif
