/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

const NULL_STR: String | null = null;
let str = 'abc';
let longStr = 'abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijkl';

interface Container {
    payload?: 'short' | 'long';
    padding?: String;
}

enum Size {
    SHORT = 0, LONG = 1,
}

function genericEquals<T>(obj1: T, obj2: T): boolean {
    // A separated function to test the generic comparison with `Ets[Strict]Equals` and also with this intrinsic
    // replaced with `StdCoreStringEquals` after JIT.
    return obj1 == obj2;
}

function __noinline__genericEquals<T>(obj1: T, obj2: T): boolean {
    // A separated function to test the generic comparison with `Ets[Strict]Equals`
    return obj1 == obj2;
}

function testUndefinedStringEquals() {
    let expectedEmpty: Container = { padding: 'padding' };
    let expectedFull: Container = { payload: 'short', padding: 'padding' };
    let resultEmpty: Container = { padding: 'padding' };
    let resultFull: Container = { payload: 'short', padding: 'padding'};

    arktest.assertTrue(__noinline__genericEquals(expectedEmpty.payload, resultEmpty.payload),
        'undefined string must be == to another undefined string');
    arktest.assertTrue(genericEquals(expectedEmpty.payload, resultEmpty.payload),
        'undefined string must be == to another undefined string');
    arktest.assertEQ(expectedEmpty.payload, resultEmpty.payload, '\'undefined\' must be === to \'undefined\'');

    arktest.assertFalse(__noinline__genericEquals(expectedEmpty.payload, resultFull.payload),
        'undefined string must be != to a string');
    arktest.assertFalse(genericEquals(expectedEmpty.payload, resultFull.payload),
        'undefined string must be != to a string');
    arktest.assertNE(expectedEmpty.payload, resultFull.payload, '\'undefined\' must be !== to a string');

    arktest.assertFalse(__noinline__genericEquals(expectedFull.payload, resultEmpty.payload),
        'a string must be != to undefined string');
    arktest.assertFalse(genericEquals(expectedFull.payload, resultEmpty.payload),
        'a string must be != to undefined string');
    arktest.assertNE(expectedFull.payload, resultEmpty.payload, 'a string must be !== to \'undefined\'');

    arktest.assertTrue(expectedEmpty.payload == resultEmpty.payload,
         'undefined string must be equal to another undefined string');
    arktest.assertFalse(expectedEmpty.payload == resultFull.payload,
         'undefined string must not be equal to a string');
    arktest.assertFalse(expectedFull.payload == resultEmpty.payload,
         'a string must not be equal to undefined string');

    arktest.assertTrue(expectedEmpty.payload === resultEmpty.payload,
         'undefined string must be strictly equal to another undefined string');
    arktest.assertFalse(expectedEmpty.payload === resultFull.payload,
         'undefined string must not be strictly equal to a string');
    arktest.assertFalse(expectedFull.payload === resultEmpty.payload,
         'a string must not be strictly equal to undefined string');
}

function testLineStringEquals() {
    arktest.assertNE(str, NULL_STR, 'String must not be equal to null');
    arktest.assertNE(NULL_STR, str, 'Null string must not be equal to non-null one');
    arktest.assertEQ(NULL_STR, NULL_STR, 'Null values of type String must be equal');
    arktest.assertEQ(str, str, 'String must be equal to itself');
    let abcstr = String.fromCharCode(0x61, 0x62, 0x63);
    arktest.assertEQ(str, abcstr, 'String must be equal to \'abc\'');
    arktest.assertTrue(str.equals(str), 'String must be equal to itself');
    arktest.assertTrue(str.equals(abcstr), 'String must be equal to \'abc\'');
    arktest.assertTrue(genericEquals(str, str), 'String must be equal to itself (generic)');
    arktest.assertTrue(genericEquals(str, abcstr), 'String must be equal to \'abc\' (generic)');

    let longCompareStr = 'abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijkl';
    arktest.assertEQ(longStr.length, 64, 'Long string must be of length 64');
    arktest.assertEQ(longStr, longCompareStr, 'long string must be == to its copy');
    arktest.assertTrue(longStr.equals(longCompareStr), 'Long string must be equal to its copy');
    arktest.assertTrue(genericEquals(longStr, longCompareStr), 'Long string must be equal to its copy (generic)');
}

