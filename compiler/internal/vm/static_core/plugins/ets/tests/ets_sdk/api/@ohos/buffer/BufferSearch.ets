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

import buffer from "@ohos.buffer";

function main(): int {
    const suite = new arktest.ArkTestsuite("Buffer search methods")
    
    suite.addTest("Test includes with numbers", testIncludesNumbers)
    suite.addTest("Test includes with strings", testIncludesStrings)
    suite.addTest("Test includes with buffers", testIncludesBuffers)
    suite.addTest("Test indexOf with numbers", testIndexOfNumbers)
    suite.addTest("Test indexOf with strings", testIndexOfStrings)
    suite.addTest("Test lastIndexOf with numbers", testLastIndexOfNumbers)
    suite.addTest("Test lastIndexOf with strings", testLastIndexOfStrings)
    suite.addTest("Test indexOf and lastIndexOf with buffers", testIndexOfLastIndexOfBuffers)
    suite.addTest("Test indexOf and lastIndexOf with OutOfBounds", testIndexOfOutOfBounds)
    suite.addTest("Test indexOf and lastIndexOf with OutOfBounds", testLastIndexOfOutOfBounds)
    suite.addTest("Test indexOf and lastIndexOf with buffers", testIndexOfPartialMatch)
    suite.addTest("Test indexOf and lastIndexOf with buffers", testLastIndexOfPartialMatch)
    suite.addTest("Test lastIndexOf with base64", testLastIndexOfBase64)
    suite.addTest('Test Buffer from', testBufferFrom)
    suite.addTest('Test Buffer from error', testBufferFromError)
    
    return suite.run()
}

function testIncludesNumbers() {
    const buf = buffer.from([10, 20, 30, 40, 50])
    arktest.assertEQ(buf.includes(30 as double), true)
    arktest.assertEQ(buf.includes(60 as double), false)
}

function testIncludesStrings() {
    const buf = buffer.from("hello world", "utf8")
    arktest.assertEQ(buf.includes("world"), true)
    arktest.assertEQ(buf.includes("test"), false)
}

function testIncludesBuffers() {
    const buf = buffer.from([5, 10, 15, 20, 25])
    const subBuf = buffer.from([10, 15])
    arktest.assertEQ(buf.includes(subBuf), true)
    arktest.assertEQ(buf.includes(buffer.from([30])), false)
}

function testIndexOfNumbers() {
    const buf = buffer.from([1, 2, 3, 4, 5, 3, 7])
    arktest.assertEQ(buf.indexOf(3 as double), 2)
    arktest.assertEQ(buf.indexOf(3 as double, 3), 5)
    arktest.assertEQ(buf.indexOf(8 as double), -1)
}

function testIndexOfStrings() {
    const buf = buffer.from("hello world", "utf8")
    arktest.assertEQ(buf.indexOf("world"), 6)
    arktest.assertEQ(buf.indexOf("hello"), 0)
    arktest.assertEQ(buf.indexOf("test"), -1)
}

function testLastIndexOfNumbers() {
    const buf = buffer.from([1, 2, 3, 4, 3, 5])
    arktest.assertEQ(buf.lastIndexOf(3 as double), 4)
    arktest.assertEQ(buf.lastIndexOf(3 as double, 3), 2)
    arktest.assertEQ(buf.lastIndexOf(6 as double), -1)
}

function testLastIndexOfStrings() {
    const buf = buffer.from("banana", "utf8");
    arktest.assertEQ(buf.lastIndexOf("a"), 5)
    arktest.assertEQ(buf.lastIndexOf("an"), 3)
    arktest.assertEQ(buf.lastIndexOf("z"), -1)
}

function testIndexOfLastIndexOfBuffers() {
    const buf = buffer.from([10, 20, 30, 40, 20, 50])
    const searchBuf = buffer.from([20])
    arktest.assertEQ(buf.indexOf(searchBuf), 1)
    arktest.assertEQ(buf.lastIndexOf(searchBuf), 4)
}

function testIndexOfOutOfBounds() {
    const buf = buffer.from([10, 20, 30, 40, 50])
    arktest.assertEQ(buf.indexOf(30 as double, 10), -1)
    arktest.assertEQ(buf.indexOf(30 as double, -100), 2)
}

function testLastIndexOfOutOfBounds() {
    const buf = buffer.from([10, 20, 30, 40, 50])
    arktest.assertEQ(buf.lastIndexOf(30 as double, 100), 2)
    arktest.assertEQ(buf.lastIndexOf(30 as double, -100), -1)
}

function testIndexOfPartialMatch() {
    const buf = buffer.from("banana", "utf8")
    arktest.assertEQ(buf.indexOf("nan"), 2)
    arktest.assertEQ(buf.indexOf("na"), 2)
    arktest.assertEQ(buf.indexOf("anaa"), -1)
}

function testLastIndexOfPartialMatch() {
    const buf = buffer.from("banana", "utf8")
    arktest.assertEQ(buf.lastIndexOf("nan"), 2)
    arktest.assertEQ(buf.lastIndexOf("na"), 4)
    arktest.assertEQ(buf.lastIndexOf("anaa"), -1)
}

function testLastIndexOfBase64() {
    let buf = buffer.from("Hello World")
    let value = "World"
    const index = buf.lastIndexOf(value, 1, "base64")
    arktest.assertEQ(index, -1)
}

function testBufferFrom() {
    let sharedArr = new ArrayBuffer(20)
    let uint8Arr = new Uint8Array(sharedArr)
    uint8Arr[16] = 9
    let bufObj = buffer.from(sharedArr, 0, 20)
    uint8Arr[15] = 10
    arktest.assertEQ(bufObj.length, 20)
    let compareArr: Array<int> = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 9, 0, 0, 0]
    for(let i = 0; i < 20; i++) {
        arktest.assertEQ(bufObj[i], compareArr[i])
    }
    let bufObj2 = buffer.from(sharedArr, 0, 0)
    arktest.assertEQ(bufObj2.length, 0)
    let bufObj3 = buffer.from(sharedArr, 10, undefined)
    arktest.assertEQ(bufObj3.length, 10)
    let compareArr2: Array<int> = [0, 0, 0, 0, 0, 10, 9, 0, 0, 0]
    for(let i = 0; i < 10; i++) {
        arktest.assertEQ(bufObj3[i], compareArr2[i])
    }
}

function testBufferFromError() {
    let sharedArr = new ArrayBuffer(20)
    let uint8Arr = new Uint8Array(sharedArr)
    uint8Arr[16] = 9
    arktest.expectError(() => {
        let bufObj = buffer.from(sharedArr, 0, 22)
    }, 'The value of \"length\" is out of range. It must be >= 0 and <= 20. Received value is: 22');
    arktest.expectError(() => {
        let bufObj = buffer.from(sharedArr, 0, -10)
    }, 'The value of \"length\" is out of range. It must be >= 0 and <= 20. Received value is: -10');
    arktest.expectError(() => {
        let bufObj = buffer.from(sharedArr, 10, 11)
    }, 'The value of \"length\" is out of range. It must be >= 0 and <= 10. Received value is: 11');
}