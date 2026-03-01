/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include <stdbool.h>
#include <stdint.h>
#include "utf8_fix.h"

static inline __attribute__((always_inline)) uint8_t countl_one(uint8_t byte)
{
    // The result of __builtin_clz is undefined for 0.
    // Just return 8.
    if (byte == 0xFF) {
        return 8;
    }
    // __builtin_clz perform on uint32, input is uint8, the result should minus 24
    return __builtin_clz((uint8_t)(~byte)) - 24;
}

// Forces a byte to be a valid UTF-8 continuation byte.
static inline __attribute__((always_inline)) void ForceContinuationByte(uint8_t* byte)
{
    *byte = (*byte | (1u << 7u)) & ~(1u << 6u);
}

int64_t CODE_fix_to_utf8(uint8_t* data, int64_t data_len)
{
    int offset = 0;
    bool stop = false;
    while (!stop && offset < data_len) {
        uint8_t c0 = data[offset];
        // In UTF8 encoding, the number of subsequent bytes required is determined by the number of consecutive '1's
        // starting with the first byte, and the subsequent bytes are forced to be converted to ContinuationByte
        switch (countl_one(c0)) {
            case 0: {
                // Starts with 0 '1's, which is 0xxxxxxx, indicating 1-byte UTF8, which is legal and does not need to be
                // processed
                offset += 1;
                break; // breakout switch
            }
            case 1: {
                // Characters starting with 1 '1' cannot be used as the starting character, but can only be used as
                // subsequent characters. They need to be converted to case 2. Requires 2 bytes
                if (offset + 2 > data_len) {
                    stop = true; // breakout loop
                    break;       // breakout switch
                }
                ForceContinuationByte(&data[offset + 1]);

                // Convert the first byte from 10xxxxxx to 110xxxxx
                c0 = (c0 & 0b00011111) | 0b11000000;
                data[offset] = c0;

                // c0 is 110xxxxx, to avoid codepoint falling in [0,2**7), it is necessary to ensure that xxxxx cannot
                // be 0000x
                if ((c0 & 0b11110) == 0) {
                    // Need to fix c0, fix xxxxx to xxx1x
                    c0 |= 0b00000010;
                    data[offset] = c0;
                }
                // Processing 2 bytes completed
                offset += 2;
                break; // breakout switch
            }
            case 2: {
                // Requires 2 bytes
                if (offset + 2 > data_len) {
                    stop = true; // breakout loop
                    break;       // breakout switch
                }
                ForceContinuationByte(&data[offset + 1]);
                // c0 is 110xxxxx, to avoid codepoint falling in [0,2**7), it is necessary to ensure that xxxxx cannot
                // be 0000x
                if ((c0 & 0b11110) == 0) {
                    // Need to fix c0, fix xxxxx to xxx1x
                    c0 |= 0b00000010;
                    data[offset] = c0;
                }
                // Processing 2 bytes completed
                offset += 2;
                break;
            }
            case 3: {
                // Requires 3 bytes
                if (offset + 3 > data_len) {
                    stop = true; // breakout loop
                    break;       // breakout switch
                }
                ForceContinuationByte(&data[offset + 1]);
                ForceContinuationByte(&data[offset + 2]);

                // c0 is 1110xxxx, to avoid codepoint falling in [0,2**11), it is necessary to ensure that when
                // xxxx==0000, yyyyyy cannot be 0yyyyyy
                uint8_t c1 = data[offset + 1];
                if ((c0 & 0b1111) == 0 && (c1 & 0b00100000) == 0) {
                    // Need to fix c1, write yyyyyy as 1yyyyy
                    c1 |= 0b00100000;
                    data[offset + 1] = c1;
                }
                // To prevent the codepoint from falling within the range [0xD800, 0xE000), it is necessary to ensure
                // that when xxxx==1101, yyyyyy cannot be 1yyyyyy
                if ((c0 & 0b1111) == 0b1101 && (c1 & 0b00100000) != 0) {
                    // Need to fix c1, write yyyyyy to 0yyyyy
                    c1 &= 0b11011111;
                    data[offset + 1] = c1;
                }
                // Processing 3 bytes completed
                offset += 3;
                break; // breakout switch
            }
            case 4: {
                // Requires 4 bytes
                if (offset + 4 > data_len) {
                    stop = true; // breakout loop
                    break;       // breakout switch
                }
                ForceContinuationByte(&data[offset + 1]);
                ForceContinuationByte(&data[offset + 2]);
                ForceContinuationByte(&data[offset + 3]);

                uint8_t c1 = data[offset + 1];
                // c0 is 11110xxx, the codepoint range is [2**16, 2**21), and the value range of xxx is only 000, 001,
                // 010, 011, 100
                if ((c0 & 0b111) > 0b100) {
                    // xxx is 101/110/111
                    // Need to fix c0, write 101/110/111 to 001/010/011
                    c0 &= 0b11111011;
                    data[offset] = c0;
                }
                // To avoid codepoint falling in [0,2**16), it is necessary to ensure that when xxx==000, yyyyyy cannot
                // be 00yyyy
                if ((c0 & 0b111) == 0 && (c1 & 0b00110000) == 0) {
                    // xxx is 000
                    // Need to fix c1, write yyyyyy to y1yyyy
                    c1 |= 0b00010000;
                    data[offset + 1] = c1;
                }
                // To avoid codepoint falling in the range of [0x11000000,2**21), when xxx==100 is required, yyyyyy must
                // be 00yyyy
                if ((c0 & 0b111) == 0b100 && (c1 & 0b00110000) != 0) {
                    // xxx is 100
                    // Need to fix c1, write yyyyyy as 00yyyy
                    c1 &= 0b11001111;
                    data[offset + 1] = c1;
                }
                // Processing 4 bytes completed
                offset += 4;
                break; // breakout switch
            }
            default: {
                // 0b111110xx, 0b1111110x, 0b11111110
                // Illegal, convert directly to ascii
                data[offset] = c0 & 0b01111111;
                offset += 1;
                break; // breakout switch
            }
        }
    }
    // At this time, the entire data is repaired, and the repair range is [0, offset), and the data outside the range
    // remains unchanged.
    return offset;
}
