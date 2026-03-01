/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include "securec.h"
#include "process_ffi_unix.h"

#define EXPAND_MUL (2)
#define MAX_PATH_LEN (4096)
#define BUF_LEN (4096)
#ifndef __android__
#define CHAR_MAX (255)
#endif

typedef struct EnvListNode {
    char* env;
    struct EnvListNode* next;
} EnvListNode;

static void FreeList(EnvListNode* listHead, bool force)
{
    EnvListNode* current = listHead;
    EnvListNode* next = NULL;
    while (current != NULL) {
        next = current->next;
        if (force) {
            free(current->env);
        }
        free(current);
        current = next;
    }
}

static char* ExpandStr(char* strDest, size_t oldsize, size_t newsize)
{
    if (newsize <= oldsize) {
        return NULL;
    }

    if (strDest == NULL) {
        return NULL;
    }

    char* buffer = (char*)calloc(newsize, sizeof(char));
    if (buffer == NULL) {
        return NULL;
    }

    if (memcpy_s(buffer, newsize, strDest, oldsize) != EOK) {
        free(buffer);
        return NULL;
    }
    return buffer;
}

static char* ReadUntil(FILE* fp, int32_t tag, size_t* readLen)
{
    char* buffer = (char*)calloc(BUF_LEN, sizeof(char));
    if (buffer == NULL) {
        return NULL;
    }

    size_t lenth = BUF_LEN;
    size_t i;
    for (i = 0;; i++) {
        if (i >= (lenth - 1)) {
            char* oldbuf = buffer;
            buffer = ExpandStr(oldbuf, lenth, lenth * EXPAND_MUL);
            if (buffer == NULL) {
                free(oldbuf);
                return NULL;
            }

            lenth = lenth * EXPAND_MUL;
            free(oldbuf);
        }

        int32_t c = fgetc(fp);
        if (c == tag) {
            break;
        }
        if (c == EOF || c < 0 || c > CHAR_MAX) {
            free(buffer);
            return NULL;
        }
        buffer[i] = (char)c;
    }

    if (strlen(buffer) == 0) {
        free(buffer);
        return NULL;
    }

    if (readLen != NULL) {
        *readLen = i;
    }
    return buffer;
}

// Note: This function reads the environment variables snapshot of a process with the given PID, not realtime.
static char** GetEnvironmentSnapshot(int32_t pid)
{
    char filename[MAX_PATH_LEN] = {0};
    if (sprintf_s(filename, MAX_PATH_LEN, "/proc/%d/environ", pid) < 0) {
        return NULL;
    }

    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        return NULL;
    }

    EnvListNode* envListHead = (EnvListNode*)calloc(1, sizeof(EnvListNode));
    if (envListHead == NULL) {
        (void)fclose(fp);
        return NULL;
    }

    EnvListNode* before = envListHead;
    size_t envCount = 0;
    for (;;) {
        char* env = ReadUntil(fp, 0, NULL);
        if (env == NULL) {
            break;
        }

        EnvListNode* current = (EnvListNode*)calloc(1, sizeof(EnvListNode));
        if (current == NULL) {
            free(env);
            FreeList(envListHead, true);
            (void)fclose(fp);
            return NULL;
        }

        before->next = current;
        current->env = env;
        before = current;
        envCount++;
    }
    (void)fclose(fp);

    char** environment = (char**)calloc(envCount + 1, sizeof(char*));
    if (environment == NULL) {
        FreeList(envListHead, true);
        return NULL;
    }

    EnvListNode* eachEnv = envListHead->next;
    for (size_t i = 0; i < envCount; i++) {
        if (eachEnv == NULL) {
            break;
        }
        environment[i] = eachEnv->env;
        eachEnv = eachEnv->next;
    }
    environment[envCount] = NULL;
    FreeList(envListHead, false);

    return environment;
}

// For current process, use posix `environ` to get realtime environment variables.
static char** GetEnvironment(int32_t pid)
{
    if (pid == getpid()) {
        return GetCurrentProcessEnvironment(pid);
    } else {
        return GetEnvironmentSnapshot(pid);
    }
}

static char* GetWorkingDirectory(int32_t pid)
{
    char filename[MAX_PATH_LEN] = {0};
    if (sprintf_s(filename, MAX_PATH_LEN, "/proc/%d/cwd", pid) < 0) {
        return NULL;
    }

    char* workingDirectory = (char*)calloc(MAX_PATH_LEN, sizeof(char));
    if (workingDirectory == NULL) {
        return NULL;
    }

    ssize_t readLen = readlink(filename, workingDirectory, MAX_PATH_LEN);
    if (readLen == -1) {
        free(workingDirectory);
        return NULL;
    }

    return workingDirectory;
}

