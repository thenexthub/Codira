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

import {util} from "@ohos.util"
import { BusinessError } from "@ohos.base";

const success = 0
const fail = 1

const expectedRangeErrors = (e: Error): boolean => {
    return (e instanceof BusinessError);
}

function main(): int {
    const suite = new arktest.ArkTestsuite("LRUCache API Error tests")

    suite.addTest("LRUCache Constructor Zero", testLRUCacheConstructor_Zero);
    suite.addTest("LRUCache Constructor Negative", testLRUCacheConstructor_Negative);
    suite.addTest("LRUCache update&getCapacity Zero", testLRUCacheUpdateGetCapacity_Zero);
    suite.addTest("LRUCache update&getCapacity Negative", testLRUCacheUpdateGetCapacity_Negative);

    return suite.run()
}

function testLRUCacheUpdateGetCapacity_Zero() {
    let that = new util.LRUCache<number, number>();
    try {
        that.updateCapacity(0);
    } catch (err) {
        arktest.assertEQ(err.message, "Parameter error. The type of 0 must be small integer")
        return
    }
    arktest.assertTrue(false, "LRUCache should throw an error when capacity is not int");
}

function testLRUCacheUpdateGetCapacity_Negative() {
    let that = new util.LRUCache<number, number>();
    try {
        that.updateCapacity(-1);
    } catch (err) {
        arktest.assertEQ(err.message, "Parameter error. The type of -1 must be small integer")
        return
    }
    arktest.assertTrue(false, "LRUCache should throw an error when capacity is negative");
}

function testLRUCacheConstructor_Zero() {
    try {
        let that = new util.LRUCache<number, number>(0);
    } catch (err) {
        arktest.assertEQ(err.message, "Parameter error. The type of 0 must be small integer")
        return
    }
}

function testLRUCacheConstructor_Negative() {
    try {
        let that = new util.LRUCache<number, number>(-1);
    } catch (err) {
        arktest.assertEQ(err.message, "Parameter error. The type of -1 must be small integer")
        return
    }
    arktest.assertTrue(false, "LRUCache should throw an error when capacity is negative");
}

function testInvalidInputKeyAtPut() {
    let that = new util.LRUCache<number | undefined | null, number>();
    arktest.expectThrow(() => { that.put(undefined, 1) }, expectedRangeErrors)
    arktest.expectThrow(() => { that.put(null, 1) }, expectedRangeErrors)
}

function testInvalidInputValueAtPut() {
    let that = new util.LRUCache<number, number | undefined | null>();
    arktest.expectThrow(() => { that.put(1, undefined) }, expectedRangeErrors)
    arktest.expectThrow(() => { that.put(1, null) }, expectedRangeErrors)
}

function testInvalidInputKeyAtGet() {
    let that = new util.LRUCache<number | undefined | null, number>();
    that.put(1, 1);
    that.put(2, 2);
    arktest.expectThrow(() => { that.get(undefined) }, expectedRangeErrors)
    arktest.expectThrow(() => { that.get(null) }, expectedRangeErrors)
}

function testInvalidInputKeyAtContains() {
    let that = new util.LRUCache<number | undefined | null, number>();
    that.put(1, 1);
    that.put(2, 2);
    arktest.expectThrow(() => { that.contains(undefined) }, expectedRangeErrors)
    arktest.expectThrow(() => { that.contains(null) }, expectedRangeErrors)
}

function testInvalidInputKeyAtRemove() {
    let that = new util.LRUCache<number | undefined | null, number>();
    that.put(1, 1);
    that.put(2, 2);
    arktest.expectThrow(() => { that.remove(undefined) }, expectedRangeErrors)
    arktest.expectThrow(() => { that.remove(null) }, expectedRangeErrors)
}
