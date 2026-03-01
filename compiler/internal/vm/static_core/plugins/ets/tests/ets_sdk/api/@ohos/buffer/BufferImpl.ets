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

import buffer from "@ohos.buffer"
import { jsonx } from "std/core"
import { BusinessError } from '@ohos.base';


function main(): int {
    const suite = new arktest.ArkTestsuite("Buffer API tests")

    suite.addTest("Test basic string encodings", testBasicStringEncodings)
    suite.addTest("Test Unicode string handling", testUnicodeString)
    suite.addTest("Test partial buffer reading", testPartialBufferReading)
    suite.addTest("Test mixed content handling", testMixedContent)
    suite.addTest("Test numbers handling", testNumbersHandling)
    suite.addTest("Test special characters", testSpecialCharacters)
    suite.addTest("Test edge cases", testEdgeCases)
    suite.addTest("Test all supported encodings", testAllSupportedEncodings)

    suite.addTest("Test byteLength with ASCII string", testByteLengthAscii)
    suite.addTest("Test byteLength with UTF8 string", testByteLengthUtf8)
    suite.addTest("Test byteLength with special characters", testByteLengthSpecialChars)
    suite.addTest("Test byteLength with TypedArrays", testByteLengthTypedArrays)
    suite.addTest("Test byteLength with Buffer and Views", testByteLengthBufferAndViews)
    suite.addTest("Test byteOffset", testByteOffset)

    suite.addTest("Test buffer.from with number array", testFromArray)
    suite.addTest("Test buffer.from with ArrayBuffer", testFromArrayBuffer)
    suite.addTest("Test buffer.from with Uint8Array", testFromUint8Array)
    suite.addTest("Test buffer.from with Buffer/Uint8Array", testFromBufferOrUint8Array)
    suite.addTest("Test buffer.from with string", testFromString)
    suite.addTest("Test buffer.from with string and base64 encoding", testFromStringWithBase64Encoding)
    suite.addTest("Test buffer.from with string with invalid length", testFromStringWithInvalidLength)
    suite.addTest("Test buffer.from with invalid base64 chars", testFromStringWithInvalidBase64Chars)
    suite.addTest("Test buffer.from with invalid base64url chars", testFromStringWithInvalidBase64UrlChars)
    suite.addTest("Test buffer.from with invalid hex chars", testFromStringWithInvalidHexChars)
    suite.addTest("Test buffer.from with invalid ascii chars", testFromStringWithInvalidAsciiChars)
    suite.addTest("Test buffer.from with invalid utf8 chars", testFromStringWithInvalidUtf8Chars)
    suite.addTest("Test buffer.from with invalid latin1 chars", testFromStringWithInvalidLatin1Chars)

    suite.addTest("Test alloc", testAlloc)
    suite.addTest("Test allocUninitialized", testAllocUninitialized)
    suite.addTest("Test allocUninitializedFromPool", testAllocUninitializedFromPool)
    suite.addTest("Test compare", testCompare)
    suite.addTest("Test concat", testConcat)
    suite.addTest("Test isBuffer", testIsBuffer)
    suite.addTest("Test isEncoding", testIsEncoding)
    suite.addTest("Test transcode", testTranscode)
    suite.addTest("Test Buffer.compare", testBufferCompare)
    suite.addTest("Test Buffer.compare with Buffer/Uint8Array", testBufferCompareUint8Array)
    suite.addTest("Test Buffer.copy", testBufferCopy)
    suite.addTest("Test Buffer.copy with params", testBufferCopyWithParams)
    suite.addTest("Test Buffer.copy with params 1", testBufferCopyWithParams1)
    suite.addTest("Test Buffer.copy with params 2", testBufferCopyWithParams2)
    suite.addTest("Test Buffer.copy with undefined", testBufferCopyWithUndefined)
    suite.addTest("Test entries", testEntries)
    suite.addTest("Test equals", testEquals)
    suite.addTest("Test fill", testFill)
    suite.addTest("Test fill hex", testFillHex);
    suite.addTest("Test fill empty", testFillEmpty);
    suite.addTest("Test fill ascii with unicode", testFillAsciiWithUnicode)
    suite.addTest("Test fill utf8 with unicode", testFillUtf8WithUnicode)
    suite.addTest("Test fill base64 with unicode", testFillBase64WithUnicode)
    suite.addTest("Test fill base64url with unicode", testFillBase64UrlWithUnicode)
    suite.addTest("Test fill ucs2 with unicode", testFillUcs2WithUnicode)
    suite.addTest("Test fill utf16le with unicode", testFillUtf16leWithUnicode);
    suite.addTest("Test includes hex", testIncludesHex);
    suite.addTest("Test from hex", testFromHex);
    suite.addTest("Test keys", testKeys)
    suite.addTest("Test subarray", testSubarray)
    suite.addTest("Test swap16", testSwap16)
    suite.addTest("Test swap32", testSwap32)
    suite.addTest("Test swap64", testSwap64)
    suite.addTest("Test values", testValues)

    suite.addTest("Test Int8 read/write", testInt8ReadWrite)
    suite.addTest("Test Int16 BE/LE read/write", testInt16ReadWrite)
    suite.addTest("Test Int32 BE/LE read/write", testInt32ReadWrite)
    suite.addTest("Test UInt8 read/write", testUInt8ReadWrite)
    suite.addTest("Test UInt16 BE/LE read/write", testUInt16ReadWrite)
    suite.addTest("Test UInt32 BE/LE read/write", testUInt32ReadWrite)
    suite.addTest("Test Float BE/LE read/write", testFloatReadWrite)
    suite.addTest("Test Double BE/LE read/write", testDoubleReadWrite)
    suite.addTest("Test BigInt64 BE/LE read/write", testBigInt64ReadWrite)
    suite.addTest("Test BigUInt64 BE/LE read/write", testBigUInt64ReadWrite)
    suite.addTest("Test variable length Int BE/LE read/write", testVarLengthIntReadWrite)
    suite.addTest("Test variable length UInt BE/LE read/write", testVarLengthUIntReadWrite)
    suite.addTest("Test string write", testStringWrite)
    suite.addTest("Test string write with invalid encoding", testBufferWrite0022)
    suite.addTest("Test string write base64", testBufferWriteStringBase64)
    suite.addTest("Test string write single parameter", testBufferWriteStringSingleParam)
    suite.addTest("Test string write with infinity length",  testBufferWriteStringInfinity)
    suite.addTest("Test string write wrong offset", testBufferWriteStringWrongOffset)
    suite.addTest("Test string write out range offset", testBufferWriteStringOutRangeOffset)
    suite.addTest("Test string write out range length", testBufferWriteStringOutRangeLength)
    suite.addTest("Test buffer from string in base64", testBufferFrom40002)
    suite.addTest("Test buffer from string in utf8", testBufferFrom30006)
    suite.addTest("Test buffer from Uint8Array in base64url", testBufferBase64Url)
    suite.addTest("Test UInt32 BE write", testUInt32BEWrite)
    suite.addTest("Test UInt32 LE write", testUInt32LEWrite)
    suite.addTest("Test UInt BE write", testUIntBEWrite)
    suite.addTest("Test UInt LE write", testUIntLEWrite)

    suite.addTest("Test buffer toJSON", testBufferToJSON)
    suite.addTest("Test readIntLE, readUIntLE, readIntBE, readUIntBE", testReadIntUintLEBE)
    suite.addTest("Test buffer.from, toString, transcode", testBufferToString00001)
    return suite.run()
}