static char* GetProcessCmdline(int32_t pid, size_t* cmdLen)
{
    char filename[MAX_PATH_LEN] = {0};
    if (sprintf_s(filename, MAX_PATH_LEN, "/proc/%d/cmdline", pid) < 0) {
        return NULL;
    }

    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        return NULL;
    }

    char* cmdLine = ReadUntil(fp, EOF, cmdLen);
    if (cmdLine == NULL) {
        (void)fclose(fp);
        return NULL;
    }

    (void)fclose(fp);
    return cmdLine;
}

static char* GetCommand(char* cmdInfo, size_t cmdLen)
{
    if (cmdInfo == NULL || cmdLen == 0 || cmdInfo[cmdLen - 1] != '\0') {
        return NULL;
    }

    size_t index = 0;
    for (size_t i = 0; i < cmdLen; i++) {
        if (cmdInfo[i] == '\0') {
            index = i;
            break;
        }
    }

    char* command = (char*)calloc(index + 1, sizeof(char));
    if (command == NULL) {
        return NULL;
    }

    if (memcpy_s(command, index + 1, cmdInfo, index + 1) != EOK) {
        free(command);
        return NULL;
    }

    return command;
}

static char** GetArguments(char* cmdInfo, size_t cmdLen)
{
    if (cmdInfo == NULL || cmdLen == 0 || cmdInfo[cmdLen - 1] != '\0') {
        return NULL;
    }

    char** arguments = NULL;
    size_t argCount = 0;
    size_t index = 0;
    bool tag = false;
    for (size_t i = 0; i < cmdLen; i++) {
        if (cmdInfo[i] == '\0') {
            if (tag == false) {
                index = i;
                tag = true;
            }
            argCount++;
        }
    }
    arguments = (char**)calloc(argCount, sizeof(char*));
    if (arguments == NULL) {
        return NULL;
    }

    size_t left = index + 1;
    size_t right = index + 1;
    size_t argIndex = 0;
    for (size_t i = index + 1; i < cmdLen; i++) {
        if (cmdInfo[i] == '\0') {
            right = i;
            char* arg = (char*)calloc((right - left) + 1, sizeof(char));
            if (arg == NULL) {
                FreeTwoDimensionalArray(arguments);
                return NULL;
            }

            arguments[argIndex] = arg;
            if (memcpy_s(arg, (right - left) + 1, cmdInfo + left, (right - left) + 1) != EOK) {
                FreeTwoDimensionalArray(arguments);
                return NULL;
            }

            left = right + 1;
            argIndex++;
            if (argIndex == argCount - 1) {
                arguments[argCount - 1] = NULL;
                break;
            }
        }
    }
    return arguments;
}

static char** GetCommandLine(char* command, char** arguments)
{
    if (command == NULL || arguments == NULL) {
        return NULL;
    }

    size_t argCount = 0;
    for (int32_t i = 0; arguments[i] != NULL; i++) {
        argCount++;
    }
    argCount++;                                                         // Reserve space for command
    char** commandLine = (char**)calloc((argCount + 1), sizeof(char*)); // Reserve space for NULL
    if (commandLine == NULL) {
        return NULL;
    }

    for (size_t i = 0; i < (argCount + 1); i++) {
        if (i == 0) {
            char* cmd = (char*)calloc(strlen(command) + 1, sizeof(char));
            if (cmd == NULL) {
                FreeTwoDimensionalArray(commandLine);
                return NULL;
            }

            commandLine[i] = cmd;
            if (memcpy_s(cmd, strlen(command) + 1, command, strlen(command) + 1) != EOK) {
                FreeTwoDimensionalArray(commandLine);
                return NULL;
            }

            continue;
        }

        if (i == argCount) {
            commandLine[i] = NULL;
            break;
        }

        if (arguments[i - 1] == NULL) {
            FreeTwoDimensionalArray(commandLine);
            return NULL;
        }

        char* arg = (char*)calloc(strlen(arguments[i - 1]) + 1, sizeof(char));
        if (arg == NULL) {
            FreeTwoDimensionalArray(commandLine);
            return NULL;
        }

        commandLine[i] = arg;
        if (memcpy_s(arg, strlen(arguments[i - 1]) + 1, arguments[i - 1], strlen(arguments[i - 1]) + 1) != EOK) {
            FreeTwoDimensionalArray(commandLine);
            return NULL;
        }
    }

    return commandLine;
}

