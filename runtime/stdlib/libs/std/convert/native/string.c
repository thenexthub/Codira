/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#ifndef _MSC_BUILD
#include "securec.h"
#else
#define EOK 0
#endif

#include <stdlib.h>

#define BUFFER_SIZE 512

static inline unsigned char* GetFormatString(const char* temp, const size_t size)
{
    if (size > BUFFER_SIZE) {
        return NULL;
    }
    unsigned char* ret = (unsigned char*)malloc(size);
    if (ret == NULL) {
        return NULL;
    }
    memcpy_s(ret, size, temp, size);
    return ret;
}

extern unsigned char* CODE_FORMAT_Float64Formatter(const double num, const char* format)
{
    char temp[BUFFER_SIZE];
    int size = snprintf_s(temp, BUFFER_SIZE, BUFFER_SIZE - 1, format, num);
    if (size <= 0) {
        return NULL;
    }
    return GetFormatString(temp, (size_t)(size + 1));
}
