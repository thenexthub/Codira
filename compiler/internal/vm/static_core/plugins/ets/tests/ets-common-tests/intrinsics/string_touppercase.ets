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

function expI() {
    return isSpecialILocale ? "İ" : "I";
}

function testToUpperCase() {
    // Test cases for LineString
    const valiedUTF16String: String = 'Hello 你好 नमस्ते 😊👩💻\uD83D\uDC68\u200D\uD83D\uDCBBАаБбВвГгДдЕеЁё';
    arktest.assertEQ(valiedUTF16String.toUpperCase(), 'HELLO 你好 नमस्ते 😊👩💻👨‍💻ААББВВГГДДЕЕЁЁ', 'valid Utf16Chars');
    const validEngChars: String = 'ALPHABETalphabetABCabic';
    arktest.assertEQ(validEngChars.toUpperCase(), 'ALPHABETALPHABETABCAB' + expI() + 'C', 'valid EngChars');
    const invalidUTF16String: String = 'Normal start\uD83DMid content\uD83D\uDC68end';
    arktest.assertEQ(invalidUTF16String.toUpperCase(), 'NORMAL START\uD83DM' + expI() + 'D CONTENT👨END',
        'invalid UTF16Chars');

    // Test cases for TreeString
    const validUTF16TreeString: String = 'Hello 你好 नमस्ते 😊👩💻'.concat('😊👩💻ABCabc')   // Multilingual + Emoji
    arktest.assertEQ(validUTF16TreeString.toUpperCase(), 'HELLO 你好 नमस्ते 😊👩💻😊👩💻ABCABC',
        'valid UTF-16 TreeString');
    const validEngTreeString: String = 'ALPHABETalphabetABCabic'.concat('ABCabc');
    arktest.assertEQ(validEngTreeString.toUpperCase(), 'ALPHABETALPHABETABCAB' + expI() + 'CABCABC',
        'valid EngChars TreeString');
    const invalidUTF16TreeString: String = 'Normal start\uD83DMid content\uD83D\uDC68end'.concat('ABCabc');
    arktest.assertEQ(invalidUTF16TreeString.toUpperCase(), 'NORMAL START\uD83DM' + expI() +
        'D CONTENT\uD83D\uDC68ENDABCABC', 'invalid UTF16Chars TreeString');

    // Test cases for SlicedString
    const slicedValidUTF16String: String = valiedUTF16String.substring(4, valiedUTF16String.length - 4);
    arktest.assertEQ(slicedValidUTF16String.toUpperCase(), 'O 你好 नमस्ते 😊👩💻👨‍💻ААББВВГГДД',
        'valid UTF-16 SlicedString');
    const slicedValidEngChars: String = validEngChars.substring(1, validEngChars.length - 1);
    arktest.assertEQ(slicedValidEngChars.toUpperCase(), 'LPHABETALPHABETABCAB' + expI(),
        'valid EngChars SlicedString');
    const slicedInvalidUTF16String: String = invalidUTF16String.substring(2, invalidUTF16String.length - 2);
    arktest.assertEQ(slicedInvalidUTF16String.toUpperCase(), 'RMAL START\uD83DM' + expI() +
        'D CONTENT\uD83D\uDC68E', 'invalid UTF16Chars SlicedString');
}

/**
 * Tests the uppercase conversion for a short (less than 8 characters) compressible (latin-only) strings.
 */