// byteLength tests
function testByteLengthAscii() {
    const testStr = 'ABC123'

    // Test different ASCII-compatible encodings
    arktest.assertEQ(buffer.byteLength(testStr, 'ascii'), 6)
    arktest.assertEQ(buffer.byteLength(testStr, 'utf8'), 6)
    arktest.assertEQ(buffer.byteLength(testStr, 'utf-8'), 6)
    arktest.assertEQ(buffer.byteLength(testStr, 'latin1'), 6)
    arktest.assertEQ(buffer.byteLength(testStr, 'binary'), 6)
}

function testByteLengthUtf8() {
    const testStr = 'ABC123'

    // Test UTF-16 variants
    arktest.assertEQ(buffer.byteLength(testStr, 'utf16le'), 12)
    arktest.assertEQ(buffer.byteLength(testStr, 'ucs2'), 12)
    arktest.assertEQ(buffer.byteLength(testStr, 'ucs-2'), 12)
}

function testByteLengthSpecialChars() {
    const specialStr = '你好，世界🌍'

    arktest.assertEQ(buffer.byteLength(specialStr, 'utf8'), 19)
    arktest.assertEQ(buffer.byteLength(specialStr, 'utf16le'), 38)
    arktest.assertEQ(buffer.byteLength(specialStr, 'ascii'), 19)
}

function testByteLengthTypedArrays() {
    const int8Array = new Int8Array([1, 2, 3, 4])
    arktest.assertEQ(buffer.byteLength(int8Array), 4, "int8Array byteLength is not 4")

    const uint8Array = new Uint8Array([1, 2, 3, 4])
    arktest.assertEQ(buffer.byteLength(uint8Array), 4, "uint8Array byteLength is not 4")

    const uint8ClampedArray = new Uint8ClampedArray([1, 2, 3, 4])
    arktest.assertEQ(buffer.byteLength(uint8ClampedArray), 4, "uint8ClampedArray byteLength is not 4")

    const int16Array = new Int16Array([1, 2, 3, 4])
    arktest.assertEQ(buffer.byteLength(int16Array), 8, "int16Array byteLength is not 8")

    const uint16Array = new Uint16Array([1, 2, 3, 4])
    arktest.assertEQ(buffer.byteLength(uint16Array), 8, "uint16Array byteLength is not 8")

    const int32Array = new Int32Array([1, 2, 3, 4])
    arktest.assertEQ(buffer.byteLength(int32Array), 16, "int32Array byteLength is not 16")

    const uint32Array = new Uint32Array([1, 2, 3, 4])
    arktest.assertEQ(buffer.byteLength(uint32Array), 16, "uint32Array byteLength is not 16")

    const float32Array = new Float32Array([1, 2, 3, 4])
    arktest.assertEQ(buffer.byteLength(float32Array), 16, "float32Array byteLength is not 16")

    const float64Array = new Float64Array([1, 2, 3, 4])
    arktest.assertEQ(buffer.byteLength(float64Array), 32, "float64Array byteLength is not 32")

    const bigInt64Array = new BigInt64Array([1, 2, 3, 4])
    arktest.assertEQ(buffer.byteLength(bigInt64Array), 32, "bigInt64Array byteLength is not 32")

    const bigUint64Array = new BigUint64Array([1, 2, 3, 4])
    arktest.assertEQ(buffer.byteLength(bigUint64Array), 32, "bigUint64Array byteLength is not 32")
}

function testByteLengthBufferAndViews() {
    const bufferData = buffer.from('Hello, world!')
    arktest.assertEQ(buffer.byteLength(bufferData), 13)
    arktest.assertEQ(buffer.byteLength(bufferData, 'utf8'), 13)

    const arrayBuffer = new ArrayBuffer(16)
    const dataView = new DataView(arrayBuffer)
    arktest.assertEQ(buffer.byteLength(dataView), 16)
    arktest.assertEQ(buffer.byteLength(arrayBuffer), 16)
}

// buffer.from tests
function testFromArray() {
    const arr: number[] = [72, 101, 108, 108, 111] // "Hello" in ASCII
    const buf = buffer.from(arr)
    arktest.assertEQ(buf.length, 5)
    arktest.assertEQ(buf.toString('ascii'), "Hello")

    const singular: number[] = [256, 300, -1, 255]
    const truncatedBuf = buffer.from(singular)
    arktest.assertEQ(truncatedBuf.length, 4)
    // Values should be truncated to fit in 0-255 range
    arktest.assertEQ(truncatedBuf.at(0), 0) // 256 -> 0
    arktest.assertEQ(truncatedBuf.at(1), 44) // 300 -> 44
    arktest.assertEQ(truncatedBuf.at(2), -1) // -1 -> 255
    arktest.assertEQ(truncatedBuf.at(3), -1)
}

function testFromArrayBuffer() {
    const arr = new Uint8Array([72, 101, 108, 108, 111])
    const arrayBuffer = arr.buffer as ArrayBuffer

    const buf1 = buffer.from(arrayBuffer)
    arktest.assertEQ(buf1.length, 5)
    arktest.assertEQ(buf1.toString('ascii'), "Hello")

    const buf2 = buffer.from(arrayBuffer, 1)
    arktest.assertEQ(buf2.length, 4)
    arktest.assertEQ(buf2.toString('ascii'), "ello")

    const buf3 = buffer.from(arrayBuffer as Object, 1 as int, 2 as int)
    arktest.assertEQ(buf3.length, 2)
    arktest.assertEQ(buf3.toString('ascii'), "el")
}

function testFromBufferOrUint8Array() {
    const originalBuf = buffer.from([72, 101, 108, 108, 111])
    const copiedBuf = buffer.from(originalBuf)
    arktest.assertEQ(copiedBuf.length, originalBuf.length)
    arktest.assertEQ(copiedBuf.toString(), originalBuf.toString())

    const uint8Arr = new Uint8Array([72, 101, 108, 108, 111])
    const bufFromUint8 = buffer.from(uint8Arr)
    arktest.assertEQ(bufFromUint8.length, uint8Arr.length)
    arktest.assertEQ(bufFromUint8.toString('ascii'), "Hello")

    let b: ArrayBuffer = new ArrayBuffer(7);
    for (let i: int = 0; i < b.getByteLength(); ++i) {
        b.set(i, i.toByte())
    }
    const bufOffset = buffer.from(b as Object, 3, 4); // Make Buffer with offset
    arktest.assertEQ(bufOffset.at(0), 3)
    arktest.assertEQ(bufOffset.at(1), 4)
    arktest.assertEQ(bufOffset.at(2), 5)
    arktest.assertEQ(bufOffset.at(3), 6)
    arktest.assertEQ(bufOffset.length, 4)
    arktest.assertEQ(bufOffset.byteOffset, 0)

    let ab1 = ArrayBuffer.from("1236", "utf-8")
    arktest.assertEQ(ab1.byteLength, 4)

    let buf = buffer.from("1236")
    let ab = buf.buffer
    arktest.assertEQ(ab.byteLength, 4)
    arktest.assertEQ(buf.length, 4)
    let arr = new Uint8Array(ab)
    arktest.assertEQ(arr.length, 4)

    let a = new Uint8Array(buf.buffer, buf.byteOffset, buf.length);
    arktest.assertEQ(a.toString(), "49,50,51,54")
}

function testFromUint8Array() {
    let src = new Uint8Array(10);
    let buf3 = buffer.from(src);
    buf3.fill(1)

    arktest.assertEQ(src.toString(), '1,1,1,1,1,1,1,1,1,1')
    arktest.assertEQ(buf3.toString('hex'), '01010101010101010101')
}

