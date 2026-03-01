/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include "json_string_escape.h"

int64_t CODE_JSON_ReplaceEscapeChar(const uint8_t* input, int64_t inputlen, uint8_t* buffer, bool htmlSafe)
{
    uint8_t* pointer = buffer;
    const uint8_t* inputPointer = input;
    int64_t remainlen = inputlen;
    int64_t len = 0;
    static uint8_t hex[] = "0123456789abcdef";
    for (; remainlen > 0; inputPointer++, pointer++, remainlen--) {
        uint8_t b = *inputPointer;
        if (b == 0X7f) {
            *pointer++ = '\\';
            *pointer++ = 'u';
            *pointer++ = '0';
            *pointer++ = '0';
            *pointer++ = '7';
            *pointer = 'f';
            len += 6; // 6 is the sizeof("\u007f")
            continue;
        }

        if ((b >= 32) && (b != '\"') && (b != '\\') && ((!htmlSafe && b == '&') || (b != '&'))) { // ASCII code over 32 don't need escape
            *pointer = *inputPointer;
            len++;
            continue;
        }

        switch (b) {
            case '\b':
                *pointer++ = '\\';
                *pointer = 'b';
                len += 2; // add 2 bytes
                break;
            case '\f':
                *pointer++ = '\\';
                *pointer = 'f';
                len += 2; // add 2 bytes
                break;
            case '\n':
                *pointer++ = '\\';
                *pointer = 'n';
                len += 2; // add 2 bytes
                break;
            case '\r':
                *pointer++ = '\\';
                *pointer = 'r';
                len += 2; // add 2 bytes
                break;
            case '\t':
                *pointer++ = '\\';
                *pointer = 't';
                len += 2; // add 2 bytes
                break;
            case '\\':
                *pointer++ = '\\';
                *pointer = '\\';
                len += 2; // add 2 bytes
                break;
            case '\"':
                *pointer++ = '\\';
                *pointer = '\"';
                len += 2; // add 2 bytes
                break;
            default:
                *pointer++ = '\\';
                *pointer++ = 'u';
                *pointer++ = '0';
                *pointer++ = '0';
                *pointer++ = hex[b >> 4]; // num of high 4 bits
                *pointer = hex[b & 0xF];  // num of low 4 bits
                len += 6;                 // add 6 bytes
                break;
        }
    }
    return len;
}

int64_t CODE_JSON_WriteBufferAppendInt(uint8_t* buffer, const int64_t num)
{
    if (num < 0) {
        uint64_t unum = (uint64_t)-num;
        buffer[0] = '-';
        buffer++;
        return CODE_JSON_WriteBufferAppendUint(buffer, unum) + 1;
    } else {
        return CODE_JSON_WriteBufferAppendUint(buffer, (uint64_t)num);
    }
}

int64_t CODE_JSON_WriteBufferAppendUint(uint8_t* buffer, const uint64_t num)
{
    int64_t index = 0, numStartPos = 0;
    uint64_t unum = num;
    do {
        buffer[index++] = unum % 10 + '0'; // mod 10
        unum /= 10;                        // mod 10
    } while (unum > 0);

    uint8_t temp;
    int64_t i = 0, iOpposite;
    int64_t mid = (index - 1) / 2;
    for (i = numStartPos; i <= mid; i++) {
        iOpposite = (index + numStartPos - i) - 1;
        temp = buffer[i];
        buffer[i] = buffer[iOpposite];
        buffer[iOpposite] = temp;
    }
    return index;
}

int64_t CODE_JSON_StringEscapeCharNumGet(const uint8_t* input, int64_t strlen, bool htmlSafe)
{
    const uint8_t* inputPointer;
    int64_t len = strlen;
    uint32_t escapeCharacters = 0;

    for (inputPointer = input; len > 0; inputPointer++, len--) {
        if (*inputPointer == 127) { // ASCII 127 is DEL
            escapeCharacters += 5;  // 5 is the size need to add
            continue;
        }

        // ASCII code over 32 don't need escape
        if ((*inputPointer >= 32) && (*inputPointer != '\"') && (*inputPointer != '\\') && ((!htmlSafe && *inputPointer == '&') || *inputPointer != '&')) {
            continue;
        }

        switch (*inputPointer) {
            case '\r':
            case '\f':
            case '\t':
            case '\b':
            case '\n':
            case '\"':
            case '\\':
                escapeCharacters++;
                break;
            default:
                escapeCharacters += 5; // 5 is the size need to add
                break;
        }
    }
    return escapeCharacters;
}
