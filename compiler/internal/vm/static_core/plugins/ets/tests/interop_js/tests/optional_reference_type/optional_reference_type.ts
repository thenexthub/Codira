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

export function fnWithAnyParams(arr?: any[]): object {
	return arr ? arr[0] : 'Argument not found';
}

type City = 'Moscow' | 'London' | 'Paris';

export function fnWithLiteralParam(arr?: City[]): City | string {
	return arr ? arr[0] : 'Argument not found';
}

export function fnWithExtraSetParam(arr?: unknown): {} {
	return arr ? arr[0] : 'Argument not found';
}

export interface AddressType {
	street: string;
	city: string;
}

export interface TestUserType {
	id: number;
	name: string;
	age: number;
	address?: AddressType;
}

export interface UserPick {
    name: string;
    address?: AddressType;
};
export interface UserOmit {
    name: string;
    address?: AddressType;
};
export interface UserPartial {
    id?: number;
    name?: string;
    age?: number;
    address?: AddressType;
}

export function fnWithSubsetPick(obj: UserPick): string {
	return obj.address ? obj.address.city : 'Adress not found';
}

export function fnWithSubsetOmit(obj: UserOmit): string {
	return obj.address ? obj.address.city : 'Adress not found';
}

export function fnWithSubsetPartial(obj: UserPartial): string {
	return obj.address ? obj.address.city : 'Adress not found';
}

export type UnionArrOrObj = string[] | { [key: string]: {} };

export function fnWithUnionParam(obj?: UnionArrOrObj): string {
	if (obj) {
		if (Array.isArray(obj)) {
			return 'This is an array';
		} else if (typeof obj === 'object') {
			return 'This is an object';
		}
	} else {
		return 'Argument not found';
	}
	return 'Argument not found';
}

export class TestUserClass {
	id: number;
	name: string;

	constructor(id: number, name: string) {
		this.id = id;
		this.name = name;
	}
}

export function fnWithUserClass(obj?: TestUserClass): string {
	return obj ? obj.name : 'Class was not passed';
}

export function fnWithUserInterface(obj?: TestUserType): string {
	return obj ? obj.name : 'Object was not passed';
}
export interface ObjectTypeWithArr {
	id: number;
	arr?: string[];
}

export function fnWithAnyParamObject(obj: ObjectTypeWithArr): {} {
	return obj.arr ? obj.arr[0] : obj.id;
}
