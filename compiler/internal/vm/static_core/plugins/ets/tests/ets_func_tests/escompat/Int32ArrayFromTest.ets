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
	return "Int32Array.from - " + testCase;
}

function testArrayLikeNumber(): void {
	const arr: number[] = [1, -2, 2147483647, -2147483648];
	const ta = Int32Array.from(arr);
	arktest.assertEQ(ta.length, 4);
	arktest.assertEQ(ta.$_get(0).toDouble(), 1);
	arktest.assertEQ(ta.$_get(1).toDouble(), -2);
	arktest.assertEQ(ta.$_get(2).toDouble(), 2147483647);
	arktest.assertEQ(ta.$_get(3).toDouble(), -2147483648);
}

function testIterableNumberWithoutMap(): void {
	const iterable = new Set<number>([1000, -2000, 3000]);
	const ta = Int32Array.from(iterable);
	arktest.assertEQ(ta.length, 3);
	arktest.assertEQ(ta.$_get(0).toDouble(), 1000);
	arktest.assertEQ(ta.$_get(1).toDouble(), -2000);
	arktest.assertEQ(ta.$_get(2).toDouble(), 3000);
}

function testIterableNumberWithMap(): void {
	const arr: number[] = [10, 20, 30];
	const ta = Int32Array.from(arr, (v: number, idx: number): number => {
		return v * 5;
	});
	arktest.assertEQ(ta.length, 3);
	arktest.assertEQ(ta.$_get(0).toDouble(), 50);
	arktest.assertEQ(ta.$_get(1).toDouble(), 100);
	arktest.assertEQ(ta.$_get(2).toDouble(), 150);
}

function testArrayLikeTWithMap(): void {
	const strings = ["100000", "-100000", "123456"];
	const ta = Int32Array.from(strings, (v: string): number => parseInt(v));
	arktest.assertEQ(ta.length, 3);
	arktest.assertEQ(ta.$_get(0).toDouble(), 100000);
	arktest.assertEQ(ta.$_get(1).toDouble(), -100000);
	arktest.assertEQ(ta.$_get(2).toDouble(), 123456);
}

function testMapfnInfinity(): void {
	const arr = [1, 2];
	const ta = Int32Array.from(arr, () => Infinity);
	arktest.assertEQ(ta.length, 2);
	arktest.assertEQ(ta.$_get(0).toDouble(), 0);
	arktest.assertEQ(ta.$_get(1).toDouble(), 0);
}

function testEmpty(): void {
	const ta = Int32Array.from([] as number[]);
	arktest.assertEQ(ta.length, 0);
}

function testDynamicReallocation(): void {
	const iterable = new Set<number>([0,1,2,3,4,5,6,7,8,9]);
	const ta = Int32Array.from(iterable);
	arktest.assertEQ(ta.length, 10);
	for (let i: int = 0; i < 10; ++i) {
		arktest.assertEQ(ta.$_get(i).toDouble(), i);
	}
}

function testDecimalTruncation(): void {
	const ta = Int32Array.from([123.9, -123.9] as number[]);
	arktest.assertEQ(ta.$_get(0).toDouble(), 123);
	arktest.assertEQ(ta.$_get(1).toDouble(), -123);
}

function testBufferIndependence(): void {
	const src = [7, 8, 9];
	const ta = Int32Array.from(src);
	src[0] = 99;
	arktest.assertEQ(ta.$_get(0).toDouble(), 7);
}

function main(): int {
	const suite = new arktest.ArkTestsuite("Int32ArrayFrom");
	suite.addTest(getTestName("ArrayLike Number"), testArrayLikeNumber);
	suite.addTest(getTestName("Iterable Number No Map"), testIterableNumberWithoutMap);
	suite.addTest(getTestName("Iterable Number With Map"), testIterableNumberWithMap);
	suite.addTest(getTestName("ArrayLike T With Map"), testArrayLikeTWithMap);
	suite.addTest(getTestName("Mapfn Infinity"), testMapfnInfinity);
	suite.addTest(getTestName("Empty"), testEmpty);
	suite.addTest(getTestName("Dynamic Reallocation"), testDynamicReallocation);
	suite.addTest(getTestName("Decimal Truncation"), testDecimalTruncation);
	suite.addTest(getTestName("Buffer Independence"), testBufferIndependence);
	return suite.run();
}