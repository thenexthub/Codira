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

function getTestName(testCase: string): string {
	return "Uint8Array.from - " + testCase;
}

function testArrayLikeNumber(): void {
	const arr: number[] = [1, -2, 255, 256];
	const ta = Uint8Array.from(arr);
	arktest.assertEQ(ta.length, 4);
	arktest.assertEQ(ta.$_get(0).toDouble(), 1);
	arktest.assertEQ(ta.$_get(1).toDouble(), 254);
	arktest.assertEQ(ta.$_get(2).toDouble(), 255);
	arktest.assertEQ(ta.$_get(3).toDouble(), 0);
}

function testIterableNumberWithoutMap(): void {
	const iterable = new Set<number>([10, 20, 300]);
	const ta = Uint8Array.from(iterable);
	arktest.assertEQ(ta.length, 3);
	arktest.assertEQ(ta.$_get(0).toDouble(), 10);
	arktest.assertEQ(ta.$_get(1).toDouble(), 20);
	arktest.assertEQ(ta.$_get(2).toDouble(), 44);
}

function testIterableNumberWithMap(): void {
	const arr: number[] = [1, 2, 3];
	const ta = Uint8Array.from(arr, (v: number, _idx: number): number => v * 100);
	arktest.assertEQ(ta.length, 3);
	arktest.assertEQ(ta.$_get(0).toDouble(), 100);
	arktest.assertEQ(ta.$_get(1).toDouble(), 200);
	arktest.assertEQ(ta.$_get(2).toDouble(), 44);
}

function testDecimalTruncation(): void {
	const ta = Uint8Array.from([3.7, -1.9] as number[]);
	arktest.assertEQ(ta.length, 2);
	arktest.assertEQ(ta.$_get(0).toDouble(), 3);
	arktest.assertEQ(ta.$_get(1).toDouble(), 255);
}

function testNaNAndInfinity(): void {
	const arr = [NaN, Infinity, -Infinity];
	const ta = Uint8Array.from(arr);
	arktest.assertEQ(ta.length, 3);
	arktest.assertEQ(ta.$_get(0).toDouble(), 0);
	arktest.assertEQ(ta.$_get(1).toDouble(), 0);
	arktest.assertEQ(ta.$_get(2).toDouble(), 0);
}

function testArrayLikeTWithMap(): void {
	const strings = ["10", "256", "-1"];
	const ta = Uint8Array.from(strings, (v: string): number => parseInt(v));
	arktest.assertEQ(ta.length, 3);
	arktest.assertEQ(ta.$_get(0).toDouble(), 10);
	arktest.assertEQ(ta.$_get(1).toDouble(), 0);
	arktest.assertEQ(ta.$_get(2).toDouble(), 255);
}

function testEmpty(): void {
	const ta = Uint8Array.from([] as number[]);
	arktest.assertEQ(ta.length, 0);
}

function testDynamicReallocation(): void {
	const iterable = new Set<number>();
	for (let i = 0; i < 10; ++i) {
		iterable.add(i);
	}
	const ta = Uint8Array.from(iterable);
	arktest.assertEQ(ta.length, 10);
	for (let i: int = 0; i < 10; ++i) {
		arktest.assertEQ(ta.$_get(i).toDouble(), i);
	}
}

function testBufferIndependence(): void {
	const src = [1, 2, 3];
	const ta = Uint8Array.from(src);
	src[0] = 9;
	arktest.assertEQ(ta.$_get(0).toDouble(), 1);
}

function main(): int {
	const suite = new arktest.ArkTestsuite("Uint8ArrayFrom");
	suite.addTest(getTestName("ArrayLike Number"), testArrayLikeNumber);
	suite.addTest(getTestName("Iterable Number No Map"), testIterableNumberWithoutMap);
	suite.addTest(getTestName("Iterable Number With Map"), testIterableNumberWithMap);
	suite.addTest(getTestName("Decimal Truncation"), testDecimalTruncation);
	suite.addTest(getTestName("NaN and Infinity"), testNaNAndInfinity);
	suite.addTest(getTestName("ArrayLike T With Map"), testArrayLikeTWithMap);
	suite.addTest(getTestName("Empty"), testEmpty);
	suite.addTest(getTestName("Dynamic Reallocation"), testDynamicReallocation);
	suite.addTest(getTestName("Buffer Independence"), testBufferIndependence);
	return suite.run();
}
