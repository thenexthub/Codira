/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

function main() {
    let suite = new arktest.ArkTestsuite('StringToLocaleLowerCase');
    suite.addTest('testTurkishLocaleSpecific', testTurkishLocaleSpecific);
    suite.addTest('testEnglishLocaleSpecific', testEnglishLocaleSpecific);
    suite.addTest('testLithuanianLocaleSpecific', testLithuanianLocaleSpecific);
    suite.addTest('testCompressedTurkishLocaleSpecific', testCompressedTurkishLocaleSpecific);
    suite.addTest('testCompressedEnglishLocaleSpecificLinearLocale',
        testCompressedEnglishLocaleSpecificLinearLocale);
    suite.addTest('testCompressedEnglishLocaleSpecificLinearShortLocale',
        testCompressedEnglishLocaleSpecificLinearShortLocale);
    suite.addTest('testCompressedEnglishLocaleSpecificTreeLocale',
        testCompressedEnglishLocaleSpecificTreeLocale);
    suite.addTest('testCompressedEnglishLocaleSpecificLinearSlicedLocale',
        testCompressedEnglishLocaleSpecificSlicedLocale);
    suite.addTest('testCompressedRussianLocaleSpecificLinearLocale',
        testCompressedRussianLocaleSpecificLinearLocale);
    suite.addTest('testCompressedRussianLocaleSpecificLinearShortLocale',
        testCompressedRussianLocaleSpecificLinearShortLocale);
    suite.addTest('testCompressedRussianLocaleSpecificTreeLocale',
        testCompressedRussianLocaleSpecificTreeLocale);
    suite.addTest('testCompressedRussianLocaleSpecificLinearSlicedLocale',
        testCompressedRussianLocaleSpecificSlicedLocale);
    suite.addTest('testCompressedFrenchLocaleSpecificLinearShortLocale',
        testCompressedFrenchLocaleSpecificLinearShortLocale);
    suite.addTest('testCompressedGermanLocaleSpecificLinearShortLocale',
        testCompressedGermanLocaleSpecificLinearShortLocale);
    suite.addTest('testCompressedDutchLocaleSpecificLinearShortLocale',
        testCompressedDutchLocaleSpecificLinearShortLocale);
    suite.addTest('testCompressedItalianLocaleSpecificLinearShortLocale',
        testCompressedItalianLocaleSpecificLinearShortLocale);
    suite.addTest('testCompressedSpanishLocaleSpecificLinearShortLocale',
        testCompressedSpanishLocaleSpecificLinearShortLocale);
    suite.addTest('testCompressedPortugueseLocaleSpecificLinearShortLocale',
        testCompressedPortugueseLocaleSpecificLinearShortLocale);
    suite.addTest('testCompressedMalayLocaleSpecificLinearShortLocale',
        testCompressedMalayLocaleSpecificLinearShortLocale);
    suite.addTest('testCompressedHindiLocaleSpecificLinearShortLocale',
        testCompressedHindiLocaleSpecificLinearShortLocale);
    suite.addTest('testCompressedHebrewLocaleSpecificLinearShortLocale',
        testCompressedHebrewLocaleSpecificLinearShortLocale);
    suite.addTest('testCompressedGreekLocaleSpecificLinearShortLocale',
        testCompressedGreekLocaleSpecificLinearShortLocale);
    suite.addTest('testCompressedLithuanianLocaleSpecificLinearShortLocale',
        testCompressedLithuanianLocaleSpecificLinearShortLocale);
    suite.addTest('testCompressedChineseLocaleSpecificLinearShortLocale',
        testCompressedChineseLocaleSpecificLinearShortLocale);
    suite.addTest('testCompressedChineseLocaleSpecificLinearLocale',
        testCompressedChineseLocaleSpecificLinearLocale);
    suite.addTest('testCompressedChineseLocaleSpecificTreeLocale',
        testCompressedChineseLocaleSpecificTreeLocale);
    suite.addTest('testCompressedChineseLocaleSpecificLinearSlicedLocale',
        testCompressedChineseLocaleSpecificSlicedLocale);
    suite.addTest('testShortestCompressedEnglishLocaleSpecificLinearLocale',
        testShortestCompressedEnglishLocaleSpecificLinearLocale);
    suite.addTest('testShortCompressedEnglishLocaleSpecificLinearLocale',
        testShortCompressedEnglishLocaleSpecificLinearLocale);
    suite.addTest('testWrongLocale', testWrongLocale);
    suite.addTest('testToShortLocale', testToShortLocale);
    return suite.run();
}

