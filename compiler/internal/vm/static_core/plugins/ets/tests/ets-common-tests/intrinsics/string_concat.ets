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

let init: String[] = ['', 'Z', 'perseverance', 'Я', 'длинношеее']
let addend1: String[] = ['', 'Q', ' credentials', 'Ф', ' распространение']
let addend2: String[] = ['', 'R', ' essentially', 'Ж', ' многочлен']
let addend3: String[] = ['', 'W', ' hilariously', 'Ю', ' обоерукий']
let golden1: String[][] = [['', 'Q', ' credentials', 'Ф', ' распространение'],
                            ['Z', 'ZQ', 'Z  credentials', 'ZФ', 'Z распространение'],
                            ['perseverance', 'perseveranceZ', 'perseverance credentials',
                              'perseveranceФ', 'perseverance распространение'],
                            ['Я', 'ЯQ', 'Я credentials', 'ЯФ', 'Я распространение'],
                            ['длинношеее', 'длинношеееQ', 'длинношеее credentials',
                              'длинношеееФ', 'длинношеее распространение']];
let golden2: String[][] = [['', 'QR', ' credentials essentially', 'ФЖ',
                            'длинношеее распространение'],
                           ['Z', 'ZQR', 'Z credentials', 'Z credentials essentially',
                            'ZФЖ', 'Z распространение многочлен'],
                           ['perseverance', 'perseveranceQR', 'perseverance credentials essentially',
                            'perseveranceФЖ', 'perseverance распространение многочлен'],
                           ['Я', 'ЯQR', 'Я credentials essentially', 'ЯФЖ', 'Я распространение многочлен'],
                           ['длинношеее', 'длинношеееQR', 'длинношеее credentials essentially',
                            'длинношеееФЖ', 'длинношеее распространение многочлен']]


let golden3: String[][] = [['', 'QRW', ' credentials essentially hilariously',
                            'ФЖЮ', ' распространение многочлен обоерукий'],
                           ['Z', 'ZQRW', 'Z credentials essentially hilariously',
                            'ZФЖЮ', 'Z распространение многочлен обоерукий'],
                           ['perseverance', 'perseveranceQRW', 'perseverance credentials essentially hilariously',
                            'perseveranceФЖЮ', 'perseverance распространение многочлен обоерукий'],
                           ['Я', 'ЯQRW', 'Я credentials essentially hilariously',
                            'ЯФЖЮ', 'Я распространение многочлен обоерукий'],
                           ['длинношеее', 'длинношеееQRW', 'длинношеее credentials essentially hilariously',
                            'длинношеееФЖЮ', 'длинношеее распространение многочлен обоерукий']]

function testConcat2() {
    for (let i: int = 0; i < init.length; ++i) {
        for (let j: int = 0; i < addend1.length; ++i) {
            let str: String = init[i];
            let str1: String = addend1[i];
            str.concat(str1);
            arktest.assertEQ(str, golden1[i][j], 'concat2 failed at iteration: ' + i + ', ' + j +
                '\nexpected: ' + golden1[i][j] + '\nactual:  ' + str);
        }
    }
}

function testConcat3() {
    for (let i: int = 0; i < init.length; ++i) {
        for (let j: int = 0; i < addend1.length; ++i) {
            let str: String = init[i];
            let str1: String = addend1[i];
            let str2: String = addend2[i];
            str.concat(str1, str2);
            arktest.assertEQ(str, golden2[i][j], 'concat3 failed at iteration: ' + i + ', ' + j +
                '\nexpected: ' + golden2[i][j] + '\nactual:  ' + str);
        }
    }
}

function testConcat4() {
    for (let i: int = 0; i < init.length; ++i) {
        for (let j: int = 0; i < addend1.length; ++i) {
            let str: String = init[i];
            let str1: String = addend1[i];
            let str2: String = addend2[i];
            let str3: String = addend3[i];
            str.concat(str1, str2, str3);
            arktest.assertEQ(str, golden3[i][j], 'concat4 failed at iteration: ' + i + ', ' + j +
                '\nexpected: ' + golden3[i][j] + '\nactual:  ' + str);
        }
    }
}

function concatTest() {
    let basestring: String = '1234567'
    let basestring2: String = '8901234'
    let basestring3: String = 'abcdefg'
    let basestring4: String = 'hijklmn'

    let treeconcat2 = basestring.concat(basestring2)
    let treeconcat3 = basestring.concat(basestring2, basestring3)
    let treeconcat4 = basestring.concat(basestring2, basestring3, basestring4)
    arktest.assertEQ(treeconcat2, '12345678901234')
    arktest.assertEQ(treeconcat3, '12345678901234abcdefg')
    arktest.assertEQ(treeconcat4, '12345678901234abcdefghijklmn')

    const slicedstring: String = String('12345678901234567890').substring(1, 15) // 23456789012345
    let slicedconcat2 = slicedstring.concat(basestring2)
    let slicedconcat3 = slicedstring.concat(basestring2, basestring3)
    let slicedconcat4 = slicedstring.concat(basestring2, basestring3, basestring4)
    arktest.assertEQ(slicedconcat2, '234567890123458901234')
    arktest.assertEQ(slicedconcat3, '234567890123458901234abcdefg')
    arktest.assertEQ(slicedconcat4, '234567890123458901234abcdefghijklmn')
}

function main(): int {
    let suite = new arktest.ArkTestsuite('StringConcat');
    suite.addTest('concatTest', concatTest);
    suite.addTest('testConcat2', testConcat2);
    suite.addTest('testConcat3', testConcat3);
    suite.addTest('testConcat4', testConcat4);
    return suite.run();
}
