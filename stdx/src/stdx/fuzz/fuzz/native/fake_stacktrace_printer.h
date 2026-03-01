/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#ifndef CODE_SANCOV_STANDARD_FAKE_STACKTRACE_PRINTER_H
#define CODE_SANCOV_STANDARD_FAKE_STACKTRACE_PRINTER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct FakeInfo {
    void* PC;
    const char* packageName;
    const char* fileName;
    const char* funcName;
    uint64_t lineNumber;
};

struct FakeRegion {
    uintptr_t start, end;
};

void fake_info_formatter(const struct FakeInfo* info, const char* fmt, char* out_buf, uintptr_t out_buf_size);

#ifdef __cplusplus
}
#endif

#endif // CODE_SANCOV_STANDARD_FAKE_STACKTRACE_PRINTER_H
