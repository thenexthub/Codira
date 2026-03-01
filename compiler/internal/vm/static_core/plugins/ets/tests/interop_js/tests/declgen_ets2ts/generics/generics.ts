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
const FIVE = 5;
const PRECISION = 3;

import { GenericClass, BaseGeneric, Identity, ForEach } from './lib';
import type { IGeneric0, IGeneric1 } from './lib';

function assertEq<T>(a: T, b: T): void {
	// @ts-ignore
	print(`assertEq: '${a}' === '${b}'`);
	if (a !== b) {
		throw new Error(`assertEq failed: '${a}' === '${b}'`);
	}
}

export function main(): void {
	testGenerics();
}

export class DerivedGeneric<T, U, V> extends BaseGeneric<T, U> implements IGeneric0<U>, IGeneric1<IGeneric0<V>> {
	I0Method(a: U): U {
		return a;
	}
	I1Method(a: IGeneric0<V>): IGeneric0<V> {
		return a;
	}
}

function testGenerics(): void {
	const tStr = new GenericClass<String>();
	assertEq(tStr.identity('Test generic class'), 'Test generic class');
	const tNumber = new GenericClass<Number>();
	assertEq(tNumber.identity(FIVE).toFixed(PRECISION), FIVE.toFixed(PRECISION));

	assertEq(Identity(THREE + TWENTY_TWO_HUNDREDTH).toFixed(PRECISION), (THREE + TWENTY_TWO_HUNDREDTH).toFixed(PRECISION));
	assertEq(Identity('Panda identity string'), 'Panda identity string');

	// NOTE(ivagin): enable when supported by interop #12808
	if (false) {
		const intArr = [ONE, TWO, THREE];
		ForEach(intArr, (e: number, idx: number) => {
			assertEq(e, intArr[idx]);
		});
		const intStr = ['1', '2', '3'];
		ForEach(intStr, (e: string, idx: number) => {
			assertEq(e, intStr[idx]);
		});
	}
}