function testOddLengthCompressedLineStringEquals() {
    let oddstr1 = 'abcabcabcdefxhijklmnopqrsXQsjdiwjsjdhoelsmnxhfh';
    let oddstr2 = String.fromCharCode(97, 98, 99, 97, 98, 99, 97, 98, 99, 100, 101, 102, 120, 104, 105, 106, 107,
        108, 109, 110, 111, 112, 113, 114, 115, 88, 81, 115, 106, 100, 105, 119, 106, 115, 106, 100, 104, 111, 101,
        108, 115, 109, 110, 120, 104, 102, 104);
    arktest.assertEQ(oddstr1.length, 47);
    arktest.assertEQ(oddstr2.length, 47);
    arktest.assertTrue(oddstr1.equals(oddstr2),
        'String of 47 compressible symbols, \'abcabc...\'. must be equal to \'abcabc...\'');
}

function testEvenLengthCompressedLineStringEquals() {
    let evenstr1 = 'abcabcabcdefxhijklmnopqrsXQsjdiwjsjdhoelsmnxhf';
    let evenstr2 = String.fromCharCode(97, 98, 99, 97, 98, 99, 97, 98, 99, 100, 101, 102, 120, 104, 105, 106, 107,
        108, 109, 110, 111, 112, 113, 114, 115, 88, 81, 115, 106, 100, 105, 119, 106, 115, 106, 100, 104, 111, 101,
        108, 115, 109, 110, 120, 104, 102);
    arktest.assertEQ(evenstr1.length, 46);
    arktest.assertEQ(evenstr2.length, 46);
    arktest.assertTrue(evenstr1.equals(evenstr2),
        'String of 46 compressible symbols, \'abcabc...\'. must be equal to \'abcabc...\'');
}

function testFullAlignedCompressedLineStringEquals() {
    let aligned1 = 'abcabcabcdefxhijklmnopqrsXQsjdiwjsjdhoelsmnxhfha';
    let aligned2 = String.fromCharCode(97, 98, 99, 97, 98, 99, 97, 98, 99, 100, 101, 102, 120, 104, 105, 106, 107,
        108, 109, 110, 111, 112, 113, 114, 115, 88, 81, 115, 106, 100, 105, 119, 106, 115, 106, 100, 104, 111, 101,
        108, 115, 109, 110, 120, 104, 102, 104, 97);
    arktest.assertEQ(aligned1.length % 8, 0);
    arktest.assertEQ(aligned2.length % 8, 0);
    arktest.assertTrue(aligned1.equals(aligned2),
        'String of 48 compressible symbols, \'abcabc...\'. must be equal to \'abcabc...\'');
}

function testTreeStringEquals() {
    let treeStr = str.concat('defghijklmnopqrstuvwxyz');
    arktest.assertTrue(treeStr.equals(treeStr), 'TreeString must be equal to itself');
    arktest.assertTrue(treeStr.equals('abcdefghijklmnopqrstuvwxyz'),
        'TreeString must be equal to \'abcdefghijklmnopqrstuvwxyz\'');

    let treeStr2 = str.concat('defghijklmnopqrstuvwxy1');
    arktest.assertNE(treeStr2, NULL_STR, 'TreeString must not be equal to null');
    arktest.assertNE(NULL_STR, treeStr2, 'Null string must not be equal to non-null one');
    arktest.assertEQ(treeStr2, treeStr2, 'TreeString must be equal to itself');
    arktest.assertEQ(treeStr2, 'abcdefghijklmnopqrstuvwxy1',
        'TreeString must be equal to \'abcdefghijklmnopqrstuvwxy1\'');

    let treeStr3 = str.concat('defghijklmnopqrstuvwxy2');
    arktest.assertTrue(genericEquals(treeStr3, 'abcdefghijklmnopqrstuvwxy2'),
        'TreeString must be equal to \'abcdefghijklmnopqrstuvwxy2\' (generic)');
}

function __noinline__stringEquals(str1: string, str2: string): boolean {
    // A separated function to not allow the JIT to eliminate the call to Intrinsic.StdCoreStringEquals
    return str1.equals(str2);
}

