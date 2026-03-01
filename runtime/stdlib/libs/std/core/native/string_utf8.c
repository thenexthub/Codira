/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include <stdint.h>
#include "core.h"

#define HIGH_1_MASK 0b10000000 /* 0x80 */
#define HIGH_2_MASK 0b11000000 /* 0xc0 */
#define HIGH_3_MASK 0b11100000 /* 0xe0 */
#define HIGH_4_MASK 0b11110000 /* 0xf0 */
#define HIGH_5_MASK 0b11111000 /* 0xf8 */
#define HIGH_6_MASK 0b11111100 /* 0xfc */
#define SHIFT_30 30
#define SHIFT_24 24
#define SHIFT_18 18
#define SHIFT_12 12
#define SHIFT_6 6
#define LOW_6_MASK 0b00111111 /* 0x3f */
#define LOW_5_MASK 0b00011111 /* 0x1f */
#define LOW_4_MASK 0b00001111 /* 0x0f */
#define LOW_3_MASK 0b00000111 /* 0x07 */
#define LOW_2_MASK 0b00000011 /* 0x03 */
#define LOW_1_MASK 0b00000001 /* 0x01 */
#define UTF8_6_MAX 0x7FFFFFFF
#define UTF8_5_MAX 0x3FFFFFF
#define UTF8_4_MAX 0x10FFFF
#define UTF8_3_MAX 0xFFFF
#define UTF8_2_MAX 0x07FF
#define UTF8_1_MAX 0x7F
#define SUBSCRIPT_0 0
#define SUBSCRIPT_1 1
#define SUBSCRIPT_2 2
#define SUBSCRIPT_3 3
#define SUBSCRIPT_4 4
#define SUBSCRIPT_5 5
#define LENTH_1 1
#define LENTH_2 2
#define LENTH_3 3
#define LENTH_4 4
#define LENTH_5 5
#define LENTH_6 6

int64_t CODE_CORE_FromCharToUtf8(uint32_t c, uint8_t* itemBytes)
{
    if (c <= UTF8_1_MAX) {
        itemBytes[SUBSCRIPT_0] = (uint8_t)c;
        return LENTH_1;
    } else if (c <= UTF8_2_MAX) {
        itemBytes[SUBSCRIPT_0] = (uint8_t)(((c >> SHIFT_6) & LOW_5_MASK) | HIGH_2_MASK);
        itemBytes[SUBSCRIPT_1] = (uint8_t)((c & LOW_6_MASK) | HIGH_1_MASK);
        return LENTH_2;
    } else if (c <= UTF8_3_MAX) {
        itemBytes[SUBSCRIPT_0] = (uint8_t)(((c >> SHIFT_12) & LOW_4_MASK) | HIGH_3_MASK);
        itemBytes[SUBSCRIPT_1] = (uint8_t)(((c >> SHIFT_6) & LOW_6_MASK) | HIGH_1_MASK);
        itemBytes[SUBSCRIPT_2] = (uint8_t)((c & LOW_6_MASK) | HIGH_1_MASK);
        return LENTH_3;
    } else if (c <= UTF8_4_MAX) {
        itemBytes[SUBSCRIPT_0] = (uint8_t)(((c >> SHIFT_18) & LOW_3_MASK) | HIGH_4_MASK);
        itemBytes[SUBSCRIPT_1] = (uint8_t)(((c >> SHIFT_12) & LOW_6_MASK) | HIGH_1_MASK);
        itemBytes[SUBSCRIPT_2] = (uint8_t)(((c >> SHIFT_6) & LOW_6_MASK) | HIGH_1_MASK);
        itemBytes[SUBSCRIPT_3] = (uint8_t)((c & LOW_6_MASK) | HIGH_1_MASK);
        return LENTH_4;
    } else if (c <= UTF8_5_MAX) {
        itemBytes[SUBSCRIPT_0] = (uint8_t)(((c >> SHIFT_24) & LOW_2_MASK) | HIGH_5_MASK);
        itemBytes[SUBSCRIPT_1] = (uint8_t)(((c >> SHIFT_18) & LOW_6_MASK) | HIGH_1_MASK);
        itemBytes[SUBSCRIPT_2] = (uint8_t)(((c >> SHIFT_12) & LOW_6_MASK) | HIGH_1_MASK);
        itemBytes[SUBSCRIPT_3] = (uint8_t)(((c >> SHIFT_6) & LOW_6_MASK) | HIGH_1_MASK);
        itemBytes[SUBSCRIPT_4] = (uint8_t)((c & LOW_6_MASK) | HIGH_1_MASK);
        return LENTH_5;
    } else if (c <= UTF8_6_MAX) {
        itemBytes[SUBSCRIPT_0] = (uint8_t)(((c >> SHIFT_30) & LOW_1_MASK) | HIGH_6_MASK);
        itemBytes[SUBSCRIPT_1] = (uint8_t)(((c >> SHIFT_24) & LOW_6_MASK) | HIGH_1_MASK);
        itemBytes[SUBSCRIPT_2] = (uint8_t)(((c >> SHIFT_18) & LOW_6_MASK) | HIGH_1_MASK);
        itemBytes[SUBSCRIPT_3] = (uint8_t)(((c >> SHIFT_12) & LOW_6_MASK) | HIGH_1_MASK);
        itemBytes[SUBSCRIPT_4] = (uint8_t)(((c >> SHIFT_6) & LOW_6_MASK) | HIGH_1_MASK);
        itemBytes[SUBSCRIPT_5] = (uint8_t)((c & LOW_6_MASK) | HIGH_1_MASK);
        return LENTH_6;
    }
    return -1;
}
