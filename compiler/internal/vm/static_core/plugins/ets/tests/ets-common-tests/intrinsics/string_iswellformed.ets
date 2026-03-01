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

const valiedUTF16String: String = 'Hello 你好 नमस्ते 😊👩💻\uD83D\uDC68\u200D\uD83D\uDCBBАаБбВвГгДдЕеЁё';
const loneHighSurrogateString: String = 'Normal start\uD83DMid content\uD83D\uDC68end';
const loneLowSurrogateString: String = 'abc\uDC68Invalid startMid content\uD83D\uDCBBEnd\uDCBBabc';
const reversedSurrogateString: String = 'Normal part\uDCBB\uD83DOther content';

const createValidUTF16TreeString = (): String => {
    return 'Hello 你好 नमस्ते 😊👩💻'   // Multilingual + Emoji
        .concat('\uD83D\uDC68\u200D\uD83D\uDCBB', 'АаБбВвГгДдЕеЁё')   //+ 👨💻 (valid surrogate pair) and rus chars
}

const createLoneHighSurrogateTreeString = (): String => {
    return 'Normal start\uD83D'        // Isolated high surrogate
        .concat('Mid content\uD83D\uDC68')      // Valid pair
        .concat('\uD83Dend');                   // Isolated high surrogate
}

const createLoneLowSurrogateTreeString = (): String => {
    return '\uDC68Invalid start'  // Isolated low surrogate
        .concat('Mid content\uD83D\uDCBB')          // Valid pair
        .concat('End\uDCBB');                       // Isolated low surrogate
}

const createReversedSurrogateTreeString = (): String => {
    return 'Normal part\uDCBB\uD83D'  // Reversed pair (low-high)
        .concat('Other content');         // Normal content
}

function testIsWellFormedLineString(): void {
    // Tests for LineString
    arktest.assertTrue(valiedUTF16String.isWellFormed(), 'Valid UTF-16');  // true
    arktest.assertFalse(loneHighSurrogateString.isWellFormed(), 'Lone high surrogate');  // false
    arktest.assertFalse(loneLowSurrogateString.isWellFormed(), 'Lone low surrogate');    // false
    arktest.assertFalse(reversedSurrogateString.isWellFormed(), 'Reversed surrogates');  // false
}

function testIsWellFormedSlicedString(): void {
    // Tests for SlicedString
    arktest.assertEQ(valiedUTF16String.substring(3, valiedUTF16String.length - 3).isWellFormed(), true, 'Valid UTF-16 SlicedString');  // true
    arktest.assertEQ(loneHighSurrogateString.substring(2, loneHighSurrogateString.length - 2).isWellFormed(), false, 'Lone high surrogate SlicedString');  // false
    arktest.assertEQ(loneLowSurrogateString.substring(1, loneHighSurrogateString.getLength() - 1).isWellFormed(), false, 'Lone low surrogate SlicedString');    // false
    arktest.assertEQ(reversedSurrogateString.substring(1, reversedSurrogateString.length - 1).isWellFormed(), false, 'Reversed surrogates SlicedString');  // false
}

function testIsWellFormedTreeString(): void {
    // Tests for TreeString
    arktest.assertTrue(createValidUTF16TreeString().isWellFormed(), 'Valid UTF-16 TreeString');  // true
    arktest.assertFalse(createLoneHighSurrogateTreeString().isWellFormed(), 'Lone high surrogate TreeString');  // false
    arktest.assertFalse(createLoneLowSurrogateTreeString().isWellFormed(), 'Lone low surrogate TreeString');    // false
    arktest.assertFalse(createReversedSurrogateTreeString().isWellFormed(), 'Reversed surrogates TreeString');  // false
}

function testUtf8IsWellFormed(): void {
    // Utf8 Tests
    let utf8Str: String = 'Hello1234567890abcdefg';
    arktest.assertEQ(utf8Str.isWellFormed(), true, 'Valid UTF-8');  // true
    arktest.assertEQ(utf8Str.concat('1234567890abcdefg').isWellFormed(), true, 'Valid UTF-8 TreeString');  // true
    arktest.assertEQ(utf8Str.substring(0, 5).isWellFormed(), true, 'Valid UTF-8 SlicedString');  // true
}
function main(): int {
    let suite = new arktest.ArkTestsuite('StringIsWellFormed');
    suite.addTest('testIsWellFormedLineString', testIsWellFormedLineString);
    suite.addTest('testIsWellFormedSlicedString', testIsWellFormedSlicedString);
    suite.addTest('testIsWellFormedTreeString', testIsWellFormedTreeString);
    suite.addTest('testUtf8IsWellFormed', testUtf8IsWellFormed);
    return suite.run();
}
