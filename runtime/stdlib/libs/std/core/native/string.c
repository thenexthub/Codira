/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include <string.h>
#include <math.h>
#include "core.h"
#include "string_SIMD.h"

#define DECIMAL 10
#define MAXLENTH_INT64 21

#define FL_TO_STR_MAX 512 /* 512 is the maxlength of double to string size */

static uint8_t* CloneString(const uint8_t* temp, const int size)
{
    if (size <= 0) {
        return NULL;
    }

    uint8_t* ret = (uint8_t*)malloc((size_t)size);
    if (ret == NULL) {
        return NULL;
    }
    errno_t rc = memcpy_s(ret, (size_t)size, temp, (size_t)size);
    if (rc != EOK) {
        free(ret);
        return NULL;
    }
    return ret;
}

extern uint8_t* CODE_CORE_Float64ToCPointer(const double num)
{
    uint8_t temp[FL_TO_STR_MAX]; // 512
    int size;
    if (isnan(num)) {
        // Nan doesn't have a sign in cangjie. So, we should always print nan.
        size = snprintf_s((char*)temp, FL_TO_STR_MAX, FL_TO_STR_MAX - 1, "%s", "nan");
    } else {
        size = snprintf_s((char*)temp, FL_TO_STR_MAX, FL_TO_STR_MAX - 1, "%f", num);
    }
    if (size <= 0) {
        return NULL;
    }
    return CloneString(temp, size + 1);
}

extern int64_t CODE_BUFFER_Int64ToCPointer(const int64_t num, uint8_t* dest, const int64_t destMax)
{
    uint8_t buff[MAXLENTH_INT64]; // maxlength of INT64_MIN is 20
    int64_t itemLen = 0;
    int64_t item = num;
    do {
        buff[(MAXLENTH_INT64 - 1) - itemLen] = (uint8_t)abs((int)(item % DECIMAL)) + '0';
        item = item / DECIMAL;
        itemLen++;
    } while (item != 0);

    if (num < 0) {
        buff[(MAXLENTH_INT64 - 1) - itemLen] = '-';
        itemLen++;
    }
    if (itemLen > destMax) {
        return -1;
    }
    for (int i = 0; i < itemLen; i++) {
        dest[i] = buff[(MAXLENTH_INT64 - itemLen) + i];
    }
    return itemLen;
}

extern int64_t CODE_BUFFER_UInt64ToCPointer(const uint64_t num, uint8_t* dest, const int64_t destMax)
{
    uint8_t buff[MAXLENTH_INT64]; // maxlength of UINT64_MAX is 20
    int64_t itemLen = 0;
    uint64_t item = num;
    do {
        buff[(MAXLENTH_INT64 - 1) - itemLen] = item % DECIMAL + '0';
        item = item / DECIMAL;
        itemLen++;
    } while (item != 0);

    if (itemLen > destMax) {
        return -1;
    }
    for (int i = 0; i < itemLen; i++) {
        dest[i] = buff[(MAXLENTH_INT64 - itemLen) + i];
    }
    return itemLen;
}

extern int64_t CODE_BUFFER_Float64ToCPointer(const double num, uint8_t* buf, const int64_t destMax)
{
    if (destMax <= 0 || NULL == buf) {
        return -1;
    }
    size_t newSize = FL_TO_STR_MAX; // 512
    if (destMax < FL_TO_STR_MAX) {
        newSize = (size_t)destMax;
    }
    int size = snprintf_s((char*)buf, newSize, newSize - 1, "%f", num);
    return size;
}

extern int64_t CODE_CORE_StringMemcmp(const uint8_t* str1, const uint8_t* str2, const size_t n)
{
    if (str1 == str2) {
        return 0;
    }
    return memcmp(str1, str2, n);
}

extern int64_t CODE_CORE_IndexOfString(
    const uint8_t* orgStr, const uint8_t* subStr, int64_t orgSize, int64_t subSize, int64_t start);
extern int64_t CODE_CORE_LastIndexOfString(
    const uint8_t* orgStr, const uint8_t* subStr, int64_t orgSize, int64_t subSize, int64_t start);
extern int64_t CODE_CORE_CountString(const uint8_t* orgStr, const uint8_t* subStr, int64_t orgSize, int64_t subSize);
extern int64_t* CODE_CORE_CountAndIndexString(
    const uint8_t* orgStr, const uint8_t* subStr, int64_t orgSize, int64_t subSize);

