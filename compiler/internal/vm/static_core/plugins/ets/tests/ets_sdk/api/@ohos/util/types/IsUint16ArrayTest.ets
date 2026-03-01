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
    const suite = new arktest.ArkTestsuite("CommonUtil types.isUint16Array() API tests")
    
    suite.addTest("Verify Uint16Array is identified as Uint16Array", testIsUint16Array001)
    suite.addTest("Verify Uint8Array is not identified as Uint16Array", testIsUint16Array002)
    suite.addTest("Verify Uint32Array is not identified as Uint16Array", testIsUint16Array003)
    suite.addTest("Verify Uint8ClampedArray is not identified as Uint16Array", testIsUint16Array004)
    suite.addTest("Verify Int8Array is not identified as Uint16Array", testIsUint16Array005)

    return suite.run()
}

/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_IS_UINT16_ARRAY_001
 * @tc.name: testIsUint16Array001
 * @tc.desc: Verify Uint16Array is identified as Uint16Array
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsUint16Array001() {
    let proc = new util.types();
    let result = proc.isUint16Array(new Uint16Array());
    arktest.assertTrue(result, "The Uint16Array should be of the same type as the Uint16Array");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_IS_UINT16_ARRAY_002
 * @tc.name: testIsUint16Array002
 * @tc.desc: Verify Uint8Array is not identified as Uint16Array
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsUint16Array002() {
    let proc = new util.types();
    let result = proc.isUint16Array(new Uint8Array());
    arktest.assertTrue(result == false, "The Uint8Array should be of the same type as the Uint8Array");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_IS_UINT16_ARRAY_003
 * @tc.name: testIsUint16Array003
 * @tc.desc: Verify Uint32Array is not identified as Uint16Array
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsUint16Array003() {
    let proc = new util.types();
    let result = proc.isUint16Array(new Uint32Array());
    arktest.assertTrue(result == false, "The Uint32Array should be of the same type as the Uint32Array");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_IS_UINT16_ARRAY_004
 * @tc.name: testIsUint16Array004
 * @tc.desc: Verify Uint8ClampedArray is not identified as Uint16Array
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsUint16Array004() {
    let proc = new util.types();
    let result = proc.isUint16Array(new Uint8ClampedArray());
    arktest.assertTrue(result == false, "The Uint8ClampedArray should be of the same type as the Uint8ClampedArray");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_IS_UINT16_ARRAY_005
 * @tc.name: testIsUint16Array005
 * @tc.desc: Verify Int8Array is not identified as Uint16Array
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsUint16Array005() {
    let proc = new util.types();
    let result = proc.isUint16Array(new Int8Array());
    arktest.assertTrue(result == false, "The Int8Array should be of the same type as the Int8Array");
}
