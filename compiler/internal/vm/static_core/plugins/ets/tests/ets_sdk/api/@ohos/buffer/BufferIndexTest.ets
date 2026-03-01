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
import { BusinessError } from "@ohos.base";

function main(): int {
    const suite = new arktest.ArkTestsuite("Buffer indexer methods")

    suite.addTest("Test get via indexer", testIndexerGet)
    suite.addTest("Test set via indexer", testIndexerSet)
    suite.addTest("Test combined get and set via indexer", testIndexerGetSet)
    suite.addTest("Test indexer out of bounds", testIndexerOutOfBounds)

    return suite.run()
}

function testIndexerGet() {
    const buf = buffer.from([5, 10, 15, 20])
    arktest.assertEQ(buf[0], 5)
    arktest.assertEQ(buf[2], 15)
}

function testIndexerSet() {
    const buf = buffer.alloc(4)
    buf[1] = 100
    buf[3] = 255
    arktest.assertEQ(buf[1], 100)
    arktest.assertEQ(buf[3], 255)
}

function testIndexerGetSet() {
    const buf = buffer.from("abcd", "utf8")
    buf[0] = 0x78  // 'x'
    buf[3] = 0x79  // 'y'
    arktest.assertEQ(buf[0], 0x78)
    arktest.assertEQ(buf[3], 0x79)
    arktest.assertEQ(buf.toString(), "xbcy")
}

function testIndexerOutOfBounds() {
    const buf = buffer.from([1, 2, 3])
    checkOutsideRangeError(buf, -1);
    checkOutsideRangeError(buf, 5);
    buf[10] = 99
    arktest.assertEQ(buf.length, 3)
    arktest.assertEQ(buf.toString(), "\u0001\u0002\u0003")
}

function checkOutsideRangeError(buf: buffer.Buffer, index: int) {
    let errMsg = `The value of "length" is out of range. \
It must be >= 0 and <= ${buf.length}. Received value is: ${index}`;
    const OutOfBoundsErrorCodeId: int = 10200001;
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.message == errMsg && e.code == OutOfBoundsErrorCodeId;
        }
        return false;
    }
    arktest.expectThrow((): void => { buf[index] }, exceptionCheck)
}