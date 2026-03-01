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
    const suite = new arktest.ArkTestsuite("CommonUtil types.isFloat32Array() API tests")

    suite.addTest("Verify Float32Array is identified as Float32Array", testIsFloat32Array001)
    suite.addTest("Verify Int8Array is not identified as Float32Array", testIsFloat32Array002)
    suite.addTest("Verify Int16Array is not identified as Float32Array", testIsFloat32Array003)
    suite.addTest("Verify Float64Array is not identified as Float32Array", testIsFloat32Array004)
    suite.addTest("Verify Uint8Array is not identified as Float32Array", testIsFloat32Array005)

    return suite.run()
}

/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0061
 * @tc.name: testIsFloat32Array001
 * @tc.desc: Verify Float32Array is identified as Float32Array
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsFloat32Array001() {
  let proc = new util.types();
  let result = proc.isFloat32Array(new Float32Array());
  arktest.assertTrue(result, "The Float32Array should be of the same type as the Float32Array");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0062
 * @tc.name: testIsFloat32Array002
 * @tc.desc: Verify Int8Array is not identified as Float32Array
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsFloat32Array002() {
  let proc = new util.types();
  let result = proc.isFloat32Array(new Int8Array());
  arktest.assertTrue(result == false, "The Int8Array should not be of the same type as the Float32Array");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0063
 * @tc.name: testIsFloat32Array003
 * @tc.desc: Verify Int16Array is not identified as Float32Array
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsFloat32Array003() {
  let proc = new util.types();
  let result = proc.isFloat32Array(new Int16Array());
  arktest.assertTrue(result == false, "The Int16Array should not be of the same type as the Float32Array");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0064
 * @tc.name: testIsFloat32Array004
 * @tc.desc: Verify Float64Array is not identified as Float32Array
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsFloat32Array004() {
  let proc = new util.types();
  let result = proc.isFloat32Array(new Float64Array());
  arktest.assertTrue(result == false, "The Float64Array should not be of the same type as the Float32Array");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0065
 * @tc.name: testIsFloat32Array005
 * @tc.desc: Verify Uint8Array is not identified as Float32Array
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsFloat32Array005() {
  let proc = new util.types();
  let result = proc.isFloat32Array(new Uint8Array());
  arktest.assertTrue(result == false, "The Uint8Array should not be of the same type as the Float32Array");
}