int64_t CODE_CORE_IndexOfString(
    const uint8_t* orgStr, const uint8_t* subStr, int64_t orgSize, int64_t subSize, int64_t start)
{
    if (start < 0 || start + subSize > orgSize || NULL == subStr || NULL == orgStr) {
        return -1;
    }
    int64_t n;
    if (subSize == 1) {
        n = CODE_CORE_IndexOfByte(orgStr + start, orgSize - start, subStr[0]);
        return (n < 0) ? -1 : n + start;
    } else {
        if (CODE_CORE_CanUseSIMD()) {
            n = FastStrstr(orgStr + start, orgSize - start, subStr, subSize);
        } else {
            uint32_t hash = 0;
            uint32_t pow = 0;
            CODE_CORE_HashStr(subStr, subSize, &hash, &pow);
            Strptr org = {(uint8_t*)orgStr + start, orgSize - start};
            Strptr sub = {(uint8_t*)subStr, subSize};
            n = CODE_CORE_RabinKarp(&org, &sub, hash, pow);
        }
    }
    return (n < 0) ? -1 : n + start;
}

int64_t CODE_CORE_LastIndexOfString(
    const uint8_t* orgStr, const uint8_t* subStr, int64_t orgSize, int64_t subSize, int64_t start)
{
    if (start < 0 || subSize < 0 || start + subSize > orgSize) {
        return -1;
    }
    int64_t n;
    if (subSize == 1) {
        n = CODE_CORE_LastIndexOfByte(orgStr + start, orgSize - start, subStr[0]);
    } else {
        uint32_t hash = 0;
        uint32_t pow = 0;
        CODE_CORE_HashStrRev(subStr, subSize, &hash, &pow);
        Strptr org = {(uint8_t*)orgStr + start, orgSize - start};
        Strptr sub = {(uint8_t*)subStr, subSize};
        n = CODE_CORE_RabinKarpR(&org, &sub, hash, pow);
    }
    return (n < 0) ? -1 : n + start;
}

int64_t CODE_CORE_CountString(const uint8_t* orgStr, const uint8_t* subStr, int64_t orgSize, int64_t subSize)
{
    if (subSize == 1) {
        return CODE_CORE_CountOfByte(orgStr, orgSize, subStr[0]);
    }
    if (orgSize <= 0) {
        return 0;
    }
    int64_t n;
    int64_t i = 0;
    int64_t total = 0;
    if (CODE_CORE_CanUseSIMD()) {
        while (i < orgSize) {
            n = FastStrstr(orgStr + i, orgSize - i, subStr, subSize);
            if (n < 0) {
                return total;
            }
            total++;
            i += n + subSize;
        }
    } else {
        uint32_t hash = 0;
        uint32_t pow = 0;
        CODE_CORE_HashStr(subStr, subSize, &hash, &pow);
        Strptr sub = {(uint8_t*)subStr, subSize};
        while (i < orgSize) {
            Strptr org = {(uint8_t*)orgStr + i, orgSize - i};
            n = CODE_CORE_RabinKarp(&org, &sub, hash, pow);
            if (n < 0) {
                return total;
            }
            total++;
            i += n + subSize;
        }
    }
    return total;
}

int64_t* CODE_CORE_CountAndIndexString(const uint8_t* orgStr, const uint8_t* subStr, int64_t orgSize, int64_t subSize)
{
    if (orgSize <= 0 || subSize <= 0) {
        return NULL;
    }
    if (subSize == 1) {
        return CODE_CORE_CountAndIndexOfByte(orgStr, orgSize, subStr[0]);
    }
    int64_t n;
    int64_t i = 0;
    int64_t total = 0;
    int64_t* result = (int64_t*)malloc(sizeof(int64_t) * (size_t)((orgSize + subSize - 1) / subSize + 1));
    if (result == NULL) {
        return NULL;
    }
    if (CODE_CORE_CanUseSIMD()) {
        while (i < orgSize) {
            n = FastStrstr(orgStr + i, orgSize - i, subStr, subSize);
            if (n < 0) {
                result[0] = total;
                return result;
            }
            result[total + 1] = i + n;
            total++;
            i += n + subSize;
        }
    } else {
        uint32_t hash = 0;
        uint32_t pow = 0;
        CODE_CORE_HashStr(subStr, subSize, &hash, &pow);
        Strptr sub = {(uint8_t*)subStr, subSize};
        while (i < orgSize) {
            Strptr org = {(uint8_t*)orgStr + i, orgSize - i};
            n = CODE_CORE_RabinKarp(&org, &sub, hash, pow);
            if (n < 0) {
                result[0] = total;
                return result;
            }
            result[total + 1] = i + n;
            total++;
            i += n + subSize;
        }
    }
    result[0] = total;
    return result;
}

extern int64_t CODE_CORE_StringSize(const uint8_t* str, int64_t len)
{
    if (CODE_CORE_CanUseSIMD()) {
        return FastSize(str, len);
    } else {
        int64_t size = 0;
        uint8_t* tmp = (uint8_t*)str;
        uint8_t* end = (uint8_t*)str + len;
        for (; tmp < end; ++tmp) {
            if ((*tmp & 0xc0) != 0x80) {
                size += 1;
            }
        }
        return size;
    }
}