// Test Case 1: Turkish-specific rules with mixed characters
// Input: 'İIĞŞÇÖÜ12345!@#ÂΣẞ' (length: 18)
// Locale: 'tr-TR'
// Expected Output: 'iığşçöü12345!@#âσß'
function testTurkishLocaleSpecific() {
    const input = 'İIĞŞÇÖÜ12345!@#ÂΣẞ';
    const expected = 'iığşçöü12345!@#âσß';
    const result = input.toLocaleLowerCase('tr-TR');
    arktest.assertEQ(result, expected, `TR Locale Failed: ${result} vs ${expected}`);

    const input_0 = 'İIĞŞÇÖÜ1';
    const input_1 = '2345!@#ÂΣẞ';
    const inputTree = input_0.concat(input_1);
    arktest.assertEQ(inputTree.toLocaleLowerCase('tr-TR'), expected,
        `TR Locale Failed: ${inputTree.toLocaleLowerCase('tr-TR')} vs ${expected}`);

    const inputSliced = 'abcİIĞŞÇÖÜ12345!@#ÂΣẞdef'.substring(3, 21);
    arktest.assertEQ(inputSliced.toLocaleLowerCase('tr-TR'), expected,
        `TR Locale Failed: ${inputSliced.toLocaleLowerCase('tr-TR')} vs ${expected}`);
}

// Tests Case 2: Turkish locale-specific uppercase conversion rules for compressible (basic-latin-only) strings.
function testCompressedTurkishLocaleSpecific() {
    // Base string test
    const input = 'aAbBcCdDeEfFgGhHiIjJkKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ1234567890./!?-+_:*\\\'"';
    arktest.assertEQ(input.length, 26 * 2 + 10 + 12);
    const expected = 'aabbccddeeffgghhiıjjkkllmmnnooppqqrrssttuuvvwwxxyyzz1234567890./!?-+_:*\\\'"';
    arktest.assertEQ(expected.length, 26 * 2 + 10 + 12);
    const result = input.toLocaleLowerCase('tr-TR');
    arktest.assertEQ(result, expected, `compressible TR Locale Failed: ${result} vs ${expected}`);

    // TreeString test
    const input_0 = 'abcdefghijklmnopqrstuvwxyz';
    const input_1 = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ';
    const input_2 = '1234567890./!?-+_:*\\\'"';
    const inputTree = input_0.concat(input_1).concat(input_2);
    arktest.assertEQ(inputTree.length, 26 * 2 + 10 + 12);
    const expectedTree = 'abcdefghijklmnopqrstuvwxyzabcdefghıjklmnopqrstuvwxyz1234567890./!?-+_:*\\\'"';
    const resultTree = inputTree.toLocaleLowerCase('tr-TR');
    arktest.assertEQ(resultTree, expectedTree, `compressible tree TR Locale Failed: ${resultTree} vs ${expectedTree}`);

    // SlicedString test
    const inputSliced = '-*&ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890./!?-+_:&\\\'"='.substring(3, 26 + 3 + 10 + 12);
    const expectedSliced = 'abcdefghıjklmnopqrstuvwxyz1234567890./!?-+_:&\\\'"';
    const resultSliced = inputSliced.toLocaleLowerCase('tr-TR');
    arktest.assertEQ(resultSliced, expectedSliced,
        `compressible sliced TR Locale Failed: ${resultSliced} vs ${expectedSliced}`);
}

// Test Case 3: English locale with Unicode normalization
// Input: 'IİĞŞÇÖÜ12345!@#ÂΣẞ' (length: 17)
// Locale: 'en-US'
// Expected Output: 'i\u0069\u0307ğşçöü12345!@#âσß' 
function testEnglishLocaleSpecific() {
    const input = 'IİĞŞÇÖÜ12345!@#ÂΣẞ';
    const expected = 'ii̇ğşçöü12345!@#âσß';
    const result = input.toLocaleLowerCase('en-US');
    arktest.assertEQ(result, expected, `en-US Locale Failed: ${result} vs ${expected}`);

    const input_0 = 'IİĞŞÇÖÜ1';
    const input_1 = '2345!@#ÂΣẞ';
    const inputTree = input_0.concat(input_1);
    arktest.assertEQ(result, inputTree.toLocaleLowerCase('en-US'));
    arktest.assertEQ(inputTree.toLocaleLowerCase('en-US'), expected,
        `en-US Locale Failed: ${inputTree.toLocaleLowerCase('en-US')} vs ${expected}`);

    const inputSliced = 'abcIİĞŞÇÖÜ12345!@#ÂΣẞdef'.substring(3, 21);
    arktest.assertEQ(inputSliced.toLocaleLowerCase('en-US'), expected,
        `en-US Locale Failed: ${inputSliced.toLocaleLowerCase('en-US')} vs ${expected}`);
}