function testFromString() {
    const str = "Hello, World!"
    const buf1 = buffer.from(str)
    arktest.assertEQ(buf1.toString(), str)

    const buf2 = buffer.from(str, 'ascii')
    arktest.assertEQ(buf2.toString('ascii'), str)

    const specialStr = '你好，世界🌍'
    const buf4 = buffer.from(specialStr)
    arktest.assertEQ(buf4.toString(), specialStr)
}

function testFromStringWithBase64Encoding() {
    let buf = buffer.from('abc', 'base64');
    arktest.assertEQ(buf.toString(), 'i·');
    arktest.assertEQ(buf.length, 2);
    arktest.assertEQ(buf.toString('base64'), 'abc=');
}

function testFromStringWithInvalidLength() {
    let buf = buffer.from(new String('this is a test'), 'utf8', -1);
    arktest.assertEQ(buf.length, 14);
    arktest.assertEQ(buf.toString(), 'this is a test');
}

function testFromStringWithInvalidBase64Chars() {
    const raw = "S G V sb G8="
    const buf = buffer.from(raw, 'base64')
    arktest.assertEQ(buf.toString('utf8'), 'Hello')
}

function testFromStringWithInvalidBase64UrlChars() {
    const raw = "V G V zd DE yMw?"
    const buf = buffer.from(raw, 'base64url')
    arktest.assertEQ(buf.toString('utf8'), 'Test123')
}

function testFromStringWithInvalidHexChars() {
    const buf = buffer.from("ffffhj", "hex")
    const buf2 = buffer.from("hjffff", "hex")
    arktest.assertEQ(buf.length, 2)
    arktest.assertEQ(buf.toString("hex"), "ffff")
    arktest.assertEQ(buf2.length, 0)
    arktest.assertEQ(buf2.toString("hex"), "")
}

function testFromStringWithInvalidAsciiChars() {
    const buf = buffer.from("Héllo", "ascii")
    arktest.assertEQ(buf.length, 5)
    arktest.assertEQ(buf.toString("ascii"), "Hillo")
}

function testFromStringWithInvalidUtf8Chars() {
    const raw = '\uD800'
    const buf = buffer.from(raw, "utf8")
    arktest.assertEQ(buf.length, 3)
    arktest.assertEQ(buf.toString("utf8"), '\uFFFD')
}

function testFromStringWithInvalidLatin1Chars() {
    const raw = "F1FG刘"
    const buf = buffer.from(raw, "latin1");
    const bufLength = buf.length.toInt();
    arktest.assertEQ(bufLength, 5);
    arktest.assertEQ(buf.at(0), 70);
    arktest.assertEQ(buf.at(bufLength - 1), 24);
}

// Buffer.toString() tests
function testBasicStringEncodings() {
    const asciiStr = 'Hello, World!'
    const asciiBuffer = buffer.from(asciiStr)
    arktest.assertEQ(asciiBuffer.toString('ascii'), 'Hello, World!')
    arktest.assertEQ(asciiBuffer.toString('utf8'), 'Hello, World!')
    arktest.assertEQ(asciiBuffer.toString('utf-8'), 'Hello, World!')
    arktest.assertEQ(asciiBuffer.toString('base64'), 'SGVsbG8sIFdvcmxkIQ==')
    arktest.assertEQ(asciiBuffer.toString('hex'), '48656c6c6f2c20576f726c6421')
}

function testUnicodeString() {
    const unicodeStr = '你好，世界🌍'
    const unicodeBuffer = buffer.from(unicodeStr)
    arktest.assertEQ(unicodeBuffer.toString('utf8'), '你好，世界🌍')
    arktest.assertEQ(unicodeBuffer.toString('base64'), '5L2g5aW977yM5LiW55WM8J+MjQ==')
}

function testPartialBufferReading() {
    const asciiStr = 'Hello, World!'
    const asciiBuffer = buffer.from(asciiStr)
    arktest.assertEQ(asciiBuffer.toString('utf8', 0, 5), 'Hello')
}

function testMixedContent() {
    const mixedStr = 'Hi, 世界!'
    const mixedBuffer = buffer.from(mixedStr)
    arktest.assertEQ(mixedBuffer.toString('utf8'), 'Hi, 世界!')
    arktest.assertEQ(mixedBuffer.toString('ascii'), 'Hi, 世界!')
    arktest.assertEQ(mixedBuffer.toString('base64'), 'SGksIOS4lueVjCE=')
}

function testNumbersHandling() {
    const numbers = '12345'
    const numbersBuffer = buffer.from(numbers)
    arktest.assertEQ(numbersBuffer.toString('utf8'), '12345')
    arktest.assertEQ(numbersBuffer.toString('ascii'), '12345')
    arktest.assertEQ(numbersBuffer.toString('hex'), '3132333435')
}

function testSpecialCharacters() {
    const specialChars = '!@#$%^&*()'
    const specialBuffer = buffer.from(specialChars)
    arktest.assertEQ(specialBuffer.toString('utf8'), '!@#$%^&*()')
    arktest.assertEQ(specialBuffer.toString('ascii'), '!@#$%^&*()')
    arktest.assertEQ(specialBuffer.toString('base64'), 'IUAjJCVeJiooKQ==')
}

function testEdgeCases() {
    // Empty buffer
    const emptyBuffer = buffer.from('')
    arktest.assertEQ(emptyBuffer.toString('utf8'), '')
    // Null bytes
    const nullBuffer = buffer.from('\0\0\0')
    arktest.assertEQ(nullBuffer.toString('utf8'), '\0\0\0')
    // Large buffer
    const largeStr = 'A'.repeat(1000)
    const largeBuffer = buffer.from(largeStr)
    arktest.assertEQ(largeBuffer.toString('utf8', 0, 50), 'A'.repeat(50))
}

function testAllSupportedEncodings() {
    const testStr = 'Test123'
    const testBuffer = buffer.from(testStr)
    arktest.assertEQ(testBuffer.toString('ascii'), 'Test123')
    arktest.assertEQ(testBuffer.toString('utf8'), 'Test123')
    arktest.assertEQ(testBuffer.toString('utf-8'), 'Test123')
    arktest.assertEQ(testBuffer.toString('base64'), 'VGVzdDEyMw==')
    arktest.assertEQ(testBuffer.toString('base64url'), 'VGVzdDEyMw')
    arktest.assertEQ(testBuffer.toString('hex'), '54657374313233')
}

function testAlloc() {
    const buf = buffer.alloc(10, 'a');
    arktest.assertEQ(buf.length, 10);
    arktest.assertEQ(buf.toString('ascii'), 'aaaaaaaaaa');

    const buf2 = buffer.alloc(5, 0x1 as long);
    arktest.assertEQ(buf2.length, 5);
    arktest.assertEQ(buf2.at(0), 0x1);
}

function testEmptyAlloc() {
    const escape = '\0'
    let buf = buffer.alloc(2, escape, 'utf8')
    arktest.assertEQ(encodeURI(buf.toString('utf8')), '%00%00')
}

function testAllocUninitialized() {
    const buf = buffer.allocUninitialized(10);
    arktest.assertEQ(buf.length, 10);
}

function testAllocUninitializedFromPool() {
    const buf = buffer.allocUninitializedFromPool(10);
    arktest.assertEQ(buf.length, 10);
}

function testCompare() {
    const buf1 = buffer.alloc(5, 'a');
    const buf2 = buffer.alloc(5, 'b');
    arktest.assertEQ(buffer.compare(buf1, buf2), -1);
    arktest.assertEQ(buffer.compare(buf2, buf1), 1);
    arktest.assertEQ(buffer.compare(buf1, buf1), 0);
}

