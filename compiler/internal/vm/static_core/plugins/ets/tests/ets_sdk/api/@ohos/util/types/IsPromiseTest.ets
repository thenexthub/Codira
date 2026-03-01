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
    const suite = new arktest.ArkTestsuite("CommonUtil types.isPromise() API tests")

    suite.addTest("Verify Promise.resolve() returns a Promise", testIsPromise001)
    suite.addTest("Verify Promise.resolve() with a different value", testIsPromise002)
    suite.addTest("Verify Promise.resolve() with another value", testIsPromise003)
    suite.addTest("Verify Promise.reject() returns a Promise", testIsPromise004)
    suite.addTest("Verify Promise.reject() with a different error", testIsPromise005)

    return suite.run()
}

/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_IS_PROMISE_001
 * @tc.name: testIsPromise001
 * @tc.desc: Verify Promise.resolve() returns a Promise
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsPromise001() {
    let proc = new util.types();
    let result = proc.isPromise(Promise.resolve(42));
    arktest.assertTrue(result, "The Promise should be of the same type as the Promise");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_IS_PROMISE_002
 * @tc.name: testIsPromise002
 * @tc.desc: Verify Promise.resolve() with a different value
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsPromise002() {
    let proc = new util.types();
    let result = proc.isPromise(Promise.resolve(33));
    arktest.assertTrue(result, "The Promise should be of the same type as the Promise");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_IS_PROMISE_003
 * @tc.name: testIsPromise003
 * @tc.desc: Verify Promise.resolve() with another value
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsPromise003() {
    let proc = new util.types();
    let result = proc.isPromise(Promise.resolve(44));
    arktest.assertTrue(result, "The Promise should be of the same type as the Promise");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_IS_PROMISE_004
 * @tc.name: testIsPromise004
 * @tc.desc: Verify Promise.reject() returns a Promise
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsPromise004() {
    let proc = new util.types();
    let err = Error("33");
    let result = proc.isPromise(Promise.reject(err));
    arktest.assertTrue(result, "The Promise should be of the same type as the Promise");
}

/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_IS_PROMISE_005
 * @tc.name: testIsPromise005
 * @tc.desc: Verify Promise.reject() with a different error
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsPromise005() {
    let proc = new util.types();
    let err = Error("23");
    let result = proc.isPromise(Promise.reject(err));
    arktest.assertTrue(result, "The Promise should be of the same type as the Promise");
}