// Test Case 4: In Lithuanian language letter J followed by combining grave accent (2 chars)
// results in j followed by combining dot above followed by combining grave accent (3 chars)
// after conversion to lower case.
// Locale "lt-LT"
function testLithuanianLocaleSpecific() {
    let buf: char[] = [c'J', c'\u0300', c'J', c'\u0300'];
    const input = buf.join('');
    let expbuf: char[] = [c'\u006a', c'\u0307', c'\u0300', c'\u006a', c'\u0307', c'\u0300'];
    let expected = expbuf.join('');
    let result = input.toLocaleLowerCase('lt-LT');
    arktest.assertEQ(result, expected, `lt-LT Locale Failed: ${result} vs ${expected}`);

    const input_0 = [c'J', c'\u0300', c'J', c'\u0300'].join('');
    const input_1 = [c'J', c'\u0300'].join('');
    const inputTree = input_0.concat(input_1);

    expected = [c'\u006a', c'\u0307', c'\u0300', c'\u006a', c'\u0307', c'\u0300', c'\u006a', c'\u0307', c'\u0300']
        .join('');
    result = inputTree.toLocaleLowerCase('lt-LT');
    arktest.assertEQ(result, expected, `lt-LT Locale Failed: ${result} vs ${expected}`);

    const inputSliced = [c'J', c'\u0300', c'J', c'\u0300'].join('').substring(2, 4);
    expected = [c'\u006a', c'\u0307', c'\u0300'].join('');
    result = inputSliced.toLocaleLowerCase('lt-LT');
    arktest.assertEQ(result, expected, `lt-LT Locale Failed: ${result} vs ${expected}`);
}

function getTreeLocale(lang: String, country: String) {
    arktest.assertEQ(lang.length, 2, 'The language code must have exactly 2 symbols');
    arktest.assertEQ(country.length, 2, 'The country code must have exactly 2 symbols');
    const locale = `${lang}-${country}`;
    arktest.assertEQ(locale.length, 5, `Locale length must be exactly 5 symbols, failed: ${locale}`);
    return locale;
}

function getSlicedLocale(lang: String, country: String) {
    arktest.assertEQ(lang.length, 2, 'The language code must have exactly 2 symbols');
    arktest.assertEQ(country.length, 2, 'The country code must have exactly 2 symbols');
    const data: String = `co${lang}-${country}Locale`;
    const locale = data.substring(2, 7);
    arktest.assertEQ(locale.length, 5, `Locale length must be exactly 5 symbols, failed: ${locale}`);
    return locale;
}

/**
 * Tests English (US) locale lowercase conversion for compressible (basic-latin-only) strings.
 */
function testCompressedEnglishLocaleSpecificLinearLocale() {
    testCompressedCompatibleLocaleSpecific('en-US');
}

function testCompressedEnglishLocaleSpecificLinearShortLocale() {
    testCompressedCompatibleLocaleSpecific('en');
}

function testCompressedEnglishLocaleSpecificTreeLocale() {
    testCompressedCompatibleLocaleSpecific(getTreeLocale('en', 'US'))
}

function testCompressedEnglishLocaleSpecificSlicedLocale() {
    testCompressedCompatibleLocaleSpecific(getSlicedLocale('en', 'US'));
}

function testShortCompressedStringInEnglishLocale(input: String, len: int, expected: String, next: String) {
    const inputNext = next.concat('X');
    const inputNextLen = inputNext.length;
    arktest.assertEQ(input.length, len);
    const result = input.toLocaleLowerCase('en-US');
    arktest.assertEQ(result, expected, `Short Compressible en-US Locale Failed: ${result} vs ${expected}`);
    arktest.assertEQ(inputNext, next.concat('X'), 'the next string must not be touched');
    arktest.assertEQ(inputNext.length, inputNextLen, 'the next string must not be touched including the length');
}