function testCachedTreeStringEquals() {
    let treeStr = str.concat('defghijklmnopqrstuvwxyz');
    let counter: boolean = __noinline__stringEquals(treeStr, 'abcdefghijklmnopqrstuvwxyz'); // To heat the cache.
    arktest.assertTrue(treeStr.equals('abcdefghijklmnopqrstuvwxyz'),
        'TreeString (cached) must be equal to \'abcdefghijklmnopqrstuvwxyz\'');
    arktest.assertTrue(counter);
    arktest.assertTrue(genericEquals(treeStr, 'abcdefghijklmnopqrstuvwxyz'),
        'TreeString (cached) must be equal to \'abcdefghijklmnopqrstuvwxyz\' (generic)');
}

function testLongTreeStringEquals() {
    arktest.assertEQ(longStr.length, 64, 'Long string must be of length 64');
    let longTreeStr = longStr.concat('defghijklmnopqrstuvwxy1');
    let longTreeCompareStr = longStr.concat('defghijklmnopqrstuvwxy1');
    arktest.assertTrue(longTreeStr.equals(longTreeStr), 'Long TreeString must be equal to itself');
    arktest.assertTrue(longTreeStr.equals(longTreeCompareStr), 'Long TreeString must be equal to its copy');


    let longTreeStr2 = longStr.concat('defghijklmnopqrstuvwx22');
    let longTreeCompareStr2 = longStr.concat('defghijklmnopqrstuvwx22');
    arktest.assertTrue(genericEquals(longTreeStr2, longTreeCompareStr2),
        'Long TreeString must be equal to its copy (generic)');
}

function testLongCachedTreeStringEquals() {
    arktest.assertEQ(longStr.length, 64, 'Long string must be of length 64');
    let longTreeStr = longStr.concat('defghijklmnopqrstuvwxy1');
    let longTreeCompareStr = longStr.concat('defghijklmnopqrstuvwxy1');
    let counter: boolean = __noinline__stringEquals(longTreeStr, longTreeCompareStr); // To heat the cache.
    arktest.assertTrue(longTreeStr.equals(longTreeCompareStr), 'Long TreeString (cached) must be equal to its copy');
    arktest.assertTrue(counter);
    arktest.assertTrue(genericEquals(longTreeStr, longTreeCompareStr),
        'Long TreeString (cached) must be equal to its copy (generic)');
}

function testSlicedStringEquals() {
    let treeStr = str.concat('defghijklmnopqrstuvwxyz');
    let slicedStr = treeStr.substring(1, 17);  // 'bcdefghijklmnopq'
    arktest.assertTrue(slicedStr.equals(slicedStr), 'SlicedString must be equal to itself');
    arktest.assertTrue(slicedStr.equals('bcdefghijklmnopq'), 'SlicedString must be equal to \'bcde\'');
    arktest.assertTrue(genericEquals(slicedStr, 'bcdefghijklmnopq'),
        'SlicedString must be equal to \'bcde\' (generic)');

    let treeStr2 = str.concat('1efghijklmnopqrstuvwxyz');
    let slicedStr2 = treeStr2.substring(1, 17);  // 'bc1efghijklmnopq'
    arktest.assertNE(slicedStr2, NULL_STR, 'SlicedString must not be equal to null');
    arktest.assertNE(NULL_STR, slicedStr2, 'Null string must not be equal to non-null one');
    arktest.assertEQ(slicedStr2, slicedStr2, 'SlicedString must be equal to itself');
    arktest.assertEQ(slicedStr2, 'bc1efghijklmnopq', 'SlicedString must be equal to \'bc1e\'');

    let treeStr3 = str.concat('2efghijklmnopqrstuvwxyz');
    let slicedStr3 = treeStr3.substring(1, 17);  // 'bc2efghijklmnopq'
    arktest.assertTrue(genericEquals(slicedStr3, 'bc2efghijklmnopq'),
        'SlicedString must be equal to \'bc2e\' (generic)');
}

