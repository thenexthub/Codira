/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#ifdef __ios__
#include <time.h>
#else
#include <libproc.h>
#endif
#include <sys/types.h>
#include <sys/sysctl.h>
#include "securec.h"
#include "process_ffi_unix.h"

#if defined(__arm64__) || defined(__aarch64__)
#define TICKS_TO_NANOSECONDS (125.0 / 3)
#elif defined(__x86_64__)
#define TICKS_TO_NANOSECONDS (1.0)
#endif

#define MAX_PATH_LEN (4096)
#define SYSCTL_ARG_NUM_PROC_PID 4

/**
 * @brief Retrieves the total user or system time for a specific process.
 *
 * This function queries process information for a given PID and returns either the
 * total user time or total system time, depending on the specified `kind` parameter.
 * The times are returned in milliseconds.
 *
 * @param pid The process ID of the target process.
 * @param kind Specifies the type of time to retrieve. It can be one of the following:
 *             - `TIMEKIND_USER` to get the total user time of the process.
 *             - `TIMEKIND_SYSTEM` to get the total system time of the process.
 *
 * @return The requested time in milliseconds, or an error code if the request fails:
 *         - A positive integer representing the time in milliseconds if successful.
 *         - `ERROR_GET_PROCESS_TIME_FAILED` if the time could not be retrieved or process information query failed.
 */
int64_t GetSystemTimeOrUserTime(int32_t pid, int8_t kind)
{
#ifdef __ios__
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) != 0) {
        return ERROR_GET_PROCESS_TIME_FAILED;
    }
    if (kind == TIMEKIND_USER) {
        return usage.ru_utime.tv_sec * SECOND_TO_MILLI_SECOND + usage.ru_utime.tv_usec / MILLI_SECOND_TO_MICRO_SECOND;
    } else {
        return usage.ru_stime.tv_sec * SECOND_TO_MILLI_SECOND + usage.ru_stime.tv_usec / MILLI_SECOND_TO_MICRO_SECOND;
    }
#else
    struct proc_taskinfo pti;
    int result = proc_pidinfo(pid, PROC_PIDTASKINFO, 0, &pti, sizeof(pti));
    if (result <= 0) { // result == 0 means the number of bytes written into provided buffer is `0`
        return ERROR_GET_PROCESS_TIME_FAILED;
    }

    if (kind == TIMEKIND_USER) {
        return pti.pti_total_user * TICKS_TO_NANOSECONDS / MILLI_SECOND_TO_NANO_SECOND;
    } else if (kind == TIMEKIND_SYSTEM) {
        return pti.pti_total_system * TICKS_TO_NANOSECONDS / MILLI_SECOND_TO_NANO_SECOND;
    } else {
        return ERROR_GET_PROCESS_TIME_FAILED;
    }
#endif
}

int64_t GetSystemTime(int32_t pid)
{
    return GetSystemTimeOrUserTime(pid, TIMEKIND_SYSTEM);
}

int64_t GetUserTime(int32_t pid)
{
    return GetSystemTimeOrUserTime(pid, TIMEKIND_USER);
}

/**
 * @brief Retrieves a specific time metric for a process based on its PID.
 *
 * @note Inner function, no need to check @param kind.
 *
 * This function uses the `sysctl` system call to query process information and extracts
 * time metrics such as:
 * - Process start time (creation time).
 * - User-mode CPU time used.
 * - Kernel-mode CPU time used.
 *
 * @param pid The process ID (PID) of the target process. Note: PID 0 is not supported.
 * @param kind The type of time to retrieve:
 *             - TIMEKIND_CREATE: Process creation time (in milliseconds since boot).
 *             - TIMEKIND_USER: User-mode CPU time (in milliseconds).
 *             - TIMEKIND_SYSTEM: Kernel-mode CPU time (in milliseconds).
 * @return The requested time in milliseconds, or:
 *         - `ERROR_GET_PROCESS_TIME_FAILED` if the process does not exist or cannot be queried.
 */
