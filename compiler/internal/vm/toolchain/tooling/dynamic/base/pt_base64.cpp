/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

#include "tooling/dynamic/base/pt_base64.h"

namespace panda::ecmascript::tooling {
static const unsigned char DECODE_TABLE[] = {
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 62, 255, 255, 255, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 255, 255, 255, 255, 255, 255,
    255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 255, 255, 255, 255, 255,
    255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255
};

static const char ENCODE_TABLE[] = {
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/"
};

std::pair<std::size_t, bool> PtBase64::Decode(void *output, const char *input, std::size_t len)
{
    if (len < ENCODED_GROUP_BYTES || len % ENCODED_GROUP_BYTES != 0) {
        return {0, false};
    }

    auto *src = reinterpret_cast<unsigned char const *>(input);
    char *dest = static_cast<char *>(output);
    char base64Chars[ENCODED_GROUP_BYTES];
    int8_t i = 0;
    while (len-- && *src != '=') {
        auto v = DECODE_TABLE[*src];
        if (v == INVAILD_VALUE) {
            break;
        }
        base64Chars[i++] = v;

        if (i == ENCODED_GROUP_BYTES) {
            *dest++ = (base64Chars[0] << 2) | ((base64Chars[1] & 0x30) >> 4); // 2: shift 2bits, 4: shift 4bits
            *dest++ = (base64Chars[1] << 4) | ((base64Chars[2] & 0x3c) >> 2); // 2: shift 2bits, 4: shift 4bits
            *dest++ = (base64Chars[2] << 6) | base64Chars[3]; // 6: shift 6bits, 2: the second char, 3: the third char
            i = 0;
        }
        src++;
    }

    if (i != 0) {
        char tmp[UNENCODED_GROUP_BYTES];
        tmp[0] = (base64Chars[0] << 2) | ((base64Chars[1] & 0x30) >> 4); // 2: shift 2bits, 4: shift 4bits
        tmp[1] = (base64Chars[1] << 4) | ((base64Chars[2] & 0x3c) >> 2); // 2: shift 2bits, 4: shift 4bits
        tmp[2] = (base64Chars[2] << 6) | base64Chars[3]; // 6: shift 6bits, 2: the second char, 3: the third char
        for (int8_t j = 0; j < i - 1; j++) {
            *dest++ = tmp[j];
        }
    }

    len++; // get the remaining length and also avoid underflow
    size_t decodedLen = dest - static_cast<char *>(output);
    bool decodedDone = false;
    // 0: decodedDone, 1: =, 2: ==
    if (len == 0 || (len == 1 && *src == '=') || (len == 2 && *src == '=' && *(src + 1) == '=')) {
        decodedDone = true;
    }

    return {decodedLen, decodedDone};
}

size_t PtBase64::Encode(char *output, const void *input, std::size_t len)
{
    if (len == 0) {
        return 0;
    }

    auto *src = static_cast<const unsigned char *>(input);
    char *dest = output;
    for (auto n = len / UNENCODED_GROUP_BYTES; n--;) {
        *dest++ = ENCODE_TABLE[src[0] >> 2]; // 2: shift 2bits
        *dest++ = ENCODE_TABLE[((src[0] & 0x03) << 4) | (src[1] >> 4)]; // 4: shift 4bits
        *dest++ = ENCODE_TABLE[((src[1] & 0x0f) << 2) | (src[2] >> 6)]; // 2: shift 2bits, 6: shift 6bits
        *dest++ = ENCODE_TABLE[src[2] & 0x3f]; // 2: the second char

        src += UNENCODED_GROUP_BYTES;
    }

    int8_t remainder = static_cast<int8_t>(len % UNENCODED_GROUP_BYTES);
    int8_t paddingCnt = (UNENCODED_GROUP_BYTES - remainder) % UNENCODED_GROUP_BYTES;
    switch (paddingCnt) {
        case 1:
            *dest++ = ENCODE_TABLE[src[0] >> 2]; // 2: shift 2bits
            *dest++ = ENCODE_TABLE[((src[0] & 0x03) << 4) | (src[1] >> 4)]; // 4: shift 4bits
            *dest++ = ENCODE_TABLE[((src[1] & 0x0f) << 2)]; // 2: shift 2bits
            *dest++ = '=';
            break;
        case 2: // 2: "=="
            *dest++ = ENCODE_TABLE[src[0] >> 2]; // 2: shift 2bits
            *dest++ = ENCODE_TABLE[((src[0] & 0x03) << 4)]; // 4: shift 4bits
            *dest++ = '=';
            *dest++ = '=';
            break;
        default:
            break;
    }
    return dest - output;
}
}  // namespace panda::ecmascript::tooling