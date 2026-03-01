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

/*---
desc: Correct char literals
---*/

function main() {
    // check escaped characters
    let escaped_chars: Char[] = [c'\b', c'\f', c'\n', c'\r', c'\t', c'\v', c'\\', c'\'', c'\"']
    let escaped_vals: Int[] = [8, 12, 10, 13, 9, 11, 92, 39, 34]
    for (let i = 0; i < escaped_chars.length; i++) {
        arktest.assertEQ(escaped_chars[i], escaped_vals[i].toChar())
    }

    // check hex-formatted characters
    let hex_chars: Char[] = [ c'\x00', c'\x01', c'\x02', c'\x03', c'\x04', c'\x05', c'\x06',
        c'\x07', c'\x08', c'\x09', c'\x0a', c'\x0b', c'\x0c', c'\x0d', c'\x0e', c'\x0f',
        c'\x10', c'\x11', c'\x12', c'\x13', c'\x14', c'\x15', c'\x16', c'\x17', c'\x18',
        c'\x19', c'\x1a', c'\x1b', c'\x1c', c'\x1d', c'\x1e', c'\x1f', c'\x20', c'\x21',
        c'\x22', c'\x23', c'\x24', c'\x25', c'\x26', c'\x27', c'\x28', c'\x29', c'\x2a',
        c'\x2b', c'\x2c', c'\x2d', c'\x2e', c'\x2f', c'\x30', c'\x31', c'\x32', c'\x33',
        c'\x34', c'\x35', c'\x36', c'\x37', c'\x38', c'\x39', c'\x3a', c'\x3b', c'\x3c',
        c'\x3d', c'\x3e', c'\x3f', c'\x40', c'\x41', c'\x42', c'\x43', c'\x44', c'\x45',
        c'\x46', c'\x47', c'\x48', c'\x49', c'\x4a', c'\x4b', c'\x4c', c'\x4d', c'\x4e',
        c'\x4f', c'\x50', c'\x51', c'\x52', c'\x53', c'\x54', c'\x55', c'\x56', c'\x57',
        c'\x58', c'\x59', c'\x5a', c'\x5b', c'\x5c', c'\x5d', c'\x5e', c'\x5f', c'\x60',
        c'\x61', c'\x62', c'\x63', c'\x64', c'\x65', c'\x66', c'\x67', c'\x68', c'\x69',
        c'\x6a', c'\x6b', c'\x6c', c'\x6d', c'\x6e', c'\x6f', c'\x70', c'\x71', c'\x72',
        c'\x73', c'\x74', c'\x75', c'\x76', c'\x77', c'\x78', c'\x79', c'\x7a', c'\x7b',
        c'\x7c', c'\x7d', c'\x7e', c'\x7f'
    ]
    for (let i: Int = 0; i < 128; i++) {
        arktest.assertEQ(hex_chars[i], i.toChar())
    }

    // check unicode-formatted characters
    let unicode_chars: Char[] = [ c'\u0041', c'\u0429', c'\u0811', c'\u0bf9', c'\u0fe1', c'\u13c9',
        c'\u17b1', c'\u1b99', c'\u1f81', c'\u2369', c'\u2751', c'\u2b39', c'\u2f21', c'\u3309',
        c'\u36f1', c'\u3ad9', c'\u3ec1', c'\u42a9', c'\u4691', c'\u4a79', c'\u4e61', c'\u5249',
        c'\u5631', c'\u5a19', c'\u5e01', c'\u61e9', c'\u65d1', c'\u69b9', c'\u6da1', c'\u7189',
        c'\u7571', c'\u7959', c'\u7d41', c'\u8129', c'\u8511', c'\u88f9', c'\u8ce1', c'\u90c9',
        c'\u94b1', c'\u9899', c'\u9c81', c'\ua069', c'\ua451', c'\ua839', c'\uac21', c'\ub009',
        c'\ub3f1', c'\ub7d9', c'\ubbc1', c'\ubfa9', c'\uc391', c'\uc779', c'\ucb61', c'\ucf49',
        c'\ud331', c'\ud719', c'\udb01', c'\udee9', c'\ue2d1', c'\ue6b9', c'\ueaa1', c'\uee89',
        c'\uf271', c'\uf659', c'\ufa41', c'\ufe29', c'\uffff'
    ]
    let j = 0;
    for (let i: Int = 65; i < 65536; i += 1000) {
        arktest.assertEQ(unicode_chars[j++], i.toChar())
    }
}