function testConcat() {
    const buf1 = buffer.alloc(5, 'a');
    const buf2 = buffer.alloc(5, 'b');
    const concatenated = buffer.concat([buf1, buf2]);
    arktest.assertEQ(concatenated.length, 10);
    arktest.assertEQ(concatenated.toString('ascii'), 'aaaaabbbbb');
}

function testIsBuffer() {
    const buf = buffer.alloc(5);
    arktest.assertEQ(buffer.isBuffer(buf), true);
    arktest.assertEQ(buffer.isBuffer({}), false);
}

function testIsEncoding() {
    arktest.assertEQ(buffer.isEncoding('utf8'), true);
    arktest.assertEQ(buffer.isEncoding('hex'), true);
    arktest.assertEQ(buffer.isEncoding('unsupported'), false);
}

function testTranscode() {
    // invariant:
    //     X = transcode(B, S, T)
    //     B = transcode(X, T, S)
    const checkInvariant = (b: buffer.Buffer, s: buffer.BufferEncoding, t: buffer.BufferEncoding) => {
        let x = buffer.transcode(b, s, t);
        let b2 = buffer.transcode(x, t, s);
        arktest.assertEQ(b.length, b2.length, `Invariant violated: length of ${s} -> ${t} != length of ${t} -> ${s}`);
        arktest.assertEQ(b.toString(s), b2.toString(s), `Invariant violated: ${s} -> ${t} != ${t} -> ${s}`);
    }

    // 1. ascii → hex
    let b1 = buffer.from("Hello, World!", "ascii");
    let b1_hex = buffer.transcode(b1, "ascii", "hex");
    arktest.assertEQ(b1_hex.toString("hex"), "", "Case (1) failed: ascii -> hex");

    // 2. hex → base64
    let b2 = buffer.from("48656c6c6f2c20576f726c6421", "hex");
    let b2_b64 = buffer.transcode(b2, "hex", "base64");
    arktest.assertEQ(b2_b64.toString("base64"), "48656c6c6f2c20576f726c642w==", "Case (2) failed: hex -> base64");

    // 3. base64 → utf8
    let b3 = buffer.from("SGVsbG8sIFdvcmxkIQ==", "base64");
    let b3_utf8 = buffer.transcode(b3, "base64", "utf8");
    arktest.assertEQ(b3_utf8.toString("utf8"), "SGVsbG8sIFdvcmxkIQ==", "Case (3) failed: base64 -> utf8");
    checkInvariant(b3, "base64", "utf8");

    // 4. utf8 → hex
    let b4 = buffer.from("Привет", "utf8");
    let b4_hex = buffer.transcode(b4, "utf8", "hex");
    arktest.assertEQ(b4_hex.toString("hex"), "", "Case (4) failed: utf8 -> hex");

    // 5. latin1 → utf8
    let b5 = buffer.from("abc", "latin1");
    let b5_utf8 = buffer.transcode(b5, "latin1", "utf8");
    arktest.assertEQ(b5_utf8.toString("utf8"), "abc", "Case (5) failed: latin1 -> utf8");
    checkInvariant(b5, "latin1", "utf8");

    // 6. ascii → base64url
    let b6 = buffer.from("Hello-World_", "ascii");
    let b6_b64url = buffer.transcode(b6, "ascii", "base64url");
    arktest.assertEQ(b6_b64url.toString("base64url"), "Hello-World_", "Case (6) failed: ascii -> base64url");
    checkInvariant(b6, "ascii", "base64url");

    // 7. base64url → base64
    let b7 = buffer.from("SGVsbG8tV29ybGRf", "base64url");
    let b7_b64 = buffer.transcode(b7, "base64url", "base64");
    arktest.assertEQ(b7_b64.toString("base64"), "SGVsbG8tV29ybGRf", "Case (7) failed: base64url -> base64");
    checkInvariant(b7, "base64url", "base64");

    // 8. hex → ascii
    let b8 = buffer.from("48656c6c6f", "hex");
    let b8_ascii = buffer.transcode(b8, "hex", "ascii");
    arktest.assertEQ(b8_ascii.toString("ascii"), "48656c6c6f", "Case (8) failed: hex -> ascii");
    checkInvariant(b8, "hex", "ascii");

    // 9. base64 → hex
    let b9 = buffer.from("4pyTIMOgIGV4YW1wbGU=", "base64");
    let b9_hex = buffer.transcode(b9, "base64", "hex");
    arktest.assertEQ(b9_hex.toString("hex"), "", "Case (9) failed: base64 -> hex");

    //10. utf-8 → ascii
    {
        let newBuf = buffer.transcode(buffer.from('€'), 'utf-8', 'ascii')
        arktest.assertEQ(newBuf.toString(), '?', "Case (10) failed: utf-8 -> ascii")
    }

    {
        let newBuf = buffer.transcode(buffer.from('ab€gg£'), 'utf-8', 'ascii')
        arktest.assertEQ(newBuf.toString(), 'ab?gg?', "Case (10) failed: utf-8 -> ascii")
    }
}

function testBufferCompare() {
    const buf1 = buffer.alloc(5, 'a');
    const buf2 = buffer.alloc(5, 'b');
    arktest.assertEQ(buf1.compare(buf2), -1);
    arktest.assertEQ(buf2.compare(buf1), 1);
    arktest.assertEQ(buf1.compare(buf1), 0);
}

function testBufferCompareUint8Array() {
    // Call Buffer.compare Uint8Array (no byteOffset) vs Uint8Array (byteOffset)
    let a = new ArrayBuffer(10)
    let u1 = new Uint8Array(a, 0, 4) // has no byteOffset
    let u2 = new Uint8Array(a, 5, 3) // has byteOffset
    u1[0] = 1
    u1[1] = 1
    u1[2] = 1
    u1[3] = 1
    u2[0] = 22
    u2[1] = 22
    u2[2] = 22
    arktest.assertEQ(buffer.compare(u1, u2), -1)
    arktest.assertEQ(buffer.compare(u1, u1), 0)
    arktest.assertEQ(buffer.compare(u2, u1), 1)

    // Call Buffer.compare Uint8Arrays (no byteOffset) vs Buffer (no byteOffset)
    let bufNoOffset = buffer.alloc(3, 22 as long);
    arktest.assertEQ(buffer.compare(u1, bufNoOffset), -1)
    arktest.assertEQ(buffer.compare(bufNoOffset, u1), 1)

    // Call Buffer.compare Uint8Arrays (byteOffset) vs Buffer (no byteOffset)
    arktest.assertEQ(buffer.compare(u2, bufNoOffset), 0);
    arktest.assertEQ(buffer.compare(bufNoOffset, u2), 0);

    // Compare Uint8Arrays (byteOffset) vs Buffer (byteOffset)
    let b = new ArrayBuffer(7);
    for (let i: int = 0; i < b.getByteLength(); ++i) {
        b.set(i, i.toByte())
    }
    let bufOffset = buffer.from(b as Object, 3, 4); // Make Buffer with offset
    arktest.assertEQ(bufOffset.at(0), 3)
    arktest.assertEQ(bufOffset.at(1), 4)
    arktest.assertEQ(bufOffset.at(2), 5)
    arktest.assertEQ(bufOffset.at(3), 6)
    arktest.assertEQ(bufOffset.length, 4)
    arktest.assertEQ(bufOffset.byteOffset, 0)
    arktest.assertEQ(buffer.compare(u2, bufOffset), 1)
    arktest.assertEQ(buffer.compare(bufOffset, u2), -1)

    arktest.expectError(() => {
        let set = new Set<number>([97, 98, 99, 100, 101, 102, 103]);
        let buf = buffer.from(set as Object, 3, 4);
    }, 'Parameter error. The type of \"input\" must be Buffer or ArrayBuffer, Array, Array-like');
}

