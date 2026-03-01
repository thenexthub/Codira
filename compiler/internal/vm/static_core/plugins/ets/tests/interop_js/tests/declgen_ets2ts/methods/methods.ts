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

const TWENTY_TWO_HUNDREDTH = 0.22;
const ONE = 1;
const TWO = 2;
const THREE = 3;
const FOUR = 4;
const FIVE = 5;
const FIFTEEN = 15;
const PRECISION = 3;

// NOTE(ivagin): enable when supported by runtime #12808
const FIX_12808: boolean = false;

import { Methods } from './lib';

function assertEq<T>(a: T, b: T): void {
	// @ts-ignore
	print(`assertEq: '${a}' === '${b}'`);
	if (a !== b) {
		throw new Error(`assertEq failed: '${a}' === '${b}'`);
	}
}

export function main(): void {
	testMethods();
}

function testMethods(): void {
	assertEq(Methods.StaticSumDouble(THREE, TWENTY_TWO_HUNDREDTH).toFixed(PRECISION), (THREE + TWENTY_TWO_HUNDREDTH).toFixed(PRECISION));

	const o = new Methods();
	assertEq(o.IsTrue(false), false);

	assertEq(o.SumString('Hello from ', 'Panda!!!!'), 'Hello from Panda!!!!');
	assertEq(o.SumIntArray([ONE, TWO, THREE, FOUR, FIVE]), FIFTEEN);
	assertEq(o.OptionalString('Optional string'), 'Optional string');

	if (FIX_12808) {
		assertEq(o.OptionalString(undefined), undefined);
		// NOTE(ivagin): add one more check like this: "assertEq(o.OptionalString(), undefined);"
		assertEq(o.SumIntVariadic(ONE, TWO, THREE, FOUR, FIVE).toFixed(PRECISION), FIFTEEN.toFixed(PRECISION));
	}
}