function testCachedSlicedStringEquals() {
    let treeStr = str.concat('def1hijklmnopqrstuvwxyz');
    let slicedStr = treeStr.substring(1, 17);  // 'bcdefghijklmnopq'
    let counter: boolean = __noinline__stringEquals(slicedStr, 'bcdef1hijklmnopq'); // To heat the cache.
    arktest.assertTrue(slicedStr.equals('bcdef1hijklmnopq'), 'SlicedString (cached) must be equal to \'bcde\'');
    arktest.assertTrue(counter);
    arktest.assertTrue(genericEquals(slicedStr, 'bcdef1hijklmnopq'),
        'SlicedString (cached) must be equal to \'bcde\' (generic)');
}

function testLongSlicedStringEquals() {
    arktest.assertEQ(longStr.length, 64, 'Long string must be of length 64');
    let longTreeStr = longStr.concat('defghijklmnopqrstuvwxyz');
    let longTreeCompareStr = longStr.concat('defghijklmnopqrstuvwxyz');
    let longSlicedStr = longTreeStr.substring(1, 66);  // 'bcdefghijklmnopq...'
    let longSlicedCompareStr = longTreeCompareStr.substring(1, 66);  // 'bcdefghijklmnopq...'
    arktest.assertTrue(longSlicedStr.equals(longSlicedStr), 'Long SlicedString must be equal to itself');
    arktest.assertTrue(longSlicedStr.equals(longSlicedCompareStr), 'Long SlicedString must be equal to its copy');


    let longTreeStr2 = longStr.concat('d3fghijklmnopqrstuvwxyz');
    let longTreeCompareStr2 = longStr.concat('d3fghijklmnopqrstuvwxyz');
    let longSlicedStr2 = longTreeStr2.substring(1, 66);  // 'bcd3fghijklmnopq...'
    let longSlicedCompareStr2 = longTreeCompareStr2.substring(1, 66);  // 'bcd3fghijklmnopq...'
    arktest.assertTrue(genericEquals(longSlicedStr2, longSlicedCompareStr2),
        'Long SlicedString must be equal to its copy (generic)');
}

function testLongCachedSlicedStringEquals() {
    arktest.assertEQ(longStr.length, 64, 'Long string must be of length 64');
    let longTreeStr = longStr.concat('1efghijklmnopqrstuvwxyz');
    let longTreeCompareStr = longStr.concat('1efghijklmnopqrstuvwxyz');
    let longSlicedStr = longTreeStr.substring(1, 66);  // 'bc1efghijklmnopq...'
    let longSlicedCompareStr = longTreeCompareStr.substring(1, 66);  // 'bc1efghijklmnopq...'
    let counter: boolean = __noinline__stringEquals(longSlicedStr, longSlicedCompareStr); // To heat the cache.
    arktest.assertTrue(longSlicedStr.equals(longSlicedCompareStr), 'Long SlicedString (cached) must be equal to its copy');
    arktest.assertTrue(counter);
    arktest.assertTrue(genericEquals(longSlicedStr, longSlicedCompareStr),
        'Long SlicedString (cached) must be equal to its copy (generic)');
}

function main(): int {
    let suite = new arktest.ArkTestsuite('StringEquals');
    suite.addTest('testUndefinedStringEquals', testUndefinedStringEquals);
    suite.addTest('testLineStringEquals', testLineStringEquals);
    suite.addTest('testTreeStringEquals', testTreeStringEquals);
    suite.addTest('testCachedTreeStringEquals', testCachedTreeStringEquals);
    suite.addTest('testLongTreeStringEquals', testLongTreeStringEquals);
    suite.addTest('testLongCachedTreeStringEquals', testLongCachedTreeStringEquals);
    suite.addTest('testSlicedStringEquals', testSlicedStringEquals);
    suite.addTest('testCachedSlicedStringEquals', testCachedSlicedStringEquals);
    suite.addTest('testLongSlicedStringEquals', testLongSlicedStringEquals);
    suite.addTest('testLongCachedSlicedStringEquals', testLongCachedSlicedStringEquals);
    suite.addTest('testOddLengthCompressedLineStringEquals', testOddLengthCompressedLineStringEquals);
    suite.addTest('testEvenLengthCompressedLineStringEquals', testEvenLengthCompressedLineStringEquals);
    suite.addTest('testFullAlignedCompressedLineStringEquals', testFullAlignedCompressedLineStringEquals);
    return suite.run();
}
