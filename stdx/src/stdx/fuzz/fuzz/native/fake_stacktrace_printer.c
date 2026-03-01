/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include "fake_stacktrace_printer.h"
#include "securec.h"

#include <stdint.h>
#include <stdio.h>

// reference: compiler-rt/lib/sanitizer_common/sanitizer_stacktrace_printer.h
void fake_info_formatter(const struct FakeInfo* info, const char* fmt, char* out_buf, uintptr_t out_buf_size)
{
    unsigned long off = 0;
    int ret;
    for (const char* p = fmt; *p != '\0'; p++) {
        if (*p != '%') {
            out_buf[off] = *p;
            off += 1;
            continue;
        }
        p++;
        switch (*p) {
            case '%':
                ret = snprintf_s(out_buf + off, out_buf_size - off, (out_buf_size - off) - 1, "%c", *p);
                if (ret >= 0) {
                    off += (unsigned long)ret;
                } else {
                    return;
                }
                break;
            case 'p':
                ret = snprintf_s(out_buf + off, out_buf_size - off, (out_buf_size - off) - 1, "%p", info->PC);
                if (ret >= 0) {
                    off += (unsigned long)ret;
                } else {
                    return;
                }
                break;
            case 'f':
                ret = snprintf_s(out_buf + off, out_buf_size - off, (out_buf_size - off) - 1, "%s", info->funcName);
                if (ret >= 0) {
                    off += (unsigned long)ret;
                } else {
                    return;
                }
                break;
            case 's':
                ret = snprintf_s(out_buf + off, out_buf_size - off, (out_buf_size - off) - 1, "%s", info->fileName);
                if (ret >= 0) {
                    off += (unsigned long)ret;
                } else {
                    return;
                }
                break;
            case 'l':
                ret = snprintf_s(out_buf + off, out_buf_size - off, (out_buf_size - off) - 1, "%lu", info->lineNumber);
                if (ret >= 0) {
                    off += (unsigned long)ret;
                } else {
                    return;
                }
                break;
            case 'F':
                ret = snprintf_s(out_buf + off, out_buf_size - off, (out_buf_size - off) - 1, "in %s", info->funcName);
                if (ret >= 0) {
                    off += (unsigned long)ret;
                } else {
                    return;
                }
                break;
            case 'L':
                ret = snprintf_s(out_buf + off, out_buf_size - off, (out_buf_size - off) - 1, "%s", info->packageName);
                if (ret >= 0) {
                    off += (unsigned long)ret;
                } else {
                    return;
                }
                break;
            default:
                printf("Unsupported specifier in stack frame format: %c (%p)!\n", *p, (void*)p);
                return;
        }
    }
}
