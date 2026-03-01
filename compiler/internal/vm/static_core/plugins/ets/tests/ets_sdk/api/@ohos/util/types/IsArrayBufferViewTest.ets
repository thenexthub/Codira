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
    const suite = new arktest.ArkTestsuite("CommonUtil types.isArrayBufferView() API tests")

    suite.addTest("Verify Int8Array is identified as ArrayBufferView", testIsArrayBufferView001)
    suite.addTest("Verify Int32Array is identified as ArrayBufferView", testIsArrayBufferView002)
    suite.addTest("Verify DataView is identified as ArrayBufferView", testIsArrayBufferView003)
    suite.addTest("Verify Boolean is not identified as ArrayBufferView", testIsArrayBufferView004)
    suite.addTest("Verify Int16Array is identified as ArrayBufferView", testIsArrayBufferView005)

    return suite.run()
}

/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0006
 * @tc.name: testIsArrayBufferView001
 * @tc.desc: Verify Int8Array is identified as ArrayBufferView
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsArrayBufferView001() {
  let proc = new util.types();
  let result = proc.isArrayBufferView(new Int8Array());
  arktest.assertTrue(result, "The Int8Array should be of the same type as the ArrayBufferView");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0007
 * @tc.name: testIsArrayBufferView002
 * @tc.desc: Verify Int32Array is identified as ArrayBufferView
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsArrayBufferView002() {
  let proc = new util.types();
  let result = proc.isArrayBufferView(new Int32Array());
  arktest.assertTrue(result, "The Int32Array should be of the same type as the ArrayBufferView");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0008
 * @tc.name: testIsArrayBufferView003
 * @tc.desc: Verify DataView is identified as ArrayBufferView
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsArrayBufferView003() {
  let proc = new util.types();
  let result = proc.isArrayBufferView(new DataView(new ArrayBuffer(16)));
  arktest.assertTrue(result, "The DataView should be of the same type as the ArrayBufferView");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0009
 * @tc.name: testIsArrayBufferView004
 * @tc.desc: Verify Boolean is not identified as ArrayBufferView
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsArrayBufferView004() {
  let proc = new util.types();
  let result = proc.isArrayBufferView(false);
  arktest.assertTrue(result == false, "The Boolean should not be of the same type as the ArrayBufferView");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0010
 * @tc.name: testIsArrayBufferView005
 * @tc.desc: Verify Int16Array is identified as ArrayBufferView
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsArrayBufferView005() {
  let proc = new util.types();
  let result = proc.isArrayBufferView(new Int16Array());
  arktest.assertTrue(result, "The Int16Array should be of the same type as the ArrayBufferView");
}
