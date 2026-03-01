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

import {util} from "@ohos.util";

function main(): int {
    const suite = new arktest.ArkTestsuite("CommonUtil types.isTypedArray() API tests")

    suite.addTest("Verify Float64Array is a TypedArray", testIsTypedArray001)
    suite.addTest("Verify Uint8Array is a TypedArray", testIsTypedArray002)
    suite.addTest("Verify Uint16Array is a TypedArray", testIsTypedArray003)
    suite.addTest("Verify Uint32Array is a TypedArray", testIsTypedArray004)
    suite.addTest("Verify ArrayBuffer is not a TypedArray", testIsTypedArray005)

    return suite.run()
}

/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_IS_TYPED_ARRAY_001
 * @tc.name: testIsTypedArray001
 * @tc.desc: Verify Float64Array is a TypedArray
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsTypedArray001() {
    let proc = new util.types();
    let result = proc.isTypedArray(new Float64Array());
    arktest.assertTrue(result, "The Float64Array should be of the same type as the Float64Array");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_IS_TYPED_ARRAY_002
 * @tc.name: testIsTypedArray002
 * @tc.desc: Verify Uint8Array is a TypedArray
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsTypedArray002() {
    let proc = new util.types();
    let result = proc.isTypedArray(new Uint8Array());
    arktest.assertTrue(result, "The Uint8Array should be of the same type as the Uint8Array");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_IS_TYPED_ARRAY_003
 * @tc.name: testIsTypedArray003
 * @tc.desc: Verify Uint16Array is a TypedArray
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsTypedArray003() {
    let proc = new util.types();
    let result = proc.isTypedArray(new Uint16Array());
    arktest.assertTrue(result, "The Uint16Array should be of the same type as the Uint16Array");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_IS_TYPED_ARRAY_004
 * @tc.name: testIsTypedArray004
 * @tc.desc: Verify Uint32Array is a TypedArray
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsTypedArray004() {
    let proc = new util.types();
    let result = proc.isTypedArray(new Uint32Array());
    arktest.assertTrue(result, "The Uint32Array should be of the same type as the Uint32Array");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_IS_TYPED_ARRAY_005
 * @tc.name: testIsTypedArray005
 * @tc.desc: Verify ArrayBuffer is not a TypedArray
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsTypedArray005() {
    let proc = new util.types();
    let result = proc.isTypedArray(new ArrayBuffer(96));
    arktest.assertTrue(result == false, "The ArrayBuffer should be of the same type as the ArrayBuffer");
}
