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
    const suite = new arktest.ArkTestsuite("CommonUtil types.isAnyArrayBuffer() API tests")

    suite.addTest("Verify ArrayBuffer is identified as AnyArrayBuffer", testIsAnyArrayBuffer001)
    suite.addTest("Verify Boolean is not identified as AnyArrayBuffer", testIsAnyArrayBuffer002)
    suite.addTest("Verify Boolean is not identified as AnyArrayBuffer", testIsAnyArrayBuffer003)
    suite.addTest("Verify ArrayBuffer is identified as AnyArrayBuffer", testIsAnyArrayBuffer004)
    suite.addTest("Verify Boolean is not identified as AnyArrayBuffer", testIsAnyArrayBuffer005)

    return suite.run()
}

/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0001
 * @tc.name: testIsAnyArrayBuffer001
 * @tc.desc: Verify ArrayBuffer is identified as AnyArrayBuffer
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsAnyArrayBuffer001() {
  let proc = new util.types();
  let result = proc.isAnyArrayBuffer(new ArrayBuffer(1));
  arktest.assertTrue(result, "The ArrayBuffer should be of the same type as the ArrayBuffer");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0002
 * @tc.name: testIsAnyArrayBuffer002
 * @tc.desc: Verify Boolean is not identified as AnyArrayBuffer
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsAnyArrayBuffer002() {
  let proc = new util.types();
  let result = proc.isAnyArrayBuffer(true);
  arktest.assertTrue(result == false, "The Boolean should not be of the same type as the ArrayBuffer");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0003
 * @tc.name: testIsAnyArrayBuffer003
 * @tc.desc: Verify Boolean is not identified as AnyArrayBuffer
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsAnyArrayBuffer003() {
  let proc = new util.types();
  let result = proc.isAnyArrayBuffer(false);
  arktest.assertTrue(result == false, "The Boolean should not be of the same type as the ArrayBuffer");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0004
 * @tc.name: testIsAnyArrayBuffer004
 * @tc.desc: Verify ArrayBuffer is identified as AnyArrayBuffer
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsAnyArrayBuffer004() {
  let proc = new util.types();
  let buf = new ArrayBuffer(1);
  let result = proc.isAnyArrayBuffer(buf);
  arktest.assertTrue(result, "The ArrayBuffer should be of the same type as the ArrayBuffer");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0005
 * @tc.name: testIsAnyArrayBuffer005
 * @tc.desc: Verify Boolean is not identified as AnyArrayBuffer
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsAnyArrayBuffer005() {
  let proc = new util.types();
  let result = proc.isAnyArrayBuffer(new Boolean(false));
  arktest.assertTrue(result == false, "The Boolean should not be of the same type as the ArrayBuffer");
}