/**
 * Tests english-compatible locale lowercase conversion for a shortest (less than or equal to 8 characters) compressible
 * (basic-latin-only) strings.
 */
function testShortestCompressedEnglishLocaleSpecificLinearLocale() {
    testShortCompressedStringInEnglishLocale('A', 1, 'a', 'AlphaNewString');
    testShortCompressedStringInEnglishLocale('ABCDEFG', 7, 'abcdefg', 'BetaNewString');
    testShortCompressedStringInEnglishLocale('ABCDEFGH', 8, 'abcdefgh', 'GammaNewString');
}

/**
 * Tests english-compatible locale lowercase conversion for a short (less than 9-32 characters) compressible
 * (basic-latin-only) strings.
 */
function testShortCompressedEnglishLocaleSpecificLinearLocale() {
    testShortCompressedStringInEnglishLocale('ALPHALOCA', 9, 'alphaloca', 'AlphaNewString');
    testShortCompressedStringInEnglishLocale('ALPHALOCALE1ABC', 15, 'alphalocale1abc', 'BetaNewString');
    testShortCompressedStringInEnglishLocale('ALPHALOCALE1ABCD', 16, 'alphalocale1abcd', 'GammaNewString');
    testShortCompressedStringInEnglishLocale('ALPHALOCALE1ABCDE', 17, 'alphalocale1abcde', 'DeltaNewString');
    testShortCompressedStringInEnglishLocale('ALPHALOCALE1ABCDEFXYZWOP', 24,
        'alphalocale1abcdefxyzwop', 'UpsilonNewString');
    testShortCompressedStringInEnglishLocale('ALPHALOCALE1ABCDEFXYZWOPQRS', 27,
        'alphalocale1abcdefxyzwopqrs', 'OmicronNewString');
    testShortCompressedStringInEnglishLocale('ALPHALOCALE1ABCDEFXYZWOPQRSTUVJK', 32,
        'alphalocale1abcdefxyzwopqrstuvjk', 'ZetNewString');
}

/**
 * Test Russian (RU) locale lowercase conversion for compressible (basic-latin-only) strings.
 */
function testCompressedRussianLocaleSpecificLinearLocale() {
    testCompressedCompatibleLocaleSpecific('ru-RU');
}

function testCompressedRussianLocaleSpecificLinearShortLocale() {
    testCompressedCompatibleLocaleSpecific('ru');
}

function testCompressedRussianLocaleSpecificTreeLocale() {
    testCompressedCompatibleLocaleSpecific(getTreeLocale('ru', 'RU'))
}

function testCompressedRussianLocaleSpecificSlicedLocale() {
    testCompressedCompatibleLocaleSpecific(getSlicedLocale('ru', 'RU'));
}

/**
 * Test French locale lowercase conversion for compressible (basic-latin-only) strings.
 */
function testCompressedFrenchLocaleSpecificLinearShortLocale() {
    testCompressedCompatibleLocaleSpecific('fr');
}

/**
 * Test German locale lowercase conversion for compressible (basic-latin-only) strings.
 */
function testCompressedGermanLocaleSpecificLinearShortLocale() {
    testCompressedCompatibleLocaleSpecific('de');
}

/**
 * Test Dutch locale lowercase conversion for compressible (basic-latin-only) strings.
 */
function testCompressedDutchLocaleSpecificLinearShortLocale() {
    testCompressedCompatibleLocaleSpecific('nl');
}

/**
 * Test Italian locale lowercase conversion for compressible (basic-latin-only) strings.
 */
function testCompressedItalianLocaleSpecificLinearShortLocale() {
    testCompressedCompatibleLocaleSpecific('it');
}

/**
 * Test Spanish locale lowercase conversion for compressible (basic-latin-only) strings.
 */
function testCompressedSpanishLocaleSpecificLinearShortLocale() {
    testCompressedCompatibleLocaleSpecific('es');
}

/**
 * Test Portuguese locale lowercase conversion for compressible (basic-latin-only) strings.
 */
function testCompressedPortugueseLocaleSpecificLinearShortLocale() {
    testCompressedCompatibleLocaleSpecific('pt');
}

/**
 * Test Malay locale lowercase conversion for compressible (basic-latin-only) strings.
 */
function testCompressedMalayLocaleSpecificLinearShortLocale() {
    testCompressedCompatibleLocaleSpecific('ms');
}

/**
 * Test Hindi locale lowercase conversion for compressible (basic-latin-only) strings.
 */
