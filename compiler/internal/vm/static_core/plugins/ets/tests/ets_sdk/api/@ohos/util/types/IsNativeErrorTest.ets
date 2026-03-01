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
    const suite = new arktest.ArkTestsuite("CommonUtil types.isNativeError() API tests")

    suite.addTest("Verify TypeError is identified as NativeError", testIsNativeError001)
    suite.addTest("Verify Error is identified as NativeError", testIsNativeError002)
    suite.addTest("Verify RangeError is identified as NativeError", testIsNativeError003)
    suite.addTest("Verify Boolean is not identified as NativeError", testIsNativeError004)
    suite.addTest("Verify Boolean is not identified as NativeError", testIsNativeError005)

    return suite.run()
}

/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0108
 * @tc.name: testIsNativeError001
 * @tc.desc: Verify TypeError is identified as NativeError
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsNativeError001() {
  let proc = new util.types();
  let result = proc.isNativeError(new TypeError());
  arktest.assertTrue(result, "The TypeError should be of the same type as the NativeError");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0109
 * @tc.name: testIsNativeError002
 * @tc.desc: Verify Error is identified as NativeError
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsNativeError002() {
  let proc = new util.types();
  let result = proc.isNativeError(new Error());
  arktest.assertTrue(result, "The Error should be of the same type as the NativeError");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0110
 * @tc.name: testIsNativeError003
 * @tc.desc: Verify RangeError is identified as NativeError
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsNativeError003() {
  let proc = new util.types();
  let result = proc.isNativeError(new RangeError());
  arktest.assertTrue(result, "The RangeError should be of the same type as the NativeError");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0111
 * @tc.name: testIsNativeError004
 * @tc.desc: Verify Boolean is not identified as NativeError
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsNativeError004() {
  let proc = new util.types();
  let result = proc.isNativeError(true);
  arktest.assertTrue(result == false, "The primitive Boolean should not be of the same type as the NativeError");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0112
 * @tc.name: testIsNativeError005
 * @tc.desc: Verify Boolean is not identified as NativeError
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsNativeError005() {
  let proc = new util.types();
  let result = proc.isNativeError(false);
  arktest.assertTrue(result == false, "The primitive Boolean should not be of the same type as the NativeError");
}
