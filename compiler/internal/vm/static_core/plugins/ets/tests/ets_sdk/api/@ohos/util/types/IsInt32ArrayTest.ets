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
    const suite = new arktest.ArkTestsuite("CommonUtil types.isInt32Array() API tests")

    suite.addTest("Verify Int32Array is identified as Int32Array", testIsInt32Array001)
    suite.addTest("Verify Int8Array is not identified as Int32Array", testIsInt32Array002)
    suite.addTest("Verify Int16Array is not identified as Int32Array", testIsInt32Array003)
    suite.addTest("Verify Float64Array is not identified as Int32Array", testIsInt32Array004)
    suite.addTest("Verify Uint8Array is not identified as Int32Array", testIsInt32Array005)

    return suite.run()
}

/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0089
 * @tc.name: testIsInt32Array001
 * @tc.desc: Verify Int32Array is identified as Int32Array
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsInt32Array001() {
  let proc = new util.types();
  let result = proc.isInt32Array(new Int32Array());
  arktest.assertTrue(result, "The Int32Array should be of the same type as the Int32Array");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0090
 * @tc.name: testIsInt32Array002
 * @tc.desc: Verify Int8Array is not identified as Int32Array
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsInt32Array002() {
  let proc = new util.types();
  let result = proc.isInt32Array(new Int8Array());
  arktest.assertTrue(result == false, "The Int8Array should not be of the same type as the Int32Array");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0091
 * @tc.name: testIsInt32Array003
 * @tc.desc: Verify Int16Array is not identified as Int32Array
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsInt32Array003() {
  let proc = new util.types();
  let result = proc.isInt32Array(new Int16Array());
  arktest.assertTrue(result == false, "The Int16Array should not be of the same type as the Int32Array");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0092
 * @tc.name: testIsInt32Array004
 * @tc.desc: Verify Float64Array is not identified as Int32Array
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsInt32Array004() {
  let proc = new util.types();
  let result = proc.isInt32Array(new Float64Array());
  arktest.assertTrue(result == false, "The Float64Array should not be of the same type as the Int32Array");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0093
 * @tc.name: testIsInt32Array005
 * @tc.desc: Verify Uint8Array is not identified as Int32Array
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsInt32Array005() {
  let proc = new util.types();
  let result = proc.isInt32Array(new Uint8Array());
  arktest.assertTrue(result == false, "The Uint8Array should not be of the same type as the Int32Array");
}
