/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <limits.h>

#define PCRE2_STATIC

#ifndef PCRE2_CODE_UNIT_WIDTH
#define PCRE2_CODE_UNIT_WIDTH 8
#endif

#include "pcre2.h"

#ifndef ERR_MSG_LEN
#define ERR_MSG_LEN (256)
#endif

typedef struct {
    pcre2_code* re;
    int errorCode;
    PCRE2_SIZE errorOffset;
} CompileResult;

typedef struct {
    uint32_t nameCount;
    uint32_t nameEntrySize;
    uint8_t* nameTable;
} NamedTableInfo;

extern CompileResult* CODE_REGEX_Compile(const unsigned char* pattern, const uint32_t options)
{
    CompileResult* result = (CompileResult*)malloc(sizeof(CompileResult));
    if (result == NULL) {
        return result;
    }

    int errorCode = INT_MIN; // PCRE2 error message will be `No text for error ...`
    PCRE2_SIZE errorOffset;

    pcre2_code* re = pcre2_compile(pattern, PCRE2_ZERO_TERMINATED, options, &errorCode, &errorOffset, NULL);

    result->re = re;
    result->errorCode = errorCode;
    result->errorOffset = errorOffset;

    return result;
}

extern pcre2_match_data* CODE_REGEX_CreateMatchData(const pcre2_code* re)
{
    return pcre2_match_data_create_from_pattern(re, NULL);
}

extern int CODE_REGEX_Match(const pcre2_code* re, const unsigned char* subject, const PCRE2_SIZE length,
    const PCRE2_SIZE offset, pcre2_match_data* matchData)
{
    return pcre2_match(re, subject, length, offset, 0, matchData, NULL);
}

extern PCRE2_SIZE* CODE_REGEX_GetOvector(pcre2_match_data* matchData)
{
    return pcre2_get_ovector_pointer(matchData);
}

extern unsigned char* CODE_REGEX_GetErrorMsg(const int errorCode)
{
    unsigned char* msg = (uint8_t*)malloc(ERR_MSG_LEN);
    if (msg == NULL) {
        return msg;
    }
    pcre2_get_error_message(errorCode, msg, ERR_MSG_LEN);
    return msg;
}

static uint32_t CODE_REGEX_GetNameCount(const pcre2_code* re)
{
    uint32_t count;
    (void)pcre2_pattern_info(re, /* the compiled pattern */
        PCRE2_INFO_NAMECOUNT,    /* count of named capture group */
        &count);                 /* where to put the answer */
    return count;
}

static uint32_t CODE_REGEX_GetNameEntrySize(const pcre2_code* re)
{
    uint32_t size;
    (void)pcre2_pattern_info(re,  /* the compiled pattern */
        PCRE2_INFO_NAMEENTRYSIZE, /* size of each entry in the table */
        &size);                   /* where to put the answer */
    return size;
}

static uint8_t* CODE_REGEX_GetNameTable(const pcre2_code* re)
{
    PCRE2_SPTR table;
    (void)pcre2_pattern_info(re, /* the compiled pattern */
        PCRE2_INFO_NAMETABLE,    /* address of the table */
        &table);                 /* where to put the answer */
    return (uint8_t*)table;
}

extern NamedTableInfo* CODE_REGEX_GetNameTableInfo(const pcre2_code* re)
{
    NamedTableInfo* info = (NamedTableInfo*)malloc(sizeof(NamedTableInfo));
    if (info == NULL) {
        return info;
    }

    info->nameCount = CODE_REGEX_GetNameCount(re);
    info->nameEntrySize = CODE_REGEX_GetNameEntrySize(re);
    info->nameTable = CODE_REGEX_GetNameTable(re);

    return info;
}

extern void CODE_REGEX_FreeCode(pcre2_code* re)
{
    pcre2_code_free(re);
}

extern void CODE_REGEX_FreeMatchData(pcre2_match_data* md)
{
    pcre2_match_data_free(md);
}

extern int64_t CODE_REGEX_Count(const pcre2_code* re, const unsigned char* subject, const PCRE2_SIZE length,
    const PCRE2_SIZE offset, const PCRE2_SIZE end, pcre2_match_data* matchData)
{
    PCRE2_SIZE startOffset = offset;
    int64_t count = 0;
    while (startOffset <= end && pcre2_match(re, subject, length, startOffset, 0, matchData, NULL) > 0) {
        PCRE2_SIZE* ovector = pcre2_get_ovector_pointer(matchData);
        startOffset = ovector[1] + (startOffset == ovector[1]);
        count++;
    }
    return count;
}