function testBufferCopy() {
    const buf1 = buffer.alloc(5, 'a');
    const buf2 = buffer.alloc(5);
    buf1.copy(buf2);
    arktest.assertEQ(buf2.toString('ascii'), 'aaaaa');
}

function testBufferCopyWithParams() {
    let b1 = buffer.from("abcdefg")
    let b2 = buffer.from("1235789")
    let num = b1.copy(b2, 2, 1, 3)
    arktest.assertEQ(num, 2)
    arktest.assertEQ(b2.toString(), "12bc789")
}

function testBufferCopyWithParams1() {
    const arr1 = new Uint8Array([97, 98, 99, 100, 101, 102, 103])
    let b1 = buffer.from(arr1.buffer, 2);
    const arr2 = new Uint8Array([49, 50, 51, 52, 53, 54, 55])
    let b2 = buffer.from(arr2.buffer, 1);
    let num = b1.copy(b2, 2, 1, 3)
    arktest.assertEQ(num, 2)
    arktest.assertEQ(b2.toString(), "23de67")
}
      
function testBufferCopyWithParams2() {
    const arr1 = new Uint8Array([97, 98, 99, 100, 101, 102, 103])
    let b1 = buffer.from(arr1.buffer, 2);
    const arr2 = new Uint8Array([49, 50, 51, 52, 53, 54, 55])
    let b2 = buffer.from(arr2.buffer, 1);
    let num = b1.copy(b2, 2, 1, 7)
    arktest.assertEQ(num, 4)
    arktest.assertEQ(b2.toString(), "23defg")
}


function testBufferCopyWithUndefined() {
    let buf1 = buffer.allocUninitializedFromPool(26);
    let buf2 = buffer.allocUninitializedFromPool(26).fill('!');
    for (let i = 0; i < 26; i++) {
        buf1.writeInt8(i + 97, i);
    }
    buf1.copy(buf2, undefined, 1);
    arktest.assertEQ(buf2.toString(), "bcdefghijklmnopqrstuvwxyz!")
}

function testEntries() {
    const buf = buffer.alloc(3, 'a');
    const entries = Array.from(buf.entries());
    arktest.assertEQ(entries.length, 3);
    arktest.assertEQ(entries[0][1], 97);
}

function testEquals() {
    const buf1 = buffer.alloc(5, 'a');
    const buf2 = buffer.alloc(5, 'a');
    const buf3 = buffer.alloc(5, 'b');
    arktest.assertEQ(buf1.equals(buf2), true);
    arktest.assertEQ(buf1.equals(buf3), false);
}

function testFill() {
    const buf = buffer.alloc(5);
    buf.fill('b');
    arktest.assertEQ(buf.toString('ascii'), 'bbbbb');

    let ab = new ArrayBuffer(10)
    let buf3 = buffer.from(ab)
    arktest.expectNoThrow(() => {
        arktest.assertEQ(JSON.stringify(buf3), '{"type":"Buffer","data":[0,0,0,0,0,0,0,0,0,0]}')
        buf3.fill(1 as long)
        arktest.assertEQ(new Uint8Array(ab).toString(), "1,1,1,1,1,1,1,1,1,1")
        arktest.assertEQ(JSON.stringify(buf3), '{"type":"Buffer","data":[1,1,1,1,1,1,1,1,1,1]}')
        arktest.assertEQ(JSON.stringify(new Uint8Array(ab)), '{"0":1,"1":1,"2":1,"3":1,"4":1,"5":1,"6":1,"7":1,"8":1,"9":1}')
    })
}

function testKeys() {
    const buf = buffer.alloc(3, 'a');
    const keys = Array.from(buf.keys());
    arktest.assertEQ(keys.length, 3);
    arktest.assertEQ(keys[0], 0);
}

function testSubarray() {
    let buf = buffer.alloc(5, 'a');
    let subBuf = buf.subarray(0, 3);
    arktest.assertEQ(subBuf.length, 3);
    arktest.assertEQ(subBuf.toString('ascii'), 'aaa');

    buf = buffer.from("1236")
    subBuf = buf.subarray(0, 3)
    arktest.assertEQ(subBuf.toString(), "123")

    subBuf = buf.subarray(undefined)
    arktest.assertEQ(subBuf.toString(), "1236")

    subBuf = buf.subarray(3, 4)
    arktest.assertEQ(subBuf.toString(), "6")

    subBuf = buf.subarray(-3, 0)
    arktest.assertEQ(subBuf.length, 0)

    subBuf = buf.subarray(6, 9)
    arktest.assertEQ(subBuf.length, 0)

    let buf1 = buffer.allocUninitializedFromPool(26)
    for (let i = 0; i < 26; ++i) {
        buf1.writeInt8(i + 97, i)
    }

    let buf2 = buf1.subarray(-1)
    const bufLength: int = buf2.length.toInt()
    let res = buf2.toString('ascii', 0, bufLength)
    arktest.assertEQ(res, '')

    buf2 = buf1.subarray(10, -1)
    res = buf2.toString('ascii', 0, bufLength)
    arktest.assertEQ(res, '')

    buf2 = buf1.subarray(2, 1)
    res = buf2.toString('ascii', 0, bufLength)
    arktest.assertEQ(res, '')

    buf2 = buf1.subarray(2, 2)
    res = buf2.toString('ascii', 0, bufLength)
    arktest.assertEQ(res, '')

    const arr1 = new Uint8Array([97, 98, 99, 100, 101, 102, 103])
    buf = buffer.from(arr1.buffer, 1);
    subBuf = buf.subarray(2, 5)
    arktest.assertEQ(subBuf.toString(), "def")
}

function testSwap16() {
    const uint8Array = new Uint8Array([0x1, 0x2, 0x3, 0x4]);
    const buf = buffer.from(uint8Array);
    buf.swap16();
    arktest.assertEQ(buf.at(0), 0x2);
    arktest.assertEQ(buf.at(1), 0x1);
    arktest.assertEQ(buf.at(2), 0x4);
    arktest.assertEQ(buf.at(3), 0x3);
}

function testSwap32() {
    const uint8Array = new Uint8Array([0x1, 0x2, 0x3, 0x4]);
    const buf = buffer.from(uint8Array);
    buf.swap32();
    arktest.assertEQ(buf.at(0), 0x4);
    arktest.assertEQ(buf.at(1), 0x3);
    arktest.assertEQ(buf.at(2), 0x2);
    arktest.assertEQ(buf.at(3), 0x1);
}

function testSwap64() {
    const uint8Array = new Uint8Array(
        [0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8]
    );
    const buf = buffer.from(uint8Array);
    buf.swap64();
    arktest.assertEQ(buf.at(0), 0x8);
    arktest.assertEQ(buf.at(1), 0x7);
    arktest.assertEQ(buf.at(2), 0x6);
    arktest.assertEQ(buf.at(3), 0x5);
    arktest.assertEQ(buf.at(4), 0x4);
    arktest.assertEQ(buf.at(5), 0x3);
    arktest.assertEQ(buf.at(6), 0x2);
    arktest.assertEQ(buf.at(7), 0x1);
}

function testValues() {
    const buf = buffer.alloc(3, 'a');
    const values = Array.from(buf.values());
    arktest.assertEQ(values.length, 3);
    arktest.assertEQ(values[0], 97);
}

