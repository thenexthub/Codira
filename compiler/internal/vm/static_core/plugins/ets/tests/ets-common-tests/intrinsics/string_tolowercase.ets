/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

const defaultLocale = new Intl.NumberFormat().resolvedOptions().locale;
const isSpecialILocale = defaultLocale.startsWith("tr") || defaultLocale.startsWith("az");

function expi() {
    return isSpecialILocale ? "ı" : "i";
}

function testToLowerCase() {
    // Test cases for LineString
    const valiedUTF16String: String = 'Hello 你好 नमस्ते 😊👩💻\uD83D\uDC68\u200D\uD83D\uDCBBАаБбВвГгДдЕеЁё';
    arktest.assertEQ(valiedUTF16String.toLowerCase(), 'hello 你好 नमस्ते 😊👩💻👨‍💻ааббввггддееёё', 'valid Utf16Chars');
    const validEngChars: String = 'ALPHABETalphabetABCabIc';
    arktest.assertEQ(validEngChars.toLowerCase(), 'alphabetalphabetabcab' + expi() + 'c', 'valid EngChars');
    const invalidUTF16String: String = 'Normal start\uD83DMID content\uD83D\uDC68end';
    arktest.assertEQ(invalidUTF16String.toLowerCase(), 'normal start\uD83Dm' + expi() + 'd content\uD83D\uDC68end',
        'invalid UTF16Chars');
    
    // Test cases for TreeString
    const validUTF16TreeString: String = 'Hello 你好 नमस्ते 😊👩💻'.concat('😊👩💻ABCabc')   // Multilingual + Emoji
    arktest.assertEQ(validUTF16TreeString.toLowerCase(), 'hello 你好 नमस्ते 😊👩💻😊👩💻abcabc',
        'valid UTF-16 TreeString');
    const validEngTreeString: String = 'ALPHABETalphabetABCabIc'.concat('ABCabc');
    arktest.assertEQ(validEngTreeString.toLowerCase(), 'alphabetalphabetabcab' + expi() + 'cabcabc',
        'valid EngChars TreeString');
    const invalidUTF16TreeString: String = 'Normal start\uD83DMID content\uD83D\uDC68end'.concat('ABCabc');
    arktest.assertEQ(invalidUTF16TreeString.toLowerCase(), 'normal start\uD83Dm' + expi() +
        'd content\uD83D\uDC68endabcabc', 'invalid UTF16Chars TreeString');

    // Test cases for SlicedString
    const slicedValidUTF16String: String = valiedUTF16String.substring(4, valiedUTF16String.length - 4);
    arktest.assertEQ(slicedValidUTF16String.toLowerCase(), 'o 你好 नमस्ते 😊👩💻👨‍💻ааббввггдд',
        'valid UTF-16 SlicedString');
    const slicedValidEngChars: String = validEngChars.substring(1, validEngChars.length - 1);
    arktest.assertEQ(slicedValidEngChars.toLowerCase(), 'lphabetalphabetabcab' + expi(), 'valid EngChars SlicedString');
    const slicedInvalidUTF16String: String = invalidUTF16String.substring(2, invalidUTF16String.length - 2);
    arktest.assertEQ(slicedInvalidUTF16String.toLowerCase(), 'rmal start\uD83Dm' + expi() +
        'd content\uD83D\uDC68e', 'invalid UTF16Chars SlicedString');
}

/**
 * Tests the lowercase conversion for a short (less than 8 characters) compressible (latin-only) strings.
 */
