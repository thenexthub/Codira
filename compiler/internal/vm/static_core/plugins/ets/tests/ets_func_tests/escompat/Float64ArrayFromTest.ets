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
	return "Float64Array.from - " + testCase;
}

function testArrayLikeNumber(): void {
	const arr: number[] = [1.23456789012345, -2.34567890123456, 3.45678901234567, -4.56789012345678];
	const ta = Float64Array.from(arr);
	arktest.assertEQ(ta.length, 4);
	const epsilon = 1e-12;
	arktest.assertDoubleEQ(ta.$_get(0).toDouble(), 1.23456789012345, epsilon);
	arktest.assertDoubleEQ(ta.$_get(1).toDouble(), -2.34567890123456, epsilon);
	arktest.assertDoubleEQ(ta.$_get(2).toDouble(), 3.45678901234567, epsilon);
	arktest.assertDoubleEQ(ta.$_get(3).toDouble(), -4.56789012345678, epsilon);
}

function testIterableNumberWithoutMap(): void {
	const iterable = new Set<number>([0.125, -0.875, 2.5]);
	const ta = Float64Array.from(iterable);
	arktest.assertEQ(ta.length, 3);
	const epsilon = 1e-12;
	arktest.assertDoubleEQ(ta.$_get(0).toDouble(), 0.125, epsilon);
	arktest.assertDoubleEQ(ta.$_get(1).toDouble(), -0.875, epsilon);
	arktest.assertDoubleEQ(ta.$_get(2).toDouble(), 2.5, epsilon);
}

function testIterableNumberWithMap(): void {
	const arr: number[] = [1, 2, 3];
	const ta = Float64Array.from(arr, (v: number, idx: number): number => v + idx * 0.5);
	arktest.assertEQ(ta.length, 3);
	const epsilon = 1e-12;
	arktest.assertDoubleEQ(ta.$_get(0).toDouble(), 1.0, epsilon);
	arktest.assertDoubleEQ(ta.$_get(1).toDouble(), 2.5, epsilon);
	arktest.assertDoubleEQ(ta.$_get(2).toDouble(), 4.0, epsilon);
}

function testMapfnInfinity(): void {
	const arr = [Infinity, -Infinity];
	const ta = Float64Array.from(arr, () => Infinity);
	arktest.assertEQ(ta.length, 2);
	arktest.assertTrue(ta.$_get(0).toDouble() === Infinity);
	arktest.assertTrue(ta.$_get(1).toDouble() === Infinity);
}

function testMapfnNaN(): void {
	const arr = [1, 2];
	const ta = Float64Array.from(arr, () => NaN);
	arktest.assertEQ(ta.length, 2);
	arktest.assertTrue(isNaN(ta.$_get(0).toDouble()));
	arktest.assertTrue(isNaN(ta.$_get(1).toDouble()));
}

function testEmpty(): void {
	const ta = Float64Array.from([] as number[]);
	arktest.assertEQ(ta.length, 0);
}

function testDynamicReallocation(): void {
	const iterable = new Set<number>();
	for (let i = 0; i < 16; ++i) {
		iterable.add(i * 0.25);
	}
	const ta = Float64Array.from(iterable);
	arktest.assertEQ(ta.length, 16);
	const epsilon = 1e-12;
	let idx = 0;
	for (const v of iterable) {
		arktest.assertDoubleEQ(ta.$_get(idx).toDouble(), v, epsilon);
		idx++;
	}
}

function testBufferIndependence(): void {
	const src = [0.123, 0.456, 0.789];
	const ta = Float64Array.from(src);
	src[0] = 9.999;
	const epsilon = 1e-12;
	arktest.assertDoubleEQ(ta.$_get(0).toDouble(), 0.123, epsilon);
}

function main(): int {
	const suite = new arktest.ArkTestsuite("Float64ArrayFrom");
	suite.addTest(getTestName("ArrayLike Number"), testArrayLikeNumber);
	suite.addTest(getTestName("Iterable Number No Map"), testIterableNumberWithoutMap);
	suite.addTest(getTestName("Iterable Number With Map"), testIterableNumberWithMap);
	suite.addTest(getTestName("Mapfn Infinity"), testMapfnInfinity);
	suite.addTest(getTestName("Mapfn NaN"), testMapfnNaN);
	suite.addTest(getTestName("Empty"), testEmpty);
	suite.addTest(getTestName("Dynamic Reallocation"), testDynamicReallocation);
	suite.addTest(getTestName("Buffer Independence"), testBufferIndependence);
	return suite.run();
}