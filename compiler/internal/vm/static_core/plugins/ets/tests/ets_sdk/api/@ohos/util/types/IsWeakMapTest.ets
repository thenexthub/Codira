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
    const suite = new arktest.ArkTestsuite("CommonUtil types.isWeakMap() API tests")

    suite.addTest("Verify WeakMap is identified as WeakMap", testIsWeakMap001)
    suite.addTest("Verify Map is not identified as WeakMap", testIsWeakMap002)
    suite.addTest("Verify Set is not identified as WeakMap", testIsWeakMap003)
    suite.addTest("Verify String is not identified as WeakMap", testIsWeakMap004)
    suite.addTest("Verify WeakSet is not identified as WeakMap", testIsWeakMap005)

    return suite.run()
}

/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_IS_WEAK_MAP_001
 * @tc.name: testIsWeakMap001
 * @tc.desc: Verify WeakMap is identified as WeakMap
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsWeakMap001() {
    let proc = new util.types();
    let result = proc.isWeakMap(new WeakMap<int, int>());
    arktest.assertTrue(result, "The WeakMap should be of the same type as the WeakMap");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_IS_WEAK_MAP_002
 * @tc.name: testIsWeakMap002
 * @tc.desc: Verify Map is not identified as WeakMap
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsWeakMap002() {
    let proc = new util.types();
    let result = proc.isWeakMap(new Map<int, int>());
    arktest.assertTrue(result == false, "The Map should be of the same type as the Map");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_IS_WEAK_MAP_003
 * @tc.name: testIsWeakMap003
 * @tc.desc: Verify Set is not identified as WeakMap
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsWeakMap003() {
    let proc = new util.types();
    let result = proc.isWeakMap(new Set<int>());
    arktest.assertTrue(result == false, "The Set should be of the same type as the Set");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_IS_WEAK_MAP_004
 * @tc.name: testIsWeakMap004
 * @tc.desc: Verify String is not identified as WeakMap
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsWeakMap004() {
    let proc = new util.types();
    let result = proc.isWeakMap(new String());
    arktest.assertTrue(result == false, "The String should be of the same type as the String");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_IS_WEAK_MAP_005
 * @tc.name: testIsWeakMap005
 * @tc.desc: Verify WeakSet is not identified as WeakMap
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsWeakMap005() {
    let proc = new util.types();
    let result = proc.isWeakMap(new WeakSet<int>());
    arktest.assertTrue(result == false, "The WeakSet should be of the same type as the WeakSet");
}
