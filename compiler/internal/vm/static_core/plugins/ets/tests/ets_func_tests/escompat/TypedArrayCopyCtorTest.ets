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

function getTestName(arrayType: string): string {
	return arrayType + " copy constructor"
}

function testInt8ArrayCopyCtor(): void {
	const bytes = Int8Array.of(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14)

	const bytesFromBuf = new Int8Array(bytes.buffer, 1, 7)
	const bytesFromBufCopy = new Int8Array(bytesFromBuf)

 arktest.assertEQ(bytesFromBuf.byteLength, bytesFromBufCopy.byteLength)
 arktest.assertEQ(0, bytesFromBufCopy.byteOffset)
 arktest.assertEQ(bytesFromBuf.toString(), bytesFromBufCopy.toString())
}

function testInt32ArrayCopyCtor(): void {
	const ints = Int32Array.of(34, 56, 67, 89, 567, 789, 123)

	const intsFromBuf = new Int32Array(ints.buffer, 4, 4)
	const intsFromBufCopy = new Int32Array(intsFromBuf)

 arktest.assertEQ(intsFromBuf.byteLength, intsFromBufCopy.byteLength)
 arktest.assertEQ(0, intsFromBufCopy.byteOffset)
 arktest.assertEQ(intsFromBuf.toString(), intsFromBufCopy.toString())
}

function main(): int {
	const suite = new arktest.ArkTestsuite("TypedArray")

	suite.addTest(getTestName("Int8Array"), testInt8ArrayCopyCtor)
	suite.addTest(getTestName("Int32Array"), testInt32ArrayCopyCtor)

	return suite.run()
}