/**
 * @brief Retrieves the system boot time in milliseconds since the Unix epoch.
 *
 * This function reads the `/proc/stat` file to extract the `btime` value, which represents
 * the system boot time in seconds since the Unix epoch. The boot time is then converted
 * to milliseconds.
 *
 * @return The boot time in milliseconds since the Unix epoch, or:
 *         - `ERROR_NOT_FIND_BOOTTIME` if the `btime` value cannot be found or read.
 */
static int64_t GetBootTime()
{
    FILE* fp = fopen("/proc/stat", "r");
    if (fp == NULL) {
        return ERROR_NOT_FIND_BOOTTIME;
    }

    char path[MAX_PATH_LEN];
    int64_t bootTime = -1;
    while (fgets(path, sizeof(path), fp) != NULL) {
        if (strncmp(path, "btime", strlen("btime")) == 0) {
            if (sscanf_s(path, "btime %ld", &bootTime) != 1) {
                fclose(fp);
                return ERROR_NOT_FIND_BOOTTIME;
            }
            break;
        }
    }

    (void)fclose(fp);

    if (bootTime != -1) {
        return (int64_t)(bootTime * SECOND_TO_MILLI_SECOND);
    }
    return ERROR_NOT_FIND_BOOTTIME;
}

/**
 * @brief Retrieves a specific type of time information for a process based on its PID.
 *
 * This function reads the `/proc/[pid]/stat` file to fetch process-specific timing data such as:
 * - Process start time since boot.
 * - User-mode CPU time consumed.
 * - Kernel-mode CPU time consumed.
 *
 * @param pid The process ID (PID) of the target process.
 * @param kind The type of time to retrieve:
 *             - TIMEKIND_CREATE: Start time since boot (in ticks).
 *             - TIMEKIND_SYSTEM: Kernel-mode CPU time (in ticks).
 *             - TIMEKIND_USER: User-mode CPU time (in ticks).
 * @return The requested time in milliseconds, or:
 *         - `ERROR_GET_PROCESS_TIME_FAILED` if the process does not exist or cannot be queried.
 */
static int64_t GetProcessTime(int32_t pid, int8_t kind)
{
    char filename[MAX_PATH_LEN] = {0};
    if (sprintf_s(filename, MAX_PATH_LEN, "/proc/%d/stat", pid) < 0) {
        return ERROR_GET_PROCESS_TIME_FAILED;
    }

    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        return ERROR_GET_PROCESS_TIME_FAILED;
    }

    int64_t processTime[PROCESS_TIME_ARRAY_LENGTH] = {0}; // starttime, stime, utime
    if (fscanf_s(fp,
        "%*d %*s %*c %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %ld %ld %*d %*d %*d %*d %*d %*d %lld %*d %*d %*d %*d",
        &processTime[TIMEKIND_USER], &processTime[TIMEKIND_SYSTEM],
        &processTime[TIMEKIND_CREATE]) != PROCESS_TIME_ARRAY_LENGTH) {
        (void)fclose(fp);
        return ERROR_GET_PROCESS_TIME_FAILED;
    }

    (void)fclose(fp);
    return (int64_t)(((double)processTime[kind] / sysconf(_SC_CLK_TCK)) * SECOND_TO_MILLI_SECOND);
}

/**
 * @brief Retrieves the start time of a process relative to the system boot.
 *
 * This function provides the time (in milliseconds) since the system boot when the
 * process was created.
 *
 * @param pid The process ID (PID) of the target process.
 * @return The start time since boot in milliseconds, or `ERROR_GET_PROCESS_TIME_FAILED`
 *         if the process does not exist.
 */
extern int64_t CODE_OS_GetStartTimeFromBoot(int32_t pid)
{
    return GetProcessTime(pid, TIMEKIND_CREATE);
}

/**
 * @brief Retrieves the start time of a process in milliseconds since the Unix epoch.
 *
 * Combines the system boot time and the process's start time relative to the boot
 * to calculate the absolute start time in Unix epoch milliseconds.
 *
 * @param pid The process ID (PID) of the target process.
 * @return The process start time in milliseconds since the Unix epoch, or:
 *         - `ERROR_GET_PROCESS_TIME_FAILED` if either the boot time or process time cannot be retrieved.
 */
