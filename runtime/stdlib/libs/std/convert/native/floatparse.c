/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include <stdlib.h>
#include <string.h>

extern double* CODE_STRTOD(char* str)
{
    if (str == NULL) {
        return NULL;
    }
    char* p = NULL;
    double value = strtod(str, &p);
    if (p == NULL || strlen(p) > 0) {
        return NULL;
    }
    double* floatNumber = (double*)malloc(sizeof(double));
    if (floatNumber == NULL) {
        return NULL;
    }
    *floatNumber = value;
    return floatNumber;
}
