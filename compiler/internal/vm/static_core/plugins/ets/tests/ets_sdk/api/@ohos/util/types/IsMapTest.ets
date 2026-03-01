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
    const suite = new arktest.ArkTestsuite("CommonUtil types.isMap() API tests")

    suite.addTest("Verify Map is identified as Map", testIsMap001)
    suite.addTest("Verify Set is not identified as Map", testIsMap002)
    suite.addTest("Verify String is not identified as Map", testIsMap003)
    suite.addTest("Verify Object is not identified as Map", testIsMap004)
    suite.addTest("Verify Boolean is not identified as Map", testIsMap005)

    return suite.run()
}

/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0094
 * @tc.name: testIsMap001
 * @tc.desc: Verify Map is identified as Map
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsMap001() {
  let proc = new util.types();
  let result = proc.isMap(new Map<int, int>());
  arktest.assertTrue(result, "The Map should be of the same type as the Map");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0095
 * @tc.name: testIsMap002
 * @tc.desc: Verify Set is not identified as Map
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsMap002() {
  let proc = new util.types();
  let result = proc.isMap(new Set<int>());
  arktest.assertTrue(result == false, "The Set should not be of the same type as the Map");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0096
 * @tc.name: testIsMap003
 * @tc.desc: Verify String is not identified as Map
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsMap003() {
  let proc = new util.types();
  let result = proc.isMap(new String());
  arktest.assertTrue(result == false, "The String should not be of the same type as the Map");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0097
 * @tc.name: testIsMap004
 * @tc.desc: Verify Object is not identified as Map
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsMap004() {
  let proc = new util.types();
  let result = proc.isMap(new Object());
  arktest.assertTrue(result == false, "The Object should not be of the same type as the Map");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0098
 * @tc.name: testIsMap005
 * @tc.desc: Verify Boolean is not identified as Map
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsMap005() {
  let proc = new util.types();
  let result = proc.isMap(new Boolean());
  arktest.assertTrue(result == false, "The Boolean should not be of the same type as the Map");
}
