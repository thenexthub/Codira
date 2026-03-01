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
    const suite = new arktest.ArkTestsuite("CommonUtil types.isSet() API tests")

    suite.addTest("Verify new Set() creates a Set object", testIsSet001)
    suite.addTest("Verify Map is not identified as Set", testIsSet002)
    suite.addTest("Verify String is not identified as Set", testIsSet003)
    suite.addTest("Verify Boolean is not identified as Set", testIsSet004)
    suite.addTest("Verify WeakSet is not identified as Set", testIsSet005)

    return suite.run()
}

/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_IS_SET_001
 * @tc.name: testIsSet001
 * @tc.desc: Verify new Set() creates a Set object
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsSet001() {
    let proc = new util.types();
    let result = proc.isSet(new Set<int>());
    arktest.assertTrue(result, "The Set should be of the same type as the Set");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_IS_SET_002
 * @tc.name: testIsSet002
 * @tc.desc: Verify Map is not identified as Set
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsSet002() {
    let proc = new util.types();
    let result = proc.isSet(new Map<int, int>());
    arktest.assertTrue(result == false, "The Number should be of the same type as the Number");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_IS_SET_003
 * @tc.name: testIsSet003
 * @tc.desc: Verify String is not identified as Set
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsSet003() {
    let proc = new util.types();
    let result = proc.isSet(new String());
    arktest.assertTrue(result == false, "The String should be of the same type as the String");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_IS_SET_004
 * @tc.name: testIsSet004
 * @tc.desc: Verify Boolean is not identified as Set
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsSet004() {
    let proc = new util.types();
    let result = proc.isSet(new Boolean());
    arktest.assertTrue(result == false, "The Boolean should be of the same type as the Boolean");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_IS_SET_005
 * @tc.name: testIsSet005
 * @tc.desc: Verify WeakSet is not identified as Set
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsSet005() {
    let proc = new util.types();
    let result = proc.isSet(new WeakSet<int>());
    arktest.assertTrue(result == false, "The WeakSet should be of the same type as the WeakSet");
}
