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

function testStringFromString() {
    const uncompressableStringsTable: String[] = [
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


    let compressableStringsTable: String[] = [
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

    for (let i: int = 0; i < compressableStringsTable.length; i++) {
        let tmp = compressableStringsTable[i];
        arktest.assertEQ(tmp, compressableStringsTable[i]);
    }
    for (let i: int = 0; i < uncompressableStringsTable.length; i++) {
        let tmp = uncompressableStringsTable[i];
        arktest.assertEQ(tmp, uncompressableStringsTable[i]);
    }
}
function main(): int {
    let suite = new arktest.ArkTestsuite('StringFromString');
    suite.addTest('testStringFromString', testStringFromString);
    return suite.run();
}
