/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#ifndef PROCESS_FFI_H
#define PROCESS_FFI_H

#ifdef __cplusplus
extern "C" {
#endif

#define STDIN (0)
#define STDOUT (1)
#define STDERR (2)
#define STD_COUNT (3)

#define READ (0)
#define WRITE (1)
#define WR_COUNT (2)

#define ERROR_GET_PROCESS_TIME_FAILED (-1)
#define ERROR_NOT_FIND_BOOTTIME (-1)

#define SECOND_TO_MILLI_SECOND (1000)
#define SECOND_TO_MICRO_SECOND (1000000)
#define MILLI_SECOND_TO_NANO_SECOND (1000000)
#define MILLI_SECOND_TO_HUNDRED_NANO_SECOND (10000)
#define MILLI_SECOND_TO_MICRO_SECOND (1000)

typedef struct ProcessInfo {
    char* command;
    char** commandLine;
    char** arguments;
    char* workingDirectory;
    char** environment;
} ProcessInfo;

#define TIMEKIND_CREATE (0)
#define TIMEKIND_SYSTEM (1)
#define TIMEKIND_USER (2)
#define TIMEKIND_EXIT (3)
#define PROCESS_TIME_ARRAY_LENGTH (3) // for Unix, plus one is fine on Windows because of extra exitTime.

#define PROCESS_STATUS_ALIVE (0)
#define PROCESS_STATUS_NOT_EXIST (-1)
#define PROCESS_STATUS_PID_REUSED (-2)

#ifdef __cplusplus
} // extern "C"
#endif
#endif // PROCESS_FFI_H
