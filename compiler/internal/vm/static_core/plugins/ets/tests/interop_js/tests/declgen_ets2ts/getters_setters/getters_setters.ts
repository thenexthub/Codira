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

const TWO = 2;

import { C } from './lib';

// NOTE(ivagin): enable when supported by interop #12808
const FIX_12808 = false;

function assertEq<T>(a: T, b: T): void {
	// @ts-ignore
	print(`assertEq: '${a}' === '${b}'`);
	if (a !== b) {
		throw new Error(`assertEq failed: '${a}' === '${b}'`);
	}
}

export function main(): void {
	test();
}

function test(): void {
	const c1 = new C();
	if (FIX_12808) {
		assertEq(c1.x, 0);
		assertEq(c1.val, 0);
		c1.x = 1;
		assertEq(c1.x, 1);
		assertEq(c1.val, 1);
	}
}
