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
    const suite = new arktest.ArkTestsuite("CommonUtil types.isRegExp() API tests")

    suite.addTest("Verify new RegExp() creates a RegExp object", testIsRegExp001)
    suite.addTest("Verify RegExp with a string pattern", testIsRegExp002)
    suite.addTest("Verify RegExp with another pattern", testIsRegExp003)
    suite.addTest("Verify RegExp with numeric pattern", testIsRegExp004)
    suite.addTest("Verify RegExp with flags", testIsRegExp005)

    return suite.run()
}

/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_IS_REGEXP_001
 * @tc.name: testIsRegExp001
 * @tc.desc: Verify new RegExp() creates a RegExp object
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsRegExp001() {
    let proc = new util.types();
    let result = proc.isRegExp(new RegExp('abc'));
    arktest.assertTrue(result, "The RegExp should be of the same type as the RegExp");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_IS_REGEXP_002
 * @tc.name: testIsRegExp002
 * @tc.desc: Verify RegExp with a string pattern
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsRegExp002() {
    let proc = new util.types();
    let result = proc.isRegExp(new RegExp("/abc/"));
    arktest.assertTrue(result, "The Number should be of the same type as the Number");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_IS_REGEXP_003
 * @tc.name: testIsRegExp003
 * @tc.desc: Verify RegExp with another pattern
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsRegExp003() {
    let proc = new util.types();
    let result = proc.isRegExp(new RegExp('fgd'));
    arktest.assertTrue(result, "The RegExp should be of the same type as the RegExp");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_IS_REGEXP_004
 * @tc.name: testIsRegExp004
 * @tc.desc: Verify RegExp with numeric pattern
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsRegExp004() {
    let proc = new util.types();
    let result = proc.isRegExp(new RegExp('123'));
    arktest.assertTrue(result, "The RegExp should be of the same type as the RegExp");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_IS_REGEXP_005
 * @tc.name: testIsRegExp005
 * @tc.desc: Verify RegExp with flags
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsRegExp005() {
    let proc = new util.types();
    let result = proc.isRegExp(new RegExp("/ab+c/", 'i'));
    arktest.assertTrue(result, "The RegExp should be of the same type as the RegExp");
}