function testShortCompressedToUpperCase() {
    const input1 = 'a';
    const input1Next = 'alphaNewString';
    const input1NextLen = input1Next.length;
    arktest.assertEQ(input1.length, 1);
    const expected1 = 'A';
    const result1 = input1.toUpperCase();
    arktest.assertEQ(result1, expected1, `Short Compressible Failed: ${result1} vs ${expected1}`);
    arktest.assertEQ(input1Next, 'alphaNewString', 'the next string must not be touched');
    arktest.assertEQ(input1Next.length, input1NextLen, 'the next string must not be touched including the length');

    const input7 = 'abidefg';
    const input7Next = 'betaNewString';
    const input7NextLen = input7Next.length;
    arktest.assertEQ(input7.length, 7);
    const expected7 = 'AB' + expI() + 'DEFG';
    const result7 = input7.toUpperCase();
    arktest.assertEQ(result7, expected7, `Short Compressible Failed: ${result7} vs ${expected7}`);
    arktest.assertEQ(input7Next, 'betaNewString', 'the next string must not be touched');
    arktest.assertEQ(input7Next.length, input7NextLen, 'the next string must not be touched including the length');

    const input8 = 'abidefgh';
    const input8Next = 'gammaNewString';
    const input8NextLen = input8Next.length;
    arktest.assertEQ(input8.length, 8);
    const expected8 = 'AB' + expI() + 'DEFGH';
    const result8 = input8.toUpperCase();
    arktest.assertEQ(result8, expected8, `Short Compressible Failed: ${result8} vs ${expected8}`);
    arktest.assertEQ(input8Next, 'gammaNewString', 'the next string must not be touched');
    arktest.assertEQ(input8Next.length, input8NextLen, 'the next string must not be touched including the length');
}

/**
 * Tests the uppercase conversion for compressible (latin-only) strings.
 */
function testCompressedToUpperCase() {
    // Base compressible string characters
    const input = ' aAbBcCdDeEfFgGhHiIjJkKlLmMnNoOpPq/!?QrRsStTuUvVwWxXyYz(){}[]Z1234567890.-+_:*\\\'"`';
    arktest.assertEQ(input.length, 26 * 2 + 10 + 20);
    const expected = ' AABBCCDDEEFFGGHH' + expI() + 'IJJKKLLMMNNOOPPQ/!?QRRSSTTUUVVWWXXYYZ()' +
        '{}[]Z1234567890.-+_:*\\\'"`';
    arktest.assertEQ(expected.length, 26 * 2 + 10 + 20);
    const result = input.toUpperCase();
    arktest.assertEQ(result, expected, `Compressible Linear String Failed: ${result} vs ${expected}`);

    // TreeString test
    const input_0 = 'abcdefghi!?-jklmnopqrstuvwxy(){}z[]';
    const input_1 = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ';
    const input_2 = ' 1234567890./+_:*\\\'"`';
    const inputTree = input_0.concat(input_1).concat(input_2);
    arktest.assertEQ(inputTree.length, 26 * 2 + 10 + 20);
    const expectedTree = 'ABCDEFGH' + expI() + '!?-JKLMNOPQRSTUVWXY(){}Z[]ABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890' +
        './+_:*\\\'"`';
    const resultTree = inputTree.toUpperCase();
    arktest.assertEQ(resultTree, expectedTree, `Compressible Tree String Failed: ${resultTree} vs ${expectedTree}`);

    // SlicedString test
    const inputSliced = '-*& abcdefghijklmnopq!?-rstuvwxy(){}z[]1234567890./+_:&\\\'"=`x'
        .substring(3, 26 + 3 + 10 + 21);
    arktest.assertEQ(inputSliced.length, 26 + 10 + 21);
    const expectedSliced = ' ABCDEFGH' + expI() + 'JKLMNOPQ!?-RSTUVWXY(){}Z[]1234567890./+_:&\\\'"=`';
    arktest.assertEQ(expectedSliced.length, 26 + 10 + 21);
    const resultSliced = inputSliced.toUpperCase();
    arktest.assertEQ(resultSliced, expectedSliced,
        `Compressible Sliced String Failed: ${resultSliced} vs ${expectedSliced}`);
}

function main() {
    let suite = new arktest.ArkTestsuite('StringToUpperCase');
    suite.addTest('testToUpperCase', testToUpperCase);
    suite.addTest('testCompressedToUpperCase', testCompressedToUpperCase);
    suite.addTest('testShortCompressedToUpperCase', testShortCompressedToUpperCase);
    return suite.run();
}
