/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include "string_SIMD.h"
#include <string.h>

#define X86_64_OFFSET 32
#define AARCH64_OFFSET 16
#define MAX_UINT8 255
#define FIRST_AND_LAST 2
#define OFFSET_4 4
#define OFFSET_8 8
#define OFFSET_12 12
#define SIZE_0 0
#define SIZE_1 1
#define SIZE_2 2
#define SIZE_3 3
#define SIZE_4 4
#define SIZE_5 5
#define SIZE_6 6
#define SIZE_7 7
#define SIZE_8 8
#define SIZE_9 9
#define SIZE_10 10
#define SIZE_11 11
#define SIZE_12 12
#define SIZE_13 13
#define SIZE_14 14
#define SIZE_15 15
#define SIZE_16 16

__attribute__((unused)) static _Bool MemCmp0(const uint8_t* a, const uint8_t* b, int64_t l)
{
    return 1;
}

__attribute__((unused)) static _Bool MemCmp1(const uint8_t* a, const uint8_t* b, int64_t l)
{
    return a[0] == b[0];
}

__attribute__((unused)) static _Bool MemCmp2(const uint8_t* a, const uint8_t* b, int64_t l)
{
    const uint16_t x = *(const uint16_t*)a;
    const uint16_t y = *(const uint16_t*)b;
    return x == y;
}

__attribute__((unused)) static _Bool MemCmp4(const uint8_t* a, const uint8_t* b, int64_t l)
{
    const uint32_t x = *(const uint32_t*)a;
    const uint32_t y = *(const uint32_t*)b;
    return x == y;
}

__attribute__((unused)) static _Bool MemCmp5(const uint8_t* a, const uint8_t* b, int64_t l)
{
    return MemCmp4(a, b, l) && (a[OFFSET_4] == b[OFFSET_4]);
}

__attribute__((unused)) static _Bool MemCmp6(const uint8_t* a, const uint8_t* b, int64_t l)
{
    return MemCmp4(a, b, l) && MemCmp2(a + OFFSET_4, b + OFFSET_4, l);
}

__attribute__((unused)) static _Bool MemCmp8(const uint8_t* a, const uint8_t* b, int64_t l)
{
    const uint64_t x = *(const uint64_t*)a;
    const uint64_t y = *(const uint64_t*)b;
    return x == y;
}

__attribute__((unused)) static _Bool MemCmp9(const uint8_t* a, const uint8_t* b, int64_t l)
{
    const uint64_t x = *(const uint64_t*)a;
    const uint64_t y = *(const uint64_t*)b;
    return (x == y) && (a[OFFSET_8] == b[OFFSET_8]);
}

__attribute__((unused)) static _Bool MemCmp10(const uint8_t* a, const uint8_t* b, int64_t l)
{
    const uint64_t aq = *(const uint64_t*)a;
    const uint64_t bq = *(const uint64_t*)b;
    const uint16_t aw = *(const uint16_t*)(a + OFFSET_8);
    const uint16_t bw = *(const uint16_t*)(b + OFFSET_8);
    return (aq == bq) && (aw == bw);
}

__attribute__((unused)) static _Bool MemCmp12(const uint8_t* a, const uint8_t* b, int64_t l)
{
    const uint64_t aq = *(const uint64_t*)a;
    const uint64_t bq = *(const uint64_t*)b;
    const uint32_t ad = *(const uint32_t*)(a + OFFSET_8);
    const uint32_t bd = *(const uint32_t*)(b + OFFSET_8);
    return (aq == bq) && (ad == bd);
}

__attribute__((unused)) static _Bool MemCmp13(const uint8_t* a, const uint8_t* b, int64_t l)
{
    return MemCmp12(a, b, l) && (a[OFFSET_12] == b[OFFSET_12]);
}

__attribute__((unused)) static _Bool MemCmp14(const uint8_t* a, const uint8_t* b, int64_t l)
{
    return MemCmp12(a, b, l) && MemCmp2(a + OFFSET_12, b + OFFSET_12, l);
}

__attribute__((unused)) inline static _Bool MemCmpN(const uint8_t* a, const uint8_t* b, int64_t l)
{
    return memcmp(a, b, (size_t)(l - FIRST_AND_LAST)) == 0;
}

