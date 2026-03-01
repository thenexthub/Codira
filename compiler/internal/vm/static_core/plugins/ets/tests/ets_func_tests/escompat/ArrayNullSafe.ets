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

class ElementTest {
    ele: int
    constructor(e: int) {
        this.ele = e
    }
}

function testConstructor(): void {
    const len = 5
    let arr = new Array<ElementTest>(len, (index) => { return new ElementTest(index) })
    for (let idx = 0; idx < len; idx++) {
        arktest.assertEQ(arr[idx].ele, idx)
    }
}

function testArrayWithUndefined(): void {
    const len = 5
    let arr: Array<ElementTest | undefined> = new Array<ElementTest | undefined>(len, (index) => { return undefined })
    for (let idx = 0; idx < len; idx++) {
        arktest.assertEQ(arr[idx], undefined)
    }
    arr[2] = new ElementTest(2)
    arktest.assertNE(arr[2], undefined)
    arktest.assertEQ(arr[2]?.ele, 2)
}

function main(): int {
    const suite = new arktest.ArkTestsuite("Array Null Safe Check")

    suite.addTest("Array test with constructor", testConstructor)
    suite.addTest("Array test with ArrayWithUndefined", testArrayWithUndefined)

    return suite.run()
}