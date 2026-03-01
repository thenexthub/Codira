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

function main(): int {
    const uncompressable_chars_table: FixedArray<char>[] = [
        [c'\u00b0'],
        [c'\u00b0', c'\u00b1'],
        [c'\u00b0', c'\u00b1', c'\u00b2'],
        [c'\u00b0', c'\u00b1', c'\u00b2', c'\u00b3'],
        [c'\u00b0', c'\u00b1', c'\u00b2', c'\u00b3', c'\u00b4'],
        [c'\u00b0', c'\u00b1', c'\u00b2', c'\u00b3', c'\u00b4', c'\u00b5'],
        [c'\u00b0', c'\u00b1', c'\u00b2', c'\u00b3', c'\u00b4', c'\u00b5', c'\u00b6'],
        [c'\u00b0', c'\u00b1', c'\u00b2', c'\u00b3', c'\u00b4', c'\u00b5', c'\u00b6', c'\u00b7'],
        [c'\u00b0', c'\u00b1', c'\u00b2', c'\u00b3', c'\u00b4', c'\u00b5', c'\u00b6', c'\u00b7', c'\u00b8'],
        [c'\u00b0', c'\u00b1', c'\u00b2', c'\u00b3', c'\u00b4', c'\u00b5', c'\u00b6', c'\u00b7', c'\u00b8', c'\u00b9'],
        [c'\u00b0', c'\u00b1', c'\u00b2', c'\u00b3', c'\u00b4', c'\u00b5', c'\u00b6', c'\u00b7', c'\u00b8', c'\u00b9', c'\u00ba'],
        [c'\u00b0', c'\u00b1', c'\u00b2', c'\u00b3', c'\u00b4', c'\u00b5', c'\u00b6', c'\u00b7', c'\u00b8', c'\u00b9', c'\u00ba', c'\u00bb'],
        [c'\u00b0', c'\u00b1', c'\u00b2', c'\u00b3', c'\u00b4', c'\u00b5', c'\u00b6', c'\u00b7', c'\u00b8', c'\u00b9', c'\u00ba', c'\u00bb', c'\u00bc'],
        [c'\u00b0', c'\u00b1', c'\u00b2', c'\u00b3', c'\u00b4', c'\u00b5', c'\u00b6', c'\u00b7', c'\u00b8', c'\u00b9', c'\u00ba', c'\u00bb', c'\u00bc', c'\u00bd'],
        [c'\u00b0', c'\u00b1', c'\u00b2', c'\u00b3', c'\u00b4', c'\u00b5', c'\u00b6', c'\u00b7', c'\u00b8', c'\u00b9', c'\u00ba', c'\u00bb', c'\u00bc', c'\u00bd', c'\u00be'],
        [c'\u00b0', c'\u00b1', c'\u00b2', c'\u00b3', c'\u00b4', c'\u00b5', c'\u00b6', c'\u00b7', c'\u00b8', c'\u00b9', c'\u00ba', c'\u00bb', c'\u00bc', c'\u00bd', c'\u00be', c'\u00bf']
    ];

    const uncompressable_strings_table: FixedArray<String> = [
        '\u00b0',
        '\u00b0\u00b1',
        '\u00b0\u00b1\u00b2',
        '\u00b0\u00b1\u00b2\u00b3',
        '\u00b0\u00b1\u00b2\u00b3\u00b4',
        '\u00b0\u00b1\u00b2\u00b3\u00b4\u00b5',
        '\u00b0\u00b1\u00b2\u00b3\u00b4\u00b5\u00b6',
        '\u00b0\u00b1\u00b2\u00b3\u00b4\u00b5\u00b6\u00b7',
        '\u00b0\u00b1\u00b2\u00b3\u00b4\u00b5\u00b6\u00b7\u00b8',
        '\u00b0\u00b1\u00b2\u00b3\u00b4\u00b5\u00b6\u00b7\u00b8\u00b9',
        '\u00b0\u00b1\u00b2\u00b3\u00b4\u00b5\u00b6\u00b7\u00b8\u00b9\u00ba',
        '\u00b0\u00b1\u00b2\u00b3\u00b4\u00b5\u00b6\u00b7\u00b8\u00b9\u00ba\u00bb',
        '\u00b0\u00b1\u00b2\u00b3\u00b4\u00b5\u00b6\u00b7\u00b8\u00b9\u00ba\u00bb\u00bc',
        '\u00b0\u00b1\u00b2\u00b3\u00b4\u00b5\u00b6\u00b7\u00b8\u00b9\u00ba\u00bb\u00bc\u00bd',
        '\u00b0\u00b1\u00b2\u00b3\u00b4\u00b5\u00b6\u00b7\u00b8\u00b9\u00ba\u00bb\u00bc\u00bd\u00be',
        '\u00b0\u00b1\u00b2\u00b3\u00b4\u00b5\u00b6\u00b7\u00b8\u00b9\u00ba\u00bb\u00bc\u00bd\u00be\u00bf'
    ];

    let compressable_chars_table: FixedArray<char>[] = [
        [c'0'],
        [c'0', c'1'],
        [c'0', c'1', c'2'],
        [c'0', c'1', c'2', c'3'],
        [c'0', c'1', c'2', c'3', c'4'],
        [c'0', c'1', c'2', c'3', c'4', c'5'],
        [c'0', c'1', c'2', c'3', c'4', c'5', c'6'],
        [c'0', c'1', c'2', c'3', c'4', c'5', c'6', c'7'],
        [c'0', c'1', c'2', c'3', c'4', c'5', c'6', c'7', c'8'],
        [c'0', c'1', c'2', c'3', c'4', c'5', c'6', c'7', c'8', c'9'],
        [c'0', c'1', c'2', c'3', c'4', c'5', c'6', c'7', c'8', c'9', c'a'],
        [c'0', c'1', c'2', c'3', c'4', c'5', c'6', c'7', c'8', c'9', c'a', c'b'],
        [c'0', c'1', c'2', c'3', c'4', c'5', c'6', c'7', c'8', c'9', c'a', c'b', c'c'],
        [c'0', c'1', c'2', c'3', c'4', c'5', c'6', c'7', c'8', c'9', c'a', c'b', c'c', c'd'],
        [c'0', c'1', c'2', c'3', c'4', c'5', c'6', c'7', c'8', c'9', c'a', c'b', c'c', c'd', c'e'],
        [c'0', c'1', c'2', c'3', c'4', c'5', c'6', c'7', c'8', c'9', c'a', c'b', c'c', c'd', c'e', c'f'],
        [c'0', c'1', c'2', c'3', c'4', c'5', c'6', c'7', c'8', c'9', c'a', c'b', c'c', c'd', c'e', c'f', c'g'],
        [c'0', c'1', c'2', c'3', c'4', c'5', c'6', c'7', c'8', c'9', c'a', c'b', c'c', c'd', c'e', c'f', c'g', c'h'],
        [c'0', c'1', c'2', c'3', c'4', c'5', c'6', c'7', c'8', c'9', c'a', c'b', c'c', c'd', c'e', c'f', c'g', c'h', c'i'],
        [c'0', c'1', c'2', c'3', c'4', c'5', c'6', c'7', c'8', c'9', c'a', c'b', c'c', c'd', c'e', c'f', c'g', c'h', c'i', c'j'],
        [c'0', c'1', c'2', c'3', c'4', c'5', c'6', c'7', c'8', c'9', c'a', c'b', c'c', c'd', c'e', c'f', c'g', c'h', c'i', c'j', c'k']
    ];

    let compressable_strings_table: FixedArray<String> = [
        '0',
        '01',
        '012',
        '0123',
        '01234',
        '012345',
        '0123456',
        '01234567',
        '012345678',
        '0123456789',
        '0123456789a',
        '0123456789ab',
        '0123456789abc',
        '0123456789abcd',
        '0123456789abcde',
        '0123456789abcdef',
        '0123456789abcdefg',
        '0123456789abcdefgh',
        '0123456789abcdefghi',
        '0123456789abcdefghij',
        '0123456789abcdefghijk'
    ];

    for (let i: int = 0; i < compressable_chars_table.length; i++) {
        let tmp = new String(compressable_chars_table[i]);
        if (tmp != compressable_strings_table[i]) {
            return 1;
        }
    }
    for (let i: int = 0; i < uncompressable_chars_table.length; i++) {
        let tmp = new String(uncompressable_chars_table[i]);
        if (tmp != uncompressable_strings_table[i]) {
            return 2;
        }
    }
    return 0;
}
