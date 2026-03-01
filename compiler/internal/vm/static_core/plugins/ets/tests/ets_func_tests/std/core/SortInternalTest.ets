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

function main(): void {
    let suite = new arktest.ArkTestsuite("FixedArray sort")
    suite.addTest('Sort fixed array booalean', testSortSubArrayBoolean)
    suite.addTest('Sort fixed array byte', testSortSubArrayByte)
    suite.addTest('Sort fixed array int', testSortSubArrayInt)
    suite.addTest('Sort fixed array short', testSortSubArrayShort)
    suite.run()
}

function compareArray(actual: FixedArray<boolean>, expected: FixedArray<boolean>) {
    arktest.assertEQ(actual.length, expected.length)
    for (let i: int = 0; i < expected.length; i++) {
        arktest.assertEQ(actual[i], expected[i])
    }
}

function testSortSubArrayBoolean() {
    const expected: FixedArray<boolean> = [true, true, true, true, true, true, true, true, true,
        true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, 
        true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, 
        true, true, true, true, true, true, true, true, true, true, true, true, true, false, false, false, 
        false, false, false, false, false, false, false, false, false, false, false, false, false, false, 
        false, false, false, false, false, false, false, false, false, false, false, false, false, false, 
        false, false, false, false, false, false, false, false, false, false, false, false, false, false, 
        false, false]
    const booleanArr: FixedArray<boolean> = [false, true, true, true, true, true, false, true, true, false,
        false, true, true, false, false, true, false, true, false, false, false, false, true, false, true,
        false, false, false, true, false, true, false, false, false, false, true, false, false, false, true,
        false, true, false, true, true, false, true, true, true, true, true, true, false, false, false, true,
        true, false, false, true, false, false, false, true, true, true, false, true, true, true, true, false,
        false, true, false, true, false, true, false, true, false, true, true, true, true, true, true, false,
        true, true, false, true, false, false, true, true, false, true, false, true]
    sort_subarray(booleanArr, (x: boolean, y: boolean): boolean => {return x})
    compareArray(booleanArr, expected)
}

function compareArray(actual: FixedArray<byte>, expected: FixedArray<byte>) {
    arktest.assertEQ(actual.length, expected.length)
    for (let i: int = 0; i < expected.length; i++) {
        arktest.assertEQ(actual[i], expected[i])
    }
}

function testSortSubArrayByte() {
    const expected: FixedArray<byte> = [125, 124, 120, 119, 119, 113, 113, 112, 110, 109, 103, 99, 98, 97, 94, 90, 86, 85, 84, 84,
        80, 76, 76, 73, 70, 69, 69, 66, 66, 64, 64, 64, 53, 50, 47, 47, 43, 39, 35, 32, 31, 26, 24, 17, 16, 16,
        14, 7, 7, 2, 1, -2, -2, -2, -3, -3, -8, -8, -14, -17, -21, -22, -22, -25, -27, -28, -33, -33, -34, -34,
        -36, -46, -46, -49, -49, -51, -64, -65, -66, -69, -69, -73, -73, -74, -76, -81, -84, -85, -85, -86, -88,
        -89, -92, -94, -105, -105, -111, -112, -119, -126]
    const byteArr: FixedArray<byte> = [110, -85, -64, -22, 16, -126, 26, -81, 47, 112, 66, 35, -112, -17, 66, 98,
        76, -76, -25, -36, -21, -46, -46, 31, 109, -14, -51, 125, 43, 64, 94, -69, 119, -111, -49, 86, 39, -105,
        124, 69, 69, -2, 24, 113, -33, -105, 50, 17, 99, 64, -73, -33, -28, 7, -22, 120, 84, -69, -2, 16, 76, 113,
        7, -92, 85, -119, -86, -66, 103, -89, 73, 70, 119, -94, 84, -34, 32, -3, -65, -2, 80, -34, 1, 64, 90, 2,
        -84, -8, -73, 53, -8, -85, -3, -74, 14, -88, 47, 97, -49, -27]
    sort_subarray(byteArr, (x: byte, y: byte): boolean => {return x > y})
    compareArray(byteArr, expected)
}

function compareArray(actual: FixedArray<int>, expected: FixedArray<int>) {
    arktest.assertEQ(actual.length, expected.length)
    for (let i: int = 0; i < expected.length; i++) {
        arktest.assertEQ(actual[i], expected[i])
    }
}

