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
 
/*---
desc: Verifies SimplifyStringBuilder optimization pass (#19643).
      Should be run in aot mode.
tags: []
---*/

function main() {
    testNoOptimizationExpected()
    testNoOptimizationExpectedWithIntermediate()
    testOptimizationExpected()
    testOptimizationExpectedWithIntermediate()
}

function testNoOptimizationExpected() {
    // SimplifyStringBuilder should not be triggered here: 'prepend' instead of 'append'
    let array: Number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
    let result: string = ""
    for (let index:int = 0; index < array.length; ++index) {
        result = array[index].toString() + result
    }
    arktest.assertEQ(result, "9876543210")
}

function testNoOptimizationExpectedWithIntermediate() {
    // SimplifyStringBuilder should not be triggered here: 'prepend' instead of 'append' in intermediate value inside loop body
    let array: Number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
    let result: string = ""
    for (let index:int = 0; index < array.length; ++index) {
        result = result + "."
        let temp: string = result
        temp = array[index].toString() + temp
        result = temp
    }
    arktest.assertEQ(result, "9876543210..........")
}

function testOptimizationExpected() {
    // SimplifyStringBuilder should be triggered here: simple 'append' case
    let array: Number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
    let result: string = ""
    for (let index:int = 0; index < array.length; ++index) {
        result += array[index].toString()
    }
    arktest.assertEQ(result, "0123456789")
}

function testOptimizationExpectedWithIntermediate() {
    // SimplifyStringBuilder should be triggered here: 'append' with intermediate value inside loop body
    let array: Number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
    let result: string = ""
    for (let index:int = 0; index < array.length; ++index) {
        result = result + array[index].toString()
        let temp: string = result
        temp += "."
        result = temp
    }
    arktest.assertEQ(result, "0.1.2.3.4.5.6.7.8.9.")
}
