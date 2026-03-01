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
	return "Uint8ClampedArray.from - " + testCase;
}

function testArrayLikeNumber(): void {
	const arr: number[] = [ -10, 0, 128, 255, 300 ];
	const ta = Uint8ClampedArray.from(arr);
	arktest.assertEQ(ta.length, 5);
	// values below 0 clamp to 0; above 255 clamp to 255
	arktest.assertEQ(ta.$_get(0).toDouble(), 0);
	arktest.assertEQ(ta.$_get(1).toDouble(), 0);
	arktest.assertEQ(ta.$_get(2).toDouble(), 128);
	arktest.assertEQ(ta.$_get(3).toDouble(), 255);
	arktest.assertEQ(ta.$_get(4).toDouble(), 255);
}

function testIterableNumberWithoutMap(): void {
	const iterable = new Set<number>([ 10, 20, 30, 260 ]);
	const ta = Uint8ClampedArray.from(iterable);
	arktest.assertEQ(ta.length, 4);
	arktest.assertEQ(ta.$_get(0).toDouble(), 10);
	arktest.assertEQ(ta.$_get(1).toDouble(), 20);
	arktest.assertEQ(ta.$_get(2).toDouble(), 30);
	arktest.assertEQ(ta.$_get(3).toDouble(), 255);
}

function testIterableNumberWithMap(): void {
	const arr: number[] = [ 50, 100, 200 ];
	// double then clamp
	const ta = Uint8ClampedArray.from(arr, (v: number): number => v * 2);
	arktest.assertEQ(ta.length, 3);
	arktest.assertEQ(ta.$_get(0).toDouble(), 100);
	arktest.assertEQ(ta.$_get(1).toDouble(), 200);
	arktest.assertEQ(ta.$_get(2).toDouble(), 255);
}

function testDecimalRounding(): void {
	const arr: number[] = [ 1.2, 1.5, 1.8, -0.5 ];
	const ta = Uint8ClampedArray.from(arr);
	arktest.assertEQ(ta.length, 4);
	// 1.2 → 1, 1.5 → 2 (ties to nearest even?), 1.8 → 2, -0.5 → 0
	arktest.assertEQ(ta.$_get(0).toDouble(), 1);
	arktest.assertEQ(ta.$_get(1).toDouble(), 2);
	arktest.assertEQ(ta.$_get(2).toDouble(), 2);
	arktest.assertEQ(ta.$_get(3).toDouble(), 0);
}

function testNaNAndInfinity(): void {
	const arr = [ NaN, Infinity, -Infinity ];
	const ta = Uint8ClampedArray.from(arr);
	arktest.assertEQ(ta.length, 3);
	// NaN → 0; Infinity → 255; -Infinity → 0
	arktest.assertEQ(ta.$_get(0).toDouble(), 0);
	arktest.assertEQ(ta.$_get(1).toDouble(), 255);
	arktest.assertEQ(ta.$_get(2).toDouble(), 0);
}

function testEmpty(): void {
	const ta = Uint8ClampedArray.from([] as number[]);
	arktest.assertEQ(ta.length, 0);
}

function testDynamicReallocation(): void {
	const iterable = new Set<number>();
	for (let i = 0; i < 20; ++i) {
		iterable.add(i * 13);
	}
	const ta = Uint8ClampedArray.from(iterable);
	arktest.assertEQ(ta.length, 20);
	let idx = 0;
	for (const v of iterable) {
		const expected = v < 0 ? 0 : v > 255 ? 255 : v;
		arktest.assertEQ(ta.$_get(idx).toDouble(), expected);
		idx++;
	}
}

function testBufferIndependence(): void {
	const src = [ 5, 15, 25 ];
	const ta = Uint8ClampedArray.from(src);
	src[0] = 100;
	arktest.assertEQ(ta.$_get(0).toDouble(), 5);
}

function main(): int {
	const suite = new arktest.ArkTestsuite("Uint8ClampedArrayFrom");
	suite.addTest(getTestName("ArrayLike Number"), testArrayLikeNumber);
	suite.addTest(getTestName("Iterable Number No Map"), testIterableNumberWithoutMap);
	suite.addTest(getTestName("Iterable Number With Map"), testIterableNumberWithMap);
	suite.addTest(getTestName("Decimal Rounding"), testDecimalRounding);
	suite.addTest(getTestName("NaN and Infinity"), testNaNAndInfinity);
	suite.addTest(getTestName("Empty"), testEmpty);
	suite.addTest(getTestName("Dynamic Reallocation"), testDynamicReallocation);
	suite.addTest(getTestName("Buffer Independence"), testBufferIndependence);
	return suite.run();
}