function testInt8ReadWrite() {
    const buf = buffer.from(new Uint8Array(4))

    // Test positive values
    buf.writeInt8(127, 0)
    arktest.assertEQ(buf.readInt8(0), 127)

    // Test negative values
    buf.writeInt8(-128, 1)
    arktest.assertEQ(buf.readInt8(1), -128)

    // Test zero
    buf.writeInt8(0, 2)
    arktest.assertEQ(buf.readInt8(2), 0)
}

function testInt16ReadWrite() {
    const buf = buffer.from(new Uint8Array(8))

    // Test Big Endian
    buf.writeInt16BE(32767, 0)
    arktest.assertEQ(buf.readInt16BE(0), 32767)

    buf.writeInt16BE(-32768, 2)
    arktest.assertEQ(buf.readInt16BE(2), -32768)

    // Test Little Endian
    buf.writeInt16LE(32767, 4)
    arktest.assertEQ(buf.readInt16LE(4), 32767)

    buf.writeInt16LE(-32768, 6)
    arktest.assertEQ(buf.readInt16LE(6), -32768)
}

function testInt32ReadWrite() {
    const buf = buffer.from(new Uint8Array(8))

    // Test Big Endian
    buf.writeInt32BE(2147483647, 0)
    arktest.assertEQ(buf.readInt32BE(0), 2147483647)

    // Test Little Endian
    buf.writeInt32LE(-2147483648, 4)
    arktest.assertEQ(buf.readInt32LE(4), -2147483648)

    const arr1 = new Uint8Array([0, 1, 2, 3, 4, 5, 6, 7, 8])
    const buf1 = buffer.from(arr1.buffer, 1);

    // Test Big Endian
    buf1.writeInt32BE(2147483647, 0)
    arktest.assertEQ(buf.readInt32BE(0), 2147483647)

    // Test Little Endian
    buf1.writeInt32LE(-2147483648, 4)
    arktest.assertEQ(buf.readInt32LE(4), -2147483648)
}

function testUInt8ReadWrite() {
    const buf = buffer.from(new Uint8Array(3))

    // Test max value
    buf.writeUInt8(255, 0)
    arktest.assertEQ(buf.readUInt8(0), 255)

    // Test zero
    buf.writeUInt8(0, 1)
    arktest.assertEQ(buf.readUInt8(1), 0)

    // Test mid value
    buf.writeUInt8(128, 2)
    arktest.assertEQ(buf.readUInt8(2), 128)

    // Test -2 -> 254
    {
        let buf = buffer.from([1, 9, -2])
        arktest.assertEQ(buf.readUInt8(2), 254)
    }

    {
        let buf = buffer.from([-2, 12, 5])
        arktest.assertEQ(buf.readUInt8(0), 254)
    }

    {
        let buf = buffer.from([1, -2])
        arktest.assertEQ(buf.readUInt8(1), 254)
    }

    {
        let buf = buffer.from([-2])
        arktest.assertEQ(buf.readUInt8(0), 254)
    }
}

function testUInt16ReadWrite() {
    const buf = buffer.from(new Uint8Array(4))

    // Test Big Endian
    buf.writeUInt16BE(65535, 0)
    arktest.assertEQ(buf.readUInt16BE(0), 65535)

    // Test Little Endian
    buf.writeUInt16LE(65535, 2)
    arktest.assertEQ(buf.readUInt16LE(2), 65535)
}

function testUInt32ReadWrite() {
    const buf = buffer.from(new Uint8Array(8))

    // Test Big Endian
    buf.writeUInt32BE(4294967295, 0)
    arktest.assertEQ(buf.readUInt32BE(0), 4294967295)

    // Test Little Endian
    buf.writeUInt32LE(4294967295, 4)
    arktest.assertEQ(buf.readUInt32LE(4), 4294967295)
}

function testFloatReadWrite() {
    const buf = buffer.from(new Uint8Array(8))

    // Test Big Endian
    const testFloat: float = 3.14159f
    buf.writeFloatBE(testFloat, 0)
    // Using approx comparison due to float precision
    arktest.assertDoubleEQ(buf.readFloatBE(0), testFloat, 0.00001)

    // Test Little Endian
    buf.writeFloatLE(testFloat, 4)
    arktest.assertDoubleEQ(buf.readFloatLE(4), testFloat, 0.00001)
}

function testDoubleReadWrite() {
    const buf = buffer.from(new Uint8Array(16))

    // Test Big Endian
    const testDouble = Math.PI
    buf.writeDoubleBE(testDouble, 0)
    arktest.assertEQ(buf.readDoubleBE(0), testDouble)

    // Test Little Endian
    buf.writeDoubleLE(testDouble, 8)
    arktest.assertEQ(buf.readDoubleLE(8), testDouble)
}

function testBigInt64ReadWrite() {
    const buf = buffer.from(new Uint8Array(16))

    // Test Big Endian
    const max64 = new BigInt("9223372036854775807")
    const min64 = new BigInt("-9223372036854775808")

    buf.writeBigInt64BE(max64, 0)
    arktest.assertEQ(buf.readBigInt64BE(0).toString(), max64.toString())

    // Test Little Endian
    buf.writeBigInt64LE(min64, 8)
    arktest.assertEQ(buf.readBigInt64LE(8).toString(), min64.toString())
}

function testBigUInt64ReadWrite() {
    const buf = buffer.from(new Uint8Array(16))

    // Test Big Endian
    const maxU64 = new BigInt("18446744073709551615")

    buf.writeBigUInt64BE(maxU64, 0)
    arktest.assertEQ(buf.readBigUInt64BE(0).toString(), maxU64.toString())

    // Test Little Endian
    buf.writeBigUInt64LE(maxU64, 8)
    arktest.assertEQ(buf.readBigUInt64LE(8).toString(), maxU64.toString())
}

function testVarLengthIntReadWrite() {
    const buf = buffer.from(new Uint8Array(12))

    // Test 2-byte values
    buf.writeIntBE(0x1234, 0, 2)
    arktest.assertEQ(buf.readIntBE(0, 2), 0x1234)

    buf.writeIntLE(0x1234, 2, 2)
    arktest.assertEQ(buf.readIntLE(2, 2), 0x1234)

    // Test 3-byte values
    buf.writeIntBE(0x123456, 4, 3)
    arktest.assertEQ(buf.readIntBE(4, 3), 0x123456)

    buf.writeIntLE(0x123456, 7, 3)
    arktest.assertEQ(buf.readIntLE(7, 3), 0x123456)

    // Test negative values
    const negValue = -0x1234;
    buf.writeIntBE(negValue, 10, 2)
    arktest.assertEQ(buf.readIntBE(10, 2), negValue)
}