function testSortSubArrayInt() {
    const expected: FixedArray<int> = [2109977527, 2012300560, 1931684808, 1929587456, 1921519487, 1815024386, 1744369331, 1713477102,
        1697962950, 1684138993, 1631727886, 1630734246, 1628481717, 1518512590, 1430990831, 1401781736, 1380671913, 1219291838,
        1217655828, 1079938511, 1046341273, 1015050387, 997605253, 951560328, 951122696, 830254140, 822914282, 714231593, 494582522,
        476569008, 418478954, 359861230, 306559596, 246172713, 245608118, 176478440, 146830635, 144916293, 141860606, 92564286,
        66033672, 41711784, 11909251, -58150872, -170666672, -182079577, -190706634, -209156497, -267403348, -270981443, -276713497,
        -325611625, -400333014, -460905958, -491521477, -511551707, -515390360, -557714656, -638571978, -669538785, -698880583,
        -730150397, -732153823, -736576192, -823593699, -840752157, -867147851, -918547941, -1077749649, -1149181001, -1314172459,
        -1360020033, -1360504546, -1384591295, -1436995808, -1445198395, -1522383799, -1619196092, -1619233863, -1634115550,
        -1658567649, -1680944089, -1685939974, -1780632015, -1803194369, -1803948054, -1835473765, -1864387220, -1900946138,
        -1912645764, -1944536571, -1945744056, -1958679180, -1983718080, -2013550532, -2043237097, -2050135065, -2072140146,
        -2106652293, -2111130984]
    const intArr: FixedArray<int> = [245608118, -1436995808, -1685939974, -823593699, 418478954, -638571978, -730150397, -182079577,
        -2013550532, -867147851, 830254140, 1697962950, 1929587456, 476569008, -209156497, 1401781736, -1864387220, -1314172459,
        714231593, -918547941, 1079938511, 951122696, -1835473765, 146830635, -698880583, 1430990831, 1630734246, -190706634,
        -58150872, -1945744056, -1780632015, 1744369331, -1634115550, -557714656, 141860606, -736576192, -732153823,
        -460905958, -2111130984, 66033672, 1931684808, -2106652293, -170666672, 92564286, -1803948054, -1958679180, 306559596,
        1380671913, -1900946138, 1628481717, -267403348, 494582522, -515390360, -2072140146, 1046341273, 2012300560, 822914282,
        -2050135065, 1217655828, 11909251, 246172713, -276713497, -491521477, -400333014, -511551707, -1445198395, 41711784,
        -1360020033, 1219291838, -1680944089, 1518512590, 951560328, 1815024386, 997605253, -1384591295, -669538785, 144916293,
        -1944536571, -1983718080, -1912645764, -1658567649, 176478440, -270981443, 1713477102, -1522383799, 2109977527, -2043237097,
        -1619233863, 1684138993, -1077749649, -1149181001, -1360504546, -325611625, 359861230, -1803194369, -840752157, 1921519487,
        1015050387, -1619196092, 1631727886]
    sort_subarray(intArr, (x: int, y: int): boolean => {return x > y})
    compareArray(intArr, expected)
}

function compareArray(actual: FixedArray<short>, expected: FixedArray<short>) {
    arktest.assertEQ(actual.length, expected.length)
    for (let i: int = 0; i < expected.length; i++) {
        arktest.assertEQ(actual[i], expected[i])
    }
}

function testSortSubArrayShort() {
    const expected: FixedArray<short> = [28469, 27955, 27740, 24054, 23911, 21693, 20870, 16864, 16778, 16301, 14155, 12412, 11626,
        10113, 9037, 8409, 6467, 5970, 4609, 3105, 1987, 1845, 1671, 1538, 1094, 996, 983, 2, -702, -2926, -5887, -7393, -7459,
        -9275, -9446, -9712, -11008, -11927, -12763, -16254, -17644, -17919, -18099, -18568, -19247, -19905, -22119, -22989,
        -23267, -23416, -23892, -24305, -24922, -27070, -27234, -28802, -31388, -31543]
    const shortArr: FixedArray<short> = [21693, -9446, 10113, 3105, 996, -11927, -27234, -9275, -17919, -28802, -19905, 1671, 14155,
        -18099, -19247, -7393, -31388, 28469, 8409, 5970, 11626, 1987, 27740, -24305, -5887, -22119, -27070, 20870, -11008, 4609,
        -17644, -23416, 6467, -23892, 27955, -18568, -23267, 16778, 16864, 1538, 1094, -12763, 12412, 9037, 1845, -702, 983, -2926,
        -7459, -24922, -31543, -16254, 16301, 23911, -9712, -22989, 24054, 2]
    sort_subarray(shortArr, (x: short, y: short): boolean => {return x > y})
    compareArray(shortArr, expected)
}