#ifdef __x86_64__
#include <immintrin.h>
inline __attribute__((always_inline)) static int64_t StrStrN(const uint8_t* org, int64_t ol, const uint8_t* sub,
    int64_t sl, _Bool memcmpFunc(const uint8_t*, const uint8_t*, int64_t))
{
    const __m256i f = _mm256_set1_epi8((char)sub[0]);
    const __m256i l = _mm256_set1_epi8((char)sub[sl - 1]);
    int64_t el = (ol - sl) - X86_64_OFFSET;
    int64_t i = 0;
    for (; i <= el; i += X86_64_OFFSET) {
        const __m256i blockF = _mm256_loadu_si256((const __m256i*)(org + i));
        const __m256i equalF = _mm256_cmpeq_epi8(f, blockF);
        const __m256i blockL = _mm256_loadu_si256((const __m256i*)(org + i + sl - 1));
        const __m256i equalL = _mm256_cmpeq_epi8(l, blockL);
        uint32_t mask = (uint32_t)_mm256_movemask_epi8(_mm256_and_si256(equalF, equalL));
        while (mask != 0) {
            const int64_t bitPos = __builtin_ctz(mask);
            if (memcmpFunc(org + i + bitPos + 1, sub + 1, sl)) {
                return i + bitPos;
            }
            mask = mask & (mask - 1);
        }
    }
    uint8_t subF = sub[0];
    uint8_t subL = sub[sl - 1];
    for (; i <= ol - sl; i++) {
        uint8_t orgF = org[i];
        uint8_t orgL = org[i + sl - 1];
        if (subF == orgF && subL == orgL && memcmpFunc(org + i + 1, sub + 1, sl)) {
            return i;
        }
    }
    return -1;
}
#endif

#ifdef __aarch64__
#include <arm_neon.h>
inline __attribute__((always_inline)) static int64_t StrStrN(const uint8_t* org, int64_t ol, const uint8_t* sub,
    int64_t sl, _Bool memcmpFunc(const uint8_t*, const uint8_t*, int64_t))
{
    const uint8x16_t F = vdupq_n_u8(sub[0]);
    const uint8x16_t l = vdupq_n_u8(sub[sl - 1]);
    int64_t el = (ol - sl) - AARCH64_OFFSET;
    int64_t i = 0;
    for (; i <= el; i += AARCH64_OFFSET) {
        const uint8x16_t blockF = vld1q_u8(org + i);
        const uint8x16_t equalF = vceqq_u8(F, blockF);
        const uint8x16_t blockL = vld1q_u8(org + i + sl - 1);
        const uint8x16_t equalL = vceqq_u8(l, blockL);
        const uint8x16_t pred_16 = vandq_u8(equalF, equalL);
        uint64_t mask = vgetq_lane_u64(vreinterpretq_u64_u8(pred_16), 0);
        if (mask) {
            for (int j = 0; j < OFFSET_8; j++) {
                if ((mask & 0xff) & memcmpFunc(org + i + j + 1, sub + 1, sl)) {
                    return i + j;
                }
                mask >>= OFFSET_8;
            }
        }
        mask = vgetq_lane_u64(vreinterpretq_u64_u8(pred_16), 1);
        if (mask) {
            for (int j = 0; j < OFFSET_8; j++) {
                if ((mask & 0xff) & memcmpFunc(org + i + j + OFFSET_8 + 1, sub + 1, sl)) {
                    return i + j + OFFSET_8;
                }
                mask >>= OFFSET_8;
            }
        }
    }
    uint8_t subF = sub[0];
    uint8_t subL = sub[sl - 1];
    for (; i <= ol - sl; i++) {
        uint8_t orgF = org[i];
        uint8_t orgL = org[i + sl - 1];
        if (subF == orgF && subL == orgL && memcmpFunc(org + i + 1, sub + 1, sl)) {
            return i;
        }
    }
    return -1;
}
#endif

#ifdef __arm__
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

int64_t StrStrN(const uint8_t* org, int64_t ol, const uint8_t* sub,
                       int64_t sl, bool (*memcmpFunc)(const uint8_t*, const uint8_t*, int64_t))
{

    if (!org || !sub || !memcmpFunc || ol < 0 || sl < 0) {
        return -1;
    }
    if (sl == 0) {
        return 0;
    }
    if (sl > ol) {
        return -1;
    }
    if (sl == 1) {
        const uint8_t* result = (const uint8_t*)memchr(org, sub[0], ol);
        return result ? (int64_t)(result - org) : -1;
    }
    const int64_t search_limit = ol - sl;
    const uint8_t sub_first = sub[0];
    const uint8_t sub_last = sub[sl - 1];
    const int64_t cmp_len_middle = sl - 2;

    for (int64_t i = 0; i <= search_limit; ++i) {
        if (org[i] == sub_first && org[i + sl - 1] == sub_last) {
            if (cmp_len_middle <= 0 || memcmpFunc(org + i + 1, sub + 1, cmp_len_middle)) {
                return i; 
            }
        }
    }
    return -1;
}
#endif