function testReadIntUintLEBE() {
    {
        let buf = buffer.from([0x12, 0x34, 0x56, 0x78, 0x90, 0xab])

        arktest.assertEQ(buf.readIntLE(0, 6).toString(16), "-546f87a9cbee")
        arktest.assertEQ(buf.readIntBE(0, 6).toString(16), "1234567890ab")

        arktest.assertEQ(buf.readUIntLE(0, 6).toString(16), "ab9078563412")
        arktest.assertEQ(buf.readUIntBE(0, 6).toString(16), "1234567890ab")
        arktest.assertEQ(buf.readIntBE(0, 6).toString(16), buf.readUIntBE(0, 6).toString(16))
    }

    {
        let buf = buffer.from([0x00, 0x00, 0x00, 0x00, 0x00, 0x01])

        arktest.assertEQ(buf.readIntBE(0, 6).toString(16), "1")
        arktest.assertEQ(buf.readIntLE(0, 6).toString(16), "10000000000")
        arktest.assertEQ(buf.readIntLE(0, 6).toString(16), buf.readUIntLE(0, 6).toString(16))
        arktest.assertEQ(buf.readIntBE(0, 6).toString(16), buf.readUIntBE(0, 6).toString(16))
    }

    {
        let buf = buffer.from([0x01, 0x00, 0x00, 0x00, 0x00, 0x00])

        arktest.assertEQ(buf.readIntLE(0, 6).toString(16), "1")
        arktest.assertEQ(buf.readIntBE(0, 6).toString(16), "10000000000")
        arktest.assertEQ(buf.readIntLE(0, 6).toString(16), buf.readUIntLE(0, 6).toString(16))
        arktest.assertEQ(buf.readIntBE(0, 6).toString(16), buf.readUIntBE(0, 6).toString(16))
    }
    {
        let buf = buffer.from([0xff, 0xff, 0xff, 0xff, 0xff, 0xff])
        arktest.assertEQ(buf.readIntLE(0, 6).toString(16), '-1')
        arktest.assertEQ(buf.readIntBE(0, 6).toString(16), '-1')
        arktest.assertEQ(buf.readUIntLE(0, 6), 0xffffffffffff)
        arktest.assertEQ(buf.readUIntBE(0, 6), 0xffffffffffff)
    }

    {
        // test overfill
        let buf = buffer.from([0x8f, 0xff, 0xff, 0xff, 0xff, 0xff])
        arktest.assertEQ(buf.readIntLE(0, 6).toString(16), "-71")
        arktest.assertEQ(buf.readIntBE(0, 6).toString(16), "-700000000001")
        arktest.assertEQ(buf.readUIntLE(0, 6).toString(16), "ffffffffff8f")
        arktest.assertEQ(buf.readUIntBE(0, 6).toString(16), "8fffffffffff")
    }
}

function testVarLengthUIntReadWrite() {
    const buf = buffer.from(new Uint8Array(12))

    // Test 2-byte values
    buf.writeUIntBE(0xFFFF, 0, 2)
    arktest.assertEQ(buf.readUIntBE(0, 2), 0xFFFF)

    buf.writeUIntLE(0xFFFF, 2, 2)
    arktest.assertEQ(buf.readUIntLE(2, 2), 0xFFFF)

    // Test 3-byte values
    buf.writeUIntBE(0xFEDCBA, 4, 3)
    arktest.assertEQ(buf.readUIntBE(4, 3), 0xFEDCBA)

    buf.writeUIntLE(0xFEDCBA, 7, 3)
    arktest.assertEQ(buf.readUIntLE(7, 3), 0xFEDCBA)
}

function testStringWrite() {
    const buf = buffer.from(new Uint8Array(20))

    // Test basic ASCII write
    const str = "Hello"
    const strLen: int = str.length.toInt()
    const bytesWritten = buf.write(str, 0, strLen, 'utf8')
    arktest.assertEQ(bytesWritten, strLen)
    arktest.assertEQ(buf.toString('utf8', 0, strLen), str)

    // Test write with offset
    const str2 = "World"
    const strLen2: int = str2.length.toInt()
    const bytesWritten2 = buf.write(str2, 10, strLen2, 'utf8')
    arktest.assertEQ(bytesWritten2, strLen2)
    arktest.assertEQ(buf.toString('utf8', 10, 10 + strLen2), str2)

    // Test partial write
    const longStr = "ThisIsALongString"
    const bytesWritten3 = buf.write(longStr, 0, 5, 'utf8')
    arktest.assertEQ(bytesWritten3, 5)
    arktest.assertEQ(buf.toString('utf8', 0, 5), "ThisI")
}

function testBufferWrite0022() {
    const TAG = 'testBufferWrite0022';
    let buf = buffer.alloc(5);
    arktest.expectError(() => {
        const res = buf.write("abc", 0, 3, 'invalid_encoding');
    }, 'Parameter error. The type of "encoding" must be BufferEncoding. the encoding invalid_encoding is unknown');
}

function testBufferWriteStringBase64() {
    let buf = buffer.alloc(10)
    let res = buf.write("abcdefg", 3, 1, 'base64')
    arktest.assertEQ(res, 1)
    arktest.assertEQ(buf.toString('base64'), "AAAAaQAAAAAAAA==")
}

function testBufferWriteStringSingleParam() {
    const str = "abcdefg"
    let buf = buffer.alloc(10)
    let res = buf.write(str)
    let strLen: int = str.length.toInt()
    arktest.assertEQ(res, strLen)
    arktest.assertEQ(buf.toString('utf8', 0, strLen), str)
}

function testBufferWriteStringInfinity() {
    const str = "abcdefg"
    let buf = buffer.alloc(10)
    arktest.expectError(() => {
        buf.write(str, 0, Infinity.toInt());
    }, "The value of \"length\" is out of range. It must be >= 0 and <= 10. Received value is: 2147483647");
}

function testBufferWriteStringWrongOffset() {
    const str = "abc"
    let buf = buffer.alloc(10)
    arktest.expectError(() => {
		buf.write(str, -12)
    }, "The value of \"offset\" is out of range. It must be >= 0 and <= 9. Received value is: -12");
}

function testBufferWriteStringOutRangeOffset() {
    let buf = buffer.alloc(10);
    arktest.expectError(() => {
        buf.write("abc", (-Infinity).toInt(), (-Infinity).toInt(), 'utf8');
    }, "The value of \"offset\" is out of range. It must be >= 0 and <= 9. Received value is: -2147483648");
}

function testBufferWriteStringOutRangeLength() {
    let buf = buffer.alloc(10);
    arktest.expectError(() => {
        buf.write("abcdefg", 3, -1);
    }, "The value of \"length\" is out of range. It must be >= 0 and <= 10. Received value is: -1");
}

function testBufferFrom40002() {
    let buf = buffer.from('abc', 'base64');
    arktest.assertEQ(buf.toString(), 'i·')
    arktest.assertEQ(buf.length, 2);
    arktest.assertEQ(buf.toString('base64'), 'abc=');
}

function testBufferFrom30006() {
    let buf = buffer.from(new String('this is a test'), 'utf8', -1);
    arktest.assertEQ(buf.length, 14);
    arktest.assertEQ(buf.toString(), 'this is a test');
}

function testBufferBase64Url() {
    const src = new Uint8Array([250, 251, 252, 253, 254, 255])
    const encodeStr = buffer.from(src).toString("base64url")
    arktest.assertEQ(encodeStr, "-vv8_f7_")
    const bytesLength = buffer.byteLength(encodeStr, "base64url").toInt()
    arktest.assertEQ(bytesLength, 6)
}

function testByteOffset() {
    let buf = buffer.from("1236");
    let offset = buf.byteOffset;
    arktest.assertEQ(offset, 0)
    arktest.expectNoThrow(() => {
        arktest.assertEQ(JSON.stringify(buf.byteOffset), "0")
    })

    const a = new ArrayBuffer(5)
    a.set(0, 0 as byte)
    a.set(1, 11 as byte)
    a.set(2, 22 as byte)
    a.set(3, 33 as byte)
    buf = new buffer.Buffer(a, 2)
    arktest.assertEQ(buf.byteOffset, 2)
    let byteOffset: int;
    arktest.expectError((): void => {
                                        byteOffset = -1;
                                        let f = new buffer.Buffer(a, byteOffset) });
    arktest.expectError((): void => {
                                        byteOffset = a.getByteLength();
                                        let f = new buffer.Buffer(a, byteOffset)
                        });
    arktest.expectError((): void => {
                                        byteOffset = a.getByteLength() + 1;
                                        let f = new buffer.Buffer(a, byteOffset)
                        });
}

