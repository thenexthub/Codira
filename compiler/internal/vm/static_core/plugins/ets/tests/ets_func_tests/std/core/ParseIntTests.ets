/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
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

const noValidNumberError = new FormatError('String does not contain a valid number')
const overflowError = new FormatError('Value exceeds integer limits')
const invalidRadixError = new ArgumentOutOfRangeError('Invalid radix')
const invalidHexPrefixError = new FormatError('Invalid hex prefix')

function main(): int {
    const suite = new arktest.ArkTestsuite('parseInt negative tests for integer types')
    suite.addTest('Test Byte.parseInt', testByteParseInt)
    suite.addTest('Test Short.parseInt', testShortParseInt)
    suite.addTest('Test Int.parseInt', testIntParseInt)
    suite.addTest('Test Long.parseInt', testLongParseInt)
    return suite.run(); 
}

function commonErrors<T>(callback: (str: string, radix: int) => T) {
    arktest.expectError(() => { callback('', 10) }, noValidNumberError)
    arktest.expectError(() => { callback('    ', 10) }, noValidNumberError)
    arktest.expectError(() => { callback('\t\n', 10) }, noValidNumberError)
    arktest.expectError(() => { callback('   [', 10) }, noValidNumberError)

    arktest.expectError(() => { callback('+', 10) }, noValidNumberError)
    arktest.expectError(() => { callback('-', 10) }, noValidNumberError)

    arktest.expectError(() => { callback('123', 1) }, invalidRadixError)
    arktest.expectError(() => { callback('123', 37) }, invalidRadixError)
    arktest.expectError(() => { callback('123', -1) }, invalidRadixError)

    arktest.expectError(() => { callback('0b0', 16) }, invalidHexPrefixError)
    arktest.expectError(() => { callback('0x', 16) }, noValidNumberError)

    arktest.expectError(() => { callback('  -  ', 10) }, noValidNumberError)
    arktest.expectError(() => { callback('abc', 10) }, noValidNumberError)
    arktest.expectError(() => { callback('2', 2) }, noValidNumberError)
    arktest.expectError(() => { callback('GHIJ', 16) }, noValidNumberError)
    arktest.expectError(() => { callback('[', 36) }, noValidNumberError)

    arktest.expectError(() => { callback('+-123', 10) }, noValidNumberError)
    arktest.expectError(() => { callback('- 123', 10) }, noValidNumberError)
}

function testByteParseInt() {
    commonErrors((str: string, radix: int) => Byte.parseInt(str, radix))

    arktest.expectError(() => { Byte.parseInt('10000000', 2) }, overflowError)
    arktest.expectError(() => { Byte.parseInt('-10000001', 2) }, overflowError)

    arktest.expectError(() => { Byte.parseInt('200', 8) }, overflowError)
    arktest.expectError(() => { Byte.parseInt('-201', 8) }, overflowError)

    arktest.expectError(() => { Byte.parseInt('128', 10) }, overflowError)
    arktest.expectError(() => { Byte.parseInt('-129', 10) }, overflowError)

    arktest.expectError(() => { Byte.parseInt('80', 16) }, overflowError)
    arktest.expectError(() => { Byte.parseInt('-81', 16) }, overflowError)

    arktest.expectError(() => { Byte.parseInt('3K', 36) }, overflowError)
    arktest.expectError(() => { Byte.parseInt('-3L', 36) }, overflowError)
}

function testShortParseInt() {
    commonErrors((str: string, radix: int) => Short.parseInt(str, radix))

    arktest.expectError(() => { Short.parseInt('1000000000000000', 2) }, overflowError)
    arktest.expectError(() => { Short.parseInt('-1000000000000001', 2) }, overflowError)

    arktest.expectError(() => { Short.parseInt('100000', 8) }, overflowError)
    arktest.expectError(() => { Short.parseInt('-100001', 8) }, overflowError)

    arktest.expectError(() => { Short.parseInt('32768', 10) }, overflowError)
    arktest.expectError(() => { Short.parseInt('-32769', 10) }, overflowError)

    arktest.expectError(() => { Short.parseInt('8000', 16) }, overflowError)
    arktest.expectError(() => { Short.parseInt('-8001', 16) }, overflowError)

    arktest.expectError(() => { Short.parseInt('PA8', 36) }, overflowError)
    arktest.expectError(() => { Short.parseInt('-PA9', 36) }, overflowError)
}

function testIntParseInt() {
    commonErrors((str: string, radix: int) => Int.parseInt(str, radix))

    arktest.expectError(() => { Int.parseInt('10000000000000000000000000000000', 2) }, overflowError)
    arktest.expectError(() => { Int.parseInt('-10000000000000000000000000000001', 2) }, overflowError)

    arktest.expectError(() => { Int.parseInt('20000000000', 8) }, overflowError)
    arktest.expectError(() => { Int.parseInt('-20000000001', 8) }, overflowError)

    arktest.expectError(() => { Int.parseInt('2147483648', 10) }, overflowError)
    arktest.expectError(() => { Int.parseInt('-2147483649', 10) }, overflowError)

    arktest.expectError(() => { Int.parseInt('80000000', 16) }, overflowError)
    arktest.expectError(() => { Int.parseInt('-80000001', 16) }, overflowError)

    arktest.expectError(() => { Int.parseInt('ZIK0ZK', 36) }, overflowError)
    arktest.expectError(() => { Int.parseInt('-ZIK0ZL', 36) }, overflowError)
}

function testLongParseInt() {
    commonErrors((str: string, radix: int) => Long.parseInt(str, radix))

    arktest.expectError(() => { Long.parseInt('1000000000000000000000000000000000000000000000000000000000000000', 2) }, overflowError)
    arktest.expectError(() => { Long.parseInt('-1000000000000000000000000000000000000000000000000000000000000001', 2) }, overflowError)

    arktest.expectError(() => { Long.parseInt('1000000000000000000000', 8) }, overflowError)
    arktest.expectError(() => { Long.parseInt('-1000000000000000000001', 8) }, overflowError)

    arktest.expectError(() => { Long.parseInt('9223372036854775808', 10) }, overflowError)
    arktest.expectError(() => { Long.parseInt('-9223372036854775809', 10) }, overflowError)

    arktest.expectError(() => { Long.parseInt('8000000000000000', 16) }, overflowError)
    arktest.expectError(() => { Long.parseInt('-8000000000000001', 16) }, overflowError)

    arktest.expectError(() => { Long.parseInt('1Y2P0IJ32E8E8', 36) }, overflowError)
    arktest.expectError(() => { Long.parseInt('-1Y2P0IJ32E8E9', 36) }, overflowError)
}
