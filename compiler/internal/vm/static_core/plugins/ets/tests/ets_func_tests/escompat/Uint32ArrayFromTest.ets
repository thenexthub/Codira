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
	return "Uint32Array.from - " + testCase;
}

function testArrayLikeNumber(): void {
	const arr: number[] = [1, -1, 4294967295, 4294967296];
	const ta = Uint32Array.from(arr);
	arktest.assertEQ(ta.length, 4);
	arktest.assertEQ(ta.$_get(0).toDouble(), 1);
	// -1 → ToInteger = -1, mod 2^32 = 4294967295
	arktest.assertEQ(ta.$_get(1).toDouble(), 4294967295);
	// 4294967295 fits exactly
	arktest.assertEQ(ta.$_get(2).toDouble(), 4294967295);
	// 4294967296 mod 2^32 = 0
	arktest.assertEQ(ta.$_get(3).toDouble(), 0);
}

function testIterableNumberWithoutMap(): void {
	const iterable = new Set<number>([10, 20, 4294967300]);
	const ta = Uint32Array.from(iterable);
	arktest.assertEQ(ta.length, 3);
	arktest.assertEQ(ta.$_get(0).toDouble(), 10);
	arktest.assertEQ(ta.$_get(1).toDouble(), 20);
	// 4294967300 mod 2^32 = 4
	arktest.assertEQ(ta.$_get(2).toDouble(), 4);
}

function testIterableNumberWithMap(): void {
	const arr: number[] = [1, 2, 3];
	// multiply by 1e9
	const ta = Uint32Array.from(arr, (v: number, _idx: number): number => v * 1000000000);
	arktest.assertEQ(ta.length, 3);
	arktest.assertEQ(ta.$_get(0).toDouble(), 1000000000);
	arktest.assertEQ(ta.$_get(1).toDouble(), 2000000000);
	// 3000000000 mod 2^32 = 3000000000
	arktest.assertEQ(ta.$_get(2).toDouble(), 3000000000);
}

function testDecimalTruncation(): void {
	const ta = Uint32Array.from([3.9, -1.5] as number[]);
	arktest.assertEQ(ta.length, 2);
	arktest.assertEQ(ta.$_get(0).toDouble(), 3);
	// -1.5 → ToInteger = -1, mod 2^32 = 4294967295
	arktest.assertEQ(ta.$_get(1).toDouble(), 4294967295);
}

function testNaNAndInfinity(): void {
	const arr = [NaN, Infinity, -Infinity];
	const ta = Uint32Array.from(arr);
	arktest.assertEQ(ta.length, 3);
	arktest.assertEQ(ta.$_get(0).toDouble(), 0);
	arktest.assertEQ(ta.$_get(1).toDouble(), 0);
	arktest.assertEQ(ta.$_get(2).toDouble(), 0);
}

function testArrayLikeTWithMap(): void {
	const strings = ["1", "4294967300", "-1"];
	const ta = Uint32Array.from(strings, (v: string): number => parseInt(v));
	arktest.assertEQ(ta.length, 3);
	arktest.assertEQ(ta.$_get(0).toDouble(), 1);
	// 4294967300 mod 2^32 = 4
	arktest.assertEQ(ta.$_get(1).toDouble(), 4);
	// -1 mod 2^32 = 4294967295
	arktest.assertEQ(ta.$_get(2).toDouble(), 4294967295);
}

function testEmpty(): void {
	const ta = Uint32Array.from([] as number[]);
	arktest.assertEQ(ta.length, 0);
}

function testDynamicReallocation(): void {
	const values: number[] = [];
	for (let i = 0; i < 16; ++i) {
		values.push(i * 500_000_000);
	}
	const ta = Uint32Array.from(values);
	arktest.assertEQ(ta.length, values.length);
	for (let i = 0; i < values.length; ++i) {
		const expected = values[i] >>> 0;
		arktest.assertEQ(ta.$_get(i).toDouble(), expected);
	}
}

function testBufferIndependence(): void {
	const src = [1, 2, 3];
	const ta = Uint32Array.from(src);
	src[0] = 9;
	arktest.assertEQ(ta.$_get(0).toDouble(), 1);
}

function main(): int {
	const suite = new arktest.ArkTestsuite("Uint32ArrayFrom");
	suite.addTest(getTestName("ArrayLike Number"), testArrayLikeNumber);
	suite.addTest(getTestName("Iterable Number No Map"), testIterableNumberWithoutMap);
	suite.addTest(getTestName("Iterable Number With Map"), testIterableNumberWithMap);
	suite.addTest(getTestName("Decimal Truncation"), testDecimalTruncation);
	suite.addTest(getTestName("NaN and Infinity"), testNaNAndInfinity);
	suite.addTest(getTestName("ArrayLike T With Map"), testArrayLikeTWithMap);
	suite.addTest(getTestName("Empty"), testEmpty);
	suite.addTest(getTestName("Buffer Independence"), testBufferIndependence);
	return suite.run();
}