function testFillHex() {
    const buf1 = buffer.alloc(10).fill('F1刘FG', 0, 10, 'hex');
    let str1 = buf1.toString('hex');
    arktest.assertEQ(str1, "f1f1f1f1f1f1f1f1f1f1");

    const buf2 = buffer.alloc(10).fill('1A3B5C7Z9D', 0, 10, 'hex');
    let str2 = buf2.toString('hex');
    arktest.assertEQ(str2, "1a3b5c1a3b5c1a3b5c1a");

    const buf3 = buffer.alloc(10).fill('1A3BFF7', 0, 10, 'hex');
    let str3 = buf3.toString('hex');
    arktest.assertEQ(str3, "1a3bff1a3bff1a3bff1a");
}

function testIncludesHex() {
    let encodeArr: buffer.BufferEncoding[] = ['utf8', 'utf-8', 'ucs2', 'ucs-2', 'ascii', 'latin1', 'binary', 'utf16le', 'base64', 'base64url', 'hex'];
    for (const encode of encodeArr) {
        let buf = buffer.from("13236");
        let flag = buf.includes("abc", 0, encode);
        arktest.assertEQ(flag, false);
    }
}

function testFromHex() {
    const buf1 = buffer.from('F1刘FG', 'hex');
    let str = buf1.toString('hex');
    arktest.assertEQ(str, "f1");
}

function testFillAsciiWithUnicode() {
    const buf = buffer.alloc(10).fill('F1刘FG', 0, 10, 'ascii');
    const str = buf.toString('hex');
    arktest.assertEQ(str, '46311846474631184647');
}

function testFillUtf8WithUnicode() {
    const buf = buffer.alloc(10).fill('F1刘FG', 0, 10, 'utf8');
    const str = buf.toString('hex');
    arktest.assertEQ(str, '4631e5889846474631e5');
}

function testFillBase64WithUnicode() {
    const buf = buffer.alloc(10).fill('F1刘FG', 0, 10, 'base64');
    const str = buf.toString('hex');
    arktest.assertEQ(str, '17514617514617514617');
}

function testFillBase64UrlWithUnicode() {
    const buf = buffer.alloc(10).fill('F1刘FG', 0, 10, 'base64url');
    const str = buf.toString('hex');
    arktest.assertEQ(str, '17514617514617514617');
}

function testBufferToJSON() {
    // Empty Buffer
    let buf = buffer.from(new ArrayBuffer(0));
    let str = JSON.stringifyJsonElement(buf.toJSON());
    let parsed = JSON.parseJsonElement(str);
    arktest.assertEQ(str, '{"type":"Buffer","data":[]}');
    arktest.assertEQ(parsed.getElement("type").jsonType, jsonx.JsonType.JsonString);
    arktest.assertEQ(parsed.getElement("data").jsonType, jsonx.JsonType.JsonArray);
    arktest.assertEQ(parsed.getElement("data").asArray().length, 0);

    // Non empty Buffer
    buf = buffer.from(new ArrayBuffer(10));
    buf[0] = 1;
    buf[1] = 2;
    buf[2] = 3;
    buf[3] = 4;
    buf[4] = 5;
    str = JSON.stringifyJsonElement(buf.toJSON());
    parsed = JSON.parseJsonElement(str);

    arktest.assertEQ(str, '{"type":"Buffer","data":[1,2,3,4,5,0,0,0,0,0]}');
    arktest.assertEQ(parsed.getElement("type").jsonType, jsonx.JsonType.JsonString);
    arktest.assertEQ(parsed.getElement("data").jsonType, jsonx.JsonType.JsonArray);
    const array = parsed.getElement("data").asArray()
    arktest.assertEQ(array.length, 10);
    for (let i: int = 0; i < array.length; ++i) {
        arktest.assertEQ(array[i].jsonType, jsonx.JsonType.JsonNumber);
    }

    {
        let ab = new ArrayBuffer(10)
        let buf = buffer.from(ab as Object, 0, 2)
        arktest.assertEQ(JSON.stringify(buf), '{"type":"Buffer","data":[0,0]}')
    }
}

function testFillUcs2WithUnicode() {
    const buf = buffer.alloc(10).fill('F1刘FG', 0, 10, 'ucs2');
    const str = buf.toString('hex');
    arktest.assertEQ(str, '46003100185246004700');
    const buf1 = buffer.alloc(10).fill('F1刘FG', 0, 10, 'ucs-2');
    const str1 = buf1.toString('hex');
    arktest.assertEQ(str1, '46003100185246004700');
}

function testFillUtf16leWithUnicode() {
    const buf = buffer.alloc(10).fill('F1刘FG', 0, 10, 'utf16le');
    const hex = buf.toString('hex');
    arktest.assertEQ(hex, '46003100185246004700');
}

function testFillEmpty(){
    let encodeArr: (undefined | buffer.BufferEncoding)[] = [undefined, 'utf8', 'utf-8', 'ucs2', 'ucs-2', 'ascii', 'latin1', 'binary', 'utf16le', 'base64', 'base64url', 'hex'];
    const buf = buffer.alloc(20).fill('');
    for (const encode of encodeArr) {
        buf.fill('', 0, buf.length, encode);
        arktest.assertEQ(buf.length, 20, encode);
    }
}

function testBufferToString00001() {
    let buf1 = buffer.from("abc");
    let str = buf1.toString("ucs2");
    arktest.assertEQ(str, '扡')

    buf1 = buffer.from("abc");
    str = buf1.toString("utf16le");
    arktest.assertEQ(str, '扡')

    let buf = buffer.from('hello world');
    buf1 = buffer.transcode(buf, 'base64', 'utf-8');
    arktest.assertEQ(buf1.toString(), 'aGVsbG8gd29ybGQ=');
    arktest.assertEQ(buf1.length, 16)


    buf1 = buffer.from("1236");
    buf = buffer.transcode(buf1, "ascii", "ucs2");
    str = buf.toString("ucs2")
    arktest.assertEQ(str, "1236");

    buf1 = buffer.from("测试");
    buf = buffer.transcode(buf1, "utf8", "ucs2");
    str = buf.toString("ucs2")
    arktest.assertEQ(str, "测试");

    buf = buffer.from('hello');
    str = buf.toString('base64', -Infinity.toInt(), 0);
    arktest.assertEQ(str.length, 0);
    arktest.assertEQ(str, '');
}

function testUIntLEWrite() {
    let buf = buffer.allocUninitializedFromPool(4);
    arktest.expectError(() => {
        let ref = buf.writeUIntLE(0x13141516, 0, 1);
    }, `The value of "value" is out of range. It must be >= 0 and <= 255. Received value is: 320083222`)
}

function testUIntBEWrite() {
    let buf = buffer.allocUninitializedFromPool(4);
    arktest.expectError(() => {
        let ref = buf.writeUIntBE(0x13141516, 0, 1);
    }, `The value of "value" is out of range. It must be >= 0 and <= 255. Received value is: 320083222`)
}

function testUInt32BEWrite() {
    let buf = buffer.allocUninitializedFromPool(4);
    arktest.expectError(() => {
        buf.writeUInt32BE(0xfeedface, -1);
    }, `The value of "offset" is out of range. It must be >= 0 and <= 0. Received value is: -1`)
}

function testUInt32LEWrite() {
    let buf = buffer.allocUninitializedFromPool(4);
    arktest.expectError(() => {
        buf.writeUInt32LE(0xfeedface, -1);
    }, `The value of "offset" is out of range. It must be >= 0 and <= 0. Received value is: -1`)
}
