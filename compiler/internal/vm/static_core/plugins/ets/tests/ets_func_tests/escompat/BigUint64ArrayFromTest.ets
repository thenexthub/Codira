/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

function getTestName(testCase: string): string {
	return 'BigUint64Array.from - ' + testCase;
}

function testArrayLikeBigInt(): void {
	const arr: bigint[] = [1n, 18446744073709551615n, 9223372036854775808n, 0n];
	const ta = BigUint64Array.from(arr);
	arktest.assertEQ(ta.length, 4);
	arktest.assertEQ(ta[0], 1n);
	arktest.assertEQ(ta[1], 18446744073709551615n);
	arktest.assertEQ(ta[2], 9223372036854775808n);
	arktest.assertEQ(ta[3], 0n);
}

function testIterableBigIntWithoutMap(): void {
	const iterable = new Set<bigint>([10n, 20n, 18446744073709551615n]);
	const ta = BigUint64Array.from(iterable);
	arktest.assertEQ(ta.length, 3);
	arktest.assertEQ(ta[0], 10n);
	arktest.assertEQ(ta[1], 20n);
	arktest.assertEQ(ta[2], 18446744073709551615n);
}

function testIterableBigIntWithMap(): void {
	const arr: bigint[] = [1n, 2n, 3n];
	// multiply by 1000000000000000000n
	const ta = BigUint64Array.from(arr, (v: bigint, _idx: number): bigint => v * 1000000000000000000n);
	arktest.assertEQ(ta.length, 3);
	arktest.assertEQ(ta[0], 1000000000000000000n);
	arktest.assertEQ(ta[1], 2000000000000000000n);
	arktest.assertEQ(ta[2], 3000000000000000000n);
}

function testArrayLikeTWithMap(): void {
	const strings = ['1', '18446744073709551615', '9223372036854775808'];
	const ta = BigUint64Array.from(strings, (v: string): bigint => BigInt(v));
	arktest.assertEQ(ta.length, 3);
	arktest.assertEQ(ta[0], 1n);
	arktest.assertEQ(ta[1], 18446744073709551615n);
	arktest.assertEQ(ta[2], 9223372036854775808n);
}

function testEmpty(): void {
	const ta = BigUint64Array.from([] as bigint[]);
	arktest.assertEQ(ta.length, 0);
}

function testDynamicReallocation(): void {
	const values: bigint[] = [];
	for (let i = 0; i < 10; ++i) {
		values.push(BigInt(i * 1000000000000000000));
	}
	const ta = BigUint64Array.from(values);
	arktest.assertEQ(ta.length, values.length);
	for (let i = 0; i < values.length; ++i) {
		arktest.assertEQ(ta[i], values[i]);
	}
}

function testBufferIndependence(): void {
	const src = [1n, 2n, 3n];
	const ta = BigUint64Array.from(src);
	src[0] = 9n;
	arktest.assertEQ(ta[0], 1n);
}

function testLargeValues(): void {
	const largeVals: bigint[] = [
		18446744073709551615n, // max uint64
		9223372036854775808n,  // 2^63
		9223372036854775807n,  // 2^63 - 1
		0n
	];
	const ta = BigUint64Array.from(largeVals);
	arktest.assertEQ(ta.length, 4);
	arktest.assertEQ(ta[0], 18446744073709551615n);
	arktest.assertEQ(ta[1], 9223372036854775808n);
	arktest.assertEQ(ta[2], 9223372036854775807n);
	arktest.assertEQ(ta[3], 0n);
}

function main(): int {
	const suite = new arktest.ArkTestsuite('BigUint64ArrayFrom');
	suite.addTest(getTestName('ArrayLike BigInt'), testArrayLikeBigInt);
	suite.addTest(getTestName('Iterable BigInt No Map'), testIterableBigIntWithoutMap);
	suite.addTest(getTestName('Iterable BigInt With Map'), testIterableBigIntWithMap);
	suite.addTest(getTestName('ArrayLike T With Map'), testArrayLikeTWithMap);
	suite.addTest(getTestName('Empty'), testEmpty);
	suite.addTest(getTestName('Dynamic Reallocation'), testDynamicReallocation);
	suite.addTest(getTestName('Buffer Independence'), testBufferIndependence);
	suite.addTest(getTestName('Large Values'), testLargeValues);
	return suite.run();
}