extern int64_t CODE_OS_GetStartTimeFromUnixEpoch(int32_t pid)
{
    int64_t bootTime = GetBootTime();
    int64_t startTime = GetProcessTime(pid, TIMEKIND_CREATE);
    if (bootTime == ERROR_NOT_FIND_BOOTTIME || startTime == ERROR_GET_PROCESS_TIME_FAILED) {
        return ERROR_GET_PROCESS_TIME_FAILED;
    }

    return bootTime + startTime;
}

/**
 * @brief Retrieves the kernel-mode CPU time used by a process in milliseconds.
 *
 * This function provides the total kernel-mode CPU time consumed by the process.
 *
 * @param pid The process ID (PID) of the target process.
 * @return The kernel-mode time in milliseconds, or `ERROR_GET_PROCESS_TIME_FAILED`
 *         if the process does not exist.
 */
extern int64_t CODE_OS_GetSystemTime(int32_t pid)
{
    return GetProcessTime(pid, TIMEKIND_SYSTEM);
}

/**
 * @brief Retrieves the user-mode CPU time used by a process in milliseconds.
 *
 * This function provides the total user-mode CPU time consumed by the process.
 *
 * @param pid The process ID (PID) of the target process.
 * @return The user-mode time in milliseconds, or `ERROR_GET_PROCESS_TIME_FAILED`
 *         if the process does not exist.
 */
extern int64_t CODE_OS_GetUserTime(int32_t pid)
{
    return GetProcessTime(pid, TIMEKIND_USER);
}

/**
 * @brief Checks the alive status of a process based on its PID and last known start time.
 *
 * This function determines if a process is still running, if its PID has been reused,
 * or if it no longer exists by comparing the process's current start time to a given
 * last known start time.
 *
 * @param pid The process ID (PID) of the target process.
 * @param lastTime The last known start time of the process (in milliseconds since boot).
 *                 If `lastTime` is 0, it is treated as unknown.
 * @return One of the following status codes:
 *         - `PROCESS_STATUS_ALIVE`: The process is alive, and its start time matches the `lastTime`.
 *         - `PROCESS_STATUS_PID_REUSED`: The PID has been reused by a new process (new start time detected).
 *         - `PROCESS_STATUS_NOT_EXIST`: The process does not exist.
 */
extern int8_t CODE_OS_GetProcessAliveStatus(int32_t pid, int64_t lastTime)
{
    int64_t newTime = CODE_OS_GetStartTimeFromBoot(pid);
    if ((lastTime > 0 && lastTime == newTime) || (lastTime == 0 || newTime == 0)) {
        return PROCESS_STATUS_ALIVE;
    }
    return newTime > 0 ? PROCESS_STATUS_PID_REUSED : PROCESS_STATUS_NOT_EXIST;
}

extern void CODE_OS_FreeProcessInfo(ProcessInfo* info)
{
    if (info != NULL) {
        free(info->command);
        info->command = NULL;
        free(info->workingDirectory);
        info->workingDirectory = NULL;
        FreeTwoDimensionalArray(info->commandLine);
        FreeTwoDimensionalArray(info->arguments);
        FreeTwoDimensionalArray(info->environment);
        free(info);
    }
}

extern size_t CODE_OS_GetProcessHandle(int32_t pid)
{
    return -1;
}

extern void CODE_OS_CloseProcessHandle(size_t handle)
{
    return;
}

extern ProcessInfo* CODE_OS_GetProcessInfoByPid(int32_t pid)
{
    int64_t firstTime = CODE_OS_GetStartTimeFromBoot(pid);
    if (firstTime == ERROR_GET_PROCESS_TIME_FAILED) {
        return NULL;
    }

    ProcessInfo* result = (ProcessInfo*)calloc(1, sizeof(ProcessInfo));
    if (result == NULL) {
        return NULL;
    }

    size_t cmdLen = 0;
    char* cmdInfo = GetProcessCmdline(pid, &cmdLen);
    result->environment = GetEnvironment(pid);
    result->workingDirectory = GetWorkingDirectory(pid);
    result->command = GetCommand(cmdInfo, cmdLen);
    result->arguments = GetArguments(cmdInfo, cmdLen);
    result->commandLine = GetCommandLine(result->command, result->arguments);
    free(cmdInfo);
    int64_t lastTime = CODE_OS_GetStartTimeFromBoot(pid);
    if (firstTime != lastTime || lastTime == ERROR_GET_PROCESS_TIME_FAILED) {
        CODE_OS_FreeProcessInfo(result);
        return NULL;
    }

    return result;
}