// instruction set acceleration
int64_t FastStrstr(const uint8_t* org, int64_t ol, const uint8_t* sub, int64_t sl)
{
    int64_t result = -1;
    if (ol < sl) {
        return result;
    }
    switch (sl) {
        case SIZE_0:
            return SIZE_0;
        case SIZE_1: {
            const uint8_t* res = (const uint8_t*)strchr((char*)org, sub[0]);
            return ((res != NULL) && (res < org + ol)) ? res - org : -1;
        }
        case SIZE_2:
            result = StrStrN(org, ol, sub, SIZE_2, MemCmp0);
            break;
        case SIZE_3:
            result = StrStrN(org, ol, sub, SIZE_3, MemCmp1);
            break;
        case SIZE_4:
            result = StrStrN(org, ol, sub, SIZE_4, MemCmp2);
            break;
        case SIZE_5:
            result = StrStrN(org, ol, sub, SIZE_5, MemCmp4);
            break;
        case SIZE_6:
            result = StrStrN(org, ol, sub, SIZE_6, MemCmp4);
            break;
        case SIZE_7:
            result = StrStrN(org, ol, sub, SIZE_7, MemCmp5);
            break;
        case SIZE_8:
            result = StrStrN(org, ol, sub, SIZE_8, MemCmp6);
            break;
        case SIZE_9:
            result = StrStrN(org, ol, sub, SIZE_9, MemCmp8);
            break;
        case SIZE_10:
            result = StrStrN(org, ol, sub, SIZE_10, MemCmp8);
            break;
        case SIZE_11:
            result = StrStrN(org, ol, sub, SIZE_11, MemCmp9);
            break;
        case SIZE_12:
            result = StrStrN(org, ol, sub, SIZE_12, MemCmp10);
            break;
        case SIZE_13:
            result = StrStrN(org, ol, sub, SIZE_13, MemCmp12);
            break;
        case SIZE_14:
            result = StrStrN(org, ol, sub, SIZE_14, MemCmp12);
            break;
        case SIZE_15:
            result = StrStrN(org, ol, sub, SIZE_15, MemCmp13);
            break;
        case SIZE_16:
            result = StrStrN(org, ol, sub, SIZE_16, MemCmp14);
            break;
        default:
            result = StrStrN(org, ol, sub, sl, MemCmpN);
            break;
    }
    if (result <= ol - sl) {
        return result;
    } else {
        return -1;
    }
}

__attribute__((unused)) static inline int64_t DownAlign32(int64_t value)
{
    return (int64_t)((uint64_t)value & (~(0b11111))); // clear the last 5 bits to align 32
}

__attribute__((unused)) static inline int64_t DownAlign16(int64_t value)
{
    return (int64_t)((uint64_t)value & (~(0b1111))); // clear the last 4 bits to align 16
}

#ifdef __x86_64__
#include <immintrin.h>
inline __attribute__((always_inline)) static int64_t StringSize(const uint8_t* str, int64_t len)
{
    const __m256i uint191 = _mm256_set1_epi8((char)191); // -65
    //  -65 < x
    int64_t size = 0;
    int64_t i = 0;
    for (; i < DownAlign32(len); i += X86_64_OFFSET) {
        const __m256i vStr32 = _mm256_loadu_si256((const __m256i*)(str + i));
        uint32_t mask = (uint32_t)_mm256_movemask_epi8(_mm256_cmpgt_epi8(vStr32, uint191));
        size += __builtin_popcount(mask);
    }
    uint8_t* tmp = (uint8_t*)str + i;
    uint8_t* end = (uint8_t*)str + len;
    for (; tmp < end; ++tmp) {
        if ((*tmp & 0xc0) != 0x80) {
            size++;
        }
    }
    return size;
}
#endif

#ifdef __aarch64__

#include <arm_neon.h>
inline __attribute__((always_inline)) static int64_t StringSize(const uint8_t* str, int64_t len)
{
    const int8x16_t int8_191 = vdupq_n_s8((signed char)191); // -65
    int64_t size = 0;
    int64_t i = 0;
    for (; i < DownAlign16(len); i += AARCH64_OFFSET) {
        const int8x16_t v_str_16 = vld1q_s8((const signed char*)str + i);
        uint16_t res = vaddlvq_u8(vcgtq_s8(v_str_16, int8_191));
        size += (int64_t)res;
    }
    size = size / MAX_UINT8;
    uint8_t* tmp = (uint8_t*)str + i;
    uint8_t* end = (uint8_t*)str + len;
    for (; tmp < end; ++tmp) {
        size += ((*tmp & 0xc0) != 0x80);
    }
    return size;
}

#endif

#ifdef __arm__
inline __attribute__((always_inline)) static int64_t StringSize(const uint8_t* str, int64_t len)
{
    int64_t size = 0;
    uint8_t* tmp = (uint8_t*)str;
    uint8_t* end = (uint8_t*)str + len;
 
    for (; tmp < end; ++tmp) {
        size += ((*tmp & 0xc0) != 0x80);
    }
    return size;
}
 
#endif

int64_t FastSize(const uint8_t* str, int64_t len)
{
    return StringSize(str, len);
}
