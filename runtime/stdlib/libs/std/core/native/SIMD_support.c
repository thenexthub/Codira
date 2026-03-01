/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "core.h"

bool CODE_CORE_CanUseSIMD(void)
{
    static int8_t simdSUPPORT = -1;
    if (simdSUPPORT < 0) {
#if defined(__linux__) || defined(__APPLE__)
#if defined(__x86_64__)
        __builtin_cpu_init();
        simdSUPPORT = __builtin_cpu_supports("avx") && __builtin_cpu_supports("avx2");
#elif defined(__aarch64__)
        simdSUPPORT = 1;
#else
        simdSUPPORT = 0;
#endif
#else
        simdSUPPORT = 0;
#endif
    }
    return simdSUPPORT > 0;
}
