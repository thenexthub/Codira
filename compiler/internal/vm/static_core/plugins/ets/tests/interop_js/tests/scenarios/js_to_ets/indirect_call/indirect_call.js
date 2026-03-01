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
'use strict';

export function indirectCallTypeAny(arg) {
	return arg;
}

export function indirectCallTypeLiteral(arg) {
	const SUFFIX = 'bar';
	return arg + SUFFIX;
}

export function indirectCallTypeExtraSet(tuple) {
	let [str, num] = tuple;
	const SUFFIX = 'bar';

	return [str + SUFFIX, num + 1];
}

export class UserClass {
	method(arg) {
		return arg;
	}
}

/* Following TS code is transpiled to JS
interface UserInterface {
	interfaceMethod(arg: number): number;
}

class UserInterfaceImpl implements UserInterface {
	interfaceMethod(arg: number): number {
		return arg;
	}
}

export function interfaceFunc(): UserInterface {
	let impl: UserInterface = new UserInterfaceImpl();
	return impl;
}
*/
export class UserInterfaceImpl {
	interfaceMethod(arg) {
		return arg;
	}
}

export function interfaceFunc() {
	let impl = new UserInterfaceImpl();
	return impl;
}

export function indirectCallUnion(arg) {
	return arg;
}

export function indirectCallTypeByRefArray(arg) {
	return [...arg];
}

export function indirectCallTypeByRefTuple(arg) {
	return [...arg];
}

export function indirectCallTypeByRefMap(arg) {
	const KEY = 'key2';
	const VALUE = 2;

	let result = new Map(arg);
	result.set(KEY, VALUE);

	return result;
}

export function indirectCallTypeByValueNumber(arg) {
	return +arg + +arg;
}

export function indirectCallTypeByValueString(arg) {
	return ''.concat(arg, arg);
}

export function indirectCallTypeByValueBoolean(arg) {
	return !Boolean(arg);
}
