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

/**
 * Check created resizable ArrayBuffer with various length and maxByteLength parameters
 */
function createResizableArrayBuffer(): void {
    let a = new ArrayBuffer(10)
    // ArrayBuffer is resizable only if `maxByteLength` was passed
    arktest.assertFalse(a.resizable)
    arktest.assertEQ(a.byteLength, 10)
    arktest.assertFalse(a.detached)

    a = new ArrayBuffer(10, 10.0)
    arktest.assertTrue(a.resizable)
    arktest.assertEQ(a.byteLength, 10)
    arktest.assertFalse(a.detached)

    a = new ArrayBuffer(0, 10.0)
    arktest.assertTrue(a.resizable)
    arktest.assertEQ(a.byteLength, 0)
    arktest.assertFalse(a.detached)

    arktest.expectError((): void => { let a1 = new ArrayBuffer(11, -10) }, new RangeError("maxByteLength can't be less than 0"))
    arktest.expectError((): void => { let a1 = new ArrayBuffer(11, 10) }, new RangeError("length can't be greater than maxByteLength"))
}

function checkCreatedArrayBuffer(a: ArrayBuffer, bytes: FixedArray<byte>): void {
    arktest.assertEQ(a.byteLength, bytes.length)
    arktest.assertFalse(a.resizable)
    arktest.assertFalse(a.detached)

    for (let idx = 0; idx < bytes.length; idx++) {
        arktest.assertEQ(bytes[idx], a.at(idx))
    }
}

/**
 * Check ArrayBuffer created using various factory methods
 */
function createArrayBufferFrom(): void {
    let bytes : FixedArray<byte> = new FixedArray<byte>(0)
    let a = ArrayBuffer.from(bytes)
    checkCreatedArrayBuffer(a, bytes)

    bytes = [1, 2, 3]
    a = ArrayBuffer.from(bytes)
    checkCreatedArrayBuffer(a, bytes)

    a = ArrayBuffer.from(new Uint8Array(a))
    checkCreatedArrayBuffer(a, bytes)

    a = ArrayBuffer.from(a, 0, bytes.length)
    checkCreatedArrayBuffer(a, bytes)

    a = ArrayBuffer.from(a, 1, 1)
    const expectedBytes: FixedArray<byte> = [bytes[1]]
    checkCreatedArrayBuffer(a, expectedBytes)
}

/**
 * Check resize() method of ArrayBuffer with various capacities and lengths
 */
function resizeArrayBuffer(): void {
    let a = new ArrayBuffer(10)
    arktest.expectError((): void => { a.resize(5) }, new TypeError("ArrayBuffer is not resizable"))

    a = new ArrayBuffer(10, 10.0);

    // Shrink length
    a.resize(5);
    arktest.assertEQ(a.byteLength, 5)

    arktest.expectError((): void => { a.set(5, 0 .toByte()) }, new IndexOutOfBoundsError("ArrayBuffer position 5 is out of bounds"))
    arktest.expectError((): void => { a.at(5) }, new IndexOutOfBoundsError("ArrayBuffer position 5 is out of bounds"))

    // Return previous length
    a.resize(10);
    arktest.assertEQ(a.byteLength, 10)
    const VALUE = 42 .toByte()
    a.set(5, VALUE)
    arktest.assertEQ(a.at(5), VALUE)

    arktest.expectError((): void => { a.resize(11) }, new RangeError("new length cannot be larger than maxByteLength"))
    arktest.expectError((): void => { a.resize(-11) }, new RangeError("new length cannot be larger than maxByteLength"))

    a = new ArrayBuffer(10, 12.0)
    arktest.expectError((): void => { a.set(10, 0 .toByte()) }, new IndexOutOfBoundsError("ArrayBuffer position 10 is out of bounds"))
    arktest.expectError((): void => { a.at(10) }, new IndexOutOfBoundsError("ArrayBuffer position 10 is out of bounds"))

    a.resize(11)
    a.set(10, VALUE)
    arktest.assertEQ(a.at(10), VALUE)
}

function testManagedAccessors(): void {
    const REDUCED_LENGTH = 5
    const FULL_LENGTH = REDUCED_LENGTH * 2
    let a = new ArrayBuffer(FULL_LENGTH, FULL_LENGTH)
    const expectedBytes = new byte[a.getByteLength()]
    for (let i = 0; i < a.getByteLength(); i++) {
        expectedBytes[i] = i .toByte()
    }

    for (let i = 0; i < a.getByteLength(); i++) {
        a.set(i, expectedBytes[i])
        arktest.assertEQ(a.at(i), expectedBytes[i])
    }

    // Shrink length and check that non-shrinked bytes were not changed
    a.resize(REDUCED_LENGTH)
    for (let i = 0; i < a.getByteLength(); i++) {
        arktest.assertEQ(a.at(i), expectedBytes[i])
    }

    // Return the initial length, check that only previously kept (5) bytes were not changed
    a.resize(FULL_LENGTH)
    for (let i = 0; i < REDUCED_LENGTH; i++) {
        arktest.assertEQ(a.at(i), expectedBytes[i])
    }
}

function main(): int {
    const suite = new arktest.ArkTestsuite("ArrayBuffer properties")

    suite.addTest("Resizable ArrayBuffer creation", createResizableArrayBuffer)
    suite.addTest("ArrayBuffer factory methods", createArrayBufferFrom)
    suite.addTest("ArrayBuffer resize", resizeArrayBuffer)
    suite.addTest("Test at / set methods of managed ArrayBuffer instance", testManagedAccessors)

    return suite.run()
}