function testShortCompressedToLowerCase() {
    const input1 = 'A';
    const input1Next = 'AlphaNewString';
    const input1NextLen = input1Next.length;
    arktest.assertEQ(input1.length, 1);
    const expected1 = 'a';
    const result1 = input1.toLowerCase();
    arktest.assertEQ(result1, expected1, `Short Compressible Failed: ${result1} vs ${expected1}`);
    arktest.assertEQ(input1Next, 'AlphaNewString', 'the next string must not be touched');
    arktest.assertEQ(input1Next.length, input1NextLen, 'the next string must not be touched including the length');

    const input7 = 'ABIDEFG';
    const input7Next = 'BetaNewString';
    const input7NextLen = input7Next.length;
    arktest.assertEQ(input7.length, 7);
    const expected7 = 'ab' + expi() + 'defg';
    const result7 = input7.toLowerCase();
    arktest.assertEQ(result7, expected7, `Short Compressible Failed: ${result7} vs ${expected7}`);
    arktest.assertEQ(input7Next, 'BetaNewString', 'the next string must not be touched');
    arktest.assertEQ(input7Next.length, input7NextLen, 'the next string must not be touched including the length');

    const input8 = 'ABIDEFGH';
    const input8Next = 'GammaNewString';
    const input8NextLen = input8Next.length;
    arktest.assertEQ(input8.length, 8);
    const expected8 = 'ab' + expi() + 'defgh';
    const result8 = input8.toLowerCase();
    arktest.assertEQ(result8, expected8, `Short Compressible Failed: ${result8} vs ${expected8}`);
    arktest.assertEQ(input8Next, 'GammaNewString', 'the next string must not be touched');
    arktest.assertEQ(input8Next.length, input8NextLen, 'the next string must not be touched including the length');
}

/**
 * Tests the lowercase conversion for compressible (latin-only) strings.
 */
function testCompressedToLowerCase() {
    // Base compressible string characters
    const input = ' aAbBcCdDeEfFgGhHiIjJkKlLmMnNoOpPq./!?QrRsStTuUvVwWxXyYz(){}Z[]1234567890-+_:*\\\'"`';
    arktest.assertEQ(input.length, 26 * 2 + 10 + 20);
    const expected = ' aabbccddeeffgghhi' + expi() + 'jjkkllmmnnooppq./!?qrrssttuuvvwwxxyyz(){}z[]1234567890' +
        '-+_:*\\\'"`';
    arktest.assertEQ(expected.length, 26 * 2 + 10 + 20);
    const result = input.toLowerCase();
    arktest.assertEQ(result, expected, `Compressible Linear String Failed: ${result} vs ${expected}`);

    // TreeString test
    const input_0 = 'abcdefghijklmnopqrstuvwxyz';
    const input_1 = 'ABCDEFGHI/!?JKLMNOPQRSTUVWXY(){}Z[]';
    const input_2 = ' 1234567890.-+_:*\\\'"`';
    const inputTree = input_0.concat(input_1).concat(input_2);
    arktest.assertEQ(inputTree.length, 26 * 2 + 10 + 20);
    const expectedTree = 'abcdefghijklmnopqrstuvwxyzabcdefgh' + expi() + '/!?jklmnopqrstuvwxy(){}z[] 1234567890' +
        '.-+_:*\\\'"`';
    const resultTree = inputTree.toLowerCase();
    arktest.assertEQ(resultTree, expectedTree, `Compressible Tree String Failed: ${resultTree} vs ${expectedTree}`);

    // SlicedString test
    const inputSliced = '-*& ABCDEF/!?GHIJKLMNOPQRSTUVWXY(){}Z[]1234567890.-+_:&\\\'"=`x'
        .substring(3, 26 + 3 + 10 + 21);
    arktest.assertEQ(inputSliced.length, 26 + 10 + 21);
    const expectedSliced = ' abcdef/!?gh' + expi() + 'jklmnopqrstuvwxy(){}z[]1234567890.-+_:&\\\'"=`';
    arktest.assertEQ(expectedSliced.length, 26 + 10 + 21);
    const resultSliced = inputSliced.toLowerCase();
    arktest.assertEQ(resultSliced, expectedSliced,
        `Compressible Sliced String Failed: ${resultSliced} vs ${expectedSliced}`);
}

function main() {
    let suite = new arktest.ArkTestsuite('StringToLowerCase');
    suite.addTest('testToLowerCase', testToLowerCase);
    suite.addTest('testCompressedToLowerCase', testCompressedToLowerCase);
    suite.addTest('testShortCompressedToLowerCase', testShortCompressedToLowerCase);
    return suite.run();
}