extern int64_t GetProcessTime(int32_t pid, int8_t kind)
{
    // Pid 0 is a kernal task, not supported.
    if (pid == 0) {
        return ERROR_GET_PROCESS_TIME_FAILED;
    }

    if (kind == TIMEKIND_CREATE) {
        struct kinfo_proc kp = {0};
        size_t bufSize = sizeof(kp);

        int mib[SYSCTL_ARG_NUM_PROC_PID] = {CTL_KERN, KERN_PROC, KERN_PROC_PID, pid};
        int ret = sysctl(mib, SYSCTL_ARG_NUM_PROC_PID, &kp, &bufSize, NULL, 0);
        if (ret < 0 || bufSize <= 0 || kp.kp_proc.p_pid != pid) {
            return ERROR_GET_PROCESS_TIME_FAILED;
        }

        return (int64_t)(kp.kp_proc.p_starttime.tv_sec * SECOND_TO_MILLI_SECOND +
            kp.kp_proc.p_starttime.tv_usec / MILLI_SECOND_TO_MICRO_SECOND);
    } else if (kind == TIMEKIND_SYSTEM) {
        return GetSystemTime(pid);
    } else {
        return GetUserTime(pid);
    }
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
    return GetProcessTime(pid, TIMEKIND_CREATE);
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
 *         - `PROCESS_STATUS_ALIVE` (0): The process is alive, and its start time matches the `lastTime`.
 *         - `PROCESS_STATUS_PID_REUSED` (1): The PID has been reused by a new process (new start time detected).
 *         - `PROCESS_STATUS_NOT_EXIST` (2): The process does not exist.
 */
extern int8_t CODE_OS_GetProcessAliveStatus(int32_t pid, int64_t lastTime)
{
    int64_t newTime = CODE_OS_GetStartTimeFromUnixEpoch(pid);
    if ((lastTime > 0 && lastTime == newTime) || (lastTime == 0 || newTime == 0)) {
        return PROCESS_STATUS_ALIVE;
    }
    return newTime > 0 ? PROCESS_STATUS_PID_REUSED : PROCESS_STATUS_NOT_EXIST;
}

char* SkipZeroChar(char* start, char* end)
{
    char* cp = start;
    for (; cp < end; ++cp) {
        if (*cp != '\0') {
            break;
        }
    }
    return cp;
}

char** GetEnvironment(char* start, char* end)
{
    // Count the num of environment variable
    char* cp = SkipZeroChar(start, end);
    size_t count = 0;
    while (cp < end) {
        if (*cp == '\0') {
            ++count;
            cp = SkipZeroChar(cp, end);
        } else {
            ++cp;
        }
    }

    if (count <= 0) {
        return NULL;
    }

    char** result = (char**)calloc(count + 1, sizeof(char*));
    if (result == NULL) {
        return NULL;
    }

    cp = start;
    size_t index = 0;
    for (char* s = cp; index < count && cp < end; ++cp) {
        if (*cp == '\0') {
            size_t len = cp - s;
            if (len <= 0) {
                break;
            }

            char* env = (char*)malloc(len + 1);
            if (env == NULL) {
                break;
            }

            if (memcpy_s(env, len + 1, s, len) != EOK) {
                free(env);
                break;
            }
            env[len] = '\0';
            result[index++] = env;
            s = cp + 1;
        }
    }

    return result;
}

char** GetCommandline(char* argv, size_t len, size_t argc, char** envStartAddr)
{
    if (argc <= 0 || len <= 0) {
        return NULL;
    }
    char** arguments = (char**)calloc(argc + 1, sizeof(char*));
    if (arguments == NULL) {
        return NULL;
    }

    char* cp = argv;
    int argumentCount = 0;
    for (char* start = cp; argumentCount < argc && cp < argv + len; ++cp) {
        if (*cp == '\0') {
            size_t argNLen = cp - start;

            char* argN = (char*)calloc(argNLen + 1, sizeof(char));
            if (argN == NULL) {
                break;
            }

            if (argNLen > 0) {
                if (memcpy_s(argN, argNLen + 1, start, argNLen) != EOK) {
                    free(argN);
                    break;
                }
            }

            arguments[argumentCount++] = argN;
            start = cp + 1;
        }
    }
    *envStartAddr = cp;
    return arguments;
}

#define SYSCTL_ARGNUM_ARGMAX 2
#define SYSCTL_ARGNUM_PROCARGS2 3
#define MAX_RETRY 3
#define BASE_RETRY_DELAY 200

char* GetProcessArgs(pid_t pid, size_t* length)
{
    int maxArgsLen = 0;
    size_t size = sizeof(maxArgsLen);
    // Get the maximum size of the arguments
    int mib2[SYSCTL_ARGNUM_ARGMAX] = {CTL_KERN, KERN_ARGMAX};

    if (sysctl(mib2, SYSCTL_ARGNUM_ARGMAX, &maxArgsLen, &size, NULL, 0) == -1) {
        return NULL;
    }

    if (maxArgsLen <= 0) {
        return NULL;
    }

    char* argv = (char*)malloc((size_t)maxArgsLen);
    if (argv == NULL) {
        return NULL;
    }

    int mib3[SYSCTL_ARGNUM_PROCARGS2] = {CTL_KERN, KERN_PROCARGS2, pid};
    size = (size_t)maxArgsLen;
    int ret = -1;
    int retry = 0;

    while (retry < MAX_RETRY) {
        ret = sysctl(mib3, SYSCTL_ARGNUM_PROCARGS2, argv, &size, NULL, 0);
        if (ret == 0) {
            break;
        }

        switch (errno) {
            case EIO:
            case EBUSY:
            case EAGAIN:
                retry++;
                usleep(BASE_RETRY_DELAY * retry);
                break;
            default:
                retry = MAX_RETRY;
                break;
        }
    }

    if (ret == -1) {
        free(argv);
        argv = NULL;
    } else {
        *length = size;
    }

    return argv;
}

char* SkipExecPath(char* argvStart, char* argvEnd)
{
    char* cp = argvStart;
    for (; cp < argvEnd; ++cp) {
        if (*cp == '\0') {
            // End of evec_path reached.
            break;
        }
    }

    return SkipZeroChar(cp, argvEnd);
}

void GetProcessCmdAndEnv(pid_t pid, ProcessInfo* out)
{
    size_t size = 0;
    char* argv = GetProcessArgs(pid, &size);
    if (argv == NULL || size == 0) {
        return;
    }

    out->environment = GetCurrentProcessEnvironment((int32_t)pid);

    /*
     * Make a sysctl() call to get the raw argument space of the process.
     * The layout is documented in start.s, which is part of the Csu
     * project.  In summary, it looks like:
     * reference ps source code.
     * /---------------\ 0x00000000
     * :               :
     * :               :
     * |---------------|
     * | argc          |
     * |---------------|
     * | arg[0]        |
     * |---------------|
     * :               :
     * :               :
     * |---------------|
     * | arg[argc - 1] |
     * |---------------|
     * | 0             |
     * |---------------|
     * | env[0]        |
     * |---------------|
     * :               :
     * :               :
     * |---------------|
     * | env[n]        |
     * |---------------|
     * | 0             |
     * |---------------| <-- Beginning of data returned by sysctl() is here.
     * | argc          |
     * |---------------|
     * | exec_path     |
     * |:::::::::::::::|
     * |               |
     * | String area.  |
     * |               |
     * |---------------| <-- Top of stack.
     * :               :
     * :               :
     * \---------------/ 0xffffffff
     */

    int argc;
    errno_t ret = memcpy_s(&argc, sizeof(argc), argv, sizeof(argc));
    if (ret != EOK || argc <= 0) {
        free(argv);
        return;
    }

    char* argvEnd = &argv[size];
    char* cp = SkipExecPath(argv + sizeof(argc), argvEnd);
    if (cp == argvEnd) {
        free(argv);
        return;
    }

    char* argv0Start = cp;
    // Get arguments
    if (argc > 0) {
        char** commandLine = GetCommandline(argv0Start, argvEnd - argv0Start, argc, &cp);
        if (commandLine != NULL) {
            out->commandLine = commandLine;
            out->command = out->commandLine[0];
            out->arguments = &out->commandLine[1];
        }
    }

    if (cp >= argvEnd) {
        free(argv);
        return;
    }

    free(argv);
    return;
}

char* GetCurrentWorkingDirectory(pid_t pid)
{
#ifdef __ios__
    const char *home = getenv("HOME");
    if (home == NULL) {
        return NULL;
    }

    int length = strlen(home);
    char* workingDirectory = (char*)calloc(length + 1, sizeof(char));
    if (workingDirectory == NULL) {
        return NULL;
    }

    if (memcpy_s(workingDirectory, length + 1, home, length) != EOK) {
        free(workingDirectory);
        return NULL;
    }
    workingDirectory[length] = '\0';
    return workingDirectory;
#else
    struct proc_vnodepathinfo vpi;
    int ret = proc_pidinfo(pid, PROC_PIDVNODEPATHINFO, 0, &vpi, sizeof(vpi));
    if (ret <= 0) {
        return NULL;
    }

    int length = strlen(vpi.pvi_cdir.vip_path);
    if (length <= 0 || length >= MAX_PATH_LEN) {
        return NULL;
    }

    char* workingDirectory = (char*)calloc(length + 1, sizeof(char));
    if (workingDirectory == NULL) {
        return NULL;
    }

    if (memcpy_s(workingDirectory, length + 1, vpi.pvi_cdir.vip_path, length) != EOK) {
        free(workingDirectory);
        return NULL;
    }
    workingDirectory[length] = '\0';

    return workingDirectory;
#endif
}

extern void CODE_OS_FreeProcessInfo(ProcessInfo* info)
{
    if (info != NULL) {
        free(info->workingDirectory);
        info->workingDirectory = NULL;
        FreeTwoDimensionalArray(info->commandLine);
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
    int64_t firstTime = CODE_OS_GetStartTimeFromUnixEpoch(pid);
    if (firstTime == ERROR_GET_PROCESS_TIME_FAILED) {
        return NULL;
    }

    ProcessInfo* result = (ProcessInfo*)calloc(1, sizeof(ProcessInfo));
    if (result == NULL) {
        return NULL;
    }

#ifdef __ios__
    result->command = (char *)getprogname();
    result->environment = GetCurrentProcessEnvironment(pid);
#else
    GetProcessCmdAndEnv(pid, result);
#endif
    result->workingDirectory = GetCurrentWorkingDirectory(pid);
    int64_t lastTime = CODE_OS_GetStartTimeFromUnixEpoch(pid);
    if (firstTime != lastTime || lastTime == ERROR_GET_PROCESS_TIME_FAILED) {
        CODE_OS_FreeProcessInfo(result);
        return NULL;
    }

    return result;
}
