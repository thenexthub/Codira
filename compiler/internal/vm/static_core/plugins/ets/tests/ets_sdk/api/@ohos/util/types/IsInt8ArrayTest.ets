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
    const suite = new arktest.ArkTestsuite("CommonUtil types.isInt8Array() API tests")

    suite.addTest("Verify Int8Array is identified as Int8Array", testIsInt8Array001)
    suite.addTest("Verify ArrayBuffer is not identified as Int8Array", testIsInt8Array002)
    suite.addTest("Verify Int32Array is not identified as Int8Array", testIsInt8Array003)
    suite.addTest("Verify Float64Array is not identified as Int8Array", testIsInt8Array004)
    suite.addTest("Verify Uint8Array is not identified as Int8Array", testIsInt8Array005)

    return suite.run()
}

/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0079
 * @tc.name: testIsInt8Array001
 * @tc.desc: Verify Int8Array is identified as Int8Array
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsInt8Array001() {
  let proc = new util.types();
  let result = proc.isInt8Array(new Int8Array());
  arktest.assertTrue(result, "The Int8Array should be of the same type as the Int8Array");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0080
 * @tc.name: testIsInt8Array002
 * @tc.desc: Verify ArrayBuffer is not identified as Int8Array
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsInt8Array002() {
  let proc = new util.types();
  let result = proc.isInt8Array(new ArrayBuffer(0));
  arktest.assertTrue(result == false, "The ArrayBuffer should not be of the same type as the Int8Array");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0081
 * @tc.name: testIsInt8Array003
 * @tc.desc: Verify Int32Array is not identified as Int8Array
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsInt8Array003() {
  let proc = new util.types();
  let result = proc.isInt8Array(new Int32Array());
  arktest.assertTrue(result == false, "The Int32Array should not be of the same type as the Int8Array");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0082
 * @tc.name: testIsInt8Array004
 * @tc.desc: Verify Float64Array is not identified as Int8Array
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsInt8Array004() {
  let proc = new util.types();
  let result = proc.isInt8Array(new Float64Array());
  arktest.assertTrue(result == false, "The Float64Array should not be of the same type as the Int8Array");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0083
 * @tc.name: testIsInt8Array005
 * @tc.desc: Verify Uint8Array is not identified as Int8Array
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsInt8Array005() {
  let proc = new util.types();
  let result = proc.isInt8Array(new Uint8Array());
  arktest.assertTrue(result == false, "The Uint8Array should not be of the same type as the Int8Array");
}
