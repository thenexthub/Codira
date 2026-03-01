/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#ifndef CODIRA_CORE_H
#define CODIRA_CORE_H

#include <stdint.h>
#include <stdbool.h>

#if defined(_WIN32) && defined(__MINGW64__)
#define SECUREC_DLL_IMPORT
#endif

#ifndef _MSC_BUILD
#include "securec.h"
#else
#define EOK 0
#endif

typedef struct StrptrParameter {
    uint8_t* ptr;
    int64_t size;
} Strptr;

bool CODE_CORE_CanUseSIMD(void);

int64_t CODE_CORE_FromCharToUtf8(uint32_t c, uint8_t* itemBytes);

void CODE_CORE_HashStr(const uint8_t* str, int64_t strSize, uint32_t* hash, uint32_t* pow);
void CODE_CORE_HashStrRev(const uint8_t* str, int64_t strSize, uint32_t* hash, uint32_t* pow);
int64_t CODE_CORE_RabinKarp(Strptr* org, Strptr* sub, uint32_t hashP, uint32_t pow);
int64_t CODE_CORE_RabinKarpR(Strptr* org, Strptr* sub, uint32_t hashP, uint32_t pow);
int64_t CODE_CORE_IndexOfByte(const uint8_t* orgStr, int64_t orgSize, uint8_t pat);
int64_t CODE_CORE_LastIndexOfByte(const uint8_t* orgStr, int64_t orgSize, uint8_t pat);
int64_t CODE_CORE_CountOfByte(const uint8_t* orgStr, int64_t orgSize, uint8_t pat);
int64_t* CODE_CORE_CountAndIndexOfByte(const uint8_t* orgStr, int64_t orgSize, uint8_t pat);

#endif // CODIRA_CORE_H
