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
export function getArray(arr: string[]): string[] {
	return arr;
}

export class CustomArray<T> extends Array<T> {
	constructor(...items: T[]) {
		super(...items);
	}
}

export function returnCustomArray<T>(array: CustomArray<T>): CustomArray<T> {
	return array;
}

export function createStringRecord(): Record<string, string> {
	const data: Record<string, string> = {
		key1: 'A',
		key2: 'B',
		key3: 'C',
	};
	return data;
}

export function createInt8Array(length: number): Int8Array {
	const array = new Int8Array(length);
	for (let i = 0; i < length; i++) {
		array[i] = i;
	}
	return array;
}

type RecordObject = Record<string, string>;

type AnyType = {};

export const handler = {
	get: function (target: RecordObject, property: string): Object {
		if (property in target) {
			return target[property];
		} else {
			return undefined;
		}
	},
	set: function (target: RecordObject, property: string, value: AnyType): Object {
		if (typeof value === 'string') {
			target[property] = value;
			return true;
		} else {
			throw new TypeError('The value must be a string');
		}
	},
};
