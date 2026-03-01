/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include "core.h"

// PrimeRK is the prime base used in Rabin-Karp algorithm.
static const uint32_t PRIME_RK = 16777619;

void CODE_CORE_HashStr(const uint8_t* str, int64_t strSize, uint32_t* hash, uint32_t* pow)
{
    uint32_t hashV = 0;
    int64_t j = 0;
    while (j < strSize) {
        hashV = hashV * PRIME_RK + (uint32_t)str[j];
        j++;
    }
    uint64_t i = (uint64_t)strSize;
    uint32_t powV = 1;
    uint32_t sq = PRIME_RK;
    while (i > 0) {
        if ((i & 1) != 0) {
            powV *= sq;
        }
        sq *= sq;
        i = i >> 1;
    }
    *hash = hashV;
    *pow = powV;
}

void CODE_CORE_HashStrRev(const uint8_t* str, int64_t strSize, uint32_t* hash, uint32_t* pow)
{
    uint32_t hashV = 0;
    int64_t j = strSize - 1;
    while (j >= 0) {
        hashV = hashV * PRIME_RK + (uint32_t)str[j];
        j--;
    }
    uint64_t i = (uint64_t)strSize;
    uint32_t powV = 1;
    uint32_t sq = PRIME_RK;
    while (i > 0) {
        if ((i & 1) == 1) {
            powV *= sq;
        }
        sq *= sq;
        i = i >> 1;
    }
    *hash = hashV;
    *pow = powV;
}

int64_t CODE_CORE_RabinKarp(Strptr* org, Strptr* sub, uint32_t hashP, uint32_t pow)
{
    if (sub->size < 0 || org->size < 0 || org->size < sub->size) {
        return -1;
    }
    uint32_t hashO = 0;
    int64_t i;
    for (i = 0; i < sub->size; ++i) {
        hashO = hashO * PRIME_RK + (uint32_t)(org->ptr)[i];
    }
    uint8_t* itemOrgStr = org->ptr;
    if (hashP == hashO && memcmp(itemOrgStr, sub->ptr, (size_t)sub->size) == 0) {
        return 0;
    }
    i = sub->size;
    while (i < org->size) {
        hashO = hashO * PRIME_RK + (uint32_t)(org->ptr)[i];
        hashO -= pow * (uint32_t)(org->ptr)[i - sub->size];
        i++;
        itemOrgStr = org->ptr + i - sub->size;
        if (hashP == hashO && memcmp(itemOrgStr, sub->ptr, (size_t)sub->size) == 0) {
            return i - sub->size;
        }
    }
    return -1;
}

int64_t CODE_CORE_RabinKarpR(Strptr* org, Strptr* sub, uint32_t hashP, uint32_t pow)
{
    if (sub->size < 0 || org->size < 0 || (org->size - sub->size) < 0) {
        return -1;
    }
    uint32_t hashO = 0;
    int64_t i;
    for (i = org->size - 1; i >= org->size - sub->size; --i) {
        hashO = hashO * PRIME_RK + (uint32_t)(org->ptr)[i];
    }
    uint8_t* itemOrgStr = (uint8_t*)org->ptr + org->size - sub->size;
    if (hashP == hashO && memcmp(itemOrgStr, sub->ptr, (size_t)sub->size) == 0) {
        return org->size - sub->size;
    }
    for (i = (org->size - sub->size) - 1; i >= 0; --i) {
        hashO = hashO * PRIME_RK + (uint32_t)(org->ptr)[i];
        hashO -= pow * (uint32_t)(org->ptr)[i + sub->size];
        itemOrgStr = (uint8_t*)org->ptr + i;
        if (hashP == hashO && memcmp(itemOrgStr, sub->ptr, (size_t)sub->size) == 0) {
            return i;
        }
    }
    return -1;
}

int64_t CODE_CORE_IndexOfByte(const uint8_t* orgStr, int64_t orgSize, uint8_t pat)
{
    if (orgSize < 0) {
        return -1;
    }
    for (int64_t i = 0; i < orgSize; i++) {
        if (orgStr[i] == pat) {
            return i;
        }
    }
    return -1;
}

int64_t CODE_CORE_LastIndexOfByte(const uint8_t* orgStr, int64_t orgSize, uint8_t pat)
{
    if (orgSize < 0) {
        return -1;
    }
    uint8_t* start = (uint8_t*)orgStr + orgSize - 1;
    uint8_t* end = (uint8_t*)orgStr;
    for (; start >= end; start--) {
        if (*start == pat) {
            return start - orgStr;
        }
    }
    return -1;
}

int64_t CODE_CORE_CountOfByte(const uint8_t* orgStr, int64_t orgSize, uint8_t pat)
{
    if (orgSize < 0) {
        return 0;
    }
    int64_t total = 0;
    for (int64_t i = 0; i < orgSize; ++i) {
        if (orgStr[i] == pat) {
            total += 1;
        }
    }
    return total;
}

int64_t* CODE_CORE_CountAndIndexOfByte(const uint8_t* orgStr, int64_t orgSize, uint8_t pat)
{
    if (orgSize < 0) {
        return NULL;
    }
    int64_t n;
    int64_t i = 0;
    int64_t total = 0;
    int64_t* result = (int64_t*)malloc(sizeof(int64_t) * (size_t)(orgSize + 1));
    if (result == NULL) {
        return NULL;
    }
    while (i < orgSize) {
        n = CODE_CORE_IndexOfByte(orgStr + i, orgSize - i, pat);
        if (n < 0) {
            result[0] = total;
            return result;
        }
        result[total + 1] = i + n;
        total++;
        i += n + 1;
    }
    result[0] = total;
    return result;
}