function testCompressedHindiLocaleSpecificLinearShortLocale() {
    testCompressedCompatibleLocaleSpecific('hi');
}

/**
 * Test Hebrew locale lowercase conversion for compressible (basic-latin-only) strings.
 */
function testCompressedHebrewLocaleSpecificLinearShortLocale() {
    testCompressedCompatibleLocaleSpecific('he');
}

/**
 * Test Greek locale lowercase conversion for compressible (basic-latin-only) strings.
 */
function testCompressedGreekLocaleSpecificLinearShortLocale() {
    testCompressedCompatibleLocaleSpecific('el');
}

/**
 * Test Lithuanian locale lowercase conversion for compressible (basic-latin-only) strings.
 */
function testCompressedLithuanianLocaleSpecificLinearShortLocale() {
    testCompressedCompatibleLocaleSpecific('lt');
}

/**
 * Tests Chinese (zh-CN) locale lowercase conversion for compressible (basic-latin-only) strings.
 */
function testCompressedChineseLocaleSpecificLinearLocale() {
    testCompressedCompatibleLocaleSpecific('zh-CN');
}

function testCompressedChineseLocaleSpecificLinearShortLocale() {
    testCompressedCompatibleLocaleSpecific('zh');
}

function testCompressedChineseLocaleSpecificTreeLocale() {
    testCompressedCompatibleLocaleSpecific(getTreeLocale('zh', 'CN'))
}

function testCompressedChineseLocaleSpecificSlicedLocale() {
    testCompressedCompatibleLocaleSpecific(getSlicedLocale('zh', 'CN'));
}

/**
 * Tests english-compatible locale lowercase conversion for compressible (basic-latin-only) strings.
 * Important: the locale code may also be a tree or even slice string.
 */
function testCompressedCompatibleLocaleSpecific(locale: String) {
    // Base compressible string characters
    const input = ' aAbBcCdDeEfFgGhHiIjJkKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ1234567890./!?-+_:*\\\'"`(){}[]';
    arktest.assertEQ(input.length, 26 * 2 + 10 + 20);
    const expected = ' aabbccddeeffgghhiijjkkllmmnnooppqqrrssttuuvvwwxxyyzz1234567890./!?-+_:*\\\'"`(){}[]';
    arktest.assertEQ(expected.length, 26 * 2 + 10 + 20);
    const result = input.toLocaleLowerCase(locale);
    arktest.assertEQ(result, expected, `Compressible ${locale} Locale Failed: ${result} vs ${expected}`);

    // TreeString test
    const input_0 = 'abcdefghijklmnopqrstuvwxyz';
    const input_1 = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ';
    const input_2 = ' 1234567890./!?-+_:*\\\'"`(){}[]';
    const inputTree = input_0.concat(input_1).concat(input_2);
    arktest.assertEQ(inputTree.length, 26 * 2 + 10 + 20);
    const expectedTree = 'abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz 1234567890./!?-+_:*\\\'"`(){}[]';
    const resultTree = inputTree.toLocaleLowerCase(locale);
    arktest.assertEQ(resultTree, expectedTree,
        `Compressible Tree ${locale} Locale Failed: ${resultTree} vs ${expectedTree}`);

    // SlicedString test
    const inputSliced = '-*& ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890./!?-+_:&\\\'"=`(){}[]x'
        .substring(3, 26 + 3 + 10 + 21);
    arktest.assertEQ(inputSliced.length, 26 + 10 + 21);
    const expectedSliced = ' abcdefghijklmnopqrstuvwxyz1234567890./!?-+_:&\\\'"=`(){}[]';
    arktest.assertEQ(expectedSliced.length, 26 + 10 + 21);
    const resultSliced = inputSliced.toLocaleLowerCase(locale);
    arktest.assertEQ(resultSliced, expectedSliced,
        `Compressible Sliced ${locale} Locale Failed: ${resultSliced} vs ${expectedSliced}`);
}

function testWrongLocale() {
    const input = 'HELLO';
    const locale = 'ün';
    arktest.expectError(() => { input.toLocaleLowerCase(locale); return },
        `Language tag \'${locale}\' is invalid or not supported`);
}

function testToShortLocale() {
    const input = 'HELLO';
    const locale = 't';
    arktest.expectError(() => { input.toLocaleLowerCase(locale); return },
        `Language tag \'${locale}\' is invalid or not supported`);
}
