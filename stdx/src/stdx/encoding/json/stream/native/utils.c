/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include <stdint.h>
#include "securec.h"

#define FL_TO_STR_MAX 310 /* 310 is the maxlength of double to string size */

extern int64_t CODE_ReadString(const uint8_t* str, int64_t left, int64_t right)
{
    int64_t i = left;
    while (i < right) {
        unsigned char c = str[i];
        if (c <= 0x7F) {
            // ASCII character
            if (str[i] == '\"' || str[i] == '\\') {
                return i;
            }
            i++;
            continue;
        }
        switch (c >> 4) { // 4 refer to leading four bits
            case 12:      // 1100 xxxx; 12 == 1100
            case 13:      // 1101 xxxx; 13 == 1101
                // 2-byte sequence
                if (i + 1 >= right) {
                    return i;
                }
                if ((c & 0x1e) == 0 || (str[i + 1] & 0xC0) != 0x80) {
                    return -1;
                }
                i += 2; // 2-byte sequence
                break;
            case 14: // 1110 xxxx; 14 == 1110
                // 3-byte sequence
                if (i + 2 >= right) { // i + 2 refer to 3rd char
                    return i;
                }
                if ((str[i + 1] & 0xC0) != 0x80 || (str[i + 2] & 0xC0) != 0x80 || // i + 2 refer to 3rd char
                    ((c & 0b1111) == 0 && (str[i + 1] & 0b00100000) == 0) ||
                    ((c & 0b1111) == 0b1101 && (str[i + 1] & 0b00100000) != 0)) {
                    return -1;
                }
                i += 3; // 3-byte sequence
                break;
            case 15: // 1111 xxxx; 15 == 1111
                // 4-byte sequence
                if (i + 3 >= right) { // check length 3
                    return i;
                }
                if ((str[i + 1] & 0xC0) != 0x80 || (str[i + 2] & 0xC0) != 0x80 || // i + 2 refer to 3rd char
                    (str[i + 3] & 0xC0) != 0x80 ||                                // i + 3 refer to 4th char
                    (c > 0xF4) || ((c & 0b111) == 0 && (str[i + 1] & 0b00110000) == 0) ||
                    ((c & 0b111) == 0b100 && (str[i + 1] & 0b00110000) != 0)) {
                    return -1;
                }
                i += 4; // 4-byte sequence
                break;
            default:
                return -1;
        }
    }
    return i;
}

extern int64_t CODE_JSON_FloatPrint(const double num, uint8_t* dest, const int64_t destSize)
{
    return snprintf_s((char*)dest, (unsigned long)destSize, FL_TO_STR_MAX - 1, "%f", num);
}
