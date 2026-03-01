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

const RESULT_SUCCESS = 0
const RESULT_FAILURE = 1

function main(): int {
    let testsFailed = 0

    testsFailed += runTest(testArrayBufferDataView, "ArrayBufferView methods")

    return testsFailed == 0 ? RESULT_SUCCESS : RESULT_FAILURE
}

function testArrayBufferDataView(): int {
    let buffer = new ArrayBuffer(16)
    
    let bufferView: ArrayBufferView = new DataView(buffer, 4, 12)

    arktest.assertEQ(buffer, bufferView.buffer, "ArrayBufferView.buffer returned completely different buffer instance")
    arktest.assertEQ(12, bufferView.byteLength, "ArrayBufferView.byteLength")
    arktest.assertEQ(4, bufferView.byteOffset, "ArrayBufferView.byteOffset")

    return RESULT_SUCCESS
}

function runTest(test: () => int, message: String) {
  try {
    const testResult = test()

    if (testResult == RESULT_SUCCESS) console.println("PASSED: " + message)
    else console.println("FAILED: " + message);

    return testResult
  } catch (e) {
    if (e instanceof AssertionError) {
      const error = e as AssertionError
      console.println(`FAILED: '${message}' with error: ${error.name}(${error.message})`)
    } else {
      console.println(`FAILED '${message}' with error ` + e)
    }
    return RESULT_FAILURE
  }
}
