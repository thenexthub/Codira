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

function testNormalization(
    input: string,
    expectedNFC: string,
    expectedNFD: string,
    expectedNFKC: string,
    expectedNFKD: string
) {
    // test NFC
    arktest.assertTrue(input.normalize('NFC') === expectedNFC, 'NFC');
    // test NFD
    arktest.assertTrue(input.normalize('NFD') === expectedNFD, 'NFD');
    // test NFKC
    arktest.assertTrue(input.normalize('NFKC') === expectedNFKC, 'NFKC');
    // test NFKD
    arktest.assertTrue(input.normalize('NFKD') === expectedNFKD, 'NFKD');
}

const inputString = 'dghe\u0301a\u0301あ𝒜𝑨汉字𠜎abc𠁀xyzf?';

const expectedNFC = 'dgh\u00E9\u00E1あ𝒜𝑨\u6C49\u5B57𠜎abc𠁀xyzf?';
const expectedNFD = 'dghe\u0301a\u0301あ𝒜𝑨\u6C49\u5B57𠜎abc𠁀xyzf?';
const expectedNFKC = 'dgh\u00E9\u00E1あA\u0041\u6C49\u5B57𠜎abc𠁀xyzf?';
const expectedNFKD = 'dghe\u0301a\u0301あA\u0041\u6C49\u5B57𠜎abc𠁀xyzf?';

function testLineStringNormalize() {
    testNormalization(inputString, expectedNFC, expectedNFD, expectedNFKC, expectedNFKD);
}


function testTreeStringNormalize() {
    const inputStr_0 = 'dghe\u0301a\u0301あ𝒜';
    const inputStr_1 = '𝑨汉字𠜎abc𠁀xyzf?';
    const treeStr = inputStr_0.concat(inputStr_1);
    arktest.assertTrue(treeStr === inputString);
    testNormalization(treeStr, expectedNFC, expectedNFD, expectedNFKC, expectedNFKD);
}


function testSlicedStringNormalize() {
    const inputStr_2 = '-+&dghe\u0301a\u0301あ𝒜𝑨汉字𠜎abc𠁀xyzf?op]'
    const slicedStr = inputStr_2.substring(3, inputStr_2.length - 3);
    arktest.assertTrue(slicedStr === inputString);
    testNormalization(slicedStr, expectedNFC, expectedNFD, expectedNFKC, expectedNFKD);
}

function main() {
    let suite = new arktest.ArkTestsuite('StringNormalize');
    suite.addTest('testLineString string.normalize', testLineStringNormalize);
    suite.addTest('testTreeString string.normalize', testTreeStringNormalize);
    suite.addTest('testSlicedString string.normalize', testSlicedStringNormalize);
    return suite.run();
}